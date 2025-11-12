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
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <math.h>
#include <string.h>
#include <map>

using namespace MiniScript;

// Macro to reduce boilerplate for lambda intrinsics
#define INTRINSIC_LAMBDA [](Context *context, IntrinsicResult partialResult) -> IntrinsicResult

//--------------------------------------------------------------------------------
// Classes (maps) representing Raylib structs
//--------------------------------------------------------------------------------

static ValueDict ImageClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("width"), Value::zero);
		map.SetValue(String("height"), Value::zero);
		map.SetValue(String("mipmaps"), Value::zero);
		map.SetValue(String("format"), Value::zero);
	}
	return map;
}

static ValueDict TextureClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("id"), Value::zero);
		map.SetValue(String("width"), Value::zero);
		map.SetValue(String("height"), Value::zero);
		map.SetValue(String("mipmaps"), Value::zero);
		map.SetValue(String("format"), Value::zero);
	}
	return map;
}

static ValueDict FontClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("texture"), Value::null);
		map.SetValue(String("baseSize"), Value::zero);
		map.SetValue(String("glyphCount"), Value::zero);
		map.SetValue(String("glyphPadding"), Value::zero);
	}
	return map;
}

static ValueDict WaveClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("frameCount"), Value::zero);
		map.SetValue(String("sampleRate"), Value::zero);
		map.SetValue(String("sampleSize"), Value::zero);
		map.SetValue(String("channels"), Value::zero);
	}
	return map;
}

static ValueDict MusicClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("frameCount"), Value::zero);
		map.SetValue(String("looping"), Value::zero);
	}
	return map;
}

static ValueDict SoundClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("frameCount"), Value::zero);
	}
	return map;
}

static ValueDict AudioStreamClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("sampleRate"), Value::zero);
		map.SetValue(String("sampleSize"), Value::zero);
		map.SetValue(String("channels"), Value::zero);
	}
	return map;
}

static ValueDict RenderTextureClass() {
	static ValueDict map;
	if (map.Count() == 0) {
		map.SetValue(String("_handle"), Value::zero);
		map.SetValue(String("id"), Value::zero);
		map.SetValue(String("texture"), Value::zero);
	}
	return map;
}

//--------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------

// Convert a Raylib Texture to a MiniScript map
// Allocates the Texture on the heap and stores pointer in _handle
static Value TextureToValue(Texture texture) {
	Texture* texPtr = new Texture(texture);
	ValueDict map;
	map.SetValue(Value::magicIsA, TextureClass());
	map.SetValue(String("_handle"), Value((long)texPtr));
	map.SetValue(String("id"), Value((int)texture.id));
	map.SetValue(String("width"), Value(texture.width));
	map.SetValue(String("height"), Value(texture.height));
	map.SetValue(String("mipmaps"), Value(texture.mipmaps));
	map.SetValue(String("format"), Value(texture.format));
	return Value(map);
}

