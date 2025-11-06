//
//  RaylibIntrinsics.cpp
//  MSRLWeb
//
//  Raylib intrinsics for MiniScript
//

#include "RaylibIntrinsics.h"
#include "raylib.h"
#include "MiniscriptInterpreter.h"
#include "MiniscriptTypes.h"
#include <math.h>

using namespace MiniScript;

// Macro to reduce boilerplate for lambda intrinsics
#define INTRINSIC_LAMBDA [](Context *context, IntrinsicResult partialResult) -> IntrinsicResult

//--------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------

// Convert a MiniScript map to a Raylib Color
// Expects a map with "r", "g", "b", and optionally "a" keys (0-255)
static Color ValueToColor(Value value) {
	if (value.type != ValueType::Map) {
		// Default to white if not a map
		return WHITE;
	}

	ValueDict map = value.GetDict();
	Color result;

	Value rVal = map.Lookup(String("r"), Value::zero);
	Value gVal = map.Lookup(String("g"), Value::zero);
	Value bVal = map.Lookup(String("b"), Value::zero);
	Value aVal = map.Lookup(String("a"), Value::null);

	result.r = (unsigned char)(rVal.IntValue());
	result.g = (unsigned char)(gVal.IntValue());
	result.b = (unsigned char)(bVal.IntValue());
	result.a = aVal.IsNull() ? 255 : (unsigned char)(aVal.IntValue());

	return result;
}

// Convert a Raylib Color to a MiniScript map
static Value ColorToValue(Color color) {
	ValueDict map;
	map.SetValue(String("r"), Value((int)color.r));
	map.SetValue(String("g"), Value((int)color.g));
	map.SetValue(String("b"), Value((int)color.b));
	map.SetValue(String("a"), Value((int)color.a));
	return Value(map);
}



//--------------------------------------------------------------------------------
// rcore methods
//--------------------------------------------------------------------------------

