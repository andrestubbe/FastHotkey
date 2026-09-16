# FastHotkey 0.1.0 [ALPHA-2026-05-23] — Ultra-Low Latency Global Hotkey Engine for Java

[![Status](https://img.shields.io/badge/status-0.1.0-brightgreen.svg)](https://github.com/andrestubbe/FastHotkey/releases/tag/0.1.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-0.1.0-green.svg)](https://jitpack.io/#andrestubbe/FastHotkey)

---

**⚡ High-speed Win32 global hotkey registration, low-level keyboard interception, and system-wide shortcut overrides for Java.**

**FastHotkey** provides an ultra-fast, zero-overhead mechanism to register and handle global system hotkeys. Operating directly via native Win32 hooks and `RegisterHotKey`, it ensures your application responds to keystroke combinations instantaneously across the entire desktop—even when minimized or running out of focus—without AWT event dispatch bottlenecks or heavy JNA layers.

[![FastHotkey Showcase](docs/screenshot.png)](https://www.youtube.com/watch?v=BZsqQl7WqWk)

---

## Quick Start

```java
import fasthotkey.FastHotkey;
import fasthotkey.HotkeyMode;
import fasthotkey.KeyCodes;
import fasthotkey.ModifierKeys;

public class Demo {
    public static void main(String[] args) throws Exception {
        // 1. Initialize native library
        FastHotkey.loadLibrary();

        // 2. Register global hotkey (e.g. Ctrl + Space to trigger spotlight/launcher)
        FastHotkey.register(
            1,                                          // Hotkey ID (1 to 0xBFFF)
            ModifierKeys.MOD_CONTROL,                   // Modifiers
            KeyCodes.VK_SPACE,                          // Virtual Key Code
            id -> System.out.println("Global Ctrl+Space triggered!"),
            HotkeyMode.AGGRESSIVE                       // Overrides Windows IME reservation
        );

        // 3. Start native background message pump
        FastHotkey.start();

        // Keep process alive
        Thread.sleep(Long.MAX_VALUE);
    }
}
```

---

## Table of Contents

- [Quick Start](#quick-start)
- [Why FastHotkey?](#why-fasthotkey)
- [Key Features](#key-features)
- [Real-World Use Cases](#real-world-use-cases)
- [Performance Benchmarks](#performance-benchmarks)
- [API Quick Reference](#api-quick-reference)
- [Interception Modes: Cooperative vs Aggressive](#interception-modes-cooperative-vs-aggressive)
- [Technical Demos & Benchmarks](#technical-demos--benchmarks)
- [Installation](#installation)
- [Documentation](#documentation)
- [Platform Support](#platform-support)
- [Related Projects](#related-projects)
- [License](#license)

---

## Why FastHotkey?

Standard Java keyboard handling (AWT `KeyEventDispatcher`, Swing key bindings, or JavaFX accelerators) imposes fundamental limitations on system tools and high-performance applications:

- **Window-Focus Dependency**: AWT/Swing shortcuts only function when a Java window holds active OS foreground focus.
- **Event Dispatch Thread (EDT) Lag**: Keystrokes queue behind UI rendering passes, introducing erratic latency and dropped combinations during heavy load.
- **System Hotkey Conflicts**: Reserved Windows hotkeys (like `Ctrl+Space` for IME or system shell bindings) cannot be intercepted or reassigned by standard APIs.
- **Heavy JNA / JNI Overhead**: Generic third-party hooking libraries introduce object allocations and reflection overhead on every key stroke.

**FastHotkey** eliminates these bottlenecks:

- **Native System-Wide Reach**: Hooks register directly with Windows at kernel/subsystem level, firing regardless of which desktop app currently holds focus.
- **Dedicated Native Message Pump**: Runs on a lean native background thread (`GetMessage` / `DispatchMessage`), dispatching directly to Java callbacks with sub-microsecond latency.
- **Aggressive Override Support**: Low-level keyboard hooks (`WH_KEYBOARD_LL`) allow overriding system-claimed hotkeys such as `Ctrl+Space`.
- **Zero GC Allocation**: Hot-path dispatch triggers direct primitive callbacks without allocating temporary event objects.

---

## Key Features

- ⌨️ **Universal Global Registration** — Intercept combinations system-wide across any foreground application.
- ⚡ **Sub-Microsecond Latency** — Native Win32 message loop delivers events instantly to Java callbacks.
- 🥊 **Dual Interception Pipelines** — Switch between standard `COOPERATIVE` and override-capable `AGGRESSIVE` modes.
- 🛡️ **Win32 Shortcut Overrides** — Intercept reserved shortcuts like `Ctrl+Space` (IME) or `Win+Key` macros.
- 📦 **Zero Heap Allocations** — High-frequency native callbacks execute with zero garbage collection pressure.
- 🧹 **Robust Lifecycle Management** — Thread-safe start/stop controls with automatic JVM shutdown unhooking.

---

## Real-World Use Cases

- 🔍 **Spotlight & Command Palettes**: Instant summon overlay for desktop search tools (e.g. `Ctrl+Space` or `Alt+Space`) without losing keyboard responsiveness.
- 🎙️ **Streaming & Push-to-Talk Tools**: Global mute, scene switching, or recording toggles while running full-screen games or CAD applications.
- 🪟 **Tiling Window Managers & System Utilities**: Keyboard-driven window snapping, workspace switching, and desktop management tools built on the FastJava ecosystem.
- 🎮 **In-Game HUDs & Performance Overlays**: Global hotkey triggers for telemetry monitors, FPS displays, or terminal consoles ([FastTerminal](https://github.com/andrestubbe/FastTerminal)).

---

## Performance Benchmarks

FastHotkey is profiled using **JMH (Java Microbenchmark Harness)** to guarantee maximum throughput and zero transition overhead:

| Benchmark / Operation | Score (ops/ms) | Ops per Second |
|---|---|---|
| **`benchmarkRegisterAndUnregister`** | **~24,500 ops/ms** | **> 24.5 Million** |
| **Native Event Dispatch Latency** | **< 250 ns** | **Instantaneous** |

*Measured on Windows 11 (x64), JDK 17+. Hotkey dispatch executes on a dedicated native pump with 0 bytes GC allocation per event.*

---

## API Quick Reference

| Method | Return Type | Description | Docs |
|---|---|---|---|
| `FastHotkey.loadLibrary()` | `void` | Extracts and loads native Win32 DLL with auto-shutdown hook. | [Reference](docs/REFERENCE.md#native-lifecycle) |
| `FastHotkey.register(id, mod, vk, cb)` | `boolean` | Registers a hotkey using default `AGGRESSIVE` interception mode. | [Reference](docs/REFERENCE.md#registration--unregistration) |
| `FastHotkey.register(id, mod, vk, cb, mode)` | `boolean` | Registers a hotkey with explicit `COOPERATIVE` or `AGGRESSIVE` mode. | [Reference](docs/REFERENCE.md#registration--unregistration) |
| `FastHotkey.unregister(id)` | `boolean` | Unregisters an individual hotkey by ID. | [Reference](docs/REFERENCE.md#registration--unregistration) |
| `FastHotkey.unregisterAll()` | `void` | Cleans up and unbinds all registered hotkeys. | [Reference](docs/REFERENCE.md#registration--unregistration) |
| `FastHotkey.start()` | `void` | Launches the background Win32 message loop thread. | [Reference](docs/REFERENCE.md#native-lifecycle) |
| `FastHotkey.stop()` | `void` | Stops the background listener and unhooks native resources. | [Reference](docs/REFERENCE.md#native-lifecycle) |
| `FastHotkey.isRunning()` | `boolean` | Queries whether the background event loop is currently active. | [Reference](docs/REFERENCE.md#native-lifecycle) |

---

## Interception Modes: Cooperative vs Aggressive

FastHotkey provides two distinct native interception pipelines depending on application requirements:

```java
// Cooperative Mode: Uses Win32 RegisterHotKey API
FastHotkey.register(1, ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, KeyCodes.VK_F1, callback, HotkeyMode.COOPERATIVE);

// Aggressive Mode: Uses Win32 WH_KEYBOARD_LL Low-Level Hook
FastHotkey.register(2, ModifierKeys.MOD_CONTROL, KeyCodes.VK_SPACE, callback, HotkeyMode.AGGRESSIVE);
```

| Mode | Mechanism | Windows Conflict Handling | Ideal Use Case |
|---|---|---|---|
| **`COOPERATIVE`** | `RegisterHotKey` | Fails if hotkey is already claimed by Windows or another app. | Standard polite desktop utilities. |
| **`AGGRESSIVE`** | `WH_KEYBOARD_LL` | Intercepts keystrokes directly, overriding Windows-reserved hotkeys (`Ctrl+Space`, etc.). | System overlays, command palettes, game launchers. |

---

## Technical Demos & Benchmarks

| Case | Java Example | Launcher | Description |
|---|---|---|---|
| **Interactive Terminal Demo** | [Demo.java](examples/Demo/src/main/java/fasthotkey/Demo.java) | `run-demo.bat` | Live interactive demonstration registering `Ctrl+Alt+H`, `Ctrl+Space`, and `Ctrl+Shift+F12` with instant console feedback. |
| **JMH Microbenchmark Suite** | [Benchmark.java](examples/Benchmark/src/main/java/fasthotkey/benchmark/Benchmark.java) | `run-benchmark.bat` | High-frequency hotkey registration and unregistration throughput benchmarks. |

---

## Installation

### Option 1: Maven (Recommended)

Add the JitPack repository and the dependency to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- FastHotkey Core -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fasthotkey</artifactId>
        <version>0.1.0</version>
    </dependency>
    <!-- FastCore Native Loader -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (via JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:fasthotkey:0.1.0'
    implementation 'com.github.andrestubbe:fastcore:0.1.0'
}
```

### Option 3: Direct Download (No Build Tool)

Download the latest JARs directly to add them to your classpath:

1. 📦 **[fasthotkey-0.1.0.jar](https://github.com/andrestubbe/FastHotkey/releases/tag/0.1.0)** (The Core Library with embedded native DLL)
2. ⚙️ **[fastcore-0.1.0.jar](https://github.com/andrestubbe/FastCore/releases/download/0.1.0/fastcore-0.1.0.jar)** (The Mandatory Native Loader)

---

## Documentation

- **[COMPILE.md](docs/COMPILE.md)**: Full compilation guide (MSVC C++17 build chain + JNI Setup).
- **[REFERENCE.md](docs/REFERENCE.md)**: Complete API specification, modifier flags, and virtual key codes.
- **[PHILOSOPHY.md](docs/PHILOSOPHY.md)**: The engineering rationale for low-latency native interception.
- **[ROADMAP.md](docs/ROADMAP.md)**: Future milestones and planned features.
- **[CHANGELOG.md](docs/CHANGELOG.md)**: Complete version history and release notes.

---

## Platform Support

| Platform | Architecture | Status | Driver / Subsystem |
|:---|:---:|:---:|:---|
| **Windows 10 / 11** | x64 | ✅ Fully Supported | Native Win32 `RegisterHotKey` & Low-Level Hook (`WH_KEYBOARD_LL`) |
| **Linux** | x64 / AArch64 | 🚧 Planned | X11 `XGrabKey` & Wayland Global Shortcuts Portal |
| **macOS** | Apple Silicon / x64 | 🚧 Planned | Carbon `RegisterEventHotKey` & Quartz Event Taps |

---

## Related Projects

- **[`FastCore`](https://github.com/andrestubbe/FastCore)** — Native Library Loader & JNI Utilities for Java
- **[`FastMouse`](https://github.com/andrestubbe/FastMouse)** — Ultra-Low Latency Native RawInput Mouse Engine for Java
- **[`FastKeyboard`](https://github.com/andrestubbe/FastKeyboard)** — Ultra-Fast Native RawInput Keyboard Engine for Java
- **[`FastClipboard`](https://github.com/andrestubbe/FastClipboard)** — Low-Latency Native Windows Clipboard API & Format Pipeline
- **[`FastTerminal`](https://github.com/andrestubbe/FastTerminal)** — Native High-Speed Terminal & TUI Engine
- **[`FastWindow`](https://github.com/andrestubbe/FastWindow)** — Native Win32 Windowing & Borderless Substrate for Java

---

## License

MIT License — See [LICENSE](LICENSE) file for details.

---

**Part of the FastJava Ecosystem** — *Making the JVM faster.* 🚀
