/*
 * Hands-Free Recipe —— 脚踏菜谱控制器（Arduino Uno）
 *
 * 三个按钮按位置装在台面下的踢脚板：左 = D4，中 = D3，右 = D2。
 * 站点编号按位置从左到右：0 左、1 中、2 右。
 * 每个位置是哪个厨房区域（水槽 / 切配 / 灶台）由网页的「厨房设置」决定，固件不用改。
 * 舵机带动屏幕支架水平转向。
 *
 * 每个按钮的逻辑：
 *   屏幕还没朝向这个按钮的位置  →  任何操作都只做一件事：把屏幕转过来
 *   屏幕已经朝向这里：
 *     单击  →  下一步        串口发送  NEXT <站点>
 *     双击  →  上一步        串口发送  PREV <站点>
 *     长按  →  朗读这一步    串口发送  READ <站点>
 *
 * 串口（115200）——Arduino 发给电脑：
 *   READY                      开机完成
 *   STATION <i> <名字>          开始转向站点 i
 *   ARRIVED <i>                 舵机已到位
 *   NEXT <i> / PREV <i> / READ <i>
 *   LOCK <0|1>                  转向锁定状态
 * 电脑发给 Arduino（每条一行）：
 *   GOTO <i>                    转到站点 i（0/1/2）
 *   LOCK / UNLOCK               锁定转向：屏幕不动，任何按钮都直接控制菜谱
 *   ANGLE <i> <度数>            现场校准某个站点的角度（0~180）
 *   STATUS                      回报当前状态
 *
 * 注意：普通舵机无法读回实际角度，"是否已朝向" 依据的是程序记录的目标站点。
 */

#include <Servo.h>

// ---------------- 引脚与参数（按实际情况修改） ----------------
const uint8_t NUM_STATIONS = 3;
const uint8_t BUTTON_PIN[NUM_STATIONS] = {4, 3, 2};   // 左 / 中 / 右 按钮。一端接这里，另一端接 GND
const uint8_t LED_PIN[NUM_STATIONS]    = {5, 6, 7};   // 可选：左 / 中 / 右 指示灯，显示屏幕朝向哪一站
const uint8_t SERVO_PIN = 9;

int stationAngle[NUM_STATIONS] = {30, 90, 150};       // 屏幕朝向 左 / 中 / 右 时的舵机角度（方向反了就对调 30 和 150，或用 ANGLE 命令校准）
const char* const STATION_NAME[NUM_STATIONS] = {"LEFT", "MIDDLE", "RIGHT"};
const uint8_t START_STATION = 1;                      // 开机时朝向中间

const unsigned long DEBOUNCE_MS    = 25;   // 消抖
const unsigned long DOUBLE_GAP_MS  = 350;  // 两次点击间隔小于此值算双击
const unsigned long LONG_PRESS_MS  = 700;  // 按住超过此值算长按
const unsigned long SERVO_STEP_MS  = 12;   // 舵机每转 1° 的间隔，越大越慢越稳

// ---------------- 状态 ----------------
enum Gesture { SINGLE, DOUBLE, LONG };

struct Button {
  bool raw;                  // 当前读数（true = 按下）
  bool pressed;              // 消抖后的状态
  unsigned long rawChangedAt;
  unsigned long pressedAt;
  unsigned long releasedAt;
  uint8_t clicks;            // 已累计的短按次数
  bool longFired;            // 这次按住是否已触发长按
  bool consumed;             // 这次按下已用于"转向"，不再算作手势
  unsigned long ignoreUntil; // 转向后短时间内忽略连按，避免误触翻页
};

Button buttons[NUM_STATIONS];
Servo servo;

int currentAngle;
int targetAngle;
int8_t facing = START_STATION;   // 屏幕朝向（或正在转向）的站点
bool moving = false;
bool locked = false;
unsigned long lastServoStep = 0;

char rxBuf[32];
uint8_t rxLen = 0;

// ---------------- 输出 ----------------
void updateLeds() {
  for (uint8_t i = 0; i < NUM_STATIONS; i++) {
    digitalWrite(LED_PIN[i], i == facing ? HIGH : LOW);
  }
}

void turnTo(uint8_t station) {
  if (station >= NUM_STATIONS) return;
  facing = station;
  targetAngle = stationAngle[station];
  moving = (targetAngle != currentAngle);
  Serial.print(F("STATION "));
  Serial.print(station);
  Serial.print(' ');
  Serial.println(STATION_NAME[station]);
  if (!moving) {
    Serial.print(F("ARRIVED "));
    Serial.println(station);
  }
  updateLeds();
}

void sendGesture(Gesture g, uint8_t station) {
  if (g == SINGLE)      Serial.print(F("NEXT "));
  else if (g == DOUBLE) Serial.print(F("PREV "));
  else                  Serial.print(F("READ "));
  Serial.println(station);
}

