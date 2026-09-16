# FastHotkey Reference Manual

A comprehensive technical reference for **FastHotkey** — low-latency, zero-overhead global hotkey registration and interception for Java on Windows.

---

## 1. Architecture & Execution Model

FastHotkey bridges Java with native Win32 keyboard events without relying on AWT/Swing or heavy JNA layers:

- **Direct Win32 Interception**: Bypasses the JVM Event Dispatch Thread (EDT) and unneeded window handles.
- **Dual Interception Pipelines**:
  - `COOPERATIVE`: Uses Win32 `RegisterHotKey(...)` / `UnregisterHotKey(...)` for standard system-compliant shortcuts.
  - `AGGRESSIVE`: Installs a native low-level keyboard hook (`WH_KEYBOARD_LL`), enabling applications to intercept and override reserved Windows shortcuts (e.g. `Ctrl+Space` IME switching, `Win+L`, and combinations claimed by other applications).
- **Background Message Pump**: Runs a lightweight native Win32 message loop (`GetMessage` / `DispatchMessage`) on an independent thread, dispatching directly to Java callbacks.
- **Zero Heap Overhead**: Event dispatching executes without per-event object allocations or garbage collection pressure.

---

## 2. FastHotkey API (`fasthotkey.FastHotkey`)

### Native Lifecycle

```java
public static synchronized void loadLibrary() throws UnsatisfiedLinkError, IOException
```
Extracts the embedded native DLL (`fasthotkey.dll`) from the JAR resources to a temporary location and loads it into the JVM. Falls back to `System.loadLibrary("fasthotkey")` if outside a packaged JAR. Registers a JVM shutdown hook that automatically stops active loops and cleans up native hooks. Must be invoked before any registration methods.

```java
public static void start()
```
Starts the native background message loop. Spawns an internal thread and begins listening for registered hotkey triggers. Returns immediately.

```java
public static void stop()
```
Stops the background message loop, unhooks native listeners, and releases OS resources.

```java
public static boolean isRunning()
```
Returns `true` if the background hotkey listener loop is currently active.

---

### Registration & Unregistration

```java
public static boolean register(int id, int modifiers, int vkCode, HotkeyCallback callback)
```
Registers a global hotkey with default `HotkeyMode.AGGRESSIVE` mode.
- `id`: Unique integer identifier for this hotkey (`1` to `0xBFFF`).
- `modifiers`: Bitwise OR combination of flags from [`ModifierKeys`](#modifierkeys-flags).
- `vkCode`: Virtual key code from [`KeyCodes`](#keycodes-constants).
- `callback`: Functional lambda invoked when the hotkey combination is pressed.
- Returns `true` if successfully registered.

```java
public static boolean register(int id, int modifiers, int vkCode, HotkeyCallback callback, HotkeyMode mode)
```
Registers a global hotkey with the specified [`HotkeyMode`](#hotkeymode-interception-modes).
- In `COOPERATIVE` mode, registration may fail if Windows or another program already owns the combination.
- In `AGGRESSIVE` mode, the low-level hook overrides system reservations.

```java
public static boolean unregister(int id)
```
Unregisters a previously registered hotkey by its ID.

```java
public static void unregisterAll()
```
Unregisters all active hotkeys managed by this process.

---

## 3. HotkeyMode (`fasthotkey.HotkeyMode`)

| Mode | Native Implementation | System Behavior | Best For |
|---|---|---|---|
| **`COOPERATIVE`** | `RegisterHotKey` Win32 API | Fails gracefully if the shortcut is already bound by Windows or another app. | Well-behaved standard utilities, productivity shortcuts. |
| **`AGGRESSIVE`** | `WH_KEYBOARD_LL` Low-Level Hook | Overrides Windows shortcuts (e.g. `Ctrl+Space` IME, `Win+L`) and active applications. | System launchers, custom overlays, gaming macros. |

---

## 4. ModifierKeys (`fasthotkey.ModifierKeys`)

Bitwise flags to combine with the pipe (`|`) operator:

| Constant | Value | Description |
|---|---|---|
| `MOD_ALT` | `0x0001` | Either `Alt` key held |
| `MOD_CONTROL` | `0x0002` | Either `Ctrl` key held |
| `MOD_SHIFT` | `0x0004` | Either `Shift` key held |
| `MOD_WIN` | `0x0008` | Either `Windows` key held |
| `MOD_NOREPEAT` | `0x4000` | Suppresses multiple triggers from keyboard auto-repeat |

### Helper Method
```java
int flags = ModifierKeys.combine(ModifierKeys.MOD_CONTROL, ModifierKeys.MOD_SHIFT);
```

---

## 5. KeyCodes (`fasthotkey.KeyCodes`)

Standard Windows Virtual-Key (VK) codes:

- **Letters**: `VK_A` (`0x41`) through `VK_Z` (`0x5A`)
- **Numbers**: `VK_0` (`0x30`) through `VK_9` (`0x39`)
- **Function Keys**: `VK_F1` (`0x70`) through `VK_F24` (`0x87`)
- **Navigation**: `VK_ESCAPE` (`0x1B`), `VK_SPACE` (`0x20`), `VK_RETURN` (`0x0D`), `VK_TAB` (`0x09`), `VK_BACK` (`0x08`)
- **Arrows**: `VK_LEFT` (`0x25`), `VK_UP` (`0x26`), `VK_RIGHT` (`0x27`), `VK_DOWN` (`0x28`)
- **Editing**: `VK_INSERT` (`0x2D`), `VK_DELETE` (`0x2E`), `VK_HOME` (`0x24`), `VK_END` (`0x23`), `VK_PRIOR` (`0x21` / Page Up), `VK_NEXT` (`0x22` / Page Down)

---

## 6. HotkeyCallback (`fasthotkey.HotkeyCallback`)

```java
@FunctionalInterface
public interface HotkeyCallback {
    void onHotkey(int hotkeyId);
}
```
Invoked synchronously on the native message dispatcher thread when the corresponding hotkey sequence is detected. Long-running tasks should be delegated to a worker thread or executor pool to keep the message pump responsive.

---

## 7. Platform Support & Requirements

| Requirement | Specification |
|---|---|
| **OS** | Windows 10 / 11 (x64) |
| **Java** | JDK 17 or higher |
| **Permissions** | Normal user privileges for `COOPERATIVE`; Administrator privileges may be required for intercepting specific elevated window inputs in `AGGRESSIVE` mode. |

---

**Part of the FastJava Ecosystem** — *Making the JVM faster.* 🚀
