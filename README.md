# MSRLWeb - Make Web Games with MiniScript + Raylib

**Write 2D games in [MiniScript](https://miniscript.org), powered by [Raylib](https://www.raylib.com/) graphics, running in your browser via WebAssembly.**

No compiler needed. Just edit your MiniScript code and assets, refresh the browser, and play!

## 🎮 Try It Now!

**[Play the live demo](https://joestrout.github.io/MSRLWeb/)** - See MSRLWeb in action!

## 📦 Download & Use

**[Download the latest release](https://github.com/JoeStrout/MSRLWeb/releases/latest)**

Unzip and you're ready to go:

1. Start a web server in the unzipped folder:
   ```bash
   python3 -m http.server 8000
   ```

2. Open your browser to: **http://localhost:8000**

3. Edit `assets/main.ms` to change the game - then refresh to see your changes.

That's it! No build tools, no compilation, no hassle.

## ✨ What Is This?

MSRLWeb lets you create 2D games and interactive graphics using:

- **[MiniScript](https://miniscript.org)** - A simple, elegant scripting language (like Python meets JavaScript)
- **[Raylib](https://www.raylib.com/)** - A powerful 2D/3D game programming library
- **WebAssembly** - Runs anywhere, right in the browser

Your game logic lives in **easy-to-edit MiniScript files**. Change the code, refresh the browser, see results instantly. No recompiling, no build steps.

## 🚀 Quick Example

Here's a complete bouncing sprite in MiniScript:

```miniscript
// Load a sprite
spriteTex = raylib.LoadTexture("assets/Wumpus.png")

// Starting position and velocity
x = 100; y = 100
dx = 5; dy = 5

// Game loop
while true
    raylib.BeginDrawing
    raylib.ClearBackground raylib.RAYWHITE

    // Move the ball
    x += dx
    y += dy

    // Bounce off edges
    if x > 960 - 64 or x < 0 then dx = -dx
    if y > 640 - 64 or y < 0 then dy = -dy

    // Draw it
    raylib.DrawTexture spriteTex, x, y, raylib.WHITE
    raylib.DrawFPS

    raylib.EndDrawing
    yield  // Let the browser breathe
end while
```

That's it! Edit, save, refresh.

## 📚 What's Available?

MSRLWeb provides **174 Raylib functions** organized into modules:

- **rcore** (29 functions) - Window, input, timing, mouse, keyboard
- **rshapes** (35 functions) - Circles, rectangles, lines, polygons, collision detection
- **rtextures** (45 functions) - Load/draw images, procedural textures, image manipulation
- **rtext** (12 functions) - Load fonts, draw text, measure text
- **raudio** (53 functions) - Sound effects, music streaming, spatial audio

See **[RAYLIB_API.md](RAYLIB_API.md)** for the complete function list.

All functions have sensible defaults, so you can start simple:
```miniscript
raylib.DrawCircle 100, 100, 50, raylib.RED  // Works!
raylib.DrawRectangle                        // Also works! (uses defaults)
```

## 🎯 Who Is This For?

- **Game developers** who want rapid prototyping without build tools
- **Educators** teaching game development with a simple language
- **Hobbyists** making web games with minimal friction
- **Anyone** who wants to try Raylib without installing C/C++ toolchains

## 🎓 Learning Resources

**MiniScript:**
- [Interactive Tutorial](https://miniscript.org/tryit/) - Learn by doing
- [Quick Reference](https://miniscript.org/files/MiniScript-QuickRef.pdf) - Syntax cheat sheet
- [Language Manual](https://miniscript.org/files/MiniScript-Manual.pdf) - Complete guide

**Raylib:**
- [Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html) - All Raylib functions
- [Examples](https://www.raylib.com/examples.html) - Code samples (in C, but easy to translate)

## 🛠️ Building from Source

Want to hack on MSRLWeb itself? See **[BUILDING.md](BUILDING.md)** for:
- Setting up the build environment
- Compiling from source
- Adding new Raylib intrinsics
- Creating release packages

## 📄 License

- **MSRLWeb**: [LICENSE](MIT License)
- **MiniScript**: MIT License - https://miniscript.org
- **Raylib**: zlib License - https://www.raylib.com/license.html

---

**Happy game making!** 🎮

Have questions or want to share what you've made? Open an [issue](https://github.com/JoeStrout/MSRLWeb/issues) or [discussion](https://github.com/JoeStrout/MSRLWeb/discussions), or [join us on Discord](https://discord.gg/7s6zajx).
