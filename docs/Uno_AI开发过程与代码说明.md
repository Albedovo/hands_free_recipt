# Hands-Free Recipe Controller
## Arduino Uno Development Process, AI Use, and Code Explanation

**Scope:** The Arduino Uno prototype only. ESP32-C6 development is excluded.

This document combines the supplied Claude conversation summary with the final Uno firmware and recipe webpage. “I” refers to the project creator. Codex helped organize this document and inspect the source files. Historical hardware results are attributed to the supplied conversation summary; no new hardware test was performed during documentation.

## 1. Project Goal

While cooking, my hands may be wet, oily, or occupied with ingredients. I wanted to navigate a recipe without repeatedly touching a screen and to bring the display toward my working position.

The prototype defines three kitchen stations: Sink, Counter, and Stove. Each station has a button intended for foot operation. Pressing a button at a different station first turns the display toward that station. Once that station is selected, a single tap advances the recipe, a double tap goes back, and a hold reads the current page aloud.

The source implements button detection, servo commands, and webpage communication. The physical screen mount, load capacity, pedal feel, and kitchen usability require separate physical validation.

## 2. How I Used AI

I defined the cooking scenario and interaction rules, connected and operated the hardware, reported observed problems, and requested changes based on usability. AI helped implement firmware and webpage behavior, explain wiring and serial communication, and suggest debugging steps.

| Stage | Tool | Contribution and evidence |
| --- | --- | --- |
| Early exploration | ChatGPT | An initial conversation link was supplied. Its contents have not been inspected for this document, so no prompts or decisions are attributed to it here. |
| Uno prototype development | Claude / Claude Code | The supplied summary covers hardware setup, debugging, webpage connection, and interface changes. It is an edited summary, not a complete raw transcript. |
| Documentation | Codex | Inspected the final Uno firmware and webpage, explained functions and parameters, identified limitations, and included complete source snapshots. |

A line-by-line editing history was not supplied, so exact authorship of individual lines cannot be established. My role was to specify and evaluate the interaction; AI supported implementation. AI explanations and diagnostic suggestions still needed verification.

Reference materials:

