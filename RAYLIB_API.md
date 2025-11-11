# MSRLWeb Raylib API Reference

This document lists all Raylib functions available in MSRLWeb, organized by module.

Generated from: src/RaylibIntrinsics.cpp

## Summary

- **rcore**: 29 functions
- **rshapes**: 35 functions
- **rtextures**: 49 functions
- **rtext**: 12 functions
- **raudio**: 55 functions
- **Total**: 180 functions

## Functions by Module

| rcore | rshapes | rtextures | rtext | raudio |
|-------|---------|-----------|-------|--------|
| BeginDrawing | CheckCollisionCircleRec | BeginTextureMode | DrawFPS | CloseAudioDevice |
| ClearBackground | CheckCollisionCircles | DrawTexture | DrawText | GetMusicTimeLength |
| EndDrawing | CheckCollisionPointCircle | DrawTextureEx | DrawTextCodepoint | GetMusicTimePlayed |
| GetCharPressed | CheckCollisionPointRec | DrawTexturePro | DrawTextEx | InitAudioDevice |
| GetFPS | CheckCollisionPointTriangle | DrawTextureRec | DrawTextPro | IsAudioDeviceReady |
| GetFrameTime | CheckCollisionRecs | DrawTextureV | GetGlyphIndex | IsAudioStreamPlaying |
| GetKeyPressed | DrawCircle | EndTextureMode | LoadFont | IsAudioStreamProcessed |
| GetMouseDelta | DrawCircleLines | GenImageCellular | LoadFontEx | IsAudioStreamValid |
| GetMousePosition | DrawCircleV | GenImageChecked | LoadFontFromImage | IsMusicReady |
| GetMouseWheelMove | DrawEllipse | GenImageColor | MeasureText | IsMusicStreamPlaying |
| GetMouseX | DrawEllipseLines | GenImageGradientLinear | MeasureTextEx | IsSoundPlaying |
| GetMouseY | DrawLine | GenImageGradientRadial | UnloadFont | IsSoundReady |
| GetTime | DrawLineEx | GenImageGradientSquare |  | IsWaveReady |
| HideCursor | DrawLineV | GenImageWhiteNoise |  | LoadAudioStream |
| IsCursorHidden | DrawPixel | GenTextureMipmaps |  | LoadMusicStream |
| IsCursorOnScreen | DrawPixelV | ImageClearBackground |  | LoadMusicStreamFromMemory |
| IsKeyDown | DrawPoly | ImageColorBrightness |  | LoadSound |
| IsKeyPressed | DrawPolyLines | ImageColorContrast |  | LoadSoundAlias |
| IsKeyPressedRepeat | DrawPolyLinesEx | ImageColorGrayscale |  | LoadSoundFromWave |
| IsKeyReleased | DrawRectangle | ImageColorInvert |  | LoadWave |
| IsKeyUp | DrawRectangleGradientEx | ImageColorTint |  | LoadWaveFromMemory |
| IsMouseButtonDown | DrawRectangleGradientH | ImageCopy |  | PauseAudioStream |
| IsMouseButtonPressed | DrawRectangleGradientV | ImageCrop |  | PauseMusicStream |
| IsMouseButtonReleased | DrawRectangleLines | ImageDraw |  | PauseSound |
| IsMouseButtonUp | DrawRectangleLinesEx | ImageDrawCircle |  | PlayAudioStream |
| SetExitKey | DrawRectanglePro | ImageDrawCircleV |  | PlayMusicStream |
| SetMouseCursor | DrawRectangleRec | ImageDrawLine |  | PlaySound |
| SetTargetFPS | DrawRectangleRounded | ImageDrawLineV |  | ResumeAudioStream |
| ShowCursor | DrawRectangleRoundedLines | ImageDrawPixel |  | ResumeMusicStream |
|  | DrawRectangleV | ImageDrawPixelV |  | ResumeSound |
|  | DrawRing | ImageDrawRectangle |  | SeekMusicStream |
|  | DrawRingLines | ImageDrawRectangleLines |  | SetAudioStreamBufferSizeDefault |
|  | DrawTriangle | ImageDrawRectangleRec |  | SetAudioStreamPan |
|  | DrawTriangleLines | ImageDrawText |  | SetAudioStreamPitch |
|  | GetCollisionRec | ImageFlipHorizontal |  | SetAudioStreamVolume |
|  |  | ImageFlipVertical |  | SetMasterVolume |
|  |  | ImageResize |  | SetMusicPan |
|  |  | ImageResizeNN |  | SetMusicPitch |
|  |  | ImageRotateCCW |  | SetMusicVolume |
|  |  | ImageRotateCW |  | SetSoundPan |
|  |  | LoadImage |  | SetSoundPitch |
|  |  | LoadRenderTexture |  | SetSoundVolume |
|  |  | LoadTexture |  | StopAudioStream |
|  |  | LoadTextureFromImage |  | StopMusicStream |
|  |  | SetTextureFilter |  | StopSound |
|  |  | SetTextureWrap |  | UnloadAudioStream |
|  |  | UnloadImage |  | UnloadMusicStream |
|  |  | UnloadRenderTexture |  | UnloadSound |
|  |  | UnloadTexture |  | UnloadSoundAlias |
|  |  |  |  | UnloadWave |
|  |  |  |  | UpdateAudioStream |
|  |  |  |  | UpdateMusicStream |
|  |  |  |  | WaveCopy |
|  |  |  |  | WaveCrop |
|  |  |  |  | WaveFormat |

---

*Generated on Tue Nov 11 08:01:24 MST 2025*
