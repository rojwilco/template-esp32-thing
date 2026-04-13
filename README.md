# template-esp32-thing

A generic, reusable ESP32 firmware template.
Includes WiFi provisioning, NVS config persistence, a web UI, OTA updates,
CI/CD, and a full unit-test suite. A GPIO toggle serves as a concrete peripheral
placeholder — replace it with your actual hardware logic.

---

## Part 1: Using This Template

### Create a new repo from this template

**GitHub UI:** Click **Use this template** → **Create a new repository**.

**Manual clone + re-init:**
```bash
git clone https://github.com/rojwilco/template-esp32-thing my-new-project
cd my-new-project
rm -rf .git
git init
git remote add origin https://github.com/YOUR_ORG/my-new-project
```

### What to change before your first real commit

| Step | What to do |
|------|-----------|
| 1 | Rename `template-esp32-thing.ino` to `my-project.ino`; update `tests/sketch_wrapper.cpp` and `tests/CMakeLists.txt` |
| 2 | Update config variables, NVS namespace, and `saveConfig()`/`loadConfig()` in the `.ino` |
| 3 | Update `config_html.h` form fields to match your new config variables |
| 4 | Reset `version.h` to `0.1.0-dev` |
| 5 | Change the FQBN in `.github/workflows/build.yml` if using a different board |
| 6 | Replace `runTask()` GPIO toggle with your peripheral logic |
| 7 | Rewrite `README.md` and `CLAUDE.md` for your project |

See `CLAUDE.md` for the full customization checklist with details.

### Running the tests

**Requirements:** cmake ≥ 3.14, ninja, a C++17 compiler (clang or gcc), internet
access for GoogleTest download on first build.

```bash
cd tests && make          # build + run all tests (zero failures required)
cd tests && make clean    # start fresh
```

### VS Code C++ TestMate setup

These steps are not committed — configure locally after cloning.

1. Install the **C++ TestMate** extension (`matepek.vscode-catch2-test-adapter`).
2. Open the repo root folder in VS Code.
3. Add to your local `.vscode/settings.json` (not committed):
   ```json
   {
     "testMate.cpp.test.executables": "tests/build/test_*"
   }
   ```
4. Build once: `cd tests && make build`
5. Click the **Testing** panel in VS Code — tests appear under the three suites.

---

## Part 2: About This Firmware

> **Replace this section** when customizing the template for a real project.

### Hardware requirements

- Any ESP32 module (default FQBN: `esp32:esp32:d1_mini32`)
- GPIO 2 (built-in LED on most boards) used as toggle placeholder

### First-boot WiFi setup

1. Power on — device starts in AP mode.
2. Connect to WiFi SSID **`ESP32-Thing-Setup`**.
3. Open **`http://192.168.4.1`** in a browser.
4. Enter your WiFi SSID and password, click **Save**.
5. Device reconnects in station mode and the AP disappears.

### Configuration reference

| Field | Default | Range | Description |
|-------|---------|-------|-------------|
| WiFi SSID | *(empty)* | up to 63 chars | Station-mode network name |
| WiFi Password | *(empty)* | up to 63 chars | Station-mode password |
| GPIO Pin | 2 | 0 – 39 | Pin to toggle |
| Toggle Interval | 1000 ms | ≥ 100 ms | GPIO toggle period |
| Poll Interval | 1 min | 1 – 1440 min | How often `runTask()` runs |

### Flashing

```bash
# arduino-cli
arduino-cli compile --fqbn esp32:esp32:d1_mini32 .
arduino-cli upload  --fqbn esp32:esp32:d1_mini32 -p /dev/ttyUSB0 .

# Arduino IDE: open template-esp32-thing.ino, select board, Upload
```

### OTA update procedure

1. Open the web UI at the device IP address.
2. In the **OTA Firmware Update** section, click the file picker and select a `.bin`.
3. Click **Upload Firmware** — device flashes and reboots automatically.
4. Build a `.bin` with: `arduino-cli compile --fqbn esp32:esp32:d1_mini32 --output-dir ./out .`
