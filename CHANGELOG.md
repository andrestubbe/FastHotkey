# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-04-09

### Added
- Initial release of FastHotkey
- Global hotkey registration using Win32 RegisterHotKey API
- JNI-based native implementation for maximum performance
- Support for modifier keys (Alt, Ctrl, Shift, Win)
- Support for all virtual key codes (letters, numbers, function keys, special keys)
- Callback-based event handling with lambda support
- Background message loop for hotkey events
- Auto-cleanup on JVM shutdown
- Demo application with sample hotkeys
- Maven build configuration
- JAR resource extraction for native DLL

### Features
- **Global hotkeys**: Work system-wide even when app is not focused
- **Multiple hotkeys**: Register unlimited hotkeys with unique IDs
- **Simple API**: Clean Java interface with KeyCodes and ModifierKeys constants
- **High performance**: Native JNI implementation - faster than JNA

[1.0.0]: https://github.com/andrestubbe/fasthotkey/releases/tag/v1.0.0
