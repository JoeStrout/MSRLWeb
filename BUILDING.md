# Building MSRLWeb from Source

This guide is for developers who want to build MSRLWeb from source. If you just want to use MSRLWeb to make games, see the [main README](README.md) for easier options.

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
├── dist/                     # Release packages (generated)
├── docs/                     # Live demo (GitHub Pages)
├── CMakeLists.txt            # CMake build configuration
├── build.sh                  # Convenience build script
├── run.sh                    # Convenience run script
├── make_release.sh           # Creates release packages
├── generate_api_doc.sh       # Generates API documentation
└── README.md                 # User-facing documentation
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

## Running Locally

### Quick Run

```bash
./run.sh
```

Then open http://localhost:8000 in your browser (on Mac, this should happen automatically).

### Manual Run

```bash
cd build
python3 -m http.server 8000
```

Then open http://localhost:8000 in your browser.

## Build Configuration

The CMakeLists.txt includes:
- All MiniScript core source files
- Raylib web library linking
- Emscripten flags:
  - `-sASYNCIFY`: Allows async operations
  - `-sFETCH=1`: Enables runtime file loading
  - `-sALLOW_MEMORY_GROWTH=1`: Dynamic memory allocation
  - `-fexceptions`: Enable C++ exception handling
  - Optimized for size (`-Os`)

## Creating Release Packages

To create a distributable release package:

```bash
./make_release.sh 1.0.0
```

This will:
1. Build the project
2. Generate API documentation
3. Create a `dist/msrlweb-1.0.0/` folder with all necessary files
4. Create `msrlweb-1.0.0.zip` ready for GitHub release

The package includes:
- All WASM/JS/HTML files
- Sample game in `assets/`
- API documentation
- User-friendly README

## Updating the Live Demo

The `docs/` folder contains the live demo served at https://joestrout.github.io/MSRLWeb/

To update it:
```bash
# Build the project
./build.sh

# Copy to docs folder
cp build/msrlweb.html docs/
cp build/msrlweb.js docs/
cp build/msrlweb.wasm docs/
cp build/index.html docs/
cp -r build/assets docs/

# Commit and push
git add docs/
git commit -m "Update live demo"
git push
```

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

Or let the build script find it automatically if installed at `raylib/../emsdk`.

### Build fails with missing symbols

Ensure all MiniScript source files are included in CMakeLists.txt's `MINISCRIPT_SOURCES`.

### Changes to intrinsics not taking effect

After modifying `RaylibIntrinsics.cpp`, you need to rebuild:
```bash
./build.sh
```

Then clear your browser cache or do a hard refresh (Ctrl+Shift+R / Cmd+Shift+R).

## Development Workflow

1. **Edit MiniScript code** (`assets/main.ms`) - Just refresh browser
2. **Edit C++ intrinsics** (`src/RaylibIntrinsics.cpp`) - Run `./build.sh` and refresh
3. **Edit main program** (`src/main.cpp`) - Run `./build.sh` and refresh

The hot-reload workflow is:
- **MiniScript changes**: No rebuild needed! Just edit and refresh browser
- **C++ changes**: Rebuild required, then refresh browser

## License

See individual component licenses:
- MiniScript: https://miniscript.org
- Raylib: https://www.raylib.com/license.html
