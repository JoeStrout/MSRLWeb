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
// Hidden (unnamed) intrinsics for Raylib methods
//--------------------------------------------------------------------------------

// Drawing functions
Intrinsic *i_BeginDrawing = nullptr;
Intrinsic *i_EndDrawing = nullptr;
Intrinsic *i_ClearBackground = nullptr;
Intrinsic *i_DrawRectangle = nullptr;

// Timing functions
Intrinsic *i_SetTargetFPS = nullptr;
Intrinsic *i_GetFrameTime = nullptr;
Intrinsic *i_GetTime = nullptr;
Intrinsic *i_GetFPS = nullptr;

// Keyboard input functions
Intrinsic *i_IsKeyPressed = nullptr;
Intrinsic *i_IsKeyPressedRepeat = nullptr;
Intrinsic *i_IsKeyDown = nullptr;
Intrinsic *i_IsKeyReleased = nullptr;
Intrinsic *i_IsKeyUp = nullptr;
Intrinsic *i_GetKeyPressed = nullptr;
Intrinsic *i_GetCharPressed = nullptr;
Intrinsic *i_SetExitKey = nullptr;

// Mouse input functions
Intrinsic *i_IsMouseButtonPressed = nullptr;
Intrinsic *i_IsMouseButtonDown = nullptr;
Intrinsic *i_IsMouseButtonReleased = nullptr;
Intrinsic *i_IsMouseButtonUp = nullptr;
Intrinsic *i_GetMouseX = nullptr;
Intrinsic *i_GetMouseY = nullptr;
Intrinsic *i_GetMousePosition = nullptr;
Intrinsic *i_GetMouseDelta = nullptr;
Intrinsic *i_GetMouseWheelMove = nullptr;
Intrinsic *i_SetMouseCursor = nullptr;
Intrinsic *i_ShowCursor = nullptr;
Intrinsic *i_HideCursor = nullptr;
Intrinsic *i_IsCursorHidden = nullptr;
Intrinsic *i_IsCursorOnScreen = nullptr;

//--------------------------------------------------------------------------------
// Main raylib module
//--------------------------------------------------------------------------------

