# MSRLWeb API Extensions and Implementation Notes

This document describes features in MSRLWeb that extend or differ from the standard Raylib API.

## Table of Contents
- [Default Parameters](#default-parameters)
- [Flexible Parameter Formats](#flexible-paramater-formats)
- [Codepoints Parameter Enhancement](#codepoints-parameter-enhancement)
- [MiniScript-Specific Classes](#miniscript-specific-classes)

---
## Default Parameters

Many Raylib functions have been enhanced with sensible default parameters for convenience in MiniScript.

**Examples:**

**DrawText:**
```miniscript
// Standard: raylib.DrawText(text, posX, posY, fontSize, color)
// With defaults:
raylib.DrawText("Hello")  // Uses posX=0, posY=0, fontSize=20, color=BLACK
raylib.DrawText("Hello", 100, 100)  // Uses fontSize=20, color=BLACK
```

**DrawTextEx:**
```miniscript
// Defaults: position={x:0,y:0}, fontSize=20, spacing=0, tint=BLACK
raylib.DrawTextEx(font, "Hello World")
```

**DrawTextPro:**
```miniscript
// Defaults: position={x:0,y:0}, origin={x:0,y:0}, rotation=0, fontSize=20, spacing=0, tint=BLACK
raylib.DrawTextPro(font, "Rotated", {x: 200, y: 100}, null, 45)  // Rotated 45 degrees
```

**LoadFontEx:**
```miniscript
// Defaults: fontSize=20, codepoints=null, codepointCount=0
raylib.LoadFontEx("myfont.ttf")  // Loads all glyphs at size 20
raylib.LoadFontEx("myfont.ttf", 32)  // Loads all glyphs at size 32
raylib.LoadFontEx("myfont.ttf", 32, "ABC")  // Loads only A, B, C at size 32
```

---

## Flexible Parameter Formats

### Color Parameters

Any function accepting a **Color** parameter can receive the color in multiple formats:

**1. Map format (recommended):**
```miniscript
color = {"r": 255, "g": 100, "b": 50, "a": 255}
raylib.ClearBackground(color)

// You can omit alpha, it defaults to 255
raylib.DrawText("Hello", 10, 10, 20, {"r": 0, "g": 0, "b": 0})
```

**2. List format:**
```miniscript
color = [255, 100, 50, 255]  // [r, g, b, a]
raylib.ClearBackground(color)

// You can omit alpha, it defaults to 255
raylib.DrawText("Hello", 10, 10, 20, [0, 0, 0])
```

**3. Using built-in color constants:**
```miniscript
raylib.ClearBackground(raylib.RAYWHITE)
raylib.DrawText("Hello", 10, 10, 20, raylib.BLACK)
```


### Vector2 Parameters

Functions accepting **Vector2** (like positions) accept:

**1. Map format:**
```miniscript
position = {"x": 100, "y": 200}
raylib.DrawTextEx(font, "Text", position, 20, 2, raylib.BLACK)
```

**2. List format:**
```miniscript
position = [100, 200]  // [x, y]
raylib.DrawTextEx(font, "Text", position, 20, 2, raylib.BLACK)
```

### Rectangle Parameters

Functions accepting **Rectangle** accept:

**1. Map format:**
```miniscript
rect = {"x": 100, "y": 100, "width": 200, "height": 50}
raylib.DrawRectangleRec(rect, raylib.RED)
```

**2. List format:**
```miniscript
rect = [100, 100, 200, 50]  // [x, y, width, height]
raylib.DrawRectangleRec(rect, raylib.RED)
```

**Examples combining formats:**
```miniscript
// Drawing a button-like rectangle
btnRect = {"x": 340, "y": 250, "width": 120, "height": 40}
btnColor = {"r": 100, "g": 150, "b": 200, "a": 255}
textColor = [255, 255, 255]  // White as list

raylib.DrawRectangleRec(btnRect, btnColor)
raylib.DrawText("Click Me", 355, 262, 16, textColor)
```

---

## Codepoints Parameter Enhancement

**Affected Functions:**
- `raylib.LoadFontEx`
- `raylib.LoadFontFromMemory`
- `raylib.LoadFontData`
- `raylib.DrawTextCodepoints`
- `raylib.LoadUTF8`

**Enhancement:**
In standard Raylib, the `codepoints` parameter must be an array of integers. In MSRLWeb, the `codepoints` parameter accepts **either**:
1. A **list of integers** (standard behavior)
2. A **UTF-8 string** (MiniScript extension)

When a string is provided, it is automatically parsed to extract all Unicode codepoints in the string.

**Examples:**

```miniscript
// Standard usage with a list of codepoint integers
codepoints = [65, 66, 67, 97, 98, 99]  // A, B, C, a, b, c
font = raylib.LoadFontEx("myfont.ttf", 32, codepoints)

// MiniScript extension: pass a string directly
font = raylib.LoadFontEx("myfont.ttf", 32, "ABCabc")

// Useful for loading only specific characters you need
font = raylib.LoadFontEx("myfont.ttf", 48, "0123456789.,$")

// Works with Unicode characters too
font = raylib.LoadFontEx("myfont.ttf", 32, "こんにちは世界")

// Draw text using codepoints from a string
raylib.DrawTextCodepoints(font, "Hello", {x: 100, y: 100}, 32, 2, raylib.WHITE)
```


---

## MiniScript-Specific Classes

### RawData Class

MSRLWeb introduces a `RawData` class for managing binary data buffers, primarily used for audio sample manipulation and font loading from memory.

**Constructor:**
```miniscript
data = RawData.make(sizeInBytes)
```

**Key Features:**
- Manages memory with malloc/realloc for Raylib compatibility
- Ownership tracking to prevent double-free errors
- Endianness conversion support (littleEndian property)
- Typed accessors for various data types

**Methods:**
- `resize(newSize)` - Resize the buffer
- Typed getters: `getUInt8(offset)`, `getUInt16(offset)`, `getUInt32(offset)`, `getInt8(offset)`, `getInt16(offset)`, `getInt32(offset)`, `getFloat(offset)`, `getDouble(offset)`
- Typed setters: `setUInt8(offset, value)`, `setUInt16(offset, value)`, etc.
- `getUTF8(offset, count)` - Read UTF-8 string
- `setUTF8(offset, str)` - Write UTF-8 string

**Usage with Audio:**
```miniscript
// Load wave samples into RawData
data = raylib.LoadWaveSamples(wave)
// Modify samples...
sample = data.getFloat(0)
data.setFloat(0, sample * 0.5)
// Update sound with modified data
raylib.UpdateSound(sound, data, sampleCount)
// Clean up
raylib.UnloadWaveSamples(data)
```

**Usage with Font Loading:**
```miniscript
// Load font file into memory
fileData = RawData.make(fileSize)
// ... read file into fileData ...
font = raylib.LoadFontFromMemory(".ttf", fileData, 32, "ABC")
```

**Memory Management:**
- RawData uses malloc/realloc internally for Raylib compatibility
- Properly tracked ownership prevents double-free issues
- `ReleaseOwnership()` transfers ownership (e.g., to Raylib)
- `TakeOwnership()` reclaims ownership when Raylib returns it

---

## Notes on Platform Limitations

### Web Platform (Emscripten)

The following Raylib functions are **not implemented** as they are not applicable to the web platform:

**Window Management:**
- `InitWindow`, `CloseWindow`
- `SetWindowPosition`, `SetWindowSize`
- `MaximizeWindow`, `MinimizeWindow`, `RestoreWindow`
- `ToggleFullscreen`, `ToggleBorderlessWindowed`
- Window icon functions

**File System:**
- `ChangeDirectory`, `MakeDirectory`
- `DirectoryExists`, `FileExists`
- `LoadDirectoryFiles`, `UnloadDirectoryFiles`
- `SaveFileData`, `SaveFileText`, `UnloadFileData`, `UnloadFileText`

**VR:**
- All VR-related functions (`BeginVrStereoMode`, `EndVrStereoMode`, etc.)

**Monitor:**
- Monitor enumeration and query functions (may return single monitor info only)

**3D Features:**
- Limited or no 3D rendering support for now (web platform focus is 2D)

---

## Contributing

When adding new extensions or modifications to the standard Raylib API, please document them in this file with:
1. Function names affected
2. Description of the enhancement
3. Code examples
4. Rationale for the change