- [Initial ChatGPT conversation](https://chatgpt.com/s/cx_6ae108a922208191a2586ff136e7c722)
- [Supplied Claude summary of the Uno stage](https://claude.ai/artifact/PiKfUFDh2G6zDX11rfexeE)

## 3. System Structure

```text
Three buttons → Arduino Uno → USB serial → Webpage → Recipe display / computer speech
                      ↓
                 Servo signal
                      ↓
            Display mount rotation (design goal)
```

The Uno detects gestures and controls the servo. It sends short text messages to the computer. The webpage retrieves recipes, updates the display, and uses browser speech synthesis. The Uno does not download recipes or generate speech.

### Wiring

| Component | Connection in the firmware | Explanation |
| --- | --- | --- |
| Sink button | D2 to GND | Internal pull-up; LOW means pressed |
| Counter button | D3 to GND | Same input logic |
| Stove button | D4 to GND | Same input logic |
| Servo signal | D9 | Controlled with `Servo.h` |
| Optional station LEDs | D5, D6, D7 | Indicate the selected target station; external LEDs require suitable current-limiting resistors |
| Uno to computer | USB | Both sides use 115200 baud |

A basic mechanical switch has no polarity. For four-legged pushbuttons, the relevant issue is choosing contacts separated by the switch mechanism. Servo power must suit the actual load. If an external servo supply is used, its ground must be connected to the Uno ground. Software alone cannot establish correct wiring or screen-mount capacity.

## 4. Key Prompts and Development Decisions

This section groups related exchanges into three development stages instead of listing every troubleshooting message. Only representative prompts are quoted. Chinese quotations come from the supplied Claude summary; English translations were added for this document. The summary contains shortened answers and is not a complete raw transcript.

### 4.1 Defining the Interaction

**Prompt — Chinese original:**

> 这是我的厨房菜谱控制器设想，我想要你教我一下现在接下来该做什么。我要接 Arduino 并控制网页菜谱，设想使用三个按钮，分别设置在厨房的下方。点击按钮时先检测舵机是否已经转向按钮方位，如果没有先执行转向；如果已经在按钮方位，那就执行让菜谱翻页，双击则是让菜谱返回上一页，长按播放这一页的菜谱操作。

**Prompt — English translation:**

> This is my idea for a kitchen recipe controller, and I would like you to teach me what to do next. I want to connect an Arduino to control a recipe webpage, using three buttons positioned low down in the kitchen. When a button is pressed, first check whether the servo is already facing that button’s position. If it is not, turn toward it first. If it is already facing that position, advance the recipe; double-click to return to the previous page, and hold to read the instructions on the current page aloud.

I defined the station-based interaction and the three gestures. AI helped organize implementation into separate button, servo, and webpage tests. The first press at a different station was reserved for rotation so that bringing the display closer would not also skip a recipe step.

In the firmware, `onPressEdge()` calls `turnTo()` when the station differs and marks the press as `consumed`. The program remembers a target station; it does not measure the display’s physical orientation.

### 4.2 Connecting and Debugging the Prototype

Hardware and connection troubleshooting took several exchanges. I reported unresponsive buttons, a stationary servo, and a disconnected webpage. AI suggested checking button contacts and digital pins, testing the servo with the Arduino Sweep example, and checking whether the Serial Monitor was occupying the port. I carried out the physical tests and reported the results.

The supplied summary records station messages for all three buttons and my confirmation that the servo eventually turned. These are historical results, not tests repeated while preparing this document. The webpage gained connection feedback and reconnection to previously authorized ports, and TheMealDB supplied English recipes.

The main lesson was to test each layer separately: button input, servo movement, serial messages, and webpage behavior. AI diagnoses remained hypotheses to verify. In particular, `ARRIVED` reports a software angle, not sensor feedback, and the webpage’s generic `busy` error does not establish the exact cause of every port-opening failure.

### 4.3 Improving Readability and Hands-Free Control

**Representative prompt — Chinese original:**

> 我希望如果正在阅读的时候点击按钮，可以暂停阅读，如果再 hold 的话从头开始阅读。同时，每一步不要写的太长，视觉清晰一点

**Representative prompt — English translation:**

> I would like a button press during reading to pause the speech, and another hold to restart it from the beginning. Also, keep each step from being too long so the display is visually clearer.

**Representative prompt — Chinese original:**

> 食材准备和操作提示会有重叠，同时你可以在每一步标注需要的食材

**Representative prompt — English translation:**

> The ingredient preparation and instruction hints overlap. Could you also label the ingredients needed for each step?

I then requested shorter instructions, clearer ingredient layouts, ingredient labels for each step, Chinese/English switching, and better image display. AI helped implement these changes. Their purpose was to reduce reading effort and avoid repeated movement between a step and the full ingredient list.

The final code reflects these decisions with several practical limits:

- During speech, a tap or double tap stops reading without changing the page. A hold restarts the current page. This cancels speech rather than preserving a position for continuation.
- English instructions are split using punctuation and `STEP_MAX = 110` as a grouping target, not a strict maximum. Chinese instructions use a separate Markdown parser.
- Ingredient labels are based on text matching and can omit or misidentify ingredients. Chinese labels do not guarantee quantities.
- Language switching selects different recipe providers: HowToCook for Chinese and TheMealDB for English. It does not translate the same recipe.
- Chinese step images are extracted when available. The current English parser places a dish image on the overview page and leaves step images empty.

These revisions came from my feedback on the interaction. The source explanations below show where the resulting behaviors are implemented and how I can adjust them directly.

## 5. Final Interaction Rules

The table assumes rotation is unlocked. “Selected station” means the software target, not a physically measured position.

| Condition | Single tap | Double tap | Hold |
| --- | --- | --- | --- |
| A different station is pressed | Rotate; consume that press | Initial press rotates, followed by an ignore window | Initial press rotates without reading |
| Selected station, speech inactive | Next page | Previous page | Read current page |
| Selected station, speech active | Stop speech | Stop speech | Restart current page speech |

Rotation lock makes button gestures control the recipe regardless of station. A `GOTO` command can still request rotation; lock does not block every rotation path.

A single tap waits for the 350 ms double-tap window after release. A hold triggers after 700 ms. Once a station is selected, subsequent gestures are not gated by `moving`, so recipe actions can occur while the servo is still moving.

## 6. Uno Firmware Explained

### 6.1 Parameters and State

`NUM_STATIONS` is the station count. Button pins, LED pins, angles, and names share an index: 0 is Sink, 1 is Counter, and 2 is Stove.

`stationAngle` contains the target angles, initially `{30, 90, 150}`. `START_STATION = 1` selects Counter at startup. `currentAngle` is the software command angle, `targetAngle` is its destination, `facing` is the selected target station, and `moving` indicates that gradual updates are still underway. None measures the physical screen position.

Each `Button` stores its own input history:

| Field | Purpose |
| --- | --- |
| `raw`, `rawChangedAt` | Most recent input reading and when it changed |
| `pressed` | Debounced pressed state |
| `pressedAt`, `releasedAt` | Timestamps used to recognize gestures |
| `clicks` | Number of accumulated short presses |
| `longFired` | Prevents repeated hold events during one press |
| `consumed` | Marks a press used for rotation or deliberately ignored |
| `ignoreUntil` | Briefly ignores this button after a consumed press is released |

`locked` changes whether a different station triggers rotation. `lastServoStep` tracks the previous servo update. `rxBuf` and `rxLen` collect incoming serial characters until a complete command is available. `Gesture` gives names to the three gesture types.

### 6.2 Function Responsibilities

| Function | Responsibility |
| --- | --- |
| `setup()` | Starts serial, configures inputs and LEDs, initializes the servo target, and announces startup |
| `loop()` | Repeatedly updates buttons and servo motion and receives computer commands |
| `updateButton()` | Debounces the input and distinguishes taps, double taps, and holds |
| `onPressEdge()` | Records press time and decides whether the press should rotate the display |
| `onReleaseEdge()` | Counts short presses and emits a double-tap event on the second release |
| `sendGesture()` | Converts gestures into `NEXT`, `PREV`, or `READ` messages |
| `turnTo()` | Sets the target station and angle, sends `STATION`, and updates LEDs |
| `updateServo()` | Moves the command angle one degree every 12 ms and sends `ARRIVED` at the target |
| `updateLeds()` | Lights the LED for the selected station |
| `readSerial()` | Collects characters and passes newline-terminated commands to the handler |
| `handleCommand()` | Handles `GOTO`, `LOCK`, `UNLOCK`, `ANGLE`, and `STATUS` |
| `reportStatus()` | Reports the software station, angle, and lock state |

### 6.3 How Gesture Detection Works

Mechanical contacts can bounce between HIGH and LOW when pressed. `updateButton()` first notices a raw change, then waits until it has remained stable for `DEBOUNCE_MS` before accepting it.

When a confirmed press begins, `onPressEdge()` records its time. If the station differs and rotation is unlocked, it selects that station and consumes the press. Otherwise, the press can become a gesture.

While held, reaching `LONG_PRESS_MS` emits one hold event and clears pending clicks. Releasing a short press increments `clicks`. A second release produces a double tap immediately. If only one click remains after `DOUBLE_GAP_MS`, the code emits a single tap.

### 6.4 Why the Main Loop Uses Time Checks

The firmware uses `millis()` and elapsed-time checks rather than a long `delay()` during rotation. Each loop can process buttons, move the command angle when due, and read serial input. This keeps the subsystems responsive to one another.

If recipe actions should be disabled during motion, an explicit check of `moving` would be needed. The current firmware does not impose that restriction.

### 6.5 One Tap Through the Entire System

Suppose Counter is already selected and I briefly press its button:

1. `updateButton(1, now)` accepts the input after 25 ms of stability.
2. `onPressEdge()` records the time and sees that no station change is required.
3. `onReleaseEdge()` records one short press.
4. After 350 ms without a second press, the Uno sends `NEXT 1`.
5. The webpage receives a complete line and `handleLine()` calls `onTap()`.
6. If speech is active, it stops. Otherwise, `next()` increments `state.page` and calls `render()`.

## 7. Webpage Code Explained

The webpage is one HTML file. HTML defines its structure, CSS controls appearance and layout, and JavaScript handles recipes, speech, serial communication, and interactions. Recipe content is loaded from external sources; no conversational model generates recipes during use.

### 7.1 Module Map

| Identifiers | Responsibility |
| --- | --- |
| `I18N`, `T` | Interface strings for each language and the currently selected dictionary |
| `store` | Safe wrappers around local storage for preferences, progress, and cached recipes |
| `state` | Current recipe, page, selected station, motion indicator, lock, and automatic reading |
| `$()` | Short form of `document.getElementById()` |
| `api()`, `MealDB` | English categories, lists, searches, and recipe details |
| `splitSteps()`, `parseMeal()` | Convert English recipe data into overview and instruction pages |
| `norm()`, `stem()`, `wordsOf()`, `ingredientsForStep()` | Normalize English words and match ingredients to steps |
| `RECIPES`, `CATEGORY`, `SOURCES` | Chinese recipe index, categories, and content locations |
| `clean()`, `imagesIn()`, `parseRecipe()` | Extract readable text, images, ingredients, and steps from Markdown |
| `HowToCook`, `PROVIDERS` | Load Chinese recipes and select the provider for the active language |
| `esc()`, `fmt()`, `KEY_INFO` | Escape displayed text and highlight time and temperature information |
| `render()`, `renderStations()`, `flash()` | Update recipe content, station indicators, and temporary feedback |
| `applyLang()`, `setLang()`, `openLast()` | Apply language settings and restore the corresponding recipe progress |
| `pickVoice()`, `forSpeech`, `pageSpeech()` | Select a voice and prepare text for speech |
| `speak()`, `stopReading()`, `setReading()` | Start or cancel speech and update its visible state |
| `onTap()`, `onDoubleTap()`, `onHold()` | Apply the gesture rules shared by serial and keyboard input |
| `next()`, `prev()`, `readPage()` | Navigate or read the current page |
| `goStation()`, `toggleLock()` | Request rotation and toggle button rotation lock |
| `openRecipe()`, `loadSeq` | Load recipes and prevent stale asynchronous results from replacing a newer selection |
| `renderList()`, `renderCats()`, `loadRows()`, `openPicker()` | Populate recipe categories, results, and the selection dialog |
| `send()`, `log()` | Send serial commands and show communication logs |
| `setConnected()`, `refreshConn()` | Update connection controls and error feedback |
| `connect()`, `openPort()` | Request permission and open serial at 115200 baud |
| `readLoop()`, `handleLine()` | Assemble complete messages and dispatch them |
| `disconnect()`, `autoConnect()` | Release a connection or try a previously authorized port |
| Event handlers and startup block | Bind controls and keyboard input, restore settings, and start loading |

### 7.2 Recipe Data and Rendering

English API responses and Chinese Markdown have different structures. Their parsers produce a common recipe object containing a title and `pages`. Overview pages contain ingredients and introductory information; instruction pages contain step text, notes, ingredient labels, and images.

`render()` reads `state.recipe.pages[state.page]`, updates the progress display and content, and saves the current page. It does not need to reparse the original recipe on each navigation action. `fmt()` highlights recognizable times and temperatures, while `esc()` escapes text before insertion into HTML.

For English ingredient matching, the code normalizes words, simplifies endings, and compares recipe ingredient words with step words. Chinese matching checks ingredient-name substrings against step text and notes. These rules are useful hints, not semantic understanding of a recipe.

### 7.3 Serial Communication

`connect()` calls `navigator.serial.requestPort()` to let the user choose a device. `openPort()` opens it, creates a text encoder for outgoing commands, starts `readLoop()`, and requests status shortly afterward.

A serial read may contain only part of a line: `NE` might arrive before `XT 1\n`. `readLoop()` accumulates chunks in `buf` and calls `handleLine()` only after finding a newline. `handleLine()` maps `NEXT`, `PREV`, and `READ` to the same gesture functions used by keyboard testing.

`autoConnect()` can attempt a previously authorized port. It does not replace the initial user authorization. `disconnect()` cancels the reader, closes the writer, and attempts to close the port.

### 7.4 Speech Control

`reading` indicates an active reading operation. `current` holds the latest speech object so old callbacks do not incorrectly update the new operation’s state.

`speak()` cancels old speech, constructs a `SpeechSynthesisUtterance`, selects language and voice, and sets `u.rate = 0.95`. A short scheduled delay separates cancellation from starting the new utterance. `pageSpeech()` assembles text from the current page, including available ingredient labels.

`onTap()` and `onDoubleTap()` stop speech first when it is active. `onHold()` calls `readPage()` and starts the page again. This is restart behavior, not pause-and-resume behavior.

### 7.5 Language and Persistence

`setLang()` stops speech, switches provider, clears the previous display, and restores the last recipe for the selected language. Storage keys include `en` or `zh`, keeping their progress separate.

`localStorage` belongs to the current browser environment; it does not synchronize across devices. Cached recipe text does not ensure that external images or every recipe remain available offline.

## 8. Serial Protocol

Each message ends with a newline. Station indices are 0, 1, and 2.

| Direction | Example | Meaning |
| --- | --- | --- |
| Uno → webpage | `READY` | Firmware startup complete |
| Uno → webpage | `STATION 0 SINK` | Sink selected as target |
| Uno → webpage | `ARRIVED 0` | Software command angle reached its target |
| Uno → webpage | `NEXT 1` | Counter single tap |
| Uno → webpage | `PREV 1` | Counter double tap |
| Uno → webpage | `READ 1` | Counter hold |
| Uno → webpage | `LOCK 1` | Button rotation lock enabled |
| Uno → webpage | `STATUS facing=1 angle=90 locked=0` | Software state report |
| Computer → Uno | `GOTO 2` | Select Stove as rotation target |
| Computer → Uno | `LOCK` / `UNLOCK` | Enable / disable button rotation lock |
| Serial tool → Uno | `ANGLE 0 25` | Temporarily set Sink to 25 degrees |
| Computer → Uno | `STATUS` | Request current state |

`ANGLE` changes RAM only. Restarting restores the firmware defaults. For a permanent change, edit `stationAngle` and upload the firmware again.

## 9. Where to Make Common Changes

| Desired change | File and search term | Current setting / guidance |
| --- | --- | --- |
| Button pins | Firmware: `BUTTON_PIN` | `{2, 3, 4}` |
| Servo signal pin | Firmware: `SERVO_PIN` | `9` |
| Station angles | Firmware: `stationAngle` | `{30, 90, 150}`; calibrate against the physical setup |
| Startup station | Firmware: `START_STATION` | `1`, Counter |
| Debounce interval | Firmware: `DEBOUNCE_MS` | `25` ms |
| Double-tap interval | Firmware: `DOUBLE_GAP_MS` | `350` ms; also affects single-tap latency and the post-rotation ignore window |
| Hold threshold | Firmware: `LONG_PRESS_MS` | `700` ms |
| Servo update speed | Firmware: `SERVO_STEP_MS` | `12` ms per degree; a larger value slows command updates |
| English step grouping | Webpage: `STEP_MAX` | `110` characters; not a hard maximum |
| Speech speed | Webpage: `u.rate` | `0.95` |
| Gesture behavior | Webpage: `onTap`, `onDoubleTap`, `onHold` | Controls speech interruption and navigation |
| Default recipes | Webpage: each provider’s `defaultId` | Saved history takes precedence when available |
| Interface wording | Webpage: `I18N` | Edit the relevant language dictionary |
| Fonts, spacing, images | Webpage: `<style>` | Edit the relevant CSS selectors |
| Baud rate | Firmware: `Serial.begin`; webpage: `baudRate` | Both must match; currently `115200` |

To require a one-second hold, change the firmware parameter:

```cpp
const unsigned long LONG_PRESS_MS = 1000;
```

To slow speech slightly, change this line inside `speak()`:

```javascript
u.rate = 0.85;
```

Keep an original copy and change one hardware parameter at a time so that differences are easy to identify.

## 10. Startup and Play-Test Checklist

These are proposed checks, not newly completed test results.

1. Upload the supplied Uno firmware to an Arduino Uno using the Arduino IDE.
2. Check button wiring, servo signal, power, and common ground.
3. Close any Serial Monitor using the same port. Open `recipe_display.html` in a browser environment supporting `navigator.serial`; the historical workflow used Chrome or Edge.
4. Select the Uno through the connection button and inspect the connection state and logs.
5. Choose a recipe and check station changes, single taps, double taps, holds, and speech interruption.
6. Check recipe loading, images, and progress restoration separately in each language.

The webpage also includes keyboard controls for testing without button hardware:

| Key | Action |
| --- | --- |
| Right arrow | Single-tap behavior: next page or stop speech |
| Left arrow | Double-tap behavior: previous page or stop speech |
| Space | Read / restart the current page |
| 1 / 2 / 3 | Request a station; update only the visual state if disconnected |
| L | Toggle button rotation lock |
| A | Toggle automatic reading after navigation |
| / | Open the recipe picker |

Keyboard simulation does not validate physical wiring, servo motion, or the screen mount.

## 11. Limitations and Reflection

- The servo has no position feedback in this prototype. `facing`, station LEDs, and `ARRIVED` describe software state.
- Speech interruption cancels reading. Holding restarts the page instead of continuing from an earlier position.
- Ingredient association uses text matching. English step splitting uses punctuation rules and does not guarantee one cooking action per page.
- Recipes and images depend on external sources. Speech voice availability depends on the runtime environment.
- The supplied webpage retains a compatibility comment mentioning another board type and requests `STATUS` after opening the port. This source detail does not establish use of that board; the documented project remains Uno-only.
- AI diagnostic suggestions need evidence. A stationary servo does not identify one unique hardware fault, and a port-opening error does not uniquely prove port contention.
- Keeping CSS, a large recipe index, and application logic in one HTML file makes locating changes harder. The module and parameter tables are intended to make simple changes possible without asking AI again.

The project helped me translate an interaction idea into inputs, state transitions, and outputs that I can inspect. AI accelerated implementation, but I still need to explain how gestures are detected, why the first press changes stations without advancing the recipe, how messages reach the webpage, and where to change timing or speech behavior.

## 12. Source Code Appendix

The firmware below is complete. The webpage listing omits the long RECIPES catalog for readability; all other source content is retained. This shortened listing is for reference; use the original recipe_display.html to run the project. Existing source comments and interface strings retain their original languages. The hashes identify the original files, not the shortened listing.

Source files:

- `recipt/recipe_controller/recipe_controller.ino`
- `recipt/recipe_display.html`


### A. Complete Uno Firmware

Source SHA-256: `2262302e43d710611501d7802b5fe6e4e2e16b832e0d5187637d568c84cb08d3`

```cpp
/*
 * Hands-Free Recipe —— 脚踏菜谱控制器（Arduino Uno）
 *
 * 三个按钮分别装在 水槽 / 备菜台 / 灶台 下方的踢脚板位置。
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
const uint8_t BUTTON_PIN[NUM_STATIONS] = {2, 3, 4};   // 按钮一端接这里，另一端接 GND
const uint8_t LED_PIN[NUM_STATIONS]    = {5, 6, 7};   // 可选：指示屏幕朝向哪一站
const uint8_t SERVO_PIN = 9;

int stationAngle[NUM_STATIONS] = {30, 90, 150};       // 水槽 / 备菜台 / 灶台 对应的舵机角度
const char* const STATION_NAME[NUM_STATIONS] = {"SINK", "COUNTER", "STOVE"};
const uint8_t START_STATION = 1;                      // 开机时朝向备菜台

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

```

### B. Webpage Code (Recipe Catalog Omitted)

Source SHA-256: `971f6ceca4c0298be43bc4d1425e52c3ee6ead49a8b76ad219394c6e8ca353c4`

```html
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Hands-Free Recipe</title>
<style>
  :root {
    --bg: #f6f1e8;
    --surface: #fffdf8;
    --ink: #2b2420;
    --muted: #7a6d63;
    --line: #e4d9c9;
    --accent: #c4552d;
    --accent-soft: #f6dccf;
    --ok: #3f7d4e;
    --mark: #fbe7a8;
    --mark-ink: #4a3a10;
  }
  @media (prefers-color-scheme: dark) {
    :root {
      --bg: #1c1916;
      --surface: #26221e;
      --ink: #f1e9df;
      --muted: #a89a8c;
      --line: #3b342d;
      --accent: #ec7b4f;
      --accent-soft: #4a2c1f;
      --ok: #7cc28d;
      --mark: #5a4a1c;
      --mark-ink: #fbe7a8;
    }
  }
  * { box-sizing: border-box; }
  html, body { height: 100%; }
  body {
    margin: 0;
    background: var(--bg);
    color: var(--ink);
    font-family: system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
    display: flex;
    flex-direction: column;
  }
  button {
    font: inherit;
    color: inherit;
    background: var(--surface);
    border: 1px solid var(--line);
    border-radius: 999px;
    padding: 8px 16px;
    cursor: pointer;
  }
  button:hover { border-color: var(--muted); }
  button.primary { background: var(--accent); border-color: var(--accent); color: #fff; }
  button[aria-pressed="true"] { background: var(--accent-soft); border-color: var(--accent); }

  header {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 10px 14px;
    padding: 14px 24px;
    border-bottom: 1px solid var(--line);
  }
  .brand { font-weight: 700; letter-spacing: .02em; margin-right: auto; }
  .brand small { font-weight: 400; color: var(--muted); margin-left: 8px; }
  #recipeBtn { font-weight: 600; max-width: 40vw; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
  .conn { display: inline-flex; align-items: center; gap: 8px; color: var(--muted); font-size: 14px; }
  .dot { width: 10px; height: 10px; border-radius: 50%; background: var(--line); }
  .dot.on { background: var(--ok); }

  .stations { display: flex; gap: 6px; }
  .station {
    padding: 6px 14px;
    border-radius: 999px;
    border: 1px solid var(--line);
    color: var(--muted);
    font-size: 15px;
    transition: background .2s, color .2s;
  }
  .station.active { background: var(--accent); border-color: var(--accent); color: #fff; }
  .station.moving { animation: pulse 0.9s ease-in-out infinite; }
  .station.ping { box-shadow: 0 0 0 4px var(--accent-soft); }
  @keyframes pulse { 50% { opacity: .55; } }

  main {
    flex: 1;
    display: flex;
    flex-direction: column;
    justify-content: center;
    padding: 4vh 6vw;
    min-height: 0;
  }
  .progress { display: flex; gap: 4px; margin-bottom: 3vh; }
  .progress span { flex: 1; height: 6px; border-radius: 3px; background: var(--line); }
  .progress span.done { background: var(--accent-soft); }
  .progress span.now { background: var(--accent); }
  .meta { display: flex; gap: 16px; align-items: baseline; color: var(--muted); font-size: clamp(16px, 1.8vw, 22px); margin-bottom: 2vh; }
  .meta .count { color: var(--accent); font-weight: 700; }
  #stepText {
    font-size: clamp(28px, 4.4vw, 60px);
    line-height: 1.35;
    font-weight: 600;
    margin: 0 0 3vh;
    text-wrap: pretty;
  }
  #stepText.title { font-size: clamp(36px, 5.5vw, 76px); }
  mark { background: var(--mark); color: var(--mark-ink); border-radius: 6px; padding: 0 .2em; }
  .paras p { font-size: clamp(18px, 1.9vw, 26px); line-height: 1.6; color: var(--muted); margin: 0 0 1em; max-width: 60em; }
  ul.items {
    list-style: none;
    padding: 0;
    margin: 0;
    font-size: clamp(20px, 2.2vw, 30px);
    line-height: 1.5;
    columns: 2 18em;
    column-gap: 3em;
  }
  ul.items li { break-inside: avoid; padding: 4px 0 4px 1.1em; position: relative; }
  ul.items li::before { content: ""; position: absolute; left: 0; top: .75em; width: .4em; height: .4em; border-radius: 50%; background: var(--accent); }
  ul.items.notes { columns: 1; color: var(--muted); }

  .photos { display: flex; gap: 12px; margin-top: 2vh; overflow-x: auto; }
  .photos img { height: min(28vh, 260px); border-radius: 12px; border: 1px solid var(--line); }

  .flash {
    position: fixed;
    left: 50%;
    top: 22%;
    transform: translateX(-50%);
    background: var(--ink);
    color: var(--bg);
    padding: 14px 28px;
    border-radius: 16px;
    font-size: clamp(20px, 2.4vw, 32px);
    opacity: 0;
    pointer-events: none;
    transition: opacity .25s;
  }
  .flash.show { opacity: .92; }

  footer {
    display: flex;
    flex-wrap: wrap;
    gap: 8px 22px;
    align-items: center;
    padding: 12px 24px;
    border-top: 1px solid var(--line);
    color: var(--muted);
    font-size: 15px;
  }
  footer b { color: var(--ink); }
  footer .spacer { flex: 1; }
  details { font-size: 13px; }
  details pre { max-height: 160px; overflow: auto; background: var(--surface); border: 1px solid var(--line); border-radius: 8px; padding: 8px; margin: 6px 0 0; white-space: pre-wrap; }

  dialog {
    width: min(720px, 92vw);
    height: min(80vh, 760px);
    border: 1px solid var(--line);
    border-radius: 18px;
    background: var(--surface);
    color: var(--ink);
    padding: 0;
  }
  dialog::backdrop { background: rgba(0,0,0,.35); }
  .dlg { display: flex; flex-direction: column; height: 100%; }
  .dlg-head { padding: 16px; border-bottom: 1px solid var(--line); display: flex; flex-direction: column; gap: 10px; }
  .dlg-head input {
    font: inherit; font-size: 18px; padding: 10px 14px; border-radius: 12px;
    border: 1px solid var(--line); background: var(--bg); color: var(--ink);
  }
  .cats { display: flex; flex-wrap: wrap; gap: 6px; }
  .cats button { padding: 4px 12px; font-size: 14px; }
  #recipeList { flex: 1; overflow: auto; padding: 8px; }
  #recipeList button {
    display: flex; justify-content: space-between; width: 100%; text-align: left;
    border: none; border-radius: 10px; padding: 10px 12px; background: transparent; font-size: 17px;
  }
  #recipeList button:hover, #recipeList button:focus { background: var(--accent-soft); outline: none; }
  #recipeList small { color: var(--muted); }

  @media (max-width: 640px) {
    header { padding: 12px 16px; }
    main { padding: 3vh 16px; }
    footer { padding: 10px 16px; }
    #recipeBtn { max-width: 70vw; }
  }
</style>
<style>
  .conn.err { color: var(--accent); font-weight: 600; }
  .dot.err { background: var(--accent); }
  .hint { color: var(--accent); font-size: 14px; flex-basis: 100%; text-align: right; }
  .hint:empty { display: none; }
  .reading { margin-left: auto; color: var(--accent); font-weight: 600; animation: pulse 1.4s ease-in-out infinite; }
  .reading[hidden] { display: none; }
  #stepText { max-width: 22em; }
  #stepText mark { white-space: nowrap; }
  .tip { font-size: clamp(16px, 1.6vw, 20px); color: var(--muted); }
  /* Long pages scroll inside main instead of spilling under the header and footer */
  main { overflow-y: auto; justify-content: safe center; }
  #stepText.title.long { font-size: clamp(30px, 3.8vw, 56px); }
  ul.items.many { font-size: clamp(16px, 1.6vw, 22px); columns: 3 14em; line-height: 1.4; }
  .needs { display: flex; flex-wrap: wrap; align-items: center; gap: 10px; margin: 0 0 2vh; }
  .needs:empty { display: none; }
  .needs .label { color: var(--muted); font-size: clamp(15px, 1.4vw, 19px); text-transform: uppercase; letter-spacing: .06em; margin-right: 4px; }
  .chip {
    font-size: clamp(18px, 2vw, 28px);
    padding: 4px 14px;
    border-radius: 999px;
    background: var(--accent-soft);
    border: 1px solid var(--accent);
  }
</style>
<style>
  body { font-family: system-ui, -apple-system, "Segoe UI", "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif; }
  #langBtn { font-weight: 600; min-width: 5em; }
  /* Photos sit beside the text and are shown whole (never cropped) */
  .content { display: flex; gap: 4vw; align-items: flex-start; }
  .text { flex: 1; min-width: 0; }
  .photos { flex: 0 0 auto; flex-direction: column; margin-top: 0; overflow: visible; }
  .photos:empty { display: none; }
  .photos img {
    width: auto; height: auto;
    max-width: min(32vw, 440px);
    max-height: 52vh;
    object-fit: contain;
  }
  .photos img + img { max-height: 25vh; }
  @media (max-width: 900px) {
    .content { flex-direction: column; }
    .photos { flex-direction: row; flex-wrap: wrap; }
    .photos img { max-width: 100%; max-height: 32vh; }
  }
</style>
</head>
<body>
<header>
  <div class="brand">Hands-Free Recipe<small data-i18n="tagline"></small></div>
  <button id="recipeBtn" data-i18n="pick" data-i18n-title="pickTitle"></button>
  <div class="stations" id="stations"></div>
  <button id="lockBtn" aria-pressed="false" data-i18n="lock" data-i18n-title="lockTitle"></button>
  <button id="autoBtn" aria-pressed="false" data-i18n="auto" data-i18n-title="autoTitle"></button>
  <button id="langBtn" title="中文 / English"></button>
  <span class="conn" id="conn"><span class="dot" id="connDot"></span><span id="connText"></span></span>
  <button id="connectBtn" class="primary"></button>
  <div class="hint" id="hint"></div>
</header>

<main>
  <div class="progress" id="progress"></div>
  <div class="meta"><span class="count" id="count"></span><span id="group"></span><span class="reading" id="reading" data-i18n="reading" hidden></span></div>
  <div class="content">
    <div class="text">
      <h1 id="stepText" class="title"></h1>
      <div class="needs" id="needs"></div>
      <div class="paras" id="paras"></div>
      <ul class="items" id="items"></ul>
    </div>
    <div class="photos" id="photos"></div>
  </div>
</main>

<div class="flash" id="flash"></div>

<footer>
  <span data-i18n-html="fTap"></span>
  <span data-i18n-html="fDouble"></span>
  <span data-i18n-html="fHold"></span>
  <span data-i18n-html="fStop"></span>
  <span data-i18n="fKeys"></span>
  <span class="spacer"></span>
  <details>
    <summary data-i18n="log"></summary>
    <pre id="log"></pre>
  </details>
  <span data-i18n="source"></span>
</footer>

<dialog id="picker">
  <div class="dlg">
    <div class="dlg-head">
      <input id="search" type="search" autocomplete="off">
      <div class="cats" id="cats"></div>
    </div>
    <div id="recipeList"></div>
  </div>
</dialog>

<script>
/* ---------- Interface text ---------- */
const I18N = {
  en: {
    tagline: 'foot-pedal cookbook', pick: 'Choose a recipe', pickTitle: 'Choose a recipe (/)',
    lock: 'Lock turning', lockTitle: 'Keep the screen still; every pedal controls the recipe (L)',
    auto: 'Auto read', autoTitle: 'Read each step aloud after turning the page (A)',
    connected: 'Connected', notConnected: 'Not connected', failed: 'Connection failed',
    connect: 'Connect Arduino', disconnect: 'Disconnect',
    busy: 'The port is busy. Close the Arduino IDE Serial Monitor (and other tabs using it), then click Connect again.',
    noSerial: 'This browser has no Web Serial. Open this page in Chrome or Edge (not Safari).',
    noPort: 'No port chosen. If the list was empty, check the USB cable and that the Arduino driver is installed.',
    reading: '🔊 Reading · tap to stop', start: 'Choose a dish to start',
    fTap: '<b>Tap</b> next step', fDouble: '<b>Double tap</b> previous step',
    fHold: '<b>Hold</b> read aloud from the start', fStop: '<b>Tap while reading</b> stop',
    fKeys: 'Keyboard: → ← Space · 1/2/3 turn', log: 'Serial log', source: 'Recipes from TheMealDB',
    search: 'Search dishes, e.g. chicken', nothing: 'Nothing found', loadingList: 'Loading…',
    stations: ['Sink', 'Counter', 'Stove'],
    stepCount: (n, t) => `Step ${n} of ${t}`, ready: 'Get ready', calc: 'Amounts',
    ingredientsN: n => `${n} ingredients`, need: 'You need',
    loading: name => `Loading ${name || 'recipe'}…`,
    loadFail: 'Could not load the recipe. Check the internet connection and try again.',
    chooseFirst: 'Choose a dish first', last: 'This is the last step', first: 'This is the first page',
    next: 'Next →', prev: '← Back', readFlash: '🔊 Reading', stopped: '⏸ Stopped · hold to read again',
    noSpeech: 'This browser cannot read aloud', turning: name => `Turning to ${name}`,
    locked: 'Turning locked', unlocked: 'Turning unlocked',
  },
  zh: {
    tagline: '脚踏菜谱', pick: '选择菜谱', pickTitle: '选择菜谱 (/)',
    lock: '锁定转向', lockTitle: '锁定后屏幕不再转动，三个按钮都直接控制菜谱 (L)',
    auto: '自动朗读', autoTitle: '翻页后自动朗读 (A)',
    connected: '已连接', notConnected: '未连接', failed: '连接失败',
    connect: '连接 Arduino', disconnect: '断开',
    busy: '串口被占用：请关闭 Arduino IDE 的串口监视器（以及其他打开本页的标签），再点连接。',
    noSerial: '这个浏览器不支持网页串口，请用 Chrome 或 Edge 打开（不要用 Safari）。',
    noPort: '没有选择端口。如果列表是空的，请检查 USB 线和 Arduino 驱动。',
    reading: '🔊 朗读中 · 单击停止', start: '选择一道菜开始',
    fTap: '<b>单击</b> 下一步', fDouble: '<b>双击</b> 上一步',
    fHold: '<b>长按</b> 从头朗读', fStop: '<b>朗读时单击</b> 停止',
    fKeys: '键盘：→ ← 空格 · 1/2/3 转向', log: '串口日志', source: '菜谱来自 HowToCook（Unlicense）',
    search: '搜索菜名，例如 西红柿', nothing: '没有找到', loadingList: '加载中…',
    stations: ['水槽', '备菜台', '灶台'],
    stepCount: (n, t) => `第 ${n} / ${t} 步`, ready: '准备', calc: '用量',
    ingredientsN: n => `${n} 种原料和工具`, need: '需要',
    loading: name => `正在加载 ${name || '菜谱'}…`,
    loadFail: '菜谱加载失败，请检查网络后重试',
    chooseFirst: '先选择一道菜', last: '已经是最后一步', first: '已经是第一页',
    next: '下一步 →', prev: '← 上一步', readFlash: '🔊 朗读', stopped: '⏸ 已停止 · 长按重新朗读',
    noSpeech: '这个浏览器不支持朗读', turning: name => `转向${name}`,
    locked: '已锁定转向', unlocked: '已解除锁定',
  },
};

/* ---------- State ---------- */
const store = {
  get(k) { try { return JSON.parse(localStorage.getItem(k)); } catch (e) { return null; } },
  set(k, v) { try { localStorage.setItem(k, JSON.stringify(v)); } catch (e) {} },
};
let lang = store.get('hfr.lang') === 'zh' ? 'zh' : 'en';
let T = I18N[lang];
const state = {
  recipe: null,       // { id, title, pages }
  page: 0,
  facing: 1,
  moving: false,
  locked: false,
  autoRead: false,
};

const $ = id => document.getElementById(id);

/* ---------- English recipes: TheMealDB ---------- */
const API = 'https://www.themealdb.com/api/json/v1/1/';
async function api(path) {
  const res = await fetch(API + path);
  if (!res.ok) throw new Error(res.status);
  return res.json();
}

// Instructions come in mixed formats: "step 1" headers, "1. ..." lists, or one long paragraph.
// Every page gets one short action: sentences are packed only while they stay under STEP_MAX characters.
const STEP_MAX = 110;
function splitSteps(text) {
  const lines = text.replace(/\r/g, '').split('\n')
    .map(l => l.trim())
    .filter(l => l && !/^(step\s*)?\d+[.):]?$/i.test(l))
    .map(l => l.replace(/^(step\s*\d+\s*[-.:)]?\s*|\d+[.)]\s+|[-•▢*]\s*)/i, '').trim())
    .filter(Boolean);
  const steps = [];
  for (const line of lines) {
    const parts = line.split(/(?<=[.!?])\s+(?=[A-Z(])/)
      .flatMap(s => s.length > STEP_MAX ? s.split(/;\s+|,\s+(?=then\b)/i) : [s]);
    let cur = '';
    for (const s of parts.map(x => x.trim()).filter(Boolean)) {
      if (cur && cur.length + s.length + 1 > STEP_MAX) { steps.push(cur); cur = s; }
      else cur = cur ? cur + ' ' + s : s;
    }
    if (cur) steps.push(cur);
  }
  return steps.map(s => s.charAt(0).toUpperCase() + s.slice(1));
}

// Which ingredients does a step use?
const norm = s => s.toLowerCase().replace(/chilli/g, 'chili').replace(/[^a-z\s]/g, ' ');
const stem = w => w.replace(/ies$/, 'i').replace(/y$/, 'i').replace(/(o|s|x|ch|sh)es$/, '$1').replace(/([^s])s$/, '$1');
const wordsOf = s => norm(s).split(/\s+/).filter(Boolean).map(stem);

function ingredientsForStep(text, ingredients) {
  const words = new Set(wordsOf(text));
  // 1. every word of the ingredient name appears in the step ("chili powder")
  const full = ingredients.filter(g => g.words.length && g.words.every(w => words.has(w)));
  const used = new Set(full.flatMap(g => g.words));
  // 2. only the main noun appears ("the onion" for "Red Onions"), unless a more exact match already used that word
  //    (so "chilli powder" does not also pull in "Green Chilli")
  const loose = ingredients.filter(g => !full.includes(g) && words.has(g.head) && !used.has(g.head));
  return [...new Set([...full, ...loose].map(g => g.label))];
}

function parseMeal(m) {
  const items = [];
  const ingredients = [];
  for (let i = 1; i <= 20; i++) {
    const ing = (m['strIngredient' + i] || '').trim();
    const qty = (m['strMeasure' + i] || '').trim();
    if (!ing) continue;
    const label = qty ? `${qty} ${ing}` : ing;
    items.push(label);
    const words = wordsOf(ing);
    ingredients.push({ label, words, head: words[words.length - 1] });
  }
  const tags = [m.strArea, m.strCategory].filter(x => x && x !== 'Unknown' && x !== 'None');
  const pages = [{
    type: 'overview', text: m.strMeal,
    paras: tags.length ? [tags.join(' · ')] : [], items,
    imgs: m.strMealThumb ? [m.strMealThumb + '/medium'] : [],
  }];
  const steps = splitSteps(m.strInstructions || '');
  steps.forEach((text, i) => pages.push({
    type: 'step', group: '', text, notes: [], imgs: [], n: i + 1, total: steps.length,
    needs: ingredientsForStep(text, ingredients),
  }));
  return { id: m.idMeal, title: m.strMeal, pages };
}

const MealDB = {
  defaultId: '52772',   // Teriyaki Chicken Casserole
  defaultCat: 'Chicken',
  async categories() {
    return ((await api('list.php?c=list')).meals || []).map(c => ({ key: c.strCategory, label: c.strCategory }));
  },
  async list(cat) {
    return ((await api('filter.php?c=' + encodeURIComponent(cat))).meals || [])
      .map(m => ({ id: m.idMeal, name: m.strMeal, sub: cat }));
  },
  async search(q) {
    return ((await api('search.php?s=' + encodeURIComponent(q))).meals || [])
      .map(m => ({ id: m.idMeal, name: m.strMeal, sub: m.strArea || m.strCategory }));
  },
  async load(id) {
    let meal = store.get('hfr.en.meal.' + id);
    if (!meal) {
      const data = await api('lookup.php?i=' + encodeURIComponent(id));
      meal = data.meals && data.meals[0];
      if (!meal) throw new Error('not found');
      store.set('hfr.en.meal.' + id, meal);
    }
    return parseMeal(meal);
  },
};

/* ---------- Chinese recipes: HowToCook (Anduin2017/HowToCook, dishes/) ---------- */
// Recipe catalog omitted from this documentation for readability.
// The original recipe_display.html retains the complete RECIPES array.
const CATEGORY = {
  vegetable_dish: '素菜', meat_dish: '荤菜', aquatic: '水产', breakfast: '早餐', staple: '主食',
  soup: '汤与粥', drink: '饮料', condiment: '酱料', dessert: '甜品', 'semi-finished': '半成品'
};
const enc = p => p.split('/').map(encodeURIComponent).join('/');
const SOURCES = [
  p => 'https://raw.githubusercontent.com/Anduin2017/HowToCook/master/' + enc(p),
  p => 'https://cdn.jsdelivr.net/gh/Anduin2017/HowToCook@master/' + enc(p),
];

function clean(s) {
  return s
    .replace(/!\[[^\]]*\]\([^)]*\)/g, '')
    .replace(/\[([^\]]*)\]\([^)]*\)/g, '$1')
    .replace(/<[^>]+>/g, '')
    .replace(/\*\*([^*]+)\*\*/g, '$1')
    .trim();
}

function imagesIn(s, baseDir) {
  const out = [];
  s.replace(/!\[[^\]]*\]\(([^)\s]+)[^)]*\)/g, (_, src) => {
    if (/^https?:/.test(src)) out.push(src);
    else {
      const parts = (baseDir + '/' + src).split('/');
      const res = [];
      for (const x of parts) { if (x === '..') res.pop(); else if (x && x !== '.') res.push(x); }
      out.push(res.join('/'));
    }
    return '';
  });
  return out;
}

function parseRecipe(md, fallbackTitle, path = '') {
  const baseDir = path.split('/').slice(0, -1).join('/');
  const lines = md.replace(/\r/g, '').split('\n');
  let title = fallbackTitle;
  const intro = [];
  const sections = {};
  const order = [];
  let cur = null;
  for (const l of lines) {
    let m;
    if ((m = l.match(/^#\s+(.+)/))) { title = m[1].replace(/的做法\s*$/, '').trim(); continue; }
    if ((m = l.match(/^##\s+(.+)/))) { cur = m[1].trim(); sections[cur] = []; order.push(cur); continue; }
    (cur ? sections[cur] : intro).push(l);
  }
  const section = key => sections[order.find(o => o.includes(key))] || [];
  const bullet = /^\s*[-*+]\s+(.*)/;

  const pages = [];

  // Page 1: dish name, intro, ingredients
  const paras = intro.map(clean).filter(Boolean);
  const ingredients = section('原料').map(l => l.match(bullet)).filter(Boolean).map(m => clean(m[1])).filter(Boolean);
  const overview = { type: 'overview', text: title, paras, items: ingredients, imgs: imagesIn(intro.join('\n'), baseDir) };
  pages.push(overview);

  // Page 2: amounts
  const calc = section('计算').map(l => clean(l.replace(/^\s*[-*+]\s+/, ''))).filter(Boolean);
  if (calc.length) pages.push({ type: 'calc', text: '用量计算', items: calc });

  // Then one page per step
  const ops = section('操作');
  const hasNumbered = ops.some(l => /^\d+[.)、]\s+/.test(l));
  const top = hasNumbered ? /^\d+[.)、]\s+(.*)/ : /^[-*+]\s+(.*)/;
  let group = '';
  let last = null;
  const pending = [];
  const steps = [];
  for (const raw of ops) {
    let m;
    if ((m = raw.match(/^###+\s+(.+)/))) { group = clean(m[1]); continue; }
    const imgs = imagesIn(raw, baseDir);
    if ((m = raw.match(top)) && clean(m[1])) {
      last = { type: 'step', group, text: clean(m[1]), notes: pending.splice(0), imgs };
      steps.push(last);
      continue;
    }
    if (last) last.imgs.push(...imgs);
    const t = clean(raw.replace(/^\s*[-*+]\s+/, '').replace(/^\s*\d+[.)、]\s+/, ''));
    if (!t) continue;
    (last ? last.notes : pending).push(t);
  }
  // Ingredient names are short words ("鸡蛋", "葱"); a step uses one when its text contains the name
  const names = ingredients.map(s => s.split(/[\s（(：:,，]/)[0]).filter(Boolean);
  steps.forEach((s, i) => {
    s.n = i + 1; s.total = steps.length;
    const text = s.text + s.notes.join('');
    s.needs = [...new Set(names.filter(n => text.includes(n)))];
  });
  // No photo at the top of the recipe: show the last step photo (usually the finished dish) on the first page
  const stepImgs = steps.flatMap(s => s.imgs);
  if (!overview.imgs.length && stepImgs.length) overview.imgs = [stepImgs[stepImgs.length - 1]];
  pages.push(...steps);
  return { title, pages };
}

const HowToCook = {
  defaultId: 'dishes/vegetable_dish/西红柿炒鸡蛋.md',
  defaultCat: '',
  async categories() {
    return [{ key: '', label: '全部' }, ...Object.entries(CATEGORY).map(([key, label]) => ({ key, label }))];
  },
  async list(cat) {
    return RECIPES.filter(r => !cat || r[2] === cat).map(([id, name, c]) => ({ id, name, sub: CATEGORY[c] || c }));
  },
  async search(q) {
    return RECIPES.filter(r => r[1].includes(q)).map(([id, name, c]) => ({ id, name, sub: CATEGORY[c] || c }));
  },
  async load(path) {
    const entry = RECIPES.find(r => r[0] === path);
    if (!entry) throw new Error('not found');
    let md = store.get('hfr.md.' + path);
    if (!md) {
      let lastErr;
      for (const src of SOURCES) {
        try {
          const res = await fetch(src(path));
          if (!res.ok) throw new Error(res.status);
          md = await res.text();
          store.set('hfr.md.' + path, md);
          break;
        } catch (e) { lastErr = e; }
      }
      if (!md) throw lastErr;
    }
    return { id: path, ...parseRecipe(md, entry[1], path) };
  },
};

const PROVIDERS = { en: MealDB, zh: HowToCook };
let provider = PROVIDERS[lang];

/* ---------- Display ---------- */
function esc(s) {
  return String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
}

// Highlight what the cook has to watch: times and temperatures (and `code` spans in HowToCook)
const KEY_INFO = {
  en: /\b\d+(?:[.\/]\d+)?(?:\s*(?:-|–|to)\s*\d+(?:[.\/]\d+)?)?\s*(?:°\s*[CF]\b|°|C\b(?=\/|\s|$|[.,])|F\b|degrees|minutes?|mins?|hours?|hrs?|seconds?|secs?)/gi,
  zh: /\d+(?:\.\d+)?\s*(?:[-~～到]\s*\d+(?:\.\d+)?\s*)?(?:分钟|秒钟?|小时|℃|度)/g,
};
function fmt(s) {
  return esc(s).split(/(`[^`]+`)/).map((part, i) =>
    i % 2 ? `<mark>${part.slice(1, -1)}</mark>` : part.replace(KEY_INFO[lang], m => `<mark>${m}</mark>`)).join('');
}