static IntrinsicResult intrinsic_raylib(Context *context, IntrinsicResult partialResult) {
    static ValueDict raylibModule;

    if (raylibModule.Count() == 0) {
        // Add drawing functions
        raylibModule.SetValue("BeginDrawing", i_BeginDrawing->GetFunc());
        raylibModule.SetValue("EndDrawing", i_EndDrawing->GetFunc());
        raylibModule.SetValue("ClearBackground", i_ClearBackground->GetFunc());
        raylibModule.SetValue("DrawRectangle", i_DrawRectangle->GetFunc());

        // Add timing functions
        raylibModule.SetValue("SetTargetFPS", i_SetTargetFPS->GetFunc());
        raylibModule.SetValue("GetFrameTime", i_GetFrameTime->GetFunc());
        raylibModule.SetValue("GetTime", i_GetTime->GetFunc());
        raylibModule.SetValue("GetFPS", i_GetFPS->GetFunc());

        // Add keyboard input functions
        raylibModule.SetValue("IsKeyPressed", i_IsKeyPressed->GetFunc());
        raylibModule.SetValue("IsKeyPressedRepeat", i_IsKeyPressedRepeat->GetFunc());
        raylibModule.SetValue("IsKeyDown", i_IsKeyDown->GetFunc());
        raylibModule.SetValue("IsKeyReleased", i_IsKeyReleased->GetFunc());
        raylibModule.SetValue("IsKeyUp", i_IsKeyUp->GetFunc());
        raylibModule.SetValue("GetKeyPressed", i_GetKeyPressed->GetFunc());
        raylibModule.SetValue("GetCharPressed", i_GetCharPressed->GetFunc());
        raylibModule.SetValue("SetExitKey", i_SetExitKey->GetFunc());

        // Add mouse input functions
        raylibModule.SetValue("IsMouseButtonPressed", i_IsMouseButtonPressed->GetFunc());
        raylibModule.SetValue("IsMouseButtonDown", i_IsMouseButtonDown->GetFunc());
        raylibModule.SetValue("IsMouseButtonReleased", i_IsMouseButtonReleased->GetFunc());
        raylibModule.SetValue("IsMouseButtonUp", i_IsMouseButtonUp->GetFunc());
        raylibModule.SetValue("GetMouseX", i_GetMouseX->GetFunc());
        raylibModule.SetValue("GetMouseY", i_GetMouseY->GetFunc());
        raylibModule.SetValue("GetMousePosition", i_GetMousePosition->GetFunc());
        raylibModule.SetValue("GetMouseDelta", i_GetMouseDelta->GetFunc());
        raylibModule.SetValue("GetMouseWheelMove", i_GetMouseWheelMove->GetFunc());
        raylibModule.SetValue("SetMouseCursor", i_SetMouseCursor->GetFunc());
        raylibModule.SetValue("ShowCursor", i_ShowCursor->GetFunc());
        raylibModule.SetValue("HideCursor", i_HideCursor->GetFunc());
        raylibModule.SetValue("IsCursorHidden", i_IsCursorHidden->GetFunc());
        raylibModule.SetValue("IsCursorOnScreen", i_IsCursorOnScreen->GetFunc());

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

    return IntrinsicResult(raylibModule);
}

//--------------------------------------------------------------------------------
// Add intrinsics to interpreter
//--------------------------------------------------------------------------------

void AddRaylibIntrinsics(Interpreter* interpreter) {
    Intrinsic *f;

    // Create the unnamed intrinsics for individual methods
    i_BeginDrawing = Intrinsic::Create("");
    i_BeginDrawing->code = INTRINSIC_LAMBDA {
        BeginDrawing();
        return IntrinsicResult::Null;
    };

    i_EndDrawing = Intrinsic::Create("");
    i_EndDrawing->code = INTRINSIC_LAMBDA {
        EndDrawing();
        return IntrinsicResult::Null;
    };

    i_ClearBackground = Intrinsic::Create("");
    i_ClearBackground->AddParam("color");
    i_ClearBackground->code = INTRINSIC_LAMBDA {
        Value colorVal = context->GetVar(String("color"));
        Color color = ValueToColor(colorVal);
        ClearBackground(color);
        return IntrinsicResult::Null;
    };

    i_DrawRectangle = Intrinsic::Create("");
    i_DrawRectangle->AddParam("x");
    i_DrawRectangle->AddParam("y");
    i_DrawRectangle->AddParam("width");
    i_DrawRectangle->AddParam("height");
    i_DrawRectangle->AddParam("color");
    i_DrawRectangle->code = INTRINSIC_LAMBDA {
        int x = context->GetVar(String("x")).IntValue();
        int y = context->GetVar(String("y")).IntValue();
        int width = context->GetVar(String("width")).IntValue();
        int height = context->GetVar(String("height")).IntValue();
        Color color = ValueToColor(context->GetVar(String("color")));
        DrawRectangle(x, y, width, height, color);
        return IntrinsicResult::Null;
    };

    i_SetTargetFPS = Intrinsic::Create("");
    i_SetTargetFPS->AddParam("fps");
    i_SetTargetFPS->code = INTRINSIC_LAMBDA {
        SetTargetFPS(context->GetVar(String("fps")).IntValue());
        return IntrinsicResult::Null;
    };

    i_GetFrameTime = Intrinsic::Create("");
    i_GetFrameTime->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetFrameTime());
    };

    i_GetTime = Intrinsic::Create("");
    i_GetTime->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetTime());
    };

    i_GetFPS = Intrinsic::Create("");
    i_GetFPS->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetFPS());
    };

    i_IsKeyPressed = Intrinsic::Create("");
    i_IsKeyPressed->AddParam("key");
    i_IsKeyPressed->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsKeyPressed(context->GetVar(String("key")).IntValue()));
    };

    i_IsKeyPressedRepeat = Intrinsic::Create("");
    i_IsKeyPressedRepeat->AddParam("key");
    i_IsKeyPressedRepeat->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsKeyPressedRepeat(context->GetVar(String("key")).IntValue()));
    };

    i_IsKeyDown = Intrinsic::Create("");
    i_IsKeyDown->AddParam("key");
    i_IsKeyDown->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsKeyDown(context->GetVar(String("key")).IntValue()));
    };

    i_IsKeyReleased = Intrinsic::Create("");
    i_IsKeyReleased->AddParam("key");
    i_IsKeyReleased->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsKeyReleased(context->GetVar(String("key")).IntValue()));
    };

    i_IsKeyUp = Intrinsic::Create("");
    i_IsKeyUp->AddParam("key");
    i_IsKeyUp->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsKeyUp(context->GetVar(String("key")).IntValue()));
    };

    i_GetKeyPressed = Intrinsic::Create("");
    i_GetKeyPressed->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetKeyPressed());
    };

    i_GetCharPressed = Intrinsic::Create("");
    i_GetCharPressed->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetCharPressed());
    };

    i_SetExitKey = Intrinsic::Create("");
    i_SetExitKey->AddParam("key");
    i_SetExitKey->code = INTRINSIC_LAMBDA {
        SetExitKey(context->GetVar(String("key")).IntValue());
        return IntrinsicResult::Null;
    };

    i_IsMouseButtonPressed = Intrinsic::Create("");
    i_IsMouseButtonPressed->AddParam("button");
    i_IsMouseButtonPressed->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsMouseButtonPressed(context->GetVar(String("button")).IntValue()));
    };

    i_IsMouseButtonDown = Intrinsic::Create("");
    i_IsMouseButtonDown->AddParam("button");
    i_IsMouseButtonDown->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsMouseButtonDown(context->GetVar(String("button")).IntValue()));
    };

    i_IsMouseButtonReleased = Intrinsic::Create("");
    i_IsMouseButtonReleased->AddParam("button");
    i_IsMouseButtonReleased->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsMouseButtonReleased(context->GetVar(String("button")).IntValue()));
    };

    i_IsMouseButtonUp = Intrinsic::Create("");
    i_IsMouseButtonUp->AddParam("button");
    i_IsMouseButtonUp->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsMouseButtonUp(context->GetVar(String("button")).IntValue()));
    };

    i_GetMouseX = Intrinsic::Create("");
    i_GetMouseX->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetMouseX());
    };

    i_GetMouseY = Intrinsic::Create("");
    i_GetMouseY->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetMouseY());
    };

    i_GetMousePosition = Intrinsic::Create("");
    i_GetMousePosition->code = INTRINSIC_LAMBDA {
        Vector2 pos = GetMousePosition();
        ValueDict posMap;
        posMap.SetValue(String("x"), Value(pos.x));
        posMap.SetValue(String("y"), Value(pos.y));
        return IntrinsicResult(posMap);
    };

    i_GetMouseDelta = Intrinsic::Create("");
    i_GetMouseDelta->code = INTRINSIC_LAMBDA {
        Vector2 delta = GetMouseDelta();
        ValueDict deltaMap;
        deltaMap.SetValue(String("x"), Value(delta.x));
        deltaMap.SetValue(String("y"), Value(delta.y));
        return IntrinsicResult(deltaMap);
    };

    i_GetMouseWheelMove = Intrinsic::Create("");
    i_GetMouseWheelMove->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(GetMouseWheelMove());
    };

    i_SetMouseCursor = Intrinsic::Create("");
    i_SetMouseCursor->AddParam("cursor");
    i_SetMouseCursor->code = INTRINSIC_LAMBDA {
        SetMouseCursor(context->GetVar(String("cursor")).IntValue());
        return IntrinsicResult::Null;
    };

    i_ShowCursor = Intrinsic::Create("");
    i_ShowCursor->code = INTRINSIC_LAMBDA {
        ShowCursor();
        return IntrinsicResult::Null;
    };

    i_HideCursor = Intrinsic::Create("");
    i_HideCursor->code = INTRINSIC_LAMBDA {
        HideCursor();
        return IntrinsicResult::Null;
    };

    i_IsCursorHidden = Intrinsic::Create("");
    i_IsCursorHidden->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsCursorHidden());
    };

    i_IsCursorOnScreen = Intrinsic::Create("");
    i_IsCursorOnScreen->code = INTRINSIC_LAMBDA {
        return IntrinsicResult(IsCursorOnScreen());
    };

    // Create and register the main raylib module
    f = Intrinsic::Create("raylib");
    f->code = &intrinsic_raylib;
}