// ---------------- 按钮 ----------------
// 按下的瞬间判断方位：没朝向这里就立刻转向，并吞掉这次操作
void onPressEdge(uint8_t i, unsigned long now) {
  Button& b = buttons[i];
  b.pressedAt = now;
  b.longFired = false;

  if (now < b.ignoreUntil) {          // 刚转过来又连按：忽略
    b.consumed = true;
    return;
  }
  if (!locked && facing != i) {
    turnTo(i);
    b.consumed = true;
    b.clicks = 0;
    return;
  }
  b.consumed = false;
}

void onReleaseEdge(uint8_t i, unsigned long now) {
  Button& b = buttons[i];
  b.releasedAt = now;

  if (b.consumed) {
    b.consumed = false;
    b.ignoreUntil = now + DOUBLE_GAP_MS;
    return;
  }
  if (b.longFired) return;            // 长按已处理

  b.clicks++;
  if (b.clicks >= 2) {                // 第二次松开：立即判定双击
    b.clicks = 0;
    sendGesture(DOUBLE, i);
  }
}

void updateButton(uint8_t i, unsigned long now) {
  Button& b = buttons[i];
  bool reading = (digitalRead(BUTTON_PIN[i]) == LOW);   // INPUT_PULLUP：按下为 LOW

  if (reading != b.raw) {
    b.raw = reading;
    b.rawChangedAt = now;
  }
  if (b.raw != b.pressed && now - b.rawChangedAt >= DEBOUNCE_MS) {
    b.pressed = b.raw;
    if (b.pressed) onPressEdge(i, now);
    else           onReleaseEdge(i, now);
  }

  // 长按：按住到阈值就触发，不用等松开
  if (b.pressed && !b.consumed && !b.longFired && now - b.pressedAt >= LONG_PRESS_MS) {
    b.longFired = true;
    b.clicks = 0;
    sendGesture(LONG, i);
  }

  // 单击：松开后等一个双击窗口，没有第二下就判定为单击
  if (!b.pressed && b.clicks == 1 && now - b.releasedAt >= DOUBLE_GAP_MS) {
    b.clicks = 0;
    sendGesture(SINGLE, i);
  }
}

// ---------------- 舵机：缓慢、可预期地移动 ----------------
void updateServo(unsigned long now) {
  if (!moving || now - lastServoStep < SERVO_STEP_MS) return;
  lastServoStep = now;
  if (currentAngle < targetAngle) currentAngle++;
  else if (currentAngle > targetAngle) currentAngle--;
  servo.write(currentAngle);
  if (currentAngle == targetAngle) {
    moving = false;
    Serial.print(F("ARRIVED "));
    Serial.println(facing);
  }
}

// ---------------- 串口命令 ----------------
void reportStatus() {
  Serial.print(F("STATUS facing="));
  Serial.print(facing);
  Serial.print(F(" angle="));
  Serial.print(currentAngle);
  Serial.print(F(" locked="));
  Serial.println(locked ? 1 : 0);
}

void handleCommand(char* cmd) {
  if (strncmp(cmd, "GOTO ", 5) == 0) {
    int s = atoi(cmd + 5);
    if (s >= 0 && s < NUM_STATIONS) turnTo((uint8_t)s);
  } else if (strcmp(cmd, "LOCK") == 0) {
    locked = true;
    Serial.println(F("LOCK 1"));
  } else if (strcmp(cmd, "UNLOCK") == 0) {
    locked = false;
    Serial.println(F("LOCK 0"));
  } else if (strncmp(cmd, "ANGLE ", 6) == 0) {
    int s = -1, a = -1;
    if (sscanf(cmd + 6, "%d %d", &s, &a) == 2 && s >= 0 && s < NUM_STATIONS && a >= 0 && a <= 180) {
      stationAngle[s] = a;
      if (s == facing) turnTo((uint8_t)s);
      Serial.print(F("ANGLE "));
      Serial.print(s);
      Serial.print(' ');
      Serial.println(a);
    }
  } else if (strcmp(cmd, "STATUS") == 0) {
    reportStatus();
  }
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (rxLen > 0) {
        rxBuf[rxLen] = '\0';
        handleCommand(rxBuf);
        rxLen = 0;
      }
    } else if (rxLen < sizeof(rxBuf) - 1) {
      rxBuf[rxLen++] = c;
    }
  }
}

// ---------------- 主程序 ----------------
void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < NUM_STATIONS; i++) {
    pinMode(BUTTON_PIN[i], INPUT_PULLUP);
    pinMode(LED_PIN[i], OUTPUT);
    buttons[i] = Button{false, false, 0, 0, 0, 0, false, false, 0};
  }
  currentAngle = targetAngle = stationAngle[START_STATION];
  servo.write(currentAngle);          // 先写角度再 attach，避免上电猛转
  servo.attach(SERVO_PIN);
  updateLeds();

  Serial.println(F("READY"));
  turnTo(START_STATION);
}

void loop() {
  unsigned long now = millis();
  for (uint8_t i = 0; i < NUM_STATIONS; i++) updateButton(i, now);
  updateServo(now);
  readSerial();
}
