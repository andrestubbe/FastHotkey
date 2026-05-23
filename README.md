# FastHotkey — Low-Latency Global Hotkey API for Java [v0.1.0]

**A zero-overhead native module for the FastJava ecosystem. Register and handle global system hotkeys with minimal latency.**

[![Status](https://img.shields.io/badge/status-v0.1.0--alpha-orange.svg)]()
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

**FastHotkey** provides an ultra-fast way to handle global key combinations. By using native Windows hooks, it ensures your application responds to hotkeys even when it is not in focus, with zero JNA overhead.

## Table of Contents
- [Features](#features)
- [Installation](#installation)
- [License](#license)

## Features
- **⌨️ Global Hooks**: Register hotkeys that work system-wide.
- **⚡ Low Latency**: Native Win32 integration for instant event delivery.
- **📦 Zero Overhead**: Pure JNI implementation bypassing heavy Java input layers.
- **🚀 Focus Agnostic**: Works perfectly in the background or for system-wide tools.

## Installation

### Option 1: Maven (Recommended)
Add the JitPack repository and the dependencies to your `pom.xml`:

`xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- FastHotkey Library -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fasthotkey</artifactId>
        <version>v0.1.0</version>
    </dependency>
    <!-- FastCore (Required Native Loader) -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>v0.1.0</version>
    </dependency>
</dependencies>
`

### Option 2: Gradle (via JitPack)
`groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:.1.0'
    implementation 'com.github.andrestubbe:fastcore:v0.1.0'
}
`

### Option 3: Direct Download (No Build Tool)
Download the latest JARs directly to add them to your classpath:

1. 📦 **[fasthotkey-v0.1.0.jar](https://github.com/andrestubbe/FastHotkey/releases/download/v0.1.0/fasthotkey-v0.1.0.jar)** (The Core Library)
2. ⚙️ **[fastcore-v0.1.0.jar](https://github.com/andrestubbe/FastCore/releases/download/v0.1.0/fastcore-v0.1.0.jar)** (The Mandatory Native Loader)

> [!IMPORTANT]
> All JARs must be in your classpath for the native JNI calls to function correctly.


## License
MIT License — See [LICENSE](LICENSE) for details.

---
**Part of the FastJava Ecosystem** — *Making the JVM faster.*