function render() {
  renderStations();
  const r = state.recipe;
  if (!r) return;
  const p = r.pages[state.page];
  $('recipeBtn').textContent = r.title;

  $('progress').innerHTML = r.pages.map((_, i) =>
    `<span class="${i < state.page ? 'done' : i === state.page ? 'now' : ''}"></span>`).join('');

  if (p.type === 'step') {
    $('count').textContent = T.stepCount(p.n, p.total);
    $('group').textContent = p.group || '';
  } else {
    $('count').textContent = p.type === 'calc' ? T.calc : T.ready;
    $('group').textContent = p.type === 'overview' && p.items.length ? T.ingredientsN(p.items.length) : '';
  }

  const h = $('stepText');
  h.className = p.type === 'step' ? '' : 'title' + (p.text.length > 28 ? ' long' : '');
  h.innerHTML = p.type === 'step' ? fmt(p.text) : esc(p.text);

  $('needs').innerHTML = p.type === 'step' && p.needs.length
    ? `<span class="label">${T.need}</span>` + p.needs.map(t => `<span class="chip">${esc(t)}</span>`).join('')
    : '';

  $('paras').innerHTML = p.type === 'overview' ? p.paras.map(t => `<p>${fmt(t)}</p>`).join('') : '';

  const list = p.type === 'step' ? p.notes : p.items;
  const ul = $('items');
  ul.className = 'items' + (p.type === 'overview' ? '' : ' notes') + ((list || []).length > 10 ? ' many' : '');
  ul.innerHTML = (list || []).map(t => `<li>${fmt(t)}</li>`).join('');

  $('photos').innerHTML = (p.imgs || []).map(src => {
    const urls = /^https?:/.test(src) ? [src] : SOURCES.map(f => f(src));
    return `<img alt="" loading="lazy" src="${esc(urls[0])}" data-alt="${esc(urls[1] || '')}" onerror="if(this.dataset.alt){this.src=this.dataset.alt;this.dataset.alt=''}else{this.remove()}">`;
  }).join('');

  store.set(`hfr.${lang}.last`, { id: r.id, page: state.page });
}

