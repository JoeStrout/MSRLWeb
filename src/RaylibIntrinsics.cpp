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

Intrinsic *i_BeginDrawing = nullptr;
Intrinsic *i_EndDrawing = nullptr;
Intrinsic *i_ClearBackground = nullptr;
Intrinsic *i_DrawRectangle = nullptr;

//--------------------------------------------------------------------------------
// Intrinsic implementations
//--------------------------------------------------------------------------------

static IntrinsicResult intrinsic_BeginDrawing(Context *context, IntrinsicResult partialResult) {
    BeginDrawing();
    return IntrinsicResult::Null;
}

static IntrinsicResult intrinsic_EndDrawing(Context *context, IntrinsicResult partialResult) {
    EndDrawing();
    return IntrinsicResult::Null;
}

static IntrinsicResult intrinsic_ClearBackground(Context *context, IntrinsicResult partialResult) {
    Value colorVal = context->GetVar(String("color"));
    Color color = ValueToColor(colorVal);
    ClearBackground(color);
    return IntrinsicResult::Null;
}

static IntrinsicResult intrinsic_DrawRectangle(Context *context, IntrinsicResult partialResult) {
    Value xVal = context->GetVar(String("x"));
    Value yVal = context->GetVar(String("y"));
    Value widthVal = context->GetVar(String("width"));
    Value heightVal = context->GetVar(String("height"));
    Value colorVal = context->GetVar(String("color"));

    int x = xVal.IntValue();
    int y = yVal.IntValue();
    int width = widthVal.IntValue();
    int height = heightVal.IntValue();
    Color color = ValueToColor(colorVal);

    DrawRectangle(x, y, width, height, color);
    return IntrinsicResult::Null;
}

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
    i_BeginDrawing->code = &intrinsic_BeginDrawing;

    i_EndDrawing = Intrinsic::Create("");
    i_EndDrawing->code = &intrinsic_EndDrawing;

    i_ClearBackground = Intrinsic::Create("");
    i_ClearBackground->AddParam("color");
    i_ClearBackground->code = &intrinsic_ClearBackground;

    i_DrawRectangle = Intrinsic::Create("");
    i_DrawRectangle->AddParam("x");
    i_DrawRectangle->AddParam("y");
    i_DrawRectangle->AddParam("width");
    i_DrawRectangle->AddParam("height");
    i_DrawRectangle->AddParam("color");
    i_DrawRectangle->code = &intrinsic_DrawRectangle;

    // Create and register the main raylib module
    f = Intrinsic::Create("raylib");
    f->code = &intrinsic_raylib;
}
