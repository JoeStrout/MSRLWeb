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
#include <emscripten/fetch.h>
#include <math.h>
#include <string.h>

using namespace MiniScript;

// Macro to reduce boilerplate for lambda intrinsics
#define INTRINSIC_LAMBDA [](Context *context, IntrinsicResult partialResult) -> IntrinsicResult

//--------------------------------------------------------------------------------
// Fetch callbacks for async loading
//--------------------------------------------------------------------------------

#include <map>

// Track in-flight fetches by ID
struct FetchData {
	emscripten_fetch_t* fetch;
	bool completed;
	int status;
	FetchData() : fetch(nullptr), completed(false), status(0) {}
};

static std::map<long, FetchData> activeFetches;
static long nextFetchId = 1;

// Callback when fetch completes (success or error)
static void fetch_completed(emscripten_fetch_t *fetch) {
	// Find this fetch in our map and mark it complete
	for (auto& pair : activeFetches) {
		if (pair.second.fetch == fetch) {
			pair.second.completed = true;
			pair.second.status = fetch->status;
			printf("fetch_completed: Fetch ID %ld completed with status %d\n", pair.first, fetch->status);
			break;
		}
	}
}

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
		map.SetValue(String("baseSize"), Value::zero);
		map.SetValue(String("glyphCount"), Value::zero);
		map.SetValue(String("glyphPadding"), Value::zero);
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
	map.SetValue(String("baseSize"), Value(font.baseSize));
	map.SetValue(String("glyphCount"), Value(font.glyphCount));
	map.SetValue(String("glyphPadding"), Value(font.glyphPadding));
	return Value(map);
}

