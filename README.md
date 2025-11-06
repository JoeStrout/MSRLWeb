# MSRLWeb - MiniScript + Raylib Web

A web-based demonstration combining [MiniScript](https://miniscript.org) scripting with [Raylib](https://www.raylib.com/) graphics, compiled to WebAssembly using Emscripten.

## Try it now!

Go see the online demo at [https://joestrout.github.io/MSRLWeb/](https://joestrout.github.io/MSRLWeb/).

## Prerequisites

1. **MiniScript-cpp**: C++ implementation of MiniScript
   - Clone from: https://github.com/JoeStrout/miniscript
   - Must be in a folder named `MiniScript-cpp` **next to** the `MSRLWeb` folder
   - Directory structure should be:
     ```
     parent-folder/
     ├── MiniScript-cpp/     (cloned from GitHub)
     └── MSRLWeb/            (this project)
     ```

2. **Raylib**: Graphics library
   - Download/clone from: https://www.raylib.com/ or https://github.com/raysan5/raylib
   - Create a symlink in the MSRLWeb directory pointing to your raylib installation:
     ```bash
     cd MSRLWeb
     ln -s /path/to/your/raylib raylib
     ```
   - Build raylib for web: `cd raylib/src && make PLATFORM=PLATFORM_WEB`

3. **Emscripten SDK**: Required for compiling to WebAssembly
   - Install from: https://emscripten.org/docs/getting_started/downloads.html
   - The build script will automatically activate it if found at `raylib/../emsdk`
   - Or activate manually: `source /path/to/emsdk/emsdk_env.sh`

4. **CMake**: Build system (version 3.12 or higher)
   - Install via package manager or from: https://cmake.org/

## Project Structure

```
MSRLWeb/
├── src/
│   ├── main.cpp              # Main program (loads and runs MiniScript)
│   ├── RaylibIntrinsics.cpp  # Raylib intrinsics for MiniScript
│   └── RaylibIntrinsics.h
├── assets/
│   └── main.ms               # Your MiniScript program (main entry point)
├── MiniScript/               # Symlink to ../MiniScript-cpp/src/MiniScript
├── raylib/                   # Symlink to raylib source (you must create)
├── build/                    # Build output (generated)
├── Notes/                    # Documentation and reference materials
├── CMakeLists.txt            # CMake build configuration
├── shell.html                # Custom HTML template for the web page
├── build.sh                  # Convenience build script
├── run.sh                    # Convenience run script
└── README.md                 # This file
```

**Note:** The `MiniScript` symlink points to `../MiniScript-cpp/src/MiniScript` (relative path).
The `raylib` symlink must be created by you and is gitignored.

## Building

### Quick Build

```bash
./build.sh
```

### Manual Build

```bash
# Activate Emscripten
source /path/to/emsdk/emsdk_env.sh

# Create build directory
mkdir -p build
cd build

# Configure
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

## Running

### Quick Run

```bash
./run.sh
```

Then open http://localhost:8000/msrlweb.html in your browser (on Mac, this should happen automatically).

### Manual Run

```bash
cd build
python3 -m http.server 8000
```

Then open http://localhost:8000/msrlweb.html in your browser.

## What It Does

The application is **MiniScript-driven**, meaning your game/app logic is written in MiniScript:

1. Initializes a Raylib window (960x640)
2. Creates a MiniScript interpreter with Raylib intrinsics
3. Shows a loading screen while fetching `assets/main.ms`
4. Loads and runs your MiniScript code from `assets/main.ms`
5. Your script has full access to Raylib functions via the `raylib` module

This architecture allows you to:
- Write game logic in MiniScript (easy to modify, no recompile)
- Use Raylib for graphics, input, audio, etc.
- Edit `assets/main.ms` and refresh the browser to see changes

## Features

- **MiniScript-Driven**: Your game/app is written in MiniScript, not C++
- **Raylib Graphics**: Access to Raylib's 2D/3D graphics via intrinsics
- **Hot Reload**: Edit `assets/main.ms` and refresh browser (no rebuild needed)
- **WebAssembly**: Runs in any modern web browser
- **Async Loading**: Scripts loaded at runtime using Emscripten FETCH API

## Available Raylib Functions

Currently exposed in the `raylib` module:

**Drawing Functions:**
- `raylib.BeginDrawing()` - Start drawing
- `raylib.EndDrawing()` - End drawing
- `raylib.ClearBackground(color)` - Clear screen with color
- `raylib.DrawRectangle(x, y, width, height, color)` - Draw rectangle

**Color Constants:**
- `raylib.WHITE`, `raylib.BLACK`, `raylib.RED`, `raylib.GREEN`, `raylib.BLUE`
- `raylib.YELLOW`, `raylib.ORANGE`, `raylib.PINK`, `raylib.MAGENTA`
- `raylib.RAYWHITE`, `raylib.GRAY`, `raylib.DARKGRAY`, `raylib.LIGHTGRAY`
- `raylib.SKYBLUE`, `raylib.DARKBLUE`

Colors are maps with `r`, `g`, `b`, `a` components (0-255).

Many more RayLib functions will be coming soon!

## Example MiniScript Code

```miniscript
// assets/main.ms
print "main.ms running"  // (appears in the console output)

x = 100; y = 100
dx = 10; dy = 10

while true
	raylib.BeginDrawing
	
	raylib.ClearBackground raylib.BLACK
	raylib.DrawRectangle x, y, 200, 150, raylib.BLUE
	x += dx; y += dy
	if x + 200 > 960 and dx > 0 then dx = -dx
	if x < 0 and dx < 0 then dx = -dx
	if y + 150 > 640 and dy > 0 then dy = -dy
	if y < 0 and dy < 0 then dy = -dy
	
	raylib.EndDrawing
	yield
end while
```

## Build Configuration

The CMakeLists.txt includes:
- All MiniScript core source files
- Raylib web library linking
- Emscripten flags:
  - `-s ASYNCIFY`: Allows async operations
  - `-s FETCH=1`: Enables runtime file loading
  - `-s ALLOW_MEMORY_GROWTH=1`: Dynamic memory allocation
  - Optimized for size (`-Os`)

## Next Steps

- Add more Raylib intrinsics to MiniScript
- Implement runtime script loading
- Add interactive REPL in the web UI
- Create example games/demos

## Troubleshooting

### Raylib library not found

Make sure raylib is built for web:
```bash
cd raylib/src
make PLATFORM=PLATFORM_WEB
```

### Emscripten not found

Activate the Emscripten environment:
```bash
source /path/to/emsdk/emsdk_env.sh
```

### Build fails with missing symbols

Ensure all MiniScript source files are included in CMakeLists.txt's `MINISCRIPT_SOURCES`.

## License

See individual component licenses:
- MiniScript: https://miniscript.org
- Raylib: https://www.raylib.com/license.html