function renderStations(pingIndex) {
  $('stations').innerHTML = T.stations.map((name, i) => {
    const cls = ['station'];
    if (i === state.facing) cls.push('active');
    if (i === state.facing && state.moving) cls.push('moving');
    if (i === pingIndex) cls.push('ping');
    return `<span class="${cls.join(' ')}">${name}</span>`;
  }).join('');
  $('lockBtn').setAttribute('aria-pressed', state.locked);
  $('autoBtn').setAttribute('aria-pressed', state.autoRead);
}

let flashTimer;
function flash(msg) {
  const el = $('flash');
  el.textContent = msg;
  el.classList.add('show');
  clearTimeout(flashTimer);
  flashTimer = setTimeout(() => el.classList.remove('show'), 1200);
}

/* ---------- Language switch ---------- */
function applyLang() {
  T = I18N[lang];
  document.documentElement.lang = lang === 'zh' ? 'zh-CN' : 'en';
  document.querySelectorAll('[data-i18n]').forEach(el => { el.textContent = T[el.dataset.i18n]; });
  document.querySelectorAll('[data-i18n-html]').forEach(el => { el.innerHTML = T[el.dataset.i18nHtml]; });
  document.querySelectorAll('[data-i18n-title]').forEach(el => { el.title = T[el.dataset.i18nTitle]; });
  $('search').placeholder = T.search;
  $('langBtn').textContent = lang === 'en' ? '中文' : 'English';
  refreshConn();
  renderStations();
}