// Extract a Raylib Font from a MiniScript map
static Font ValueToFont(Value value) {
	if (value.type != ValueType::Map) {
		// Return default font if not a map
		return GetFontDefault();
	}
	ValueDict map = value.GetDict();
	Value handleVal = map.Lookup(String("_handle"), Value::zero);
	if (handleVal.IntValue() == 0) {
		// If no handle, return default font
		return GetFontDefault();
	}
	Font* fontPtr = (Font*)(long)handleVal.IntValue();
	if (fontPtr == nullptr) {
		return GetFontDefault();
	}
	return *fontPtr;
}

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
		if (partialResult.Done()) {
			// First call - start the async fetch
			String path = context->GetVar(String("fileName")).ToString();

			// Create a new fetch ID and entry
			long fetchId = nextFetchId++;
			FetchData& data = activeFetches[fetchId];

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
			attr.onsuccess = fetch_completed;
			attr.onerror = fetch_completed;

			data.fetch = emscripten_fetch(&attr, path.c_str());
			printf("LoadImage: Started fetch ID %ld for %s\n", fetchId, path.c_str());

			// Return the fetch ID as partial result
			return IntrinsicResult(Value((double)fetchId), false);
		} else {
			// Subsequent calls - check if fetch is complete
			long fetchId = (long)partialResult.Result().DoubleValue();
			auto it = activeFetches.find(fetchId);
			if (it == activeFetches.end()) {
				printf("LoadImage: Fetch ID %ld not found!\n", fetchId);
				return IntrinsicResult::Null;
			}

			FetchData& data = it->second;

			if (!data.completed) {
				// Still loading
				return partialResult;
			}

			// Fetch is complete
			emscripten_fetch_t* fetch = data.fetch;
			printf("LoadImage: Fetch ID %ld complete, status=%d for %s\n", fetchId, data.status, fetch->url);

			if (data.status == 200) {
				// Success - get file extension and load image from memory
				const char* url = fetch->url;
				const char* ext = strrchr(url, '.');
				if (ext == nullptr) ext = ".png";

				Image img = LoadImageFromMemory(ext, (const unsigned char*)fetch->data, (int)fetch->numBytes);
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult(ImageToValue(img));
			} else {
				// Error
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult::Null;
			}
		}
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
		if (partialResult.Done()) {
			// First call - start the async fetch
			String path = context->GetVar(String("fileName")).ToString();

			// Create a new fetch ID and entry
			long fetchId = nextFetchId++;
			FetchData& data = activeFetches[fetchId];

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
			attr.onsuccess = fetch_completed;
			attr.onerror = fetch_completed;

			data.fetch = emscripten_fetch(&attr, path.c_str());
			printf("LoadTexture: Started fetch ID %ld for %s\n", fetchId, path.c_str());

			// Return the fetch ID as partial result
			return IntrinsicResult(Value((double)fetchId), false);
		} else {
			// Subsequent calls - check if fetch is complete
			long fetchId = (long)partialResult.Result().DoubleValue();
			auto it = activeFetches.find(fetchId);
			if (it == activeFetches.end()) {
				printf("LoadTexture: Fetch ID %ld not found!\n", fetchId);
				return IntrinsicResult::Null;
			}

			FetchData& data = it->second;

			if (!data.completed) {
				// Still loading
				return partialResult;
			}

			// Fetch is complete
			emscripten_fetch_t* fetch = data.fetch;
			printf("LoadTexture: Fetch ID %ld complete, status=%d for %s\n", fetchId, data.status, fetch->url);

			if (data.status == 200) {
				// Success - load image from memory then create texture
				const char* url = fetch->url;
				const char* ext = strrchr(url, '.');
				if (ext == nullptr) ext = ".png";

				printf("LoadTexture: Fetched %llu bytes from %s, ext=%s\n", fetch->numBytes, url, ext);

				Image img = LoadImageFromMemory(ext, (const unsigned char*)fetch->data, (int)fetch->numBytes);
				printf("LoadTexture: Image loaded: %dx%d, data=%p\n", img.width, img.height, img.data);

				if (img.data == nullptr) {
					printf("LoadTexture: Failed to load image from memory\n");
					emscripten_fetch_close(fetch);
					activeFetches.erase(it);
					return IntrinsicResult::Null;
				}

				Texture tex = LoadTextureFromImage(img);
				printf("LoadTexture: Texture created: id=%u, %dx%d\n", tex.id, tex.width, tex.height);
				UnloadImage(img);  // Don't need the CPU image anymore
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult(TextureToValue(tex));
			} else {
				// Error
				printf("LoadTexture: Fetch failed with status %d for %s\n", data.status, fetch->url);
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult::Null;
			}
		}
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
		if (partialResult.Done()) {
			// First call - start the async fetch
			String path = context->GetVar(String("fileName")).ToString();

			// Create a new fetch ID and entry
			long fetchId = nextFetchId++;
			FetchData& data = activeFetches[fetchId];

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
			attr.onsuccess = fetch_completed;
			attr.onerror = fetch_completed;

			data.fetch = emscripten_fetch(&attr, path.c_str());
			printf("LoadFont: Started fetch ID %ld for %s\n", fetchId, path.c_str());

			// Return the fetch ID as partial result
			return IntrinsicResult(Value((double)fetchId), false);
		} else {
			// Subsequent calls - check if fetch is complete
			long fetchId = (long)partialResult.Result().DoubleValue();
			auto it = activeFetches.find(fetchId);
			if (it == activeFetches.end()) {
				printf("LoadFont: Fetch ID %ld not found!\n", fetchId);
				return IntrinsicResult::Null;
			}

			FetchData& data = it->second;

			if (!data.completed) {
				// Still loading
				return partialResult;
			}

			// Fetch is complete
			emscripten_fetch_t* fetch = data.fetch;
			printf("LoadFont: Fetch ID %ld complete, status=%d for %s\n", fetchId, data.status, fetch->url);

			if (data.status == 200) {
				// Success - get file extension and load font from memory
				const char* url = fetch->url;
				const char* ext = strrchr(url, '.');
				if (ext == nullptr) ext = ".ttf";

				Font font = LoadFontFromMemory(ext, (const unsigned char*)fetch->data, (int)fetch->numBytes, 32, nullptr, 0);
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult(FontToValue(font));
			} else {
				// Error
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult::Null;
			}
		}
	};
	raylibModule.SetValue("LoadFont", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("fileName");
	i->AddParam("fontSize", Value(20));
	i->AddParam("codepoints", Value::null);
	i->AddParam("codepointCount", Value::zero);
	i->code = INTRINSIC_LAMBDA {
		if (partialResult.Done()) {
			// First call - start the async fetch
			String path = context->GetVar(String("fileName")).ToString();

			// Create a new fetch ID and entry
			long fetchId = nextFetchId++;
			FetchData& data = activeFetches[fetchId];

			emscripten_fetch_attr_t attr;
			emscripten_fetch_attr_init(&attr);
			strcpy(attr.requestMethod, "GET");
			attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
			attr.onsuccess = fetch_completed;
			attr.onerror = fetch_completed;

			data.fetch = emscripten_fetch(&attr, path.c_str());
			printf("LoadFontEx: Started fetch ID %ld for %s\n", fetchId, path.c_str());

			// Return the fetch ID as partial result
			return IntrinsicResult(Value((double)fetchId), false);
		} else {
			// Subsequent calls - check if fetch is complete
			long fetchId = (long)partialResult.Result().DoubleValue();
			auto it = activeFetches.find(fetchId);
			if (it == activeFetches.end()) {
				printf("LoadFontEx: Fetch ID %ld not found!\n", fetchId);
				return IntrinsicResult::Null;
			}

			FetchData& data = it->second;

			if (!data.completed) {
				// Still loading
				return partialResult;
			}

			// Fetch is complete
			emscripten_fetch_t* fetch = data.fetch;
			printf("LoadFontEx: Fetch ID %ld complete, status=%d for %s\n", fetchId, data.status, fetch->url);

			if (data.status == 200) {
				// Success - get file extension and load font from memory
				const char* url = fetch->url;
				const char* ext = strrchr(url, '.');
				if (ext == nullptr) ext = ".ttf";

				int fontSize = context->GetVar(String("fontSize")).IntValue();
				// For now, ignore codepoints parameter and load all
				Font font = LoadFontFromMemory(ext, (const unsigned char*)fetch->data, (int)fetch->numBytes, fontSize, nullptr, 0);
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult(FontToValue(font));
			} else {
				// Error
				emscripten_fetch_close(fetch);
				activeFetches.erase(it);
				return IntrinsicResult::Null;
			}
		}
	};
	raylibModule.SetValue("LoadFontEx", i->GetFunc());

	i = Intrinsic::Create("");
	i->AddParam("image");
	i->AddParam("key", ColorToValue(Color{255, 0, 255, 255}));
	i->AddParam("firstChar", Value(32));
	i->code = INTRINSIC_LAMBDA {
		Image image = ValueToImage(context->GetVar(String("image")));
		Color key = ValueToColor(context->GetVar(String("key")));
		int firstChar = context->GetVar(String("firstChar")).IntValue();
		Font font = LoadFontFromImage(image, key, firstChar);
		return IntrinsicResult(FontToValue(font));
	};
	raylibModule.SetValue("LoadFontFromImage", i->GetFunc());

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

	// Create and register the main raylib module
	f = Intrinsic::Create("raylib");
	f->code = INTRINSIC_LAMBDA {
		static ValueDict raylibModule;

		if (raylibModule.Count() == 0) {
			AddRCoreMethods(raylibModule);
			AddRShapesMethods(raylibModule);
			AddRTexturesMethods(raylibModule);
			AddRTextMethods(raylibModule);
			AddConstants(raylibModule);
		}

		return IntrinsicResult(raylibModule);
	};
}
