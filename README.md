# FastHotkey — Global Hotkey Library for Java via JNI (System-Wide Hotkeys)

**⚡ Ultra-fast global hotkey library for Java — system-wide hotkeys even when your app is not focused**

[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![JitPack](https://jitpack.io/v/andrestubbe/fasthotkey.svg)](https://jitpack.io/#andrestubbe/fasthotkey)
[![GitHub stars](https://img.shields.io/github/stars/andrestubbe/fasthotkey.svg)](https://github.com/andrestubbe/fasthotkey/stargazers)

```java
// Quick Start — Global system-wide hotkeys
FastHotkey.loadLibrary();

// Register Ctrl+Shift+F1 as global hotkey
FastHotkey.register(1, 
    ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, 
    KeyCodes.VK_F1,
    id -> System.out.println("Hotkey pressed!")
);

// Start listening (works even when app is not focused)
FastHotkey.start();
```

FastHotkey is a **high-performance Java hotkey library** that replaces `java.awt.Toolkit.getDefaultToolkit().addAWTEventListener()` with a **native Windows backend** using Win32 `RegisterHotKey` API. Built for **global system-wide hotkeys**, **background key capture**, **automation shortcuts**, and ** productivity tools**.

**Keywords:** java hotkey alternative, global hotkeys, system-wide keyboard shortcuts, windows hotkey api, registerhotkey jni, java global hotkey listener, low-latency hotkeys, automation hotkeys, background hotkey capture, win32 hotkey

If you need **system-wide hotkeys** that work **even when your app is not focused**, **low-latency key capture**, or **reliable global shortcuts**, FastHotkey delivers native-level performance with Java simplicity.

---

## Features

- **Global hotkeys**: Capture hotkeys system-wide, not just when your app is focused
- **High performance**: Native JNI implementation - faster than JNA
- **Simple API**: Register hotkeys with just a few lines of code
- **Callback-based**: Clean lambda/callback interface for hotkey events
- **Multiple hotkeys**: Register unlimited hotkeys with unique IDs
- **Auto-cleanup**: Automatic resource cleanup on JVM shutdown

## Quick Start

### 1. Build the Native Library

```batch
compile.bat
```

This compiles the C++ code to `fasthotkey.dll` and copies it to resources.

### 2. Build the Java Library

```batch
mvn clean package
```

### 3. Use in Your Code

```java
import fasthotkey.*;

// Load native library
FastHotkey.loadLibrary();

// Register Ctrl+Shift+F1
FastHotkey.register(1, 
    ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, 
    KeyCodes.VK_F1,
    id -> System.out.println("Hotkey pressed!")
);

// Start listening
FastHotkey.start();

// Your app runs here...
// Press Win+Q to quit in the demo
```

Run the demo:

```batch
java -cp target/fasthotkey-1.0.0.jar fasthotkey.Demo
```

## API Reference

### Loading the Library

```java
FastHotkey.loadLibrary();
```

Must be called before any other methods. Extracts and loads the native DLL from JAR resources.

### Registering Hotkeys

```java
boolean success = FastHotkey.register(
    int id,                    // Unique hotkey ID (1-0xBFFF)
    int modifiers,             // Modifier keys (ALT, CTRL, SHIFT, WIN)
    int vkCode,                // Virtual key code (A-Z, F1-F12, etc.)
    HotkeyCallback callback    // Called when hotkey is pressed
);
```

**Modifiers:** (combine with `|`)
- `ModifierKeys.MOD_ALT`
- `ModifierKeys.MOD_CONTROL`
- `ModifierKeys.MOD_SHIFT`
- `ModifierKeys.MOD_WIN`
- `ModifierKeys.MOD_NOREPEAT` - Don't trigger on key repeat

**Key Codes:**
- Letters: `KeyCodes.VK_A` through `KeyCodes.VK_Z`
- Numbers: `KeyCodes.VK_0` through `KeyCodes.VK_9`
- Function keys: `KeyCodes.VK_F1` through `KeyCodes.VK_F12`
- Special keys: `VK_ESCAPE`, `VK_SPACE`, `VK_RETURN`, `VK_TAB`, etc.

### Starting/Stopping

```java
FastHotkey.start();   // Start the message loop (background thread)
FastHotkey.stop();    // Stop and unregister all hotkeys
```

### Unregistering

```java
FastHotkey.unregister(int id);  // Unregister specific hotkey
FastHotkey.unregisterAll();      // Unregister all hotkeys
```

## Examples

### Global Screenshot Hotkey

```java
FastHotkey.register(1, 
    ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT,
    KeyCodes.VK_S,
    id -> {
        // Take screenshot
        captureScreenshot();
    }
);
```

### Quick Launch

```java
FastHotkey.register(2, 
    ModifierKeys.MOD_WIN | ModifierKeys.MOD_ALT,
    KeyCodes.VK_T,
    id -> {
        // Launch terminal
        Runtime.getRuntime().exec("cmd.exe /c start wt");
    }
);
```

### Multiple Hotkeys

```java
// Hotkey 1: Ctrl+F1 - Action A
FastHotkey.register(1, ModifierKeys.MOD_CONTROL, KeyCodes.VK_F1, 
    id -> doActionA());

// Hotkey 2: Ctrl+F2 - Action B
FastHotkey.register(2, ModifierKeys.MOD_CONTROL, KeyCodes.VK_F2, 
    id -> doActionB());

// Hotkey 3: Win+Q - Quit
FastHotkey.register(3, ModifierKeys.MOD_WIN, KeyCodes.VK_Q, 
    id -> System.exit(0));

FastHotkey.start();
```

## Project Structure

```
fasthotkey/
├── native/                    # C++ JNI implementation
│   ├── fasthotkey.h          # Header file
│   └── fasthotkey.cpp        # Implementation
├── src/main/java/fasthotkey/
│   ├── FastHotkey.java       # Main API class
│   ├── HotkeyCallback.java   # Callback interface
│   ├── KeyCodes.java         # Virtual key code constants
│   ├── ModifierKeys.java     # Modifier flag constants
│   └── Demo.java             # Demo application
├── compile.bat               # Build native DLL
├── pom.xml                   # Maven configuration
└── README.md
```

## JNI vs JNA Comparison

| Aspect | JNI (FastHotkey) | JNA |
|--------|------------------|-----|
| **Performance** | Fastest - direct native calls | 10-50x slower (libffi overhead) |
| **Development** | Write C++, compile per-platform | Pure Java, define interfaces |
| **Deployment** | Bundle platform-specific DLLs | Single JAR (libffi handles it) |
| **Maintenance** | Harder (C++ code + build scripts) | Easier (just Java interfaces) |

FastHotkey uses JNI for maximum performance - ideal for low-latency hotkey handling.

## Requirements

- Windows 10/11 (Win32 API)
- Java 17+
- Visual Studio 2019+ (for building native code)

## Building from Source

### Prerequisites

- JDK 17 or later
- Visual Studio with C++ workload
- Maven 3.8+

### Build Steps

1. **Compile native DLL:**
   ```batch
   compile.bat
   ```

2. **Build Java library:**
   ```batch
   mvn clean package
   ```

3. **Run demo:**
   ```batch
   java -cp target/fasthotkey-1.0.0.jar fasthotkey.Demo
   ```

## How It Works

FastHotkey uses the Windows API:

1. **RegisterHotKey** - Registers global hotkeys with the system
2. **Message-only window** - Creates an invisible window to receive WM_HOTKEY messages
3. **Message loop** - Runs a Windows message loop in a background thread
4. **JNI callbacks** - Calls Java callbacks when hotkeys are pressed

## Troubleshooting

### "UnsatisfiedLinkError: Can't find dependent libraries"
The DLL requires the Visual C++ Redistributable. Install it from Microsoft.

### "Cannot find jni.h"
Set JAVA_HOME environment variable to your JDK installation.

### Hotkeys not working
Some hotkeys may be reserved by Windows or other apps. Try different combinations.

## License

MIT License - See [LICENSE](LICENSE) file.

## Related Projects

- [FastClipboard](https://github.com/andrestubbe/FastClipboard) - Ultra-fast clipboard access
- [FastRobot](https://github.com/andrestubbe/FastRobot) - High-performance screen capture
