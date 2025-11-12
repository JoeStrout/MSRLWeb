// MSRLWeb - MiniScript + Raylib Web Demo
// A MiniScript-driven application with Raylib graphics

#include "raylib.h"
#include "SimpleString.h"
#include "MiniscriptInterpreter.h"
#include "MiniscriptIntrinsics.h"
#include "MiniscriptParser.h"
#include "RaylibIntrinsics.h"
#include "loadfile.h"
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#include <stdio.h>

using namespace MiniScript;

//--------------------------------------------------------------------------------
// Global state
//--------------------------------------------------------------------------------

enum ScriptState {
	LOADING,
	RUNNING,
	ERRORED,
	COMPLETE
};

static Interpreter* interpreter = nullptr;
static ScriptState scriptState = LOADING;
static String scriptSource;
static String loadError;
static String runtimeError;
static ValueList stackTrace;

//--------------------------------------------------------------------------------
// Output callbacks for MiniScript
//--------------------------------------------------------------------------------

static void Print(String s, bool lineBreak = true) {
	printf("%s%s", s.c_str(), lineBreak ? "\n" : "");
}

static void PrintErr(String s, bool lineBreak = true) {
	//fprintf(stderr, "%s%s", s.c_str(), lineBreak ? "\n" : "");
	runtimeError = s;
	scriptState = ERRORED;
	stackTrace = Intrinsics::StackList(interpreter->vm);
	printf("%s%s", s.c_str(), lineBreak ? "\n" : "");
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
			printf("Successfully loaded script from %s\n", fetch->url);
			// State remains LOADING until RunScript is called
		} else {
			loadError = "Memory allocation failed";
			scriptState = ERRORED;
			printf("Failed to allocate memory for script\n");
		}
	} else {
		loadError = String("HTTP error: ") + String::Format(fetch->status);
		scriptState = ERRORED;
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
// Import intrinsic
//--------------------------------------------------------------------------------

#include <map>

// Track import fetches
struct ImportFetchData {
	emscripten_fetch_t* fetch;
	bool completed;
	int status;
	String libname;
	int searchPathIndex;  // Which search path we're trying (0 = assets/, 1 = assets/lib/)
	ImportFetchData() : fetch(nullptr), completed(false), status(0), searchPathIndex(0) {}
};

static std::map<long, ImportFetchData> activeImportFetches;
static long nextImportFetchId = 1;

// Callback when import fetch completes
static void import_fetch_completed(emscripten_fetch_t *fetch) {
	for (auto& pair : activeImportFetches) {
		if (pair.second.fetch == fetch) {
			pair.second.completed = true;
			pair.second.status = fetch->status;
			printf("import_fetch_completed: Fetch ID %ld completed with status %d\n", pair.first, fetch->status);
			break;
		}
	}
}

static IntrinsicResult intrinsic_import(Context *context, IntrinsicResult partialResult) {
	// State 3: Import function has finished, store result in parent context
	if (!partialResult.Done() && partialResult.Result().type == ValueType::String) {
		// The import function has finished, and stored its result in Temp 0.
		Value importedValues = context->GetTemp(0);
		// Store these imported values in the parent context under the library name
		String libname = partialResult.Result().ToString();
		Context *callerContext = context->parent;
		if (callerContext) {
			callerContext->SetVar(libname, importedValues);
		}
		return IntrinsicResult::Null;
	}

	// State 2: File has been fetched, parse and create import
	if (!partialResult.Done() && partialResult.Result().type == ValueType::Number) {
		long fetchId = (long)partialResult.Result().DoubleValue();
		auto it = activeImportFetches.find(fetchId);
		if (it == activeImportFetches.end()) {
			RuntimeException("import: internal error (fetch not found)").raise();
		}

		ImportFetchData& data = it->second;

		if (!data.completed) {
			// Still loading
			return partialResult;
		}

		// Fetch is complete
		emscripten_fetch_t* fetch = data.fetch;
		String libname = data.libname;

		if (data.status == 200) {
			// Success - parse the module source
			char* moduleData = (char*)malloc(fetch->numBytes + 1);
			if (!moduleData) {
				emscripten_fetch_close(fetch);
				activeImportFetches.erase(it);
				RuntimeException("import: memory allocation failed").raise();
			}
			memcpy(moduleData, fetch->data, fetch->numBytes);
			moduleData[fetch->numBytes] = '\0';
			String moduleSource(moduleData);
			free(moduleData);

			emscripten_fetch_close(fetch);
			activeImportFetches.erase(it);

			// Parse the code and build a function around it
			Parser parser;
			parser.errorContext = libname + ".ms";
			parser.Parse(moduleSource);
			FunctionStorage *import = parser.CreateImport();
			context->vm->ManuallyPushCall(import, Value::Temp(0));

			// Return partial result with the lib name (string type)
			// We'll get invoked again after the import function finishes
			return IntrinsicResult(libname, false);
		} else {
			// Error loading file - try next search path if available
			emscripten_fetch_close(fetch);
			int nextPathIndex = data.searchPathIndex + 1;
			activeImportFetches.erase(it);

			const char* searchPaths[] = { "assets/", "assets/lib/" };
			if (nextPathIndex < 2) {
				// Try the next path
				String path = String(searchPaths[nextPathIndex]) + libname + ".ms";

				long newFetchId = nextImportFetchId++;
				ImportFetchData& newData = activeImportFetches[newFetchId];
				newData.libname = libname;
				newData.searchPathIndex = nextPathIndex;

				emscripten_fetch_attr_t attr;
				emscripten_fetch_attr_init(&attr);
				strcpy(attr.requestMethod, "GET");
				attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
				attr.onsuccess = import_fetch_completed;
				attr.onerror = import_fetch_completed;

				newData.fetch = emscripten_fetch(&attr, path.c_str());

				return IntrinsicResult(Value((double)newFetchId), false);
			} else {
				// All paths exhausted
				RuntimeException("import: library not found: " + libname).raise();
			}
		}
	}

	// State 1: Start the import - fetch the file
	String libname = context->GetVar("libname").ToString();
	if (libname.empty()) {
		RuntimeException("import: libname required").raise();
	}
	if (libname.IndexOfB('/') >= 0) {
		RuntimeException("import: argument must be library name, not path").raise();
	}

	// Try to find the file - start with assets/
	String path = String("assets/") + libname + ".ms";

	// Start async fetch
	long fetchId = nextImportFetchId++;
	ImportFetchData& data = activeImportFetches[fetchId];
	data.libname = libname;
	data.searchPathIndex = 0;

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = import_fetch_completed;
	attr.onerror = import_fetch_completed;

	data.fetch = emscripten_fetch(&attr, path.c_str());

	// Return the fetch ID as partial result (number type)
	return IntrinsicResult(Value((double)fetchId), false);
}

//--------------------------------------------------------------------------------
// Initialize MiniScript
//--------------------------------------------------------------------------------

void InitMiniScript() {
	MiniScript::hostVersion = 0.2;
	MiniScript::hostName = "MSRLWeb";
	MiniScript::hostInfo = "https://github.com/JoeStrout/MSRLWeb";

	interpreter = new Interpreter();
	interpreter->standardOutput = &Print;
	interpreter->errorOutput = &PrintErr;
	interpreter->implicitOutput = &Print;

	// Add Raylib intrinsics
	AddRaylibIntrinsics();

	// Add import intrinsic
	Intrinsic *importFunc = Intrinsic::Create("import");
	importFunc->AddParam("libname", "");
	importFunc->code = &intrinsic_import;

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
	scriptState = RUNNING;

	// Don't run the script here - let the main loop handle incremental execution
}

//--------------------------------------------------------------------------------
// Main loop
//--------------------------------------------------------------------------------

void MainLoop() {
	// Start the script when it's loaded but not yet started
	if (scriptState == LOADING && !scriptSource.empty()) {
		RunScript();
	}

	if (scriptState == RUNNING) {
		// Script is running - hand control to MiniScript
		// MiniScript will handle BeginDrawing/EndDrawing and everything else
		if (!interpreter->Done()) {
			try {
				interpreter->RunUntilDone(0.1, false);  // Run until yield or timeout
			} catch (MiniscriptException& mse) {
				PrintErr("Runtime Exception: " + mse.message);
				interpreter->vm->Stop();
				scriptState = ERRORED;
			}
		} else {
			scriptState = COMPLETE;
			printf("Script finished\n");
		}
	} else {
		// Show loading, error, or completion screen
		BeginDrawing();
		ClearBackground(RAYWHITE);

		if (scriptState == LOADING) {
			// Loading screen
			DrawText("MSRLWeb - MiniScript + Raylib", 10, 10, 30, DARKBLUE);
			DrawText("Loading assets/main.ms...", 10, 50, 20, GRAY);

			// Simple loading animation
			int dots = ((int)(GetTime() * 2)) % 4;
			const char* dotStr[] = {"", ".", "..", "..."};
			DrawText(dotStr[dots], 250, 50, 20, GRAY);
		} else if (scriptState == ERRORED) {
			// Error screen
			DrawText("MSRLWeb - MiniScript + Raylib", 10, 10, 30, DARKBLUE);
			if (!loadError.empty()) {
				DrawText("Error loading script:", 10, 50, 20, RED);
				DrawText(loadError.c_str(), 10, 80, 16, RED);
				DrawText("Make sure assets/main.ms exists", 10, 110, 10, GRAY);
			} else if (!runtimeError.empty()) {
				DrawText("The game has halted due to an error:", 10, 50, 20, RED);
				DrawText(runtimeError.c_str(), 10, 80, 20, RED);
				int y = 110;
				for (int i = 0; i < stackTrace.Count(); i++) {
					String entry = stackTrace[i].ToString();
					DrawText(entry.c_str(), 30, y, 20, GRAY);
					y += 20;
				}
			}
		} else if (scriptState == COMPLETE) {
			// Script finished
			DrawText("Script Completed", 10, 10, 20, DARKGREEN);
			DrawText("Check console for output", 10, 50, 10, GRAY);
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
	InitAudioDevice();
	InstallLoadFileHooks();

	// Set default font to use point filtering for pixel-perfect rendering
// 	Font defaultFont = GetFontDefault();
// 	SetTextureFilter(defaultFont.texture, TEXTURE_FILTER_POINT);
// 	printf("Default font base size: %d\n", defaultFont.baseSize);
// 	printf("For pixel-perfect text, use font sizes that are multiples of %d\n", defaultFont.baseSize);

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