function setLang(l) {
  stopReading();
  lang = l;
  store.set('hfr.lang', l);
  provider = PROVIDERS[l];
  categories = [];
  rows = [];
  activeCat = store.get(`hfr.${l}.cat`) ?? provider.defaultCat;
  $('search').value = '';
  state.recipe = null;
  $('progress').innerHTML = $('count').textContent = $('group').textContent = '';
  $('needs').innerHTML = $('paras').innerHTML = $('items').innerHTML = $('photos').innerHTML = '';
  applyLang();
  openLast();
}

function openLast() {
  const last = store.get(`hfr.${lang}.last`);
  if (last && last.id) openRecipe(last.id, last.page);
  else openRecipe(provider.defaultId);
}

/* ---------- Read aloud ---------- */
const voices = { en: null, zh: null };
function pickVoice() {
  const all = speechSynthesis.getVoices();
  voices.en = all.find(v => /en[-_]US/i.test(v.lang) && v.localService)
    || all.find(v => /en[-_]US/i.test(v.lang)) || all.find(v => /^en/i.test(v.lang)) || null;
  voices.zh = all.find(v => /zh[-_]CN/i.test(v.lang)) || all.find(v => /^zh/i.test(v.lang)) || null;
}
if ('speechSynthesis' in window) {
  pickVoice();
  speechSynthesis.onvoiceschanged = pickVoice;
}

