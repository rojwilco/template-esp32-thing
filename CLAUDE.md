# CLAUDE.md — ESP32 Thing Template

> **This is a template repository.**
> When you (an AI agent) are dropped into a project derived from this template,
> your first task is to **rewrite this file** to describe the actual project.
> Follow the customization checklist below, then replace everything in this file
> with the "Project Developer Guide" structure shown at the bottom.

---

## Part 1: How to Customize This Template

Work through these steps in order. Run `cd tests && make` after each significant
change to keep the test suite green throughout.

### Checklist

- [ ] **Rename the sketch file** — rename `template-esp32-thing.ino` to match
      your project (e.g., `my-widget.ino`). Update `sketch_wrapper.cpp` and
      `CMakeLists.txt` to reference the new filename.

- [ ] **Replace the GPIO toggle placeholder** — in the `.ino`, replace `runTask()`
      with your actual peripheral logic. Keep the `g_forceRerun = false;` line at
      the top.

- [ ] **Update config variables** — change NVS namespace (default: `"esp32thing"`),
      field names, types, defaults, and validation clamps to match your hardware.

- [ ] **Update `config_html.h`** — replace GPIO-specific form fields with fields
      that match your new config variables. Keep the WiFi and OTA sections.

- [ ] **Update the project emoji** — change `⚙️` in the `<!-- Project emoji -->`
      comment in `config_html.h` to an emoji that suits your project. Update both
      the `<link rel="icon">` favicon and the `<h1>` tag.

- [ ] **Update `version.h`** — reset to `0.1.0-dev` for your new project.

- [ ] **Update `build.yml` FQBN** — change `esp32:esp32:d1_mini32` in the `env`
      block if you are targeting a different ESP32 board variant.

- [ ] **Update tests** — update `test_config.cpp`, `test_web_handlers.cpp`, and
      `test_gpio.cpp` (rename it to match your peripheral). Add or remove test
      cases to reflect the new config variables and peripheral behavior.

- [ ] **Rewrite `CLAUDE.md`** — replace this entire file with a project-specific
      developer guide (see template below).

- [ ] **Rewrite `README.md`** — replace the template instructions with actual
      project documentation.

- [ ] **Verify**: `cd tests && make` — all tests pass.

---

## Part 2: Project Developer Guide (Template — Replace After Customization)

> After customization, delete Part 1 above and replace Part 2 with real content
> following this structure:

### Build & Flash

```bash
# Compile (from repo root)
arduino-cli compile --fqbn esp32:esp32:d1_mini32 .

# Flash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:d1_mini32 .
```

Required tools:
- `arduino-cli` ≥ 0.35
- ESP32 board package `esp32:esp32` 3.3.7
- Libraries: built-in (WiFi, WebServer, Preferences, Update, DNSServer)

### First-Boot WiFi Setup

1. Power on the device. It boots into AP mode (no credentials stored).
2. Connect to WiFi SSID **ESP32-Thing-Setup**.
3. Open `http://192.168.4.1` — enter your WiFi credentials and save.
4. Device reboots into station mode.

### Architecture Overview

| Component | Role |
|-----------|------|
| `setup()` | Load NVS, init GPIO, connect WiFi or start AP, register routes, start server |
| `loop()` | `handleClient()`, reconnect if `g_pendingConnect`, run `runTask()` on schedule |
| `loadConfig()` / `saveConfig()` | NVS read/write via `Preferences` under namespace `"esp32thing"` |
| `applyHostname()` | Sets `esp32-thing-XXXXXX` from last 3 MAC bytes — must be called before `WiFi.begin()` |
| `startAPMode()` | Soft AP + captive-portal DNS |
| `runTask()` | Periodic peripheral work; clears `g_forceRerun` at entry |
| `handleRoot()` | Serves web UI with current config values injected |
| `handleSave()` | Parses POST, validates, saves NVS, sets `g_pendingConnect`/`g_forceRerun` as needed |
| `handleScan()` | WiFi scan → sorted, deduped JSON |
| `handleOtaUpload()` / `handleOtaUpdate()` | Stream-flash via `Update` class |

**Flag system:**
- `g_pendingConnect` — set by `handleSave()` when SSID or password changes; cleared in `loop()` after reconnect attempt.
- `g_forceRerun` — set by `handleSave()` when task-relevant config changes; cleared by `runTask()`.

### How to Add a Config Variable

1. Declare it as a `static` global in the `.ino`.
2. Add `prefs.get*` / `prefs.put*` calls in `loadConfig()` / `saveConfig()`.
3. Add clamping logic in `loadConfig()`.
4. Add the form field to `config_html.h`.
5. Parse it in `handleSave()`; set `g_forceRerun` if it affects `runTask()`.
6. Add round-trip and clamping tests in `test_config.cpp`.

### How to Add New Functionality

**New periodic task:** extend `runTask()` or call a helper from inside it.

**New web endpoint:**
```cpp
server.on("/myendpoint", HTTP_GET, handleMyEndpoint);
```
Register before `server.begin()`. Add a test in `test_web_handlers.cpp`.

### Testing

```bash
cd tests && make          # build + run all tests
cd tests && make build    # build only
cd tests && make test     # run only (assumes already built)
cd tests && make clean    # delete build dir
```

Mock files live in `tests/mocks/`. `sketch_wrapper.cpp` includes mocks then
`#include`s the `.ino` so the sketch compiles as a normal C++ translation unit.

**Adding a new test file:**
1. Create `tests/test_yourfeature.cpp`.
2. Add `add_sketch_test(test_yourfeature test_yourfeature.cpp)` to `tests/CMakeLists.txt`.
3. Run `make clean && make`.

**VS Code C++ TestMate setup (not committed):**
1. Install the *C++ TestMate* extension.
2. Open the repo root in VS Code.
3. In settings, set `testMate.cpp.test.executables` to `tests/build/test_*`.
4. Run `cd tests && make build` once to populate the build dir.
5. Test discovery should appear in the Testing panel.

### Versioning

- `version.h` holds the canonical version.
- Bump `PATCH` for bug fixes, `MINOR` for new features, `MAJOR` for breaking changes.
- During development use the `-dev` suffix in `FIRMWARE_VERSION`.
- Tag `vMAJOR.MINOR.PATCH` (no `-dev`) to trigger a GitHub Release.

### CI/CD

| Workflow | Trigger | What it does |
|----------|---------|--------------|
| `build.yml` | all pushes, PRs, `v*.*.*` tags | arduino-cli compile; on tag: GitHub Release with `.bin` |
| `tests.yml` | all pushes, PRs | CMake + ctest + JUnit reporter |

### Critical Constraints

- **Static buffers** — `cfg_wifi_ssid` and `cfg_wifi_pass` are `char[64]`. Always null-terminate and clamp incoming strings.
- **Hostname before `WiFi.begin()`** — `applyHostname()` must be called before connecting or `setHostname()` has no effect.
- **Mock include order** — in `sketch_wrapper.cpp`, all mock headers must be included before `#include "../sketch.ino"`. Do not reorder.
- **`g_forceRerun = false`** — must be the first line of `runTask()` to prevent infinite re-trigger if the task itself triggers a config change.
