// MSRLWeb - MiniScript + Raylib Web Demo
// A MiniScript-driven application with Raylib graphics

#include "raylib.h"
#include "SimpleString.h"
#include "MiniscriptInterpreter.h"
#include "MiniscriptIntrinsics.h"
#include "RaylibIntrinsics.h"
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <stdio.h>
#include <string>

using namespace MiniScript;

//--------------------------------------------------------------------------------
// Global state
//--------------------------------------------------------------------------------

static Interpreter* interpreter = nullptr;
static std::string outputBuffer;
static bool scriptLoaded = false;
static bool scriptRunning = false;
static bool scriptStarted = false;
static String scriptSource;
static String loadError;

//--------------------------------------------------------------------------------
// Output callbacks for MiniScript
//--------------------------------------------------------------------------------

static void Print(String s, bool lineBreak = true) {
	outputBuffer += s.c_str();
	if (lineBreak) outputBuffer += "\n";
	printf("%s%s", s.c_str(), lineBreak ? "\n" : "");
}

static void PrintErr(String s, bool lineBreak = true) {
	outputBuffer += "ERROR: ";
	outputBuffer += s.c_str();
	if (lineBreak) outputBuffer += "\n";
	fprintf(stderr, "%s%s", s.c_str(), lineBreak ? "\n" : "");
}

//--------------------------------------------------------------------------------
// Script loading via Emscripten fetch
//--------------------------------------------------------------------------------

void onScriptFetched(emscripten_fetch_t *fetch) {
	if (fetch->status == 200) {
		printf("Downloaded %llu bytes from URL %s\n", fetch->numBytes, fetch->url);

		// Copy the script source (null-terminate it)
		char* scriptData = (char*)malloc(fetch->numBytes + 1);
		if (scriptData) {
			memcpy(scriptData, fetch->data, fetch->numBytes);
			scriptData[fetch->numBytes] = '\0';
			scriptSource = String(scriptData);
			free(scriptData);
			scriptLoaded = true;
			printf("Successfully loaded script from %s\n", fetch->url);
		} else {
			loadError = "Memory allocation failed";
			printf("Failed to allocate memory for script\n");
		}
	} else {
		loadError = String("HTTP error: ") + String::Format(fetch->status);
		printf("Failed to download %s: HTTP %d\n", fetch->url, fetch->status);
	}

	emscripten_fetch_close(fetch);
}

void fetchScript(const char *url) {
	printf("Fetching script from %s...\n", url);

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = onScriptFetched;
	attr.onerror = onScriptFetched;  // Same handler checks status

	emscripten_fetch(&attr, url);
}

//--------------------------------------------------------------------------------
// Initialize MiniScript
//--------------------------------------------------------------------------------

void InitMiniScript() {
	interpreter = new Interpreter();
	interpreter->standardOutput = &Print;
	interpreter->errorOutput = &PrintErr;
	interpreter->implicitOutput = &Print;

	// Add Raylib intrinsics
	AddRaylibIntrinsics(interpreter);

	printf("MiniScript interpreter initialized with Raylib intrinsics\n");
}

//--------------------------------------------------------------------------------
// Run the loaded script (like DoCommand)
//--------------------------------------------------------------------------------

void RunScript() {
	if (scriptSource.empty()) {
		PrintErr("No script to run");
		return;
	}

	printf("Compiling script...\n");
	interpreter->Reset(scriptSource);
	interpreter->Compile();

	printf("Starting script execution...\n");
	scriptRunning = true;

	// Don't run the script here - let the main loop handle incremental execution
}

//--------------------------------------------------------------------------------
// Main loop
//--------------------------------------------------------------------------------

void MainLoop() {
	// Start the script when it's loaded but not yet started
	if (scriptLoaded && !scriptStarted) {
		RunScript();
		scriptStarted = true;
	}

	if (scriptRunning) {
		// Script is running - hand control to MiniScript
		// MiniScript will handle BeginDrawing/EndDrawing and everything else
		if (!interpreter->Done()) {
			try {
				interpreter->RunUntilDone(0.1, false);  // Run until yield or timeout
			} catch (MiniscriptException& mse) {
				PrintErr("Runtime Exception: " + mse.message);
				interpreter->vm->Stop();
				scriptRunning = false;
			}

			if (interpreter->Done()) {
				scriptRunning = false;
				printf("Script finished\n");
			}
		}
	} else {
		// Show loading or error screen (MiniScript not running yet)
		BeginDrawing();
		ClearBackground(RAYWHITE);

		if (!scriptLoaded) {
			// Loading screen
			DrawText("MSRLWeb - MiniScript + Raylib", 10, 10, 24, DARKBLUE);
			if (loadError.empty()) {
				DrawText("Loading assets/main.ms...", 10, 50, 20, GRAY);

				// Simple loading animation
				int dots = ((int)(GetTime() * 2)) % 4;
				const char* dotStr[] = {"", ".", "..", "..."};
				DrawText(dotStr[dots], 250, 50, 20, GRAY);
			} else {
				DrawText("Error loading script:", 10, 50, 20, RED);
				DrawText(loadError.c_str(), 10, 80, 16, RED);
				DrawText("Make sure assets/main.ms exists", 10, 110, 14, GRAY);
			}
		} else {
			// Script finished
			DrawText("Script Completed", 10, 10, 24, DARKGREEN);
			DrawText("Check console for output", 10, 50, 16, GRAY);
		}

		EndDrawing();
	}
}

//--------------------------------------------------------------------------------
// Cleanup
//--------------------------------------------------------------------------------

void CleanupMiniScript() {
	if (interpreter) {
		delete interpreter;
		interpreter = nullptr;
	}
}

//--------------------------------------------------------------------------------
// Main
//--------------------------------------------------------------------------------

int main() {
	// Initialize Raylib
	const int screenWidth = 960;
	const int screenHeight = 640;

	InitWindow(screenWidth, screenHeight, "MSRLWeb - MiniScript + Raylib");
	SetTargetFPS(60);

	// Initialize MiniScript
	InitMiniScript();

	// Start fetching the main script
	fetchScript("assets/main.ms");

	// Main loop
	#ifdef PLATFORM_WEB
		emscripten_set_main_loop(MainLoop, 0, 1);
	#else
		while (!WindowShouldClose()) {
			MainLoop();
		}
	#endif

	// Cleanup
	CleanupMiniScript();
	CloseWindow();

	return 0;
}