const forSpeech = {
  en: s => s
    .replace(/(\d)\s*°\s*F\b/g, '$1 degrees Fahrenheit')
    .replace(/(\d)\s*°?\s*C\b/g, '$1 degrees Celsius')
    .replace(/°/g, ' degrees')
    .replace(/(\d)\s*-\s*(\d)/g, '$1 to $2')
    .replace(/(\d)\s*g\b/g, '$1 grams')
    .replace(/(\d)\s*ml\b/gi, '$1 milliliters')
    .replace(/(\d)\s*tbsp\b/gi, '$1 tablespoons')
    .replace(/(\d)\s*tsp\b/gi, '$1 teaspoons')
    .replace(/(\d)\s*mins?\b/gi, '$1 minutes'),
  zh: s => s
    .replace(/`/g, '')
    .replace(/(\d)\s*[-~～]\s*(\d)/g, '$1到$2')
    .replace(/\s\*\s/g, ' 乘 ')
    .replace(/(\d)\s*kg/gi, '$1千克')
    .replace(/(\d)\s*ml/gi, '$1毫升')
    .replace(/(\d)\s*mm/gi, '$1毫米')
    .replace(/(\d)\s*cm/gi, '$1厘米')
    .replace(/(\d)\s*g\b/gi, '$1克')
    .replace(/(\d)\s*L\b/g, '$1升')
    .replace(/℃/g, '度'),
};

const joinZh = arr => arr.map(s => s.replace(/[。．.；;，,：:\s]+$/, '')).join('。');

function pageSpeech(p) {
  if (lang === 'zh') {
    if (p.type === 'overview') return `${p.text}。` + (p.items.length ? `需要准备：${p.items.join('、')}。` : '');
    if (p.type === 'calc') return `用量。${joinZh(p.items)}。`;
    let s = `第${p.n}步。${joinZh([p.text])}。`;
    if (p.notes.length) s += `提示：${joinZh(p.notes)}。`;
    if (p.needs.length) s += `需要：${p.needs.join('、')}。`;
    return s;
  }
  if (p.type === 'overview') return `${p.text}. ` + (p.items.length ? `You will need: ${p.items.join(', ')}.` : '');
  return `Step ${p.n}. ${p.text}` + (p.needs.length ? ` You need: ${p.needs.join(', ')}.` : '');
}

let reading = false;   // true from speak() until the voice finishes or is stopped
let current = null;
function setReading(on) {
  reading = on;
  $('reading').hidden = !on;
}

function speak(text) {
  if (!('speechSynthesis' in window)) { flash(T.noSpeech); return; }
  speechSynthesis.cancel();
  const u = new SpeechSynthesisUtterance(forSpeech[lang](text));
  u.lang = lang === 'zh' ? 'zh-CN' : 'en-US';
  if (voices[lang]) u.voice = voices[lang];
  u.rate = 0.95;
  u.onend = u.onerror = () => { if (current === u) setReading(false); };
  current = u;
  setReading(true);
  setTimeout(() => { if (current === u) speechSynthesis.speak(u); }, 60);   // Chrome sometimes drops speak() right after cancel()
}

function stopReading() {
  current = null;
  if ('speechSynthesis' in window) speechSynthesis.cancel();
  setReading(false);
}

/* ---------- Pedal gestures ---------- */
// While the voice is reading, a tap or double tap only stops it; the next tap turns the page.
// Hold always reads the current page again from the start.
function onTap() {
  if (reading) { stopReading(); flash(T.stopped); return; }
  next();
}
function onDoubleTap() {
  if (reading) { stopReading(); flash(T.stopped); return; }
  prev();
}
function onHold() {
  readPage();
}

/* ---------- Actions ---------- */
function next() {
  const r = state.recipe;
  if (!r) return flash(T.chooseFirst);
  if (state.page >= r.pages.length - 1) { flash(T.last); speak(T.last); return; }
  state.page++;
  render();
  flash(T.next);
  if (state.autoRead) readPage();
}
function prev() {
  const r = state.recipe;
  if (!r) return flash(T.chooseFirst);
  if (state.page === 0) { flash(T.first); return; }
  state.page--;
  render();
  flash(T.prev);
  if (state.autoRead) readPage();
}
function readPage() {
  const r = state.recipe;
  if (!r) return flash(T.chooseFirst);
  flash(T.readFlash);
  speak(pageSpeech(r.pages[state.page]));
}

function goStation(i) {
  if (port) send(`GOTO ${i}`);
  else { state.facing = i; renderStations(); }   // no hardware: simulate on screen only
}
function toggleLock() {
  state.locked = !state.locked;
  if (port) send(state.locked ? 'LOCK' : 'UNLOCK');
  renderStations();
  flash(state.locked ? T.locked : T.unlocked);
}

/* ---------- Load a recipe ---------- */
let loadSeq = 0;
async function openRecipe(id, page = 0, name = '') {
  const seq = ++loadSeq;
  const forLang = lang;
  $('stepText').className = 'title';
  $('stepText').textContent = T.loading(name);
  try {
    const parsed = await provider.load(id);
    if (seq !== loadSeq || forLang !== lang) return;   // a newer load or a language switch won
    state.recipe = parsed;
    state.page = Math.min(page, parsed.pages.length - 1);
    render();
  } catch (e) {
    if (seq !== loadSeq) return;
    $('stepText').textContent = T.loadFail;
    log('Load failed: ' + id + ' ' + e);
  }
}

/* ---------- Recipe picker ---------- */
let activeCat = store.get(`hfr.${lang}.cat`) ?? provider.defaultCat;
let categories = [];
let rows = [];

function renderList() {
  $('recipeList').innerHTML = rows.map(m =>
    `<button data-id="${esc(m.id)}" data-name="${esc(m.name)}">${esc(m.name)}<small>${esc(m.sub || '')}</small></button>`).join('')
    || `<p style="padding:12px;color:var(--muted)">${T.nothing}</p>`;
}
function renderCats() {
  $('cats').innerHTML = categories.map(c =>
    `<button data-cat="${esc(c.key)}" aria-pressed="${c.key === activeCat}">${esc(c.label)}</button>`).join('');
}
async function loadRows(fetchRows) {
  $('recipeList').innerHTML = `<p style="padding:12px;color:var(--muted)">${T.loadingList}</p>`;
  try { rows = await fetchRows(); }
  catch (e) { rows = []; log('List failed: ' + e); }
  renderList();
}
async function openPicker() {
  $('picker').showModal();
  $('search').focus();
  if (!categories.length) {
    try { categories = await provider.categories(); }
    catch (e) { log('Categories failed: ' + e); }
  }
  renderCats();
  if (!rows.length) loadRows(() => provider.list(activeCat));
}
let searchTimer;
$('recipeBtn').onclick = openPicker;
$('search').oninput = () => {
  clearTimeout(searchTimer);
  const q = $('search').value.trim();
  searchTimer = setTimeout(() => {
    if (q) { activeCat = null; renderCats(); loadRows(() => provider.search(q)); }
  }, 300);
};
$('cats').onclick = e => {
  const b = e.target.closest('button');
  if (!b) return;
  activeCat = b.dataset.cat;
  store.set(`hfr.${lang}.cat`, activeCat);
  $('search').value = '';
  renderCats();
  loadRows(() => provider.list(activeCat));
};
$('recipeList').onclick = e => {
  const b = e.target.closest('button[data-id]');
  if (!b) return;
  $('picker').close();
  openRecipe(b.dataset.id, 0, b.dataset.name);
};
$('picker').addEventListener('click', e => { if (e.target === $('picker')) $('picker').close(); });

/* ---------- Web Serial ---------- */
let port = null;
let writer = null;
let connErr = '';   // key into I18N for the last connection problem

function log(line) {
  const el = $('log');
  el.textContent = (el.textContent + line + '\n').split('\n').slice(-200).join('\n');
  el.scrollTop = el.scrollHeight;
}

async function send(cmd) {
  if (!writer) return;
  log('→ ' + cmd);
  await writer.write(cmd + '\n');
}

function setConnected(on, err = '') {
  connErr = on ? '' : err;
  refreshConn();
}
function refreshConn() {
  const on = !!port;
  $('connDot').className = 'dot' + (on ? ' on' : connErr ? ' err' : '');
  $('conn').className = 'conn' + (connErr ? ' err' : '');
  $('connText').textContent = on ? T.connected : connErr ? T.failed : T.notConnected;
  $('connectBtn').textContent = on ? T.disconnect : T.connect;
  $('hint').textContent = connErr ? T[connErr] : '';
}

function handleLine(line) {
  log('← ' + line);
  const [cmd, a] = line.trim().split(/\s+/);
  const i = parseInt(a, 10);
  switch (cmd) {
    case 'READY':
      if (state.locked) send('LOCK');
      break;
    case 'STATION':
      state.facing = i; state.moving = true; renderStations(i);
      flash(T.turning(T.stations[i]));
      break;
    case 'ARRIVED':
      state.facing = i; state.moving = false; renderStations();
      break;
    case 'NEXT': renderStations(i); onTap(); break;
    case 'PREV': renderStations(i); onDoubleTap(); break;
    case 'READ': renderStations(i); onHold(); break;
    case 'LOCK': state.locked = a === '1'; renderStations(); break;
    case 'STATUS': {   // STATUS facing=1 angle=90 locked=0
      const f = line.match(/facing=(\d)/), l = line.match(/locked=(\d)/);
      if (f) state.facing = +f[1];
      if (l && (l[1] === '1') !== state.locked) send(state.locked ? 'LOCK' : 'UNLOCK');
      state.moving = false;
      renderStations();
      break;
    }
  }
}

async function openPort(p) {
  try {
    await p.open({ baudRate: 115200 });
  } catch (e) {
    log('Open failed: ' + e.name + ' ' + e.message);
    // Usually the port is held by another program, most often the Arduino IDE Serial Monitor
    setConnected(false, 'busy');
    return false;
  }
  port = p;
  setConnected(true);
  const encoder = new TextEncoderStream();
  encoder.readable.pipeTo(port.writable).catch(() => {});
  writer = encoder.writable.getWriter();
  readLoop();
  // Boards on native USB (ESP32-C6) do not restart when the port opens, so ask where the screen is facing
  setTimeout(() => send('STATUS'), 300);
  return true;
}

async function connect() {
  if (!('serial' in navigator)) {
    setConnected(false, 'noSerial');
    return;
  }
  let p;
  try {
    p = await navigator.serial.requestPort();
  } catch (e) {
    log('No port chosen: ' + e.message);
    setConnected(false, 'noPort');
    return;
  }
  openPort(p);
}

async function readLoop() {
  const dec = new TextDecoderStream();
  const closed = port.readable.pipeTo(dec.writable).catch(() => {});
  const reader = dec.readable.getReader();
  port._reader = reader;
  let buf = '';
  try {
    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      buf += value;
      let n;
      while ((n = buf.indexOf('\n')) >= 0) {
        const line = buf.slice(0, n).trim();
        buf = buf.slice(n + 1);
        if (line) handleLine(line);
      }
    }
  } catch (e) {
    log('Serial disconnected: ' + e.message);
  }
  await closed;
  disconnect();
}

async function disconnect() {
  if (!port) return;
  const p = port;
  port = null;
  try { await p._reader?.cancel(); } catch (e) {}
  try { await writer?.close(); } catch (e) {}
  writer = null;
  try { await p.close(); } catch (e) {}
  setConnected(false);
}

// Reconnect on its own to a port this page was allowed to use before
async function autoConnect() {
  if (!('serial' in navigator) || port) return;
  const ports = await navigator.serial.getPorts();
  if (ports.length) openPort(ports[0]);
}
if ('serial' in navigator) {
  navigator.serial.addEventListener('connect', autoConnect);
  navigator.serial.addEventListener('disconnect', e => { if (e.target === port) disconnect(); });
}

$('connectBtn').onclick = () => (port ? disconnect() : connect());
$('lockBtn').onclick = toggleLock;
$('autoBtn').onclick = () => { state.autoRead = !state.autoRead; store.set('hfr.auto', state.autoRead); renderStations(); };
$('langBtn').onclick = () => setLang(lang === 'en' ? 'zh' : 'en');

/* ---------- Keyboard (testing without hardware) ---------- */
document.addEventListener('keydown', e => {
  if ($('picker').open || e.target.tagName === 'INPUT') return;
  if (e.key === 'ArrowRight') onTap();
  else if (e.key === 'ArrowLeft') onDoubleTap();
  else if (e.key === ' ') { e.preventDefault(); onHold(); }
  else if (['1', '2', '3'].includes(e.key)) goStation(+e.key - 1);
  else if (e.key === 'l' || e.key === 'L') toggleLock();
  else if (e.key === 'a' || e.key === 'A') $('autoBtn').click();
  else if (e.key === '/') { e.preventDefault(); openPicker(); }
});

/* ---------- Start ---------- */
state.autoRead = !!store.get('hfr.auto');
applyLang();
$('stepText').textContent = T.start;
openLast();
autoConnect();
</script>
</body>
</html>

```
