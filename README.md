# LoRa-send-and-CAN-read
Reads CAN bus, translates and sends with LoRa, and saves to an SD card on an ESP32S3

# LoRa_CAN_read_and_send
Connected to the CAN bus and read it with the ESP32-TWAI-CAN library. Translated the CAN data into floats.
Sends with LoRa using the LoRa.h library by sandeepmistry.
Saves the CAN data on a SD-card.

# LoRa_receive
Receives the packets sent by the other function.

## Library Installation in Arduino IDE

### Required Libraries

To use these Arduino sketches, you need to install the following libraries in Arduino IDE:

1. **ESP32-TWAI-CAN** (for CAN bus communication)
2. **LoRa** by Sandeep Mistry (for LoRa wireless communication)

### Installation Steps

#### Method 1: Using Arduino IDE Library Manager (Recommended)

1. Open Arduino IDE
2. Go to **Tools** → **Manage Libraries...** (or press `Ctrl+Shift+I`)
3. For each library:
   - **ESP32-TWAI-CAN**: Search for "ESP32-TWAI-CAN" and click Install
   - **LoRa**: Search for "LoRa by Sandeep Mistry" and click Install

#### Method 2: Manual Installation via GitHub

If the libraries are not available in the Library Manager:

1. **ESP32-TWAI-CAN**:
   - Download from: https://github.com/nhatuan84/esp32-softcan-module
   - In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library...**
   - Select the downloaded ZIP file

2. **LoRa**:
   - Download from: https://github.com/sandeepmistry/arduino-LoRa
   - In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library...**
   - Select the downloaded ZIP file

### Built-in Libraries (No Installation Required)

These libraries are included with the ESP32 Arduino core:
- `SPI.h` - SPI communication
- `SD.h` - SD card interface
- `FS.h` - File system interface

### ESP32 Board Setup

Make sure you have the ESP32 board support installed:

1. In Arduino IDE, go to **File** → **Preferences**
2. Add this URL to **Additional Board Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools** → **Board** → **Boards Manager**
4. Search for "ESP32" and install **esp32 by Espressif Systems**
5. Select your ESP32 board from **Tools** → **Board** → **ESP32 Arduino**

### Verification

After installation, you can verify the libraries are correctly installed by:
1. Opening one of the `.ino` files
2. Going to **Sketch** → **Include Library** → The installed libraries should appear in the list
3. Try compiling the sketch (✓ button) to check for any missing dependencies

---

## Using VSCode/Cursor with PlatformIO

Yes! You can run this Arduino code in **VSCode** or **Cursor** using **PlatformIO**. PlatformIO provides better code completion, debugging, and library management than Arduino IDE.

**Note:** If you're using **Cursor** (not VSCode), see the Cursor-specific instructions below.

### Installation

1. **Install PlatformIO Extension in VSCode:**
   
   **Method 1 - Search in Extensions:**
   - Open VSCode
   - Go to Extensions (Ctrl+Shift+X) or press `Ctrl+Shift+X`
   - Try searching for any of these terms:
     - `PlatformIO IDE` (full name)
     - `PlatformIO` (just PlatformIO)
     - `platformio` (lowercase)
   - Look for the extension by **PlatformIO** (with the PlatformIO logo)
   - Click Install

   **Method 2 - Direct Marketplace Link:**
   - Open this link in your browser: https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide
   - Click the "Install" button, which will open VSCode and install the extension

   **Method 3 - Command Palette:**
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
   - Type: `Extensions: Install Extensions`
   - Search for `platformio`

   **Method 4 - Manual Installation (if above methods don't work):**
   - Download the `.vsix` file from: https://github.com/platformio/platformio-vscode-ide/releases
   - In VSCode, go to Extensions view
   - Click the three-dot menu (⋮) → "Install from VSIX..."
   - Select the downloaded `.vsix` file

   **Note:** Make sure you're using the official Microsoft VSCode, not VSCodium (VSCodium doesn't have access to the Microsoft Marketplace).

### Installing PlatformIO in Cursor IDE

If you're using **Cursor** instead of VSCode, PlatformIO extension isn't available in Cursor's marketplace. Use PlatformIO CLI (command line) instead:

1. **Install PlatformIO Core:**
   ```bash
   # Windows (PowerShell)
   python -m pip install platformio

   # Or using pipx (recommended)
   pip install --user platformio
   ```

2. **Use from Cursor's integrated terminal:**
   - Open terminal in Cursor (`Ctrl+`` or View → Terminal)
   - Navigate to your project folder:
     ```bash
     cd LoRa_CAN_read_and_send
     ```
   - Build the project:
     ```bash
     pio run
     ```
   - Upload to board:
     ```bash
     pio run --target upload
     ```
   - Monitor serial:
     ```bash
     pio device monitor
     ```

   All the library dependencies will be automatically installed based on `platformio.ini`.

### Project Structure

Each project folder (`LoRa_CAN_read_and_send` and `LoRa_receive`) has its own `platformio.ini` configuration file. When you run PlatformIO (via extension or CLI), it will automatically:
- Detect and compile all `.ino` and `.cpp` files in the same directory
- Install ESP32 platform support
- Install required libraries from the `lib_deps` section (LoRa, ESP32-TWAI-CAN)
- Configure the ESP32-S3 board settings
- Download and install build tools and compilers

**Note:** PlatformIO will compile all `.ino` and `.cpp` files in the project directory. The main `.ino` file serves as the entry point. Your current file structure will work as-is.

### Using PlatformIO

**In VSCode (with extension installed):**

1. **Open a Project:**
   - Open VSCode in one of the project folders (e.g., `LoRa_CAN_read_and_send`)
   - PlatformIO will automatically detect the `platformio.ini` file

2. **Build the Project:**
   - Click the ✓ (Build) button in the PlatformIO toolbar (bottom status bar)
   - Or use: **PlatformIO: Build** from the command palette (Ctrl+Shift+P)

3. **Upload to Board:**
   - Click the → (Upload) button in the PlatformIO toolbar
   - Or use: **PlatformIO: Upload**
   - Make sure your ESP32-S3 is connected via USB

4. **Monitor Serial Output:**
   - Click the 🔌 (Serial Monitor) button
   - Or use: **PlatformIO: Serial Monitor**

**In Cursor (using CLI):**

Use the command line from Cursor's integrated terminal:
- Build: `pio run`
- Upload: `pio run --target upload`
- Monitor: `pio device monitor`

**Change Board Type (if needed):**
   - Open `platformio.ini`
   - Change `board = esp32-s3-devkitc-1` to match your specific ESP32 board
   - Common options: `esp32dev`, `esp32-s3-devkitc-1`, `esp32doit-devkit-v1`, etc.

### Advantages of PlatformIO over Arduino IDE

- ✅ Better code completion and IntelliSense
- ✅ Integrated serial monitor
- ✅ Library version management
- ✅ Multiple project support
- ✅ Better error messages
- ✅ Integrated debugging support
- ✅ Git-friendly project structure

### Library Management

Libraries are automatically installed from `platformio.ini`. To add new libraries:
1. Edit `platformio.ini`
2. Add to `lib_deps` section, for example:
   ```
   lib_deps = 
       sandeepmistry/LoRa@^0.8.0
       new-library-name@version
   ```
3. PlatformIO will download and install automatically on next build