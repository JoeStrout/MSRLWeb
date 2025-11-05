#!/bin/bash

# Build script for MSRLWeb (MiniScript + Raylib Web)

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "Building MSRLWeb (MiniScript + Raylib)..."

# Check if emcmake is available, if not try to activate emsdk
if ! command -v emcmake &> /dev/null; then
    echo -e "${YELLOW}emcmake not found, attempting to activate Emscripten...${NC}"

    # Try to find emsdk relative to raylib
    EMSDK_DIR="raylib/../emsdk"
    EMSDK_PATH="$EMSDK_DIR/emsdk_env.sh"

    if [ -f "$EMSDK_PATH" ]; then
        echo -e "${YELLOW}Found emsdk at $EMSDK_PATH${NC}"

        # Get absolute path to emsdk directory
        EMSDK_ABS=$(cd "$EMSDK_DIR" && pwd)

        # Reactivate emsdk for the current environment (fixes Python path issues)
        echo -e "${YELLOW}Reactivating emsdk for current Python environment...${NC}"
        cd "$EMSDK_ABS"
        ./emsdk activate latest

        if [ $? -ne 0 ]; then
            echo -e "${RED}Error: Failed to activate emsdk!${NC}"
            exit 1
        fi

        cd - > /dev/null

        # Now source the environment
        echo -e "${YELLOW}Loading Emscripten environment...${NC}"
        source "$EMSDK_ABS/emsdk_env.sh"

        # Check again if emcmake is now available
        if ! command -v emcmake &> /dev/null; then
            echo -e "${RED}Error: Failed to activate Emscripten!${NC}"
            echo -e "${YELLOW}Try running manually:${NC}"
            echo "  cd $EMSDK_DIR"
            echo "  ./emsdk activate latest"
            echo "  source ./emsdk_env.sh"
            exit 1
        fi
        echo -e "${GREEN}Emscripten activated successfully!${NC}"
    else
        echo -e "${RED}Error: emcmake (Emscripten) not found!${NC}"
        echo -e "${RED}Expected emsdk at: $EMSDK_PATH${NC}"
        echo "Please install Emscripten or update the path in build.sh"
        exit 1
    fi
fi

# Check if raylib web library exists
RAYLIB_LIB="raylib/src/libraylib.web.a"
if [ ! -f "$RAYLIB_LIB" ]; then
    echo -e "${YELLOW}Warning: Raylib web library not found at $RAYLIB_LIB${NC}"
    echo "You may need to build raylib for web first."
    echo "Attempting to build anyway..."
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build . --config Release

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo "Generated files in build/:"
    ls -lh msrlweb.html msrlweb.js msrlweb.wasm 2>/dev/null || echo "  (output files)"
    echo ""
    echo "To run: cd build && python3 -m http.server 8000"
    echo "Then open: http://localhost:8000/msrlweb.html"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