static void AddRCoreMethods(ValueDict raylibModule) {
	Intrinsic *i;

	// Drawing-related functions

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		BeginDrawing();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("BeginDrawing", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		EndDrawing();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("EndDrawing", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("color");
	i->code = INTRINSIC_LAMBDA {
		Value colorVal = context->GetVar(String("color"));
		Color color = ValueToColor(colorVal);
		ClearBackground(color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ClearBackground", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("x");
	i->AddParam("y");
	i->AddParam("width");
	i->AddParam("height");
	i->AddParam("color");
	i->code = INTRINSIC_LAMBDA {
		int x = context->GetVar(String("x")).IntValue();
		int y = context->GetVar(String("y")).IntValue();
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangle(x, y, width, height, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangle", i->GetFunc());

	// Timing functions

	i = Intrinsic::Create("");
	i->AddParam("fps");
	i->code = INTRINSIC_LAMBDA {
		SetTargetFPS(context->GetVar(String("fps")).IntValue());
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetTargetFPS", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetFrameTime());
	};
	raylibModule.SetValue("GetFrameTime", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetTime());
	};
	raylibModule.SetValue("GetTime", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetFPS());
	};
	raylibModule.SetValue("GetFPS", i->GetFunc());

	// Input-related functions: keyboard

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsKeyPressed(context->GetVar(String("key")).IntValue()));
	};
	raylibModule.SetValue("IsKeyPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsKeyPressedRepeat(context->GetVar(String("key")).IntValue()));
	};
	raylibModule.SetValue("IsKeyPressedRepeat", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsKeyDown(context->GetVar(String("key")).IntValue()));
	};
	raylibModule.SetValue("IsKeyDown", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsKeyReleased(context->GetVar(String("key")).IntValue()));
	};
	raylibModule.SetValue("IsKeyReleased", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsKeyUp(context->GetVar(String("key")).IntValue()));
	};
	raylibModule.SetValue("IsKeyUp", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetKeyPressed());
	};
	raylibModule.SetValue("GetKeyPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetCharPressed());
	};
	raylibModule.SetValue("GetCharPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("key");
	i->code = INTRINSIC_LAMBDA {
		SetExitKey(context->GetVar(String("key")).IntValue());
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetExitKey", i->GetFunc());

	// Input-related functions: mouse

	i = Intrinsic::Create("");
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsMouseButtonPressed(context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsMouseButtonPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsMouseButtonDown(context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsMouseButtonDown", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsMouseButtonReleased(context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsMouseButtonReleased", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsMouseButtonUp(context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsMouseButtonUp", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetMouseX());
	};
	raylibModule.SetValue("GetMouseX", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetMouseY());
	};
	raylibModule.SetValue("GetMouseY", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		Vector2 pos = GetMousePosition();
		ValueDict posMap;
		posMap.SetValue(String("x"), Value(pos.x));
		posMap.SetValue(String("y"), Value(pos.y));
		return IntrinsicResult(posMap);
	};
	raylibModule.SetValue("GetMousePosition", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		Vector2 delta = GetMouseDelta();
		ValueDict deltaMap;
		deltaMap.SetValue(String("x"), Value(delta.x));
		deltaMap.SetValue(String("y"), Value(delta.y));
		return IntrinsicResult(deltaMap);
	};
	raylibModule.SetValue("GetMouseDelta", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetMouseWheelMove());
	};
	raylibModule.SetValue("GetMouseWheelMove", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("cursor");
	i->code = INTRINSIC_LAMBDA {
		SetMouseCursor(context->GetVar(String("cursor")).IntValue());
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetMouseCursor", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		ShowCursor();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ShowCursor", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		HideCursor();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("HideCursor", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsCursorHidden());
	};
	raylibModule.SetValue("IsCursorHidden", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsCursorOnScreen());
	};
	raylibModule.SetValue("IsCursorOnScreen", i->GetFunc());
}

static void AddConstants(ValueDict raylibModule) {
	// Add color constants
	raylibModule.SetValue("WHITE", ColorToValue(WHITE));
	raylibModule.SetValue("BLACK", ColorToValue(BLACK));
	raylibModule.SetValue("RED", ColorToValue(RED));
	raylibModule.SetValue("GREEN", ColorToValue(GREEN));
	raylibModule.SetValue("BLUE", ColorToValue(BLUE));
	raylibModule.SetValue("YELLOW", ColorToValue(YELLOW));
	raylibModule.SetValue("ORANGE", ColorToValue(ORANGE));
	raylibModule.SetValue("PINK", ColorToValue(PINK));
	raylibModule.SetValue("MAGENTA", ColorToValue(MAGENTA));
	raylibModule.SetValue("RAYWHITE", ColorToValue(RAYWHITE));
	raylibModule.SetValue("GRAY", ColorToValue(GRAY));
	raylibModule.SetValue("DARKGRAY", ColorToValue(DARKGRAY));
	raylibModule.SetValue("LIGHTGRAY", ColorToValue(LIGHTGRAY));
	raylibModule.SetValue("SKYBLUE", ColorToValue(SKYBLUE));
	raylibModule.SetValue("DARKBLUE", ColorToValue(DARKBLUE));

	// Add keyboard key constants (commonly used keys)
	raylibModule.SetValue("KEY_SPACE", Value(KEY_SPACE));
	raylibModule.SetValue("KEY_ESCAPE", Value(KEY_ESCAPE));
	raylibModule.SetValue("KEY_ENTER", Value(KEY_ENTER));
	raylibModule.SetValue("KEY_TAB", Value(KEY_TAB));
	raylibModule.SetValue("KEY_BACKSPACE", Value(KEY_BACKSPACE));
	raylibModule.SetValue("KEY_RIGHT", Value(KEY_RIGHT));
	raylibModule.SetValue("KEY_LEFT", Value(KEY_LEFT));
	raylibModule.SetValue("KEY_DOWN", Value(KEY_DOWN));
	raylibModule.SetValue("KEY_UP", Value(KEY_UP));
	raylibModule.SetValue("KEY_A", Value(KEY_A));
	raylibModule.SetValue("KEY_B", Value(KEY_B));
	raylibModule.SetValue("KEY_C", Value(KEY_C));
	raylibModule.SetValue("KEY_D", Value(KEY_D));
	raylibModule.SetValue("KEY_E", Value(KEY_E));
	raylibModule.SetValue("KEY_F", Value(KEY_F));
	raylibModule.SetValue("KEY_G", Value(KEY_G));
	raylibModule.SetValue("KEY_H", Value(KEY_H));
	raylibModule.SetValue("KEY_I", Value(KEY_I));
	raylibModule.SetValue("KEY_J", Value(KEY_J));
	raylibModule.SetValue("KEY_K", Value(KEY_K));
	raylibModule.SetValue("KEY_L", Value(KEY_L));
	raylibModule.SetValue("KEY_M", Value(KEY_M));
	raylibModule.SetValue("KEY_N", Value(KEY_N));
	raylibModule.SetValue("KEY_O", Value(KEY_O));
	raylibModule.SetValue("KEY_P", Value(KEY_P));
	raylibModule.SetValue("KEY_Q", Value(KEY_Q));
	raylibModule.SetValue("KEY_R", Value(KEY_R));
	raylibModule.SetValue("KEY_S", Value(KEY_S));
	raylibModule.SetValue("KEY_T", Value(KEY_T));
	raylibModule.SetValue("KEY_U", Value(KEY_U));
	raylibModule.SetValue("KEY_V", Value(KEY_V));
	raylibModule.SetValue("KEY_W", Value(KEY_W));
	raylibModule.SetValue("KEY_X", Value(KEY_X));
	raylibModule.SetValue("KEY_Y", Value(KEY_Y));
	raylibModule.SetValue("KEY_Z", Value(KEY_Z));

	// Add mouse button constants
	raylibModule.SetValue("MOUSE_BUTTON_LEFT", Value(MOUSE_BUTTON_LEFT));
	raylibModule.SetValue("MOUSE_BUTTON_RIGHT", Value(MOUSE_BUTTON_RIGHT));
	raylibModule.SetValue("MOUSE_BUTTON_MIDDLE", Value(MOUSE_BUTTON_MIDDLE));

	// Add mouse cursor constants
	raylibModule.SetValue("MOUSE_CURSOR_DEFAULT", Value(MOUSE_CURSOR_DEFAULT));
	raylibModule.SetValue("MOUSE_CURSOR_ARROW", Value(MOUSE_CURSOR_ARROW));
	raylibModule.SetValue("MOUSE_CURSOR_IBEAM", Value(MOUSE_CURSOR_IBEAM));
	raylibModule.SetValue("MOUSE_CURSOR_CROSSHAIR", Value(MOUSE_CURSOR_CROSSHAIR));
	raylibModule.SetValue("MOUSE_CURSOR_POINTING_HAND", Value(MOUSE_CURSOR_POINTING_HAND));
	raylibModule.SetValue("MOUSE_CURSOR_RESIZE_EW", Value(MOUSE_CURSOR_RESIZE_EW));
	raylibModule.SetValue("MOUSE_CURSOR_RESIZE_NS", Value(MOUSE_CURSOR_RESIZE_NS));
	raylibModule.SetValue("MOUSE_CURSOR_RESIZE_NWSE", Value(MOUSE_CURSOR_RESIZE_NWSE));
	raylibModule.SetValue("MOUSE_CURSOR_RESIZE_NESW", Value(MOUSE_CURSOR_RESIZE_NESW));
	raylibModule.SetValue("MOUSE_CURSOR_RESIZE_ALL", Value(MOUSE_CURSOR_RESIZE_ALL));
	raylibModule.SetValue("MOUSE_CURSOR_NOT_ALLOWED", Value(MOUSE_CURSOR_NOT_ALLOWED));
}

//--------------------------------------------------------------------------------
// Add intrinsics to interpreter
//--------------------------------------------------------------------------------

void AddRaylibIntrinsics(Interpreter* interpreter) {
	Intrinsic *f;

	// Create and register the main raylib module
	f = Intrinsic::Create("raylib");
	f->code = INTRINSIC_LAMBDA {
		static ValueDict raylibModule;

		if (raylibModule.Count() == 0) {
			AddRCoreMethods(raylibModule);
			AddConstants(raylibModule);
		}

		return IntrinsicResult(raylibModule);
	};
}