// Extract a Raylib Texture from a MiniScript map
// Returns the Texture by dereferencing the _handle pointer
static Texture ValueToTexture(Value value) {
	if (value.type != ValueType::Map) {
		// Return empty texture if not a map
		return Texture{0, 0, 0, 0, 0};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	Texture* texPtr = (Texture*)(long)handleVal.IntValue();
	if (texPtr == nullptr) {
		return Texture{0, 0, 0, 0, 0};
	}
	return *texPtr;
}

// Convert a Raylib Image to a MiniScript map
// Allocates the Image on the heap and stores pointer in _handle
static Value ImageToValue(Image image) {
	Image* imgPtr = new Image(image);
	ValueDict map;
	map.SetValue(Value::magicIsA, ImageClass());
	map.SetValue(String("_handle"), Value((long)imgPtr));
	map.SetValue(String("width"), Value(image.width));
	map.SetValue(String("height"), Value(image.height));
	map.SetValue(String("mipmaps"), Value(image.mipmaps));
	map.SetValue(String("format"), Value(image.format));
	return Value(map);
}

// Extract a Raylib Image from a MiniScript map
// Returns the Image by dereferencing the _handle pointer
static Image ValueToImage(Value value) {
	if (value.type != ValueType::Map) {
		// Return empty image if not a map
		return Image{nullptr, 0, 0, 0, 0};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	Image* imgPtr = (Image*)(long)handleVal.IntValue();
	if (imgPtr == nullptr) {
		return Image{nullptr, 0, 0, 0, 0};
	}
	return *imgPtr;
}

// Convert a Raylib Font to a MiniScript map
static Value FontToValue(Font font) {
	Font* fontPtr = new Font(font);
	ValueDict map;
	map.SetValue(Value::magicIsA, FontClass());
	map.SetValue(String("_handle"), Value((long)fontPtr));
	map.SetValue(String("texture"), TextureToValue(font.texture));
	map.SetValue(String("baseSize"), Value(font.baseSize));
	map.SetValue(String("glyphCount"), Value(font.glyphCount));
	map.SetValue(String("glyphPadding"), Value(font.glyphPadding));
	return Value(map);
}

// Extract a Raylib Font from a MiniScript map
static Font ValueToFont(Value value) {
	if (value.type != ValueType::Map) {
		// Return default font if not a map
		printf("ValueToFont: value is not a map, returning default font\n");
		return GetFontDefault();
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	long handle = handleVal.IntValue();
	if (handle == 0) {
		// If no handle, return default font
		printf("ValueToFont: handle is 0, returning default font\n");
		return GetFontDefault();
	}
	Font* fontPtr = (Font*)handle;
	if (fontPtr == nullptr) {
		printf("ValueToFont: fontPtr is null, returning default font\n");
		return GetFontDefault();
	}
	Font font = *fontPtr;
	return font;
}

// Convert a Raylib Wave to a MiniScript map
static Value WaveToValue(Wave wave) {
	Wave* wavePtr = new Wave(wave);
	ValueDict map;
	map.SetValue(Value::magicIsA, WaveClass());
	map.SetValue(String("_handle"), Value((long)wavePtr));
	map.SetValue(String("frameCount"), Value((int)wave.frameCount));
	map.SetValue(String("sampleRate"), Value((int)wave.sampleRate));
	map.SetValue(String("sampleSize"), Value((int)wave.sampleSize));
	map.SetValue(String("channels"), Value((int)wave.channels));
	return Value(map);
}

// Extract a Raylib Wave from a MiniScript map
static Wave ValueToWave(Value value) {
	if (value.type != ValueType::Map) {
		return Wave{NULL, 0, 0, 0, 0};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	Wave* wavePtr = (Wave*)(long)handleVal.IntValue();
	if (wavePtr == nullptr) {
		return Wave{NULL, 0, 0, 0, 0};
	}
	return *wavePtr;
}

// Convert a Raylib Music to a MiniScript map
static Value MusicToValue(Music music) {
	Music* musicPtr = new Music(music);
	ValueDict map;
	map.SetValue(Value::magicIsA, MusicClass());
	map.SetValue(String("_handle"), Value((long)musicPtr));
	map.SetValue(String("frameCount"), Value((int)music.frameCount));
	map.SetValue(String("looping"), Value(music.looping ? 1 : 0));
	return Value(map);
}

// Extract a Raylib Music from a MiniScript map
static Music ValueToMusic(Value value) {
	if (value.type != ValueType::Map) {
		return Music{};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	Music* musicPtr = (Music*)(long)handleVal.IntValue();
	if (musicPtr == nullptr) {
		return Music{};
	}
	return *musicPtr;
}

// Convert a Raylib Sound to a MiniScript map
static Value SoundToValue(Sound sound) {
	Sound* soundPtr = new Sound(sound);
	ValueDict map;
	map.SetValue(Value::magicIsA, SoundClass());
	map.SetValue(String("_handle"), Value((long)soundPtr));
	map.SetValue(String("frameCount"), Value((int)sound.frameCount));
	return Value(map);
}

// Extract a Raylib Sound from a MiniScript map
static Sound ValueToSound(Value value) {
	if (value.type != ValueType::Map) {
		return Sound{};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	Sound* soundPtr = (Sound*)(long)handleVal.IntValue();
	if (soundPtr == nullptr) {
		return Sound{};
	}
	return *soundPtr;
}

// Convert a Raylib AudioStream to a MiniScript map
static Value AudioStreamToValue(AudioStream stream) {
	AudioStream* streamPtr = new AudioStream(stream);
	ValueDict map;
	map.SetValue(Value::magicIsA, AudioStreamClass());
	map.SetValue(String("_handle"), Value((long)streamPtr));
	map.SetValue(String("sampleRate"), Value((int)stream.sampleRate));
	map.SetValue(String("sampleSize"), Value((int)stream.sampleSize));
	map.SetValue(String("channels"), Value((int)stream.channels));
	return Value(map);
}

// Extract a Raylib AudioStream from a MiniScript map
static AudioStream ValueToAudioStream(Value value) {
	if (value.type != ValueType::Map) {
		return AudioStream{};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	AudioStream* streamPtr = (AudioStream*)(long)handleVal.IntValue();
	if (streamPtr == nullptr) {
		return AudioStream{};
	}
	return *streamPtr;
}

// Convert a Raylib RenderTexture2D to a MiniScript map
// Allocates the RenderTexture2D on the heap and stores pointer in _handle
static Value RenderTextureToValue(RenderTexture2D renderTexture) {
	RenderTexture2D* rtPtr = new RenderTexture2D(renderTexture);
	ValueDict map;
	map.SetValue(Value::magicIsA, RenderTextureClass());
	map.SetValue(String("_handle"), Value((long)rtPtr));
	map.SetValue(String("id"), Value((int)renderTexture.id));
	map.SetValue(String("texture"), TextureToValue(renderTexture.texture));
	return Value(map);
}

// Extract a Raylib RenderTexture2D from a MiniScript map
// Returns the RenderTexture2D by dereferencing the _handle pointer
static RenderTexture2D ValueToRenderTexture(Value value) {
	if (value.type != ValueType::Map) {
		return RenderTexture2D{};
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	RenderTexture2D* rtPtr = (RenderTexture2D*)(long)handleVal.IntValue();
	if (rtPtr == nullptr) {
		return RenderTexture2D{};
	}
	return *rtPtr;
}

// Convert a MiniScript map to a Raylib Color
// Expects a map with "r", "g", "b", and optionally "a" keys (0-255);
// or, a 3- or 4-element list in the order [r, g, b, a].
static Color ValueToColor(Value value) {
	Color result;

	// Handle list format: [r, g, b, a] or [r, g, b]
	if (value.type == ValueType::List) {
		ValueList list = value.GetList();
		if (list.Count() >= 3) {
			result.r = (unsigned char)(list[0].IntValue());
			result.g = (unsigned char)(list[1].IntValue());
			result.b = (unsigned char)(list[2].IntValue());
			result.a = list.Count() >= 4 ? (unsigned char)(list[3].IntValue()) : 255;
			return result;
		}
		// If list has fewer than 3 elements, fall through to default
	}

	// Handle map format: {"r": r, "g": g, "b": b, "a": a}
	if (value.type == ValueType::Map) {
		ValueDict map = value.GetDict();

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

	// Default to white if neither list nor map
	return WHITE;
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

// Convert a MiniScript value to a Raylib Rectangle
// Accepts either a map with "x", "y", "width", "height" keys OR a list with 4 elements
static Rectangle ValueToRectangle(Value value) {
	if (value.type == ValueType::List) {
		// List format: [x, y, width, height]
		ValueList list = value.GetList();
		float x = (list.Count() > 0) ? list[0].FloatValue() : 0;
		float y = (list.Count() > 1) ? list[1].FloatValue() : 0;
		float width = (list.Count() > 2) ? list[2].FloatValue() : 0;
		float height = (list.Count() > 3) ? list[3].FloatValue() : 0;
		return Rectangle{x, y, width, height};
	} else if (value.type == ValueType::Map) {
		// Map format: {x: ..., y: ..., width: ..., height: ...}
		ValueDict map = value.GetDict();
		Value xVal = map.Lookup(String("x"), Value::zero);
		Value yVal = map.Lookup(String("y"), Value::zero);
		Value widthVal = map.Lookup(String("width"), Value::zero);
		Value heightVal = map.Lookup(String("height"), Value::zero);

		Rectangle result;
		result.x = xVal.FloatValue();
		result.y = yVal.FloatValue();
		result.width = widthVal.FloatValue();
		result.height = heightVal.FloatValue();

		return result;
	} else {
		// Default to empty rectangle if not a map or list
		return Rectangle{0, 0, 0, 0};
	}
}

// Convert a Raylib Rectangle to a MiniScript map
static Value RectangleToValue(Rectangle rect) {
	ValueDict map;
	map.SetValue(String("x"), Value(rect.x));
	map.SetValue(String("y"), Value(rect.y));
	map.SetValue(String("width"), Value(rect.width));
	map.SetValue(String("height"), Value(rect.height));
	return Value(map);
}

// Convert a MiniScript value to a Raylib Vector2
// Accepts either a map with "x", "y" keys OR a list with 2 elements
static Vector2 ValueToVector2(Value value) {
	if (value.type == ValueType::List) {
		// List format: [x, y]
		ValueList list = value.GetList();
		float x = (list.Count() > 0) ? list[0].FloatValue() : 0;
		float y = (list.Count() > 1) ? list[1].FloatValue() : 0;
		return Vector2{x, y};
	} else if (value.type == ValueType::Map) {
		// Map format: {x: ..., y: ...}
		ValueDict map = value.GetDict();
		Value xVal = map.Lookup(String("x"), Value::zero);
		Value yVal = map.Lookup(String("y"), Value::zero);
		return Vector2{xVal.FloatValue(), yVal.FloatValue()};
	} else {
		// Default to zero vector if not a map or list
		return Vector2{0, 0};
	}
}

// Convert a Raylib Vector2 to a MiniScript map
static Value Vector2ToValue(Vector2 vec) {
	ValueDict map;
	map.SetValue(String("x"), Value(vec.x));
	map.SetValue(String("y"), Value(vec.y));
	return Value(map);
}



//--------------------------------------------------------------------------------
// rtextures methods
//--------------------------------------------------------------------------------

static void AddRTexturesMethods(ValueDict raylibModule) {
	Intrinsic *i;

	// Image loading

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Image img = LoadImage(path.c_str());
		if (!IsImageValid(img)) return IntrinsicResult::Null;
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("LoadImage", i->GetFunc());

	// Image generation

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("direction", Value::zero);
	i->AddParam("start", ColorToValue(BLACK));
	i->AddParam("end", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		int direction = context->GetVar(String("direction")).IntValue();
		Color start = ValueToColor(context->GetVar(String("start")));
		Color end = ValueToColor(context->GetVar(String("end")));
		Image img = GenImageGradientLinear(width, height, direction, start, end);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageGradientLinear", i->GetFunc());

	// Image management

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		UnloadImage(img);
		// Free the heap-allocated Image struct
		ValueDict map = context->GetVar(String("image")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Image* imgPtr = (Image*)(long)handleVal.IntValue();
		delete imgPtr;
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadImage", i->GetFunc());

	// Texture loading

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Texture tex = LoadTexture(path.c_str());
		if (!IsTextureValid(tex)) return IntrinsicResult::Null;
		return IntrinsicResult(TextureToValue(tex));
	};
	raylibModule.SetValue("LoadTexture", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		Texture tex = LoadTextureFromImage(img);
		return IntrinsicResult(TextureToValue(tex));
	};
	raylibModule.SetValue("LoadTextureFromImage", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		UnloadTexture(tex);
		// Free the heap-allocated Texture struct
		ValueDict map = context->GetVar(String("texture")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Texture* texPtr = (Texture*)(long)handleVal.IntValue();
		delete texPtr;
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadTexture", i->GetFunc());

	// Texture drawing

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTexture(tex, posX, posY, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTexture", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextureV(tex, position, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextureV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("rotation", Value::zero);
	i->AddParam("scale", Value(1.0));
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		float rotation = context->GetVar(String("rotation")).FloatValue();
		float scale = context->GetVar(String("scale")).FloatValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextureEx(tex, position, rotation, scale, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextureEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("source");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		Rectangle source = ValueToRectangle(context->GetVar(String("source")));
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextureRec(tex, source, position, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextureRec", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("source");
	i->AddParam("dest");
	i->AddParam("origin", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("rotation", Value::zero);
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		Rectangle source = ValueToRectangle(context->GetVar(String("source")));
		Rectangle dest = ValueToRectangle(context->GetVar(String("dest")));
		Vector2 origin = ValueToVector2(context->GetVar(String("origin")));
		float rotation = context->GetVar(String("rotation")).FloatValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTexturePro(tex, source, dest, origin, rotation, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTexturePro", i->GetFunc());

	// More image generation functions

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		Image img = GenImageColor(width, height, color);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageColor", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("density", Value(0.5));
	i->AddParam("inner", ColorToValue(WHITE));
	i->AddParam("outer", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		float density = context->GetVar(String("density")).FloatValue();
		Color inner = ValueToColor(context->GetVar(String("inner")));
		Color outer = ValueToColor(context->GetVar(String("outer")));
		Image img = GenImageGradientRadial(width, height, density, inner, outer);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageGradientRadial", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("density", Value(0.5));
	i->AddParam("inner", ColorToValue(WHITE));
	i->AddParam("outer", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		float density = context->GetVar(String("density")).FloatValue();
		Color inner = ValueToColor(context->GetVar(String("inner")));
		Color outer = ValueToColor(context->GetVar(String("outer")));
		Image img = GenImageGradientSquare(width, height, density, inner, outer);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageGradientSquare", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("checksX", Value(8));
	i->AddParam("checksY", Value(8));
	i->AddParam("col1", ColorToValue(WHITE));
	i->AddParam("col2", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		int checksX = context->GetVar(String("checksX")).IntValue();
		int checksY = context->GetVar(String("checksY")).IntValue();
		Color col1 = ValueToColor(context->GetVar(String("col1")));
		Color col2 = ValueToColor(context->GetVar(String("col2")));
		Image img = GenImageChecked(width, height, checksX, checksY, col1, col2);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageChecked", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("factor", Value(0.5));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		float factor = context->GetVar(String("factor")).FloatValue();
		Image img = GenImageWhiteNoise(width, height, factor);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageWhiteNoise", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("tileSize", Value(32));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		int tileSize = context->GetVar(String("tileSize")).IntValue();
		Image img = GenImageCellular(width, height, tileSize);
		return IntrinsicResult(ImageToValue(img));
	};
	raylibModule.SetValue("GenImageCellular", i->GetFunc());

	// Image manipulation

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		Image copy = ImageCopy(img);
		return IntrinsicResult(ImageToValue(copy));
	};
	raylibModule.SetValue("ImageCopy", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("crop");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		Rectangle crop = ValueToRectangle(context->GetVar(String("crop")));
		ImageCrop(&img, crop);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageCrop", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("newWidth");
	i->AddParam("newHeight");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		int newWidth = context->GetVar(String("newWidth")).IntValue();
		int newHeight = context->GetVar(String("newHeight")).IntValue();
		ImageResize(&img, newWidth, newHeight);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageResize", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("newWidth");
	i->AddParam("newHeight");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		int newWidth = context->GetVar(String("newWidth")).IntValue();
		int newHeight = context->GetVar(String("newHeight")).IntValue();
		ImageResizeNN(&img, newWidth, newHeight);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageResizeNN", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageFlipVertical(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageFlipVertical", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageFlipHorizontal(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageFlipHorizontal", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageRotateCW(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageRotateCW", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageRotateCCW(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageRotateCCW", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageColorTint(&img, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageColorTint", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageColorInvert(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageColorInvert", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		ImageColorGrayscale(&img);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageColorGrayscale", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("contrast");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		float contrast = context->GetVar(String("contrast")).FloatValue();
		ImageColorContrast(&img, contrast);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageColorContrast", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("brightness");
	i->code = INTRINSIC_LAMBDA {
		Image img = ValueToImage(context->GetVar(String("image")));
		int brightness = context->GetVar(String("brightness")).IntValue();
		ImageColorBrightness(&img, brightness);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageColorBrightness", i->GetFunc());

	// Image drawing functions

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageClearBackground(&dst, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageClearBackground", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("x", Value::zero);
	i->AddParam("y", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		int x = context->GetVar(String("x")).IntValue();
		int y = context->GetVar(String("y")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawPixel(&dst, x, y, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawPixel", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawPixelV(&dst, position, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawPixelV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("startPosX", Value::zero);
	i->AddParam("startPosY", Value::zero);
	i->AddParam("endPosX", Value::zero);
	i->AddParam("endPosY", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		int startPosX = context->GetVar(String("startPosX")).IntValue();
		int startPosY = context->GetVar(String("startPosY")).IntValue();
		int endPosX = context->GetVar(String("endPosX")).IntValue();
		int endPosY = context->GetVar(String("endPosY")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawLine(&dst, startPosX, startPosY, endPosX, endPosY, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawLine", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("start", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("end", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Vector2 start = ValueToVector2(context->GetVar(String("start")));
		Vector2 end = ValueToVector2(context->GetVar(String("end")));
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawLineV(&dst, start, end, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawLineV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("centerX", Value(100));
	i->AddParam("centerY", Value(100));
	i->AddParam("radius", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		int centerX = context->GetVar(String("centerX")).IntValue();
		int centerY = context->GetVar(String("centerY")).IntValue();
		int radius = context->GetVar(String("radius")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawCircle(&dst, centerX, centerY, radius, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawCircle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("radius", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		int radius = context->GetVar(String("radius")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawCircleV(&dst, center, radius, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawCircleV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawRectangle(&dst, posX, posY, width, height, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawRectangle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("rec");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawRectangleRec(&dst, rec, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawRectangleRec", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("rec");
	i->AddParam("thick", Value(1));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		int thick = context->GetVar(String("thick")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawRectangleLines(&dst, rec, thick, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawRectangleLines", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("src");
	i->AddParam("srcRec");
	i->AddParam("dstRec");
	i->AddParam("tint", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		Image src = ValueToImage(context->GetVar(String("src")));
		Rectangle srcRec = ValueToRectangle(context->GetVar(String("srcRec")));
		Rectangle dstRec = ValueToRectangle(context->GetVar(String("dstRec")));
		Color tint = ValueToColor(context->GetVar(String("tint")));
		ImageDraw(&dst, src, srcRec, dstRec, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDraw", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("dst");
	i->AddParam("text");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("fontSize", Value(20));
	i->AddParam("color", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		Image dst = ValueToImage(context->GetVar(String("dst")));
		String text = context->GetVar(String("text")).ToString();
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		int fontSize = context->GetVar(String("fontSize")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		ImageDrawText(&dst, text.c_str(), posX, posY, fontSize, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ImageDrawText", i->GetFunc());

	// Texture configuration

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("filter");
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		int filter = context->GetVar(String("filter")).IntValue();
		SetTextureFilter(tex, filter);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetTextureFilter", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->AddParam("wrap");
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		int wrap = context->GetVar(String("wrap")).IntValue();
		SetTextureWrap(tex, wrap);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetTextureWrap", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("texture");
	i->code = INTRINSIC_LAMBDA {
		Texture tex = ValueToTexture(context->GetVar(String("texture")));
		GenTextureMipmaps(&tex);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("GenTextureMipmaps", i->GetFunc());

	// RenderTexture2D loading/unloading

	i = Intrinsic::Create("");
	i->AddParam("width", Value(960));
	i->AddParam("height", Value(640));
	i->code = INTRINSIC_LAMBDA {
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		RenderTexture2D renderTexture = LoadRenderTexture(width, height);
		return IntrinsicResult(RenderTextureToValue(renderTexture));
	};
	raylibModule.SetValue("LoadRenderTexture", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("target");
	i->code = INTRINSIC_LAMBDA {
		RenderTexture2D target = ValueToRenderTexture(context->GetVar(String("target")));
		UnloadRenderTexture(target);
		// Free the heap-allocated RenderTexture2D struct
		ValueDict map = context->GetVar(String("target")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		RenderTexture2D* rtPtr = (RenderTexture2D*)(long)handleVal.IntValue();
		delete rtPtr;
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadRenderTexture", i->GetFunc());

	// RenderTexture2D drawing

	i = Intrinsic::Create("");
	i->AddParam("target");
	i->code = INTRINSIC_LAMBDA {
		RenderTexture2D target = ValueToRenderTexture(context->GetVar(String("target")));
		BeginTextureMode(target);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("BeginTextureMode", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		EndTextureMode();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("EndTextureMode", i->GetFunc());
}

//--------------------------------------------------------------------------------
// rtext methods
//--------------------------------------------------------------------------------

static void AddRTextMethods(ValueDict raylibModule) {
	Intrinsic *i;

	// Font loading

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Font font = LoadFont(path.c_str());
		if (!IsFontValid(font)) return IntrinsicResult::Null;
		return IntrinsicResult(FontToValue(font));
	};
	raylibModule.SetValue("LoadFont", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->AddParam("fontSize", Value(20));
	i->AddParam("codepoints", Value::null);
	i->AddParam("codepointCount", Value::zero);
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		int fontSize = context->GetVar(String("fontSize")).IntValue();
		// For now, ignore codepoints parameter and load all
		Font font = LoadFontEx(path.c_str(), fontSize, nullptr, 0);
		if (!IsFontValid(font)) return IntrinsicResult::Null;
		return IntrinsicResult(FontToValue(font));
	};
	raylibModule.SetValue("LoadFontEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("key", ColorToValue(Color{255, 0, 255, 255}));
	i->AddParam("firstChar", Value(32));
	i->code = INTRINSIC_LAMBDA {
		Image image = ValueToImage(context->GetVar(String("image")));
		Color key = ValueToColor(context->GetVar(String("key")));
		Value firstCharVal = context->GetVar(String("firstChar"));
		int firstChar;
		if (firstCharVal.type == ValueType::String) {
			String s = firstCharVal.ToString();
			firstChar = s.empty() ? 32 : (int)s[0];
		} else {
			firstChar = firstCharVal.IntValue();
		}
		Font font = LoadFontFromImage(image, key, firstChar);
		return IntrinsicResult(FontToValue(font));
	};
	raylibModule.SetValue("LoadFontFromImage", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		return IntrinsicResult(IsFontValid(font));
	};
	raylibModule.SetValue("IsFontValid", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		UnloadFont(font);
		// Free the heap-allocated Font struct
		ValueDict map = context->GetVar(String("font")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Font* fontPtr = (Font*)(long)handleVal.IntValue();
		delete fontPtr;
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadFont", i->GetFunc());

	// Text drawing

	i = Intrinsic::Create("");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->code = INTRINSIC_LAMBDA {
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		DrawFPS(posX, posY);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawFPS", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("text");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("fontSize", Value(20));
	i->AddParam("color", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		String text = context->GetVar(String("text")).ToString();
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		int fontSize = context->GetVar(String("fontSize")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawText(text.c_str(), posX, posY, fontSize, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawText", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->AddParam("text");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("fontSize", Value(20));
	i->AddParam("spacing", Value::zero);
	i->AddParam("tint", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		String text = context->GetVar(String("text")).ToString();
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		float fontSize = context->GetVar(String("fontSize")).FloatValue();
		float spacing = context->GetVar(String("spacing")).FloatValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextEx(font, text.c_str(), position, fontSize, spacing, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->AddParam("text");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("origin", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("rotation", Value::zero);
	i->AddParam("fontSize", Value(20));
	i->AddParam("spacing", Value::zero);
	i->AddParam("tint", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		String text = context->GetVar(String("text")).ToString();
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Vector2 origin = ValueToVector2(context->GetVar(String("origin")));
		float rotation = context->GetVar(String("rotation")).FloatValue();
		float fontSize = context->GetVar(String("fontSize")).FloatValue();
		float spacing = context->GetVar(String("spacing")).FloatValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextPro(font, text.c_str(), position, origin, rotation, fontSize, spacing, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextPro", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->AddParam("codepoint");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("fontSize", Value(20));
	i->AddParam("tint", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		int codepoint = context->GetVar(String("codepoint")).IntValue();
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		float fontSize = context->GetVar(String("fontSize")).FloatValue();
		Color tint = ValueToColor(context->GetVar(String("tint")));
		DrawTextCodepoint(font, codepoint, position, fontSize, tint);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTextCodepoint", i->GetFunc());

	// Text measurement

	i = Intrinsic::Create("");
	i->AddParam("text");
	i->AddParam("fontSize", Value(20));
	i->code = INTRINSIC_LAMBDA {
		String text = context->GetVar(String("text")).ToString();
		int fontSize = context->GetVar(String("fontSize")).IntValue();
		int width = MeasureText(text.c_str(), fontSize);
		return IntrinsicResult(Value(width));
	};
	raylibModule.SetValue("MeasureText", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->AddParam("text");
	i->AddParam("fontSize", Value(20));
	i->AddParam("spacing", Value::zero);
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		String text = context->GetVar(String("text")).ToString();
		float fontSize = context->GetVar(String("fontSize")).FloatValue();
		float spacing = context->GetVar(String("spacing")).FloatValue();
		Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
		ValueDict result;
		result.SetValue(String("x"), Value(size.x));
		result.SetValue(String("y"), Value(size.y));
		return IntrinsicResult(Value(result));
	};
	raylibModule.SetValue("MeasureTextEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("font");
	i->AddParam("codepoint");
	i->code = INTRINSIC_LAMBDA {
		Font font = ValueToFont(context->GetVar(String("font")));
		int codepoint = context->GetVar(String("codepoint")).IntValue();
		int index = GetGlyphIndex(font, codepoint);
		return IntrinsicResult(Value(index));
	};
	raylibModule.SetValue("GetGlyphIndex", i->GetFunc());
}

//--------------------------------------------------------------------------------
// raudio methods
//--------------------------------------------------------------------------------

static void AddRAudioMethods(ValueDict raylibModule) {
	Intrinsic *i;

	// Audio device management

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		InitAudioDevice();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("InitAudioDevice", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		CloseAudioDevice();
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("CloseAudioDevice", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsAudioDeviceReady());
	};
	raylibModule.SetValue("IsAudioDeviceReady", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("volume", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		float volume = context->GetVar(String("volume")).FloatValue();
		SetMasterVolume(volume);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetMasterVolume", i->GetFunc());

	// Wave loading

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Wave wave = LoadWave(path.c_str());
		if (!IsWaveValid(wave)) return IntrinsicResult::Null;
		return IntrinsicResult(WaveToValue(wave));
	};
	raylibModule.SetValue("LoadWave", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("fileType");
	i->AddParam("fileData");
	i->AddParam("dataSize");
	i->code = INTRINSIC_LAMBDA {
		String fileType = context->GetVar(String("fileType")).ToString();
		// Note: This would need a byte array type in MiniScript to be fully useful
		// For now, we'll skip implementing this
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("LoadWaveFromMemory", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		return IntrinsicResult(IsWaveValid(wave));
	};
	raylibModule.SetValue("IsWaveValid", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		UnloadWave(wave);
		// Also delete the heap-allocated Wave
		ValueDict map = context->GetVar(String("wave")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Wave* wavePtr = (Wave*)(long)handleVal.IntValue();
		if (wavePtr != nullptr) {
			delete wavePtr;
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadWave", i->GetFunc());

	// Wave manipulation

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		Wave copy = WaveCopy(wave);
		return IntrinsicResult(WaveToValue(copy));
	};
	raylibModule.SetValue("WaveCopy", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->AddParam("initFrame", Value::zero);
	i->AddParam("finalFrame", Value(100));
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		int initFrame = context->GetVar(String("initFrame")).IntValue();
		int finalFrame = context->GetVar(String("finalFrame")).IntValue();
		WaveCrop(&wave, initFrame, finalFrame);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("WaveCrop", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->AddParam("sampleRate", Value(44100));
	i->AddParam("sampleSize", Value(16));
	i->AddParam("channels", Value(2));
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		int sampleRate = context->GetVar(String("sampleRate")).IntValue();
		int sampleSize = context->GetVar(String("sampleSize")).IntValue();
		int channels = context->GetVar(String("channels")).IntValue();
		WaveFormat(&wave, sampleRate, sampleSize, channels);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("WaveFormat", i->GetFunc());

	// Music loading and control

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Music music = LoadMusicStream(path.c_str());
		if (!IsMusicValid(music)) return IntrinsicResult::Null;
		return IntrinsicResult(MusicToValue(music));
	};
	raylibModule.SetValue("LoadMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("fileType");
	i->AddParam("data");
	i->AddParam("dataSize");
	i->code = INTRINSIC_LAMBDA {
		String fileType = context->GetVar(String("fileType")).ToString();
		// Note: This would need a byte array type in MiniScript to be fully useful
		// For now, we'll skip implementing this
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("LoadMusicStreamFromMemory", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		return IntrinsicResult(IsMusicValid(music));
	};
	raylibModule.SetValue("IsMusicValid", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		UnloadMusicStream(music);
		// Also delete the heap-allocated Music
		ValueDict map = context->GetVar(String("music")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Music* musicPtr = (Music*)(long)handleVal.IntValue();
		if (musicPtr != nullptr) {
			delete musicPtr;
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		PlayMusicStream(music);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PlayMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		return IntrinsicResult(IsMusicStreamPlaying(music));
	};
	raylibModule.SetValue("IsMusicStreamPlaying", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		UpdateMusicStream(music);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UpdateMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		StopMusicStream(music);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("StopMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		PauseMusicStream(music);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PauseMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		ResumeMusicStream(music);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ResumeMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->AddParam("position", Value::zero);
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float position = context->GetVar(String("position")).FloatValue();
		SeekMusicStream(music, position);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SeekMusicStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->AddParam("volume", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float volume = context->GetVar(String("volume")).FloatValue();
		SetMusicVolume(music, volume);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetMusicVolume", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->AddParam("pitch", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float pitch = context->GetVar(String("pitch")).FloatValue();
		SetMusicPitch(music, pitch);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetMusicPitch", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->AddParam("pan", Value(0.5));
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float pan = context->GetVar(String("pan")).FloatValue();
		SetMusicPan(music, pan);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetMusicPan", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float length = GetMusicTimeLength(music);
		return IntrinsicResult(Value(length));
	};
	raylibModule.SetValue("GetMusicTimeLength", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("music");
	i->code = INTRINSIC_LAMBDA {
		Music music = ValueToMusic(context->GetVar(String("music")));
		float timePlayed = GetMusicTimePlayed(music);
		return IntrinsicResult(Value(timePlayed));
	};
	raylibModule.SetValue("GetMusicTimePlayed", i->GetFunc());

	// Sound loading and control

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		String path = context->GetVar(String("fileName")).ToString();
		Sound sound = LoadSound(path.c_str());
		if (!IsSoundValid(sound)) return IntrinsicResult::Null;
		return IntrinsicResult(SoundToValue(sound));
	};
	raylibModule.SetValue("LoadSound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("wave");
	i->code = INTRINSIC_LAMBDA {
		Wave wave = ValueToWave(context->GetVar(String("wave")));
		Sound sound = LoadSoundFromWave(wave);
		return IntrinsicResult(SoundToValue(sound));
	};
	raylibModule.SetValue("LoadSoundFromWave", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("source");
	i->code = INTRINSIC_LAMBDA {
		Sound source = ValueToSound(context->GetVar(String("source")));
		Sound alias = LoadSoundAlias(source);
		return IntrinsicResult(SoundToValue(alias));
	};
	raylibModule.SetValue("LoadSoundAlias", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		return IntrinsicResult(IsSoundValid(sound));
	};
	raylibModule.SetValue("IsSoundValid", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		UnloadSound(sound);
		// Also delete the heap-allocated Sound
		ValueDict map = context->GetVar(String("sound")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Sound* soundPtr = (Sound*)(long)handleVal.IntValue();
		if (soundPtr != nullptr) {
			delete soundPtr;
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadSound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("alias");
	i->code = INTRINSIC_LAMBDA {
		Sound alias = ValueToSound(context->GetVar(String("alias")));
		UnloadSoundAlias(alias);
		// Also delete the heap-allocated Sound
		ValueDict map = context->GetVar(String("alias")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		Sound* soundPtr = (Sound*)(long)handleVal.IntValue();
		if (soundPtr != nullptr) {
			delete soundPtr;
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadSoundAlias", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		PlaySound(sound);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PlaySound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		StopSound(sound);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("StopSound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		PauseSound(sound);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PauseSound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		ResumeSound(sound);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ResumeSound", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		return IntrinsicResult(IsSoundPlaying(sound));
	};
	raylibModule.SetValue("IsSoundPlaying", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->AddParam("volume", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		float volume = context->GetVar(String("volume")).FloatValue();
		SetSoundVolume(sound, volume);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetSoundVolume", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->AddParam("pitch", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		float pitch = context->GetVar(String("pitch")).FloatValue();
		SetSoundPitch(sound, pitch);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetSoundPitch", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("sound");
	i->AddParam("pan", Value(0.5));
	i->code = INTRINSIC_LAMBDA {
		Sound sound = ValueToSound(context->GetVar(String("sound")));
		float pan = context->GetVar(String("pan")).FloatValue();
		SetSoundPan(sound, pan);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetSoundPan", i->GetFunc());

	// AudioStream management

	i = Intrinsic::Create("");
	i->AddParam("sampleRate", Value(44100));
	i->AddParam("sampleSize", Value(32));
	i->AddParam("channels", Value(1));
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = LoadAudioStream(context->GetVar(String("sampleRate")).IntValue(), context->GetVar(String("sampleSize")).IntValue(), context->GetVar(String("channels")).IntValue());
		return IntrinsicResult(AudioStreamToValue(stream));
	};
	raylibModule.SetValue("LoadAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		return IntrinsicResult(IsAudioStreamValid(stream));
	};
	raylibModule.SetValue("IsAudioStreamValid", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		UnloadAudioStream(stream);
		// Also delete the heap-allocated AudioStream
		ValueDict map = context->GetVar(String("stream")).GetDict();
		Value handleVal = map.Lookup(String("_handle"), Value::zero);
		AudioStream* streamPtr = (AudioStream*)(long)handleVal.IntValue();
		if (streamPtr != nullptr) {
			delete streamPtr;
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UnloadAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->AddParam("data");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		ValueList data = context->GetVar(String("data")).GetList();

#define PROCESS_DATA(TYPE, VALUE) \
		TYPE *buffer = new TYPE[data.Count()]; \
		for (long i=0;i<data.Count();++i) { \
			buffer[i] = static_cast<TYPE>(data.Item(i).VALUE()); \
		}; \
		UpdateAudioStream(stream, buffer, data.Count());

		if (stream.sampleSize==8) {
			PROCESS_DATA(unsigned char, IntValue)
		} else if (stream.sampleSize==16) {
			PROCESS_DATA(signed short, IntValue)
		} else {
			PROCESS_DATA(float, FloatValue)
		}

#undef PROCESS_DATA

		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("UpdateAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		return IntrinsicResult(IsAudioStreamProcessed(stream));
	};
	raylibModule.SetValue("IsAudioStreamProcessed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		PlayAudioStream(stream);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PlayAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		PauseAudioStream(stream);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("PauseAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		ResumeAudioStream(stream);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ResumeAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		return IntrinsicResult(IsAudioStreamPlaying(stream));
	};
	raylibModule.SetValue("IsAudioStreamPlaying", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		StopAudioStream(stream);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("StopAudioStream", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->AddParam("volume", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		float volume = context->GetVar(String("volume")).FloatValue();
		SetAudioStreamVolume(stream, volume);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetAudioStreamVolume", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->AddParam("pitch", Value(1.0));
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		float pitch = context->GetVar(String("pitch")).FloatValue();
		SetAudioStreamPitch(stream, pitch);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetAudioStreamPitch", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("stream");
	i->AddParam("pan", Value(0.5));
	i->code = INTRINSIC_LAMBDA {
		AudioStream stream = ValueToAudioStream(context->GetVar(String("stream")));
		float pan = context->GetVar(String("pan")).FloatValue();
		SetAudioStreamPan(stream, pan);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetAudioStreamPan", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("size", Value(4096));
	i->code = INTRINSIC_LAMBDA {
		int size = context->GetVar(String("size")).IntValue();
		SetAudioStreamBufferSizeDefault(size);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetAudioStreamBufferSizeDefault", i->GetFunc());
}

//--------------------------------------------------------------------------------
// rshapes methods
//--------------------------------------------------------------------------------

static void AddRShapesMethods(ValueDict raylibModule) {
	Intrinsic *i;

	// Pixel drawing

	i = Intrinsic::Create("");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawPixel(posX, posY, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawPixel", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawPixelV(position, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawPixelV", i->GetFunc());

	// Line drawing

	i = Intrinsic::Create("");
	i->AddParam("startPosX", Value::zero);
	i->AddParam("startPosY", Value::zero);
	i->AddParam("endPosX", Value::zero);
	i->AddParam("endPosY", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int startPosX = context->GetVar(String("startPosX")).IntValue();
		int startPosY = context->GetVar(String("startPosY")).IntValue();
		int endPosX = context->GetVar(String("endPosX")).IntValue();
		int endPosY = context->GetVar(String("endPosY")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawLine(startPosX, startPosY, endPosX, endPosY, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawLine", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("startPos", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("endPos", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 startPos = ValueToVector2(context->GetVar(String("startPos")));
		Vector2 endPos = ValueToVector2(context->GetVar(String("endPos")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawLineV(startPos, endPos, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawLineV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("startPos", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("endPos", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("thick", Value(1));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 startPos = ValueToVector2(context->GetVar(String("startPos")));
		Vector2 endPos = ValueToVector2(context->GetVar(String("endPos")));
		float thick = context->GetVar(String("thick")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawLineEx(startPos, endPos, thick, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawLineEx", i->GetFunc());

	// Circle drawing

	i = Intrinsic::Create("");
	i->AddParam("centerX", Value(100));
	i->AddParam("centerY", Value(100));
	i->AddParam("radius", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int centerX = context->GetVar(String("centerX")).IntValue();
		int centerY = context->GetVar(String("centerY")).IntValue();
		float radius = context->GetVar(String("radius")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawCircle(centerX, centerY, radius, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawCircle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("radius", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		float radius = context->GetVar(String("radius")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawCircleV(center, radius, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawCircleV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("centerX", Value(100));
	i->AddParam("centerY", Value(100));
	i->AddParam("radius", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int centerX = context->GetVar(String("centerX")).IntValue();
		int centerY = context->GetVar(String("centerY")).IntValue();
		float radius = context->GetVar(String("radius")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawCircleLines(centerX, centerY, radius, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawCircleLines", i->GetFunc());

	// Ellipse drawing

	i = Intrinsic::Create("");
	i->AddParam("centerX", Value(100));
	i->AddParam("centerY", Value(100));
	i->AddParam("radiusH", Value(32));
	i->AddParam("radiusV", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int centerX = context->GetVar(String("centerX")).IntValue();
		int centerY = context->GetVar(String("centerY")).IntValue();
		float radiusH = context->GetVar(String("radiusH")).FloatValue();
		float radiusV = context->GetVar(String("radiusV")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawEllipse(centerX, centerY, radiusH, radiusV, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawEllipse", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("centerX", Value(100));
	i->AddParam("centerY", Value(100));
	i->AddParam("radiusH", Value(32));
	i->AddParam("radiusV", Value(32));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		int centerX = context->GetVar(String("centerX")).IntValue();
		int centerY = context->GetVar(String("centerY")).IntValue();
		float radiusH = context->GetVar(String("radiusH")).FloatValue();
		float radiusV = context->GetVar(String("radiusV")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawEllipseLines(centerX, centerY, radiusH, radiusV, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawEllipseLines", i->GetFunc());

	// Ring drawing

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("innerRadius", Value(20));
	i->AddParam("outerRadius", Value(32));
	i->AddParam("startAngle", Value::zero);
	i->AddParam("endAngle", Value(360));
	i->AddParam("segments", Value(36));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		float innerRadius = context->GetVar(String("innerRadius")).FloatValue();
		float outerRadius = context->GetVar(String("outerRadius")).FloatValue();
		float startAngle = context->GetVar(String("startAngle")).FloatValue();
		float endAngle = context->GetVar(String("endAngle")).FloatValue();
		int segments = context->GetVar(String("segments")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRing", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("innerRadius", Value(20));
	i->AddParam("outerRadius", Value(32));
	i->AddParam("startAngle", Value::zero);
	i->AddParam("endAngle", Value(360));
	i->AddParam("segments", Value(36));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		float innerRadius = context->GetVar(String("innerRadius")).FloatValue();
		float outerRadius = context->GetVar(String("outerRadius")).FloatValue();
		float startAngle = context->GetVar(String("startAngle")).FloatValue();
		float endAngle = context->GetVar(String("endAngle")).FloatValue();
		int segments = context->GetVar(String("segments")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRingLines(center, innerRadius, outerRadius, startAngle, endAngle, segments, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRingLines", i->GetFunc());

	// Rectangle drawing

	i = Intrinsic::Create("");
	i->AddParam("x", Value::zero);
	i->AddParam("y", Value::zero);
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("color", ColorToValue(WHITE));
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

	i = Intrinsic::Create("");
	i->AddParam("position", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("size", Vector2ToValue(Vector2{256, 256}));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 position = ValueToVector2(context->GetVar(String("position")));
		Vector2 size = ValueToVector2(context->GetVar(String("size")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleV(position, size, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleRec(rec, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleRec", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("origin", Vector2ToValue(Vector2{0, 0}));
	i->AddParam("rotation", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		Vector2 origin = ValueToVector2(context->GetVar(String("origin")));
		float rotation = context->GetVar(String("rotation")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectanglePro(rec, origin, rotation, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectanglePro", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("color");
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleLines(rec.x, rec.y, rec.width, rec.height, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleLines", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("lineThick", Value(1));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		float lineThick = context->GetVar(String("lineThick")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleLinesEx(rec, lineThick, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleLinesEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("roundness", Value(0.5));
	i->AddParam("segments", Value(36));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		float roundness = context->GetVar(String("roundness")).FloatValue();
		int segments = context->GetVar(String("segments")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleRounded(rec, roundness, segments, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleRounded", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("roundness", Value(0.5));
	i->AddParam("segments", Value(36));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		float roundness = context->GetVar(String("roundness")).FloatValue();
		int segments = context->GetVar(String("segments")).IntValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawRectangleRoundedLines(rec, roundness, segments, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleRoundedLines", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("color1", ColorToValue(WHITE));
	i->AddParam("color2", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		Color color1 = ValueToColor(context->GetVar(String("color1")));
		Color color2 = ValueToColor(context->GetVar(String("color2")));
		DrawRectangleGradientV(posX, posY, width, height, color1, color2);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleGradientV", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("posX", Value::zero);
	i->AddParam("posY", Value::zero);
	i->AddParam("width", Value(256));
	i->AddParam("height", Value(256));
	i->AddParam("color1", ColorToValue(WHITE));
	i->AddParam("color2", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		int posX = context->GetVar(String("posX")).IntValue();
		int posY = context->GetVar(String("posY")).IntValue();
		int width = context->GetVar(String("width")).IntValue();
		int height = context->GetVar(String("height")).IntValue();
		Color color1 = ValueToColor(context->GetVar(String("color1")));
		Color color2 = ValueToColor(context->GetVar(String("color2")));
		DrawRectangleGradientH(posX, posY, width, height, color1, color2);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleGradientH", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec");
	i->AddParam("col1");
	i->AddParam("col2");
	i->AddParam("col3");
	i->AddParam("col4");
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		Color col1 = ValueToColor(context->GetVar(String("col1")));
		Color col2 = ValueToColor(context->GetVar(String("col2")));
		Color col3 = ValueToColor(context->GetVar(String("col3")));
		Color col4 = ValueToColor(context->GetVar(String("col4")));
		DrawRectangleGradientEx(rec, col1, col2, col3, col4);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawRectangleGradientEx", i->GetFunc());

	// Triangle drawing

	i = Intrinsic::Create("");
	i->AddParam("v1");
	i->AddParam("v2");
	i->AddParam("v3");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 v1 = ValueToVector2(context->GetVar(String("v1")));
		Vector2 v2 = ValueToVector2(context->GetVar(String("v2")));
		Vector2 v3 = ValueToVector2(context->GetVar(String("v3")));
		Color color = ValueToColor(context->GetVar(String("color")));
		// Check winding order and ensure counter-clockwise (in screen coords where Y is down)
		float det = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
		if (det > 0) {
			// Clockwise in screen space - swap v2 and v3 to make it counter-clockwise
			DrawTriangle(v1, v3, v2, color);
		} else {
			DrawTriangle(v1, v2, v3, color);
		}
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTriangle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("v1");
	i->AddParam("v2");
	i->AddParam("v3");
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 v1 = ValueToVector2(context->GetVar(String("v1")));
		Vector2 v2 = ValueToVector2(context->GetVar(String("v2")));
		Vector2 v3 = ValueToVector2(context->GetVar(String("v3")));
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawTriangleLines(v1, v2, v3, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawTriangleLines", i->GetFunc());

	// Polygon drawing

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("sides", Value(6));
	i->AddParam("radius", Value(32));
	i->AddParam("rotation", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		int sides = context->GetVar(String("sides")).IntValue();
		float radius = context->GetVar(String("radius")).FloatValue();
		float rotation = context->GetVar(String("rotation")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawPoly(center, sides, radius, rotation, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawPoly", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("sides", Value(6));
	i->AddParam("radius", Value(32));
	i->AddParam("rotation", Value::zero);
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		int sides = context->GetVar(String("sides")).IntValue();
		float radius = context->GetVar(String("radius")).FloatValue();
		float rotation = context->GetVar(String("rotation")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawPolyLines(center, sides, radius, rotation, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawPolyLines", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center", Vector2ToValue(Vector2{100, 100}));
	i->AddParam("sides", Value(6));
	i->AddParam("radius", Value(32));
	i->AddParam("rotation", Value::zero);
	i->AddParam("lineThick", Value(1));
	i->AddParam("color", ColorToValue(WHITE));
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		int sides = context->GetVar(String("sides")).IntValue();
		float radius = context->GetVar(String("radius")).FloatValue();
		float rotation = context->GetVar(String("rotation")).FloatValue();
		float lineThick = context->GetVar(String("lineThick")).FloatValue();
		Color color = ValueToColor(context->GetVar(String("color")));
		DrawPolyLinesEx(center, sides, radius, rotation, lineThick, color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("DrawPolyLinesEx", i->GetFunc());

	// Collision detection

	i = Intrinsic::Create("");
	i->AddParam("rec1");
	i->AddParam("rec2");
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec1 = ValueToRectangle(context->GetVar(String("rec1")));
		Rectangle rec2 = ValueToRectangle(context->GetVar(String("rec2")));
		return IntrinsicResult(CheckCollisionRecs(rec1, rec2));
	};
	raylibModule.SetValue("CheckCollisionRecs", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center1");
	i->AddParam("radius1");
	i->AddParam("center2");
	i->AddParam("radius2");
	i->code = INTRINSIC_LAMBDA {
		Vector2 center1 = ValueToVector2(context->GetVar(String("center1")));
		float radius1 = context->GetVar(String("radius1")).FloatValue();
		Vector2 center2 = ValueToVector2(context->GetVar(String("center2")));
		float radius2 = context->GetVar(String("radius2")).FloatValue();
		return IntrinsicResult(CheckCollisionCircles(center1, radius1, center2, radius2));
	};
	raylibModule.SetValue("CheckCollisionCircles", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("center");
	i->AddParam("radius");
	i->AddParam("rec");
	i->code = INTRINSIC_LAMBDA {
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		float radius = context->GetVar(String("radius")).FloatValue();
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		return IntrinsicResult(CheckCollisionCircleRec(center, radius, rec));
	};
	raylibModule.SetValue("CheckCollisionCircleRec", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("point");
	i->AddParam("rec");
	i->code = INTRINSIC_LAMBDA {
		Vector2 point = ValueToVector2(context->GetVar(String("point")));
		Rectangle rec = ValueToRectangle(context->GetVar(String("rec")));
		return IntrinsicResult(CheckCollisionPointRec(point, rec));
	};
	raylibModule.SetValue("CheckCollisionPointRec", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("point");
	i->AddParam("center");
	i->AddParam("radius");
	i->code = INTRINSIC_LAMBDA {
		Vector2 point = ValueToVector2(context->GetVar(String("point")));
		Vector2 center = ValueToVector2(context->GetVar(String("center")));
		float radius = context->GetVar(String("radius")).FloatValue();
		return IntrinsicResult(CheckCollisionPointCircle(point, center, radius));
	};
	raylibModule.SetValue("CheckCollisionPointCircle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("point");
	i->AddParam("p1");
	i->AddParam("p2");
	i->AddParam("p3");
	i->code = INTRINSIC_LAMBDA {
		Vector2 point = ValueToVector2(context->GetVar(String("point")));
		Vector2 p1 = ValueToVector2(context->GetVar(String("p1")));
		Vector2 p2 = ValueToVector2(context->GetVar(String("p2")));
		Vector2 p3 = ValueToVector2(context->GetVar(String("p3")));
		return IntrinsicResult(CheckCollisionPointTriangle(point, p1, p2, p3));
	};
	raylibModule.SetValue("CheckCollisionPointTriangle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("rec1");
	i->AddParam("rec2");
	i->code = INTRINSIC_LAMBDA {
		Rectangle rec1 = ValueToRectangle(context->GetVar(String("rec1")));
		Rectangle rec2 = ValueToRectangle(context->GetVar(String("rec2")));
		Rectangle result = GetCollisionRec(rec1, rec2);
		return IntrinsicResult(RectangleToValue(result));
	};
	raylibModule.SetValue("GetCollisionRec", i->GetFunc());
}

//--------------------------------------------------------------------------------
// rcore methods
//--------------------------------------------------------------------------------

// Helper: Set window title
EM_JS(void, _SetWindowTitle, (const char *title), {
	const _title = UTF8ToString(title);
	document.title = _title;
	document.querySelector("h1").textContent = _title;
});

// Helper: Set window icon
// We need to free the buffer after we're done with it so this function won't
// return until after it's done.
EM_ASYNC_JS(void, _SetWindowIcon, (unsigned char *data, long size), {
	await new Promise((resolve, reject)=>{
		const _data = new Uint8Array(HEAP8.buffer, data, size);
		const blob = new Blob([_data], {type:"image/png"});
		const reader = new FileReader();
		reader.onloadend = () => {
			const dataURL = reader.result;
			let link = document.querySelector('link[rel="icon"]');
			if (link===null) {
				link = document.createElement("link");
				link.setAttribute("rel", "icon");
				document.head.appendChild(link);
			}
			link.href = dataURL;
			resolve();
		};
		reader.onerror = reject;
		reader.readAsDataURL(blob);
	});
});

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
	i->AddParam("color", ColorToValue(BLACK));
	i->code = INTRINSIC_LAMBDA {
		Value colorVal = context->GetVar(String("color"));
		Color color = ValueToColor(colorVal);
		ClearBackground(color);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("ClearBackground", i->GetFunc());

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

	// Input-related functions: gamepad

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsGamepadAvailable(context->GetVar(String("gamepad")).IntValue()));
	};
	raylibModule.SetValue("IsGamepadAvailable", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetGamepadName(context->GetVar(String("gamepad")).IntValue()));
	};
	raylibModule.SetValue("GetGamepadName", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsGamepadButtonPressed(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsGamepadButtonPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsGamepadButtonDown(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsGamepadButtonDown", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsGamepadButtonReleased(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsGamepadButtonReleased", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("button");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(IsGamepadButtonUp(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("button")).IntValue()));
	};
	raylibModule.SetValue("IsGamepadButtonUp", i->GetFunc());

	i = Intrinsic::Create("");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetGamepadButtonPressed());
	};
	raylibModule.SetValue("GetGamepadButtonPressed", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetGamepadAxisCount(context->GetVar(String("gamepad")).IntValue()));
	};
	raylibModule.SetValue("GetGamepadAxisCount", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("axis");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(GetGamepadAxisMovement(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("axis")).IntValue()));
	};
	raylibModule.SetValue("GetGamepadAxisMovement", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("mappings");
	i->code = INTRINSIC_LAMBDA {
		return IntrinsicResult(SetGamepadMappings(context->GetVar(String("mappings")).ToString().c_str()));
	};
	raylibModule.SetValue("SetGamepadMappings", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("gamepad", 0);
	i->AddParam("leftMotor", 0.0);
	i->AddParam("rightMotor", 0.0);
	i->AddParam("duration", 0.0);
	i->code = INTRINSIC_LAMBDA {
		SetGamepadVibration(
			context->GetVar(String("gamepad")).IntValue(),
			context->GetVar(String("leftMotor")).FloatValue(),
			context->GetVar(String("rightMotor")).FloatValue(),
			context->GetVar(String("duration")).FloatValue());
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetGamepadVibration", i->GetFunc());

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

	// Set window title/icon
	i = Intrinsic::Create("");
	i->AddParam("caption", "MSRLWeb - MiniScript + Raylib");
	i->code = INTRINSIC_LAMBDA {
		String caption = context->GetVar(String("caption")).GetString();
		_SetWindowTitle(caption.c_str());
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetWindowTitle", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->code = INTRINSIC_LAMBDA {
		Image image = ValueToImage(context->GetVar(String("image")));
		int size;
		unsigned char *data = ExportImageToMemory(image, ".png", &size);
		_SetWindowIcon(data, size);
		free(data);
		return IntrinsicResult::Null;
	};
	raylibModule.SetValue("SetWindowIcon", i->GetFunc());

	// Load text files
	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->code = INTRINSIC_LAMBDA {
		const char *fileName = context->GetVar("fileName").GetString().c_str();
		char *text = LoadFileText(fileName);
		String ret(text);
		UnloadFileText(text);
		return IntrinsicResult(ret);
	};
	raylibModule.SetValue("LoadFileText", i->GetFunc());
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

	// Add keyboard key constants
	raylibModule.SetValue("KEY_NULL", Value(KEY_NULL));

	// Alphanumeric keys
	raylibModule.SetValue("KEY_APOSTROPHE", Value(KEY_APOSTROPHE));
	raylibModule.SetValue("KEY_COMMA", Value(KEY_COMMA));
	raylibModule.SetValue("KEY_MINUS", Value(KEY_MINUS));
	raylibModule.SetValue("KEY_PERIOD", Value(KEY_PERIOD));
	raylibModule.SetValue("KEY_SLASH", Value(KEY_SLASH));
	raylibModule.SetValue("KEY_ZERO", Value(KEY_ZERO));
	raylibModule.SetValue("KEY_ONE", Value(KEY_ONE));
	raylibModule.SetValue("KEY_TWO", Value(KEY_TWO));
	raylibModule.SetValue("KEY_THREE", Value(KEY_THREE));
	raylibModule.SetValue("KEY_FOUR", Value(KEY_FOUR));
	raylibModule.SetValue("KEY_FIVE", Value(KEY_FIVE));
	raylibModule.SetValue("KEY_SIX", Value(KEY_SIX));
	raylibModule.SetValue("KEY_SEVEN", Value(KEY_SEVEN));
	raylibModule.SetValue("KEY_EIGHT", Value(KEY_EIGHT));
	raylibModule.SetValue("KEY_NINE", Value(KEY_NINE));
	raylibModule.SetValue("KEY_SEMICOLON", Value(KEY_SEMICOLON));
	raylibModule.SetValue("KEY_EQUAL", Value(KEY_EQUAL));
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
	raylibModule.SetValue("KEY_LEFT_BRACKET", Value(KEY_LEFT_BRACKET));
	raylibModule.SetValue("KEY_BACKSLASH", Value(KEY_BACKSLASH));
	raylibModule.SetValue("KEY_RIGHT_BRACKET", Value(KEY_RIGHT_BRACKET));
	raylibModule.SetValue("KEY_GRAVE", Value(KEY_GRAVE));

	// Function keys
	raylibModule.SetValue("KEY_SPACE", Value(KEY_SPACE));
	raylibModule.SetValue("KEY_ESCAPE", Value(KEY_ESCAPE));
	raylibModule.SetValue("KEY_ENTER", Value(KEY_ENTER));
	raylibModule.SetValue("KEY_TAB", Value(KEY_TAB));
	raylibModule.SetValue("KEY_BACKSPACE", Value(KEY_BACKSPACE));
	raylibModule.SetValue("KEY_INSERT", Value(KEY_INSERT));
	raylibModule.SetValue("KEY_DELETE", Value(KEY_DELETE));
	raylibModule.SetValue("KEY_RIGHT", Value(KEY_RIGHT));
	raylibModule.SetValue("KEY_LEFT", Value(KEY_LEFT));
	raylibModule.SetValue("KEY_DOWN", Value(KEY_DOWN));
	raylibModule.SetValue("KEY_UP", Value(KEY_UP));
	raylibModule.SetValue("KEY_PAGE_UP", Value(KEY_PAGE_UP));
	raylibModule.SetValue("KEY_PAGE_DOWN", Value(KEY_PAGE_DOWN));
	raylibModule.SetValue("KEY_HOME", Value(KEY_HOME));
	raylibModule.SetValue("KEY_END", Value(KEY_END));
	raylibModule.SetValue("KEY_CAPS_LOCK", Value(KEY_CAPS_LOCK));
	raylibModule.SetValue("KEY_SCROLL_LOCK", Value(KEY_SCROLL_LOCK));
	raylibModule.SetValue("KEY_NUM_LOCK", Value(KEY_NUM_LOCK));
	raylibModule.SetValue("KEY_PRINT_SCREEN", Value(KEY_PRINT_SCREEN));
	raylibModule.SetValue("KEY_PAUSE", Value(KEY_PAUSE));
	raylibModule.SetValue("KEY_F1", Value(KEY_F1));
	raylibModule.SetValue("KEY_F2", Value(KEY_F2));
	raylibModule.SetValue("KEY_F3", Value(KEY_F3));
	raylibModule.SetValue("KEY_F4", Value(KEY_F4));
	raylibModule.SetValue("KEY_F5", Value(KEY_F5));
	raylibModule.SetValue("KEY_F6", Value(KEY_F6));
	raylibModule.SetValue("KEY_F7", Value(KEY_F7));
	raylibModule.SetValue("KEY_F8", Value(KEY_F8));
	raylibModule.SetValue("KEY_F9", Value(KEY_F9));
	raylibModule.SetValue("KEY_F10", Value(KEY_F10));
	raylibModule.SetValue("KEY_F11", Value(KEY_F11));
	raylibModule.SetValue("KEY_F12", Value(KEY_F12));

	// Modifier keys
	raylibModule.SetValue("KEY_LEFT_SHIFT", Value(KEY_LEFT_SHIFT));
	raylibModule.SetValue("KEY_LEFT_CONTROL", Value(KEY_LEFT_CONTROL));
	raylibModule.SetValue("KEY_LEFT_ALT", Value(KEY_LEFT_ALT));
	raylibModule.SetValue("KEY_LEFT_SUPER", Value(KEY_LEFT_SUPER));
	raylibModule.SetValue("KEY_RIGHT_SHIFT", Value(KEY_RIGHT_SHIFT));
	raylibModule.SetValue("KEY_RIGHT_CONTROL", Value(KEY_RIGHT_CONTROL));
	raylibModule.SetValue("KEY_RIGHT_ALT", Value(KEY_RIGHT_ALT));
	raylibModule.SetValue("KEY_RIGHT_SUPER", Value(KEY_RIGHT_SUPER));
	raylibModule.SetValue("KEY_KB_MENU", Value(KEY_KB_MENU));

	// Keypad keys
	raylibModule.SetValue("KEY_KP_0", Value(KEY_KP_0));
	raylibModule.SetValue("KEY_KP_1", Value(KEY_KP_1));
	raylibModule.SetValue("KEY_KP_2", Value(KEY_KP_2));
	raylibModule.SetValue("KEY_KP_3", Value(KEY_KP_3));
	raylibModule.SetValue("KEY_KP_4", Value(KEY_KP_4));
	raylibModule.SetValue("KEY_KP_5", Value(KEY_KP_5));
	raylibModule.SetValue("KEY_KP_6", Value(KEY_KP_6));
	raylibModule.SetValue("KEY_KP_7", Value(KEY_KP_7));
	raylibModule.SetValue("KEY_KP_8", Value(KEY_KP_8));
	raylibModule.SetValue("KEY_KP_9", Value(KEY_KP_9));
	raylibModule.SetValue("KEY_KP_DECIMAL", Value(KEY_KP_DECIMAL));
	raylibModule.SetValue("KEY_KP_DIVIDE", Value(KEY_KP_DIVIDE));
	raylibModule.SetValue("KEY_KP_MULTIPLY", Value(KEY_KP_MULTIPLY));
	raylibModule.SetValue("KEY_KP_SUBTRACT", Value(KEY_KP_SUBTRACT));
	raylibModule.SetValue("KEY_KP_ADD", Value(KEY_KP_ADD));
	raylibModule.SetValue("KEY_KP_ENTER", Value(KEY_KP_ENTER));
	raylibModule.SetValue("KEY_KP_EQUAL", Value(KEY_KP_EQUAL));

	// Android keys
	raylibModule.SetValue("KEY_BACK", Value(KEY_BACK));
	raylibModule.SetValue("KEY_MENU", Value(KEY_MENU));
	raylibModule.SetValue("KEY_VOLUME_UP", Value(KEY_VOLUME_UP));
	raylibModule.SetValue("KEY_VOLUME_DOWN", Value(KEY_VOLUME_DOWN));

	// Add gamepad button constants
	raylibModule.SetValue("GAMEPAD_BUTTON_UNKNOWN", Value(GAMEPAD_BUTTON_UNKNOWN));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_FACE_UP", Value(GAMEPAD_BUTTON_LEFT_FACE_UP));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_FACE_RIGHT", Value(GAMEPAD_BUTTON_LEFT_FACE_RIGHT));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_FACE_DOWN", Value(GAMEPAD_BUTTON_LEFT_FACE_DOWN));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_FACE_LEFT", Value(GAMEPAD_BUTTON_LEFT_FACE_LEFT));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_FACE_UP", Value(GAMEPAD_BUTTON_RIGHT_FACE_UP));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_FACE_RIGHT", Value(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_FACE_DOWN", Value(GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_FACE_LEFT", Value(GAMEPAD_BUTTON_RIGHT_FACE_LEFT));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_TRIGGER_1", Value(GAMEPAD_BUTTON_LEFT_TRIGGER_1));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_TRIGGER_2", Value(GAMEPAD_BUTTON_LEFT_TRIGGER_2));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_TRIGGER_1", Value(GAMEPAD_BUTTON_RIGHT_TRIGGER_1));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_TRIGGER_2", Value(GAMEPAD_BUTTON_RIGHT_TRIGGER_2));
	raylibModule.SetValue("GAMEPAD_BUTTON_MIDDLE_LEFT", Value(GAMEPAD_BUTTON_MIDDLE_LEFT));
	raylibModule.SetValue("GAMEPAD_BUTTON_MIDDLE", Value(GAMEPAD_BUTTON_MIDDLE));
	raylibModule.SetValue("GAMEPAD_BUTTON_MIDDLE_RIGHT", Value(GAMEPAD_BUTTON_MIDDLE_RIGHT));
	raylibModule.SetValue("GAMEPAD_BUTTON_LEFT_THUMB", Value(GAMEPAD_BUTTON_LEFT_THUMB));
	raylibModule.SetValue("GAMEPAD_BUTTON_RIGHT_THUMB", Value(GAMEPAD_BUTTON_RIGHT_THUMB));

	// Add gamepad axis constants
	raylibModule.SetValue("GAMEPAD_AXIS_LEFT_X", Value(GAMEPAD_AXIS_LEFT_X));
	raylibModule.SetValue("GAMEPAD_AXIS_LEFT_Y", Value(GAMEPAD_AXIS_LEFT_Y));
	raylibModule.SetValue("GAMEPAD_AXIS_RIGHT_X", Value(GAMEPAD_AXIS_RIGHT_X));
	raylibModule.SetValue("GAMEPAD_AXIS_RIGHT_Y", Value(GAMEPAD_AXIS_RIGHT_Y));
	raylibModule.SetValue("GAMEPAD_AXIS_LEFT_TRIGGER", Value(GAMEPAD_AXIS_LEFT_TRIGGER));
	raylibModule.SetValue("GAMEPAD_AXIS_RIGHT_TRIGGER", Value(GAMEPAD_AXIS_RIGHT_TRIGGER));

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

	// Add texture filter mode constants
	raylibModule.SetValue("TEXTURE_FILTER_POINT", Value(TEXTURE_FILTER_POINT));
	raylibModule.SetValue("TEXTURE_FILTER_BILINEAR", Value(TEXTURE_FILTER_BILINEAR));
	raylibModule.SetValue("TEXTURE_FILTER_TRILINEAR", Value(TEXTURE_FILTER_TRILINEAR));
	raylibModule.SetValue("TEXTURE_FILTER_ANISOTROPIC_4X", Value(TEXTURE_FILTER_ANISOTROPIC_4X));
	raylibModule.SetValue("TEXTURE_FILTER_ANISOTROPIC_8X", Value(TEXTURE_FILTER_ANISOTROPIC_8X));
	raylibModule.SetValue("TEXTURE_FILTER_ANISOTROPIC_16X", Value(TEXTURE_FILTER_ANISOTROPIC_16X));

	// Add texture wrap mode constants
	raylibModule.SetValue("TEXTURE_WRAP_REPEAT", Value(TEXTURE_WRAP_REPEAT));
	raylibModule.SetValue("TEXTURE_WRAP_CLAMP", Value(TEXTURE_WRAP_CLAMP));
	raylibModule.SetValue("TEXTURE_WRAP_MIRROR_REPEAT", Value(TEXTURE_WRAP_MIRROR_REPEAT));
	raylibModule.SetValue("TEXTURE_WRAP_MIRROR_CLAMP", Value(TEXTURE_WRAP_MIRROR_CLAMP));
}

//--------------------------------------------------------------------------------
// Add intrinsics to interpreter
//--------------------------------------------------------------------------------

void AddRaylibIntrinsics() {
	Intrinsic *f;

	// Create accessors for the classes
	f = Intrinsic::Create("Image");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(ImageClass()); };

	f = Intrinsic::Create("Texture");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(TextureClass()); };

	f = Intrinsic::Create("Font");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(FontClass()); };

	f = Intrinsic::Create("Wave");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(WaveClass()); };

	f = Intrinsic::Create("Music");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(MusicClass()); };

	f = Intrinsic::Create("Sound");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(SoundClass()); };

	f = Intrinsic::Create("AudioStream");
	f->code = INTRINSIC_LAMBDA { return IntrinsicResult(AudioStreamClass()); };

	// Create and register the main raylib module
	f = Intrinsic::Create("raylib");
	f->code = INTRINSIC_LAMBDA {
		static ValueDict raylibModule;

		if (raylibModule.Count() == 0) {
			AddRCoreMethods(raylibModule);
			AddRShapesMethods(raylibModule);
			AddRTexturesMethods(raylibModule);
			AddRTextMethods(raylibModule);
			AddRAudioMethods(raylibModule);
			AddConstants(raylibModule);
		}

		return IntrinsicResult(raylibModule);
	};
}
