# Building from Source

## Prerequisites

- JDK 17+
- Maven 3.9+
- **Windows:** Visual Studio 2019+ or Build Tools with Windows SDK

## Build

### Windows

```bash
compile.bat
mvn clean package
```

The build script compiles the native DLL and packages it with the JAR.

## Run Examples

See README.md for usage examples.

## Installation

### JitPack (Recommended)

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fasthotkey</artifactId>
        <version>v1.0.0</version>
    </dependency>
</dependencies>
```

### Gradle (JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:fasthotkey:v1.0.0'
}
```

## Download Pre-built JAR

See [Releases Page](https://github.com/andrestubbe/FastHotkey/releases)

## Troubleshooting

### JNI UnsatisfiedLinkError

If you get `UnsatisfiedLinkError`, the native library was not found:

1. Check that `fasthotkey.dll` exists after running `compile.bat`
2. Ensure the DLL is in PATH or copy to `C:\Windows\System32`
3. Verify Windows SDK is installed for Win32 API support

### Hotkeys not working

- Run as Administrator for system-wide hotkeys
- Check that no other application is using the same hotkey
- Verify Windows SDK is installed
