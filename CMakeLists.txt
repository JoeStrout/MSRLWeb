cmake_minimum_required(VERSION 3.12)
project(MSRLWeb VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if we're building for web
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This project requires Emscripten. Please use emcmake cmake ...")
endif()

# MiniScript source files
set(MINISCRIPT_SOURCES
    MiniScript/Dictionary.cpp
    MiniScript/List.cpp
    MiniScript/MiniscriptInterpreter.cpp
    MiniScript/MiniscriptIntrinsics.cpp
    MiniScript/MiniscriptKeywords.cpp
    MiniScript/MiniscriptLexer.cpp
    MiniScript/MiniscriptParser.cpp
    MiniScript/MiniscriptTAC.cpp
    MiniScript/MiniscriptTypes.cpp
    MiniScript/QA.cpp
    MiniScript/SimpleString.cpp
    MiniScript/SimpleVector.cpp
    MiniScript/SplitJoin.cpp
    MiniScript/UnicodeUtil.cpp
    MiniScript/UnitTest.cpp
)

# Raylib library path (web version)
set(RAYLIB_WEB_LIB ${CMAKE_SOURCE_DIR}/raylib/src/libraylib.web.a)

# Check if raylib web library exists
if(NOT EXISTS ${RAYLIB_WEB_LIB})
    message(FATAL_ERROR "Raylib web library not found at ${RAYLIB_WEB_LIB}. Please build raylib for web first.")
endif()

# Create executable
add_executable(msrlweb
    src/main.cpp
    src/loadfile.cpp
    src/RaylibIntrinsics.cpp
    ${MINISCRIPT_SOURCES}
)

# Include directories
target_include_directories(msrlweb PRIVATE
    ${CMAKE_SOURCE_DIR}/MiniScript
    ${CMAKE_SOURCE_DIR}/raylib/src
)

# Emscripten compile flags (used during compilation)
set(EMSCRIPTEN_COMPILE_FLAGS
    -Os                                    # Optimize for size
    -Wall                                  # All warnings
    -DPLATFORM_WEB                        # Platform definition
    -fexceptions                           # Enable C++ exceptions
)

# Emscripten link flags (used during linking only)
set(EMSCRIPTEN_LINK_FLAGS
    -sUSE_GLFW=3                          # Use GLFW3 for window management
    -sASYNCIFY                            # Allow async operations
    -sFETCH=1                             # Enable fetch API for loading files
    -sALLOW_MEMORY_GROWTH=1               # Allow dynamic memory growth
    -sTOTAL_MEMORY=67108864               # Initial memory (64MB)
    -sSTACK_SIZE=5242880                  # Stack size (5MB)
    -fexceptions                          # Enable C++ exceptions
    -sEXPORTED_FUNCTIONS=['_main','_malloc'] # Enable malloc from JavaScript
    --pre-js ${CMAKE_SOURCE_DIR}/prevent-defaults.js  # Prevent browser defaults for game keys
)

# Apply flags to target
target_compile_options(msrlweb PRIVATE ${EMSCRIPTEN_COMPILE_FLAGS})
target_link_options(msrlweb PRIVATE ${EMSCRIPTEN_LINK_FLAGS})

# Link against raylib
target_link_libraries(msrlweb ${RAYLIB_WEB_LIB})

# Set output suffix to .html (will also generate .js and .wasm)
set_target_properties(msrlweb PROPERTIES SUFFIX ".html")

# Custom shell file (if it exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/shell.html)
    target_link_options(msrlweb PRIVATE --shell-file ${CMAKE_SOURCE_DIR}/shell.html)
endif()

# Copy assets to build directory
add_custom_command(TARGET msrlweb POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)

# Copy index.html to build directory
add_custom_command(TARGET msrlweb POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/index.html
    ${CMAKE_BINARY_DIR}/index.html
    COMMENT "Copying index.html to build directory"
)

# Install target (optional)
install(TARGETS msrlweb DESTINATION ${CMAKE_SOURCE_DIR}/build)

# Print configuration summary
message(STATUS "MSRLWeb Configuration:")
message(STATUS "  Emscripten: ${EMSCRIPTEN}")
message(STATUS "  Raylib library: ${RAYLIB_WEB_LIB}")
message(STATUS "  Output: msrlweb.html, msrlweb.js, msrlweb.wasm")
