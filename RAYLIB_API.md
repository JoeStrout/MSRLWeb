# MSRLWeb Raylib API Reference

This document lists all Raylib functions available in MSRLWeb, organized by module.

Generated from: src/R*.cpp module files

## Summary

- **rcore**: 80 functions
- **rshapes**: 61 functions
- **rtextures**: 114 functions
- **rtext**: 35 functions
- **raudio**: 58 functions
- **Total**: 348 functions

## Functions by Module

| rcore | rshapes | rtextures | rtext | raudio |
|-------|---------|-----------|-------|--------|
| BeginBlendMode | CheckCollisionCircleLine | BeginTextureMode | CodepointToUTF8 | CloseAudioDevice |
| BeginDrawing | CheckCollisionCircleRec | ColorAlpha | DrawFPS | GetMusicTimeLength |
| BeginMode2D | CheckCollisionCircles | ColorAlphaBlend | DrawText | GetMusicTimePlayed |
| BeginScissorMode | CheckCollisionLines | ColorBrightness | DrawTextCodepoint | InitAudioDevice |
| ClearBackground | CheckCollisionPointCircle | ColorContrast | DrawTextCodepoints | IsAudioDeviceReady |
| DisableCursor | CheckCollisionPointLine | ColorFromHSV | DrawTextEx | IsAudioStreamPlaying |
| EnableCursor | CheckCollisionPointPoly | ColorFromNormalized | DrawTextPro | IsAudioStreamProcessed |
| EncodeDataBase64 | CheckCollisionPointRec | ColorIsEqual | GenImageFontAtlas | IsAudioStreamValid |
| EndBlendMode | CheckCollisionPointTriangle | ColorLerp | GetCodepoint | IsMusicStreamPlaying |
| EndDrawing | CheckCollisionRecs | ColorNormalize | GetCodepointCount | IsMusicValid |
| EndMode2D | DrawCircle | ColorTint | GetCodepointNext | IsSoundPlaying |
| EndScissorMode | DrawCircleGradient | ColorToHSV | GetCodepointPrevious | IsSoundValid |
| GetCharPressed | DrawCircleLines | ColorToInt | GetFontDefault | IsWaveValid |
| GetFPS | DrawCircleLinesV | DrawTexture | GetGlyphAtlasRec | LoadAudioStream |
| GetFrameTime | DrawCircleSector | DrawTextureEx | GetGlyphIndex | LoadMusicStream |
| GetGamepadAxisCount | DrawCircleSectorLines | DrawTextureNPatch | GetGlyphInfo | LoadMusicStreamFromMemory |
| GetGamepadAxisMovement | DrawCircleV | DrawTexturePro | IsFontValid | LoadSound |
| GetGamepadButtonPressed | DrawEllipse | DrawTextureRec | LoadCodepoints | LoadSoundAlias |
| GetGamepadName | DrawEllipseLines | DrawTextureV | LoadFont | LoadSoundFromWave |
| GetGestureDetected | DrawEllipseLinesV | EndTextureMode | LoadFontData | LoadWave |
| GetGestureDragAngle | DrawEllipseV | Fade | LoadFontEx | LoadWaveFromMemory |
| GetGestureDragVector | DrawLine | GenImageCellular | LoadFontFromImage | LoadWaveSamples |
| GetGestureHoldDuration | DrawLineBezier | GenImageChecked | LoadFontFromMemory | PauseAudioStream |
| GetGesturePinchAngle | DrawLineEx | GenImageColor | LoadUTF8 | PauseMusicStream |
| GetGesturePinchVector | DrawLineStrip | GenImageGradientLinear | MeasureText | PauseSound |
| GetKeyPressed | DrawLineV | GenImageGradientRadial | MeasureTextEx | PlayAudioStream |
| GetMouseDelta | DrawPixel | GenImageGradientSquare | SetTextLineSpacing | PlayMusicStream |
| GetMousePosition | DrawPixelV | GenImagePerlinNoise | TextCopy | PlaySound |
| GetMouseWheelMove | DrawPoly | GenImageText | TextFormat | ResumeAudioStream |
| GetMouseWheelMoveV | DrawPolyLines | GenImageWhiteNoise | TextIsEqual | ResumeMusicStream |
| GetMouseX | DrawPolyLinesEx | GenTextureMipmaps | TextLength | ResumeSound |
| GetMouseY | DrawRectangle | GetColor | UnloadCodepoints | SeekMusicStream |
| GetRenderHeight | DrawRectangleGradientEx | GetImageAlphaBorder | UnloadFont | SetAudioStreamBufferSizeDefault |
| GetRenderWidth | DrawRectangleGradientH | GetImageColor | UnloadFontData | SetAudioStreamPan |
| GetScreenHeight | DrawRectangleGradientV | GetPixelColor | UnloadUTF8 | SetAudioStreamPitch |
| GetScreenWidth | DrawRectangleLines | GetPixelDataSize |  | SetAudioStreamVolume |
| GetTime | DrawRectangleLinesEx | ImageAlphaClear |  | SetMasterVolume |
| GetTouchPointCount | DrawRectanglePro | ImageAlphaCrop |  | SetMusicPan |
| GetTouchPointId | DrawRectangleRec | ImageAlphaMask |  | SetMusicPitch |
| GetTouchPosition | DrawRectangleRounded | ImageAlphaPremultiply |  | SetMusicVolume |
| GetTouchX | DrawRectangleRoundedLines | ImageBlurGaussian |  | SetSoundPan |
| GetTouchY | DrawRectangleRoundedLinesEx | ImageClearBackground |  | SetSoundPitch |
| HideCursor | DrawRectangleV | ImageColorBrightness |  | SetSoundVolume |
| IsCursorHidden | DrawRing | ImageColorContrast |  | StopAudioStream |
| IsCursorOnScreen | DrawRingLines | ImageColorGrayscale |  | StopMusicStream |
| IsFileExtension | DrawSplineBasis | ImageColorInvert |  | StopSound |
| IsGamepadAvailable | DrawSplineBezierCubic | ImageColorReplace |  | UnloadAudioStream |
| IsGamepadButtonDown | DrawSplineBezierQuadratic | ImageColorTint |  | UnloadMusicStream |
| IsGamepadButtonPressed | DrawSplineCatmullRom | ImageCopy |  | UnloadSound |
| IsGamepadButtonReleased | DrawSplineLinear | ImageCrop |  | UnloadSoundAlias |
| IsGamepadButtonUp | DrawSplineSegmentBasis | ImageDither |  | UnloadWave |
| IsGestureDetected | DrawSplineSegmentBezierCubic | ImageDraw |  | UnloadWaveSamples |
| IsKeyDown | DrawSplineSegmentBezierQuadratic | ImageDrawCircle |  | UpdateAudioStream |
| IsKeyPressed | DrawSplineSegmentCatmullRom | ImageDrawCircleLines |  | UpdateMusicStream |
| IsKeyPressedRepeat | DrawSplineSegmentLinear | ImageDrawCircleLinesV |  | UpdateSound |
| IsKeyReleased | DrawTriangle | ImageDrawCircleV |  | WaveCopy |
| IsKeyUp | DrawTriangleFan | ImageDrawLine |  | WaveCrop |
| IsMouseButtonDown | DrawTriangleLines | ImageDrawLineEx |  | WaveFormat |
| IsMouseButtonPressed | DrawTriangleStrip | ImageDrawLineV |  |  |
| IsMouseButtonReleased | GetCollisionRec | ImageDrawPixel |  |  |
| IsMouseButtonUp | SetShapesTexture | ImageDrawPixelV |  |  |
| IsWindowFocused |  | ImageDrawRectangle |  |  |
| IsWindowReady |  | ImageDrawRectangleLines |  |  |
| LoadFileText |  | ImageDrawRectangleRec |  |  |
| OpenURL |  | ImageDrawRectangleV |  |  |
| SetClipboardText |  | ImageDrawText |  |  |
| SetExitKey |  | ImageDrawTextEx |  |  |
| SetGamepadMappings |  | ImageDrawTriangle |  |  |
| SetGamepadVibration |  | ImageDrawTriangleEx |  |  |
| SetGesturesEnabled |  | ImageDrawTriangleFan |  |  |
| SetMouseCursor |  | ImageDrawTriangleLines |  |  |
| SetMouseOffset |  | ImageDrawTriangleStrip |  |  |
| SetMousePosition |  | ImageFlipHorizontal |  |  |
| SetMouseScale |  | ImageFlipVertical |  |  |
| SetTargetFPS |  | ImageFormat |  |  |
| SetWindowIcon |  | ImageFromChannel |  |  |
| SetWindowTitle |  | ImageFromImage |  |  |
| ShowCursor |  | ImageKernelConvolution |  |  |
| TakeScreenshot |  | ImageMipmaps |  |  |
| WaitTime |  | ImageResize |  |  |
|  |  | ImageResizeCanvas |  |  |
|  |  | ImageResizeNN |  |  |
|  |  | ImageRotate |  |  |
|  |  | ImageRotateCCW |  |  |
|  |  | ImageRotateCW |  |  |
|  |  | ImageText |  |  |
|  |  | ImageTextEx |  |  |
|  |  | ImageToPOT |  |  |
|  |  | IsImageValid |  |  |
|  |  | IsRenderTextureValid |  |  |
|  |  | IsTextureValid |  |  |
|  |  | LoadImage |  |  |
|  |  | LoadImageAnim |  |  |
|  |  | LoadImageAnimFromMemory |  |  |
|  |  | LoadImageColors |  |  |
|  |  | LoadImageFromMemory |  |  |
|  |  | LoadImageFromScreen |  |  |
|  |  | LoadImageFromTexture |  |  |
|  |  | LoadImagePalette |  |  |
|  |  | LoadImageRaw |  |  |
|  |  | LoadRenderTexture |  |  |
|  |  | LoadTexture |  |  |
|  |  | LoadTextureCubemap |  |  |
|  |  | LoadTextureFromImage |  |  |
|  |  | SetPixelColor |  |  |
|  |  | SetTextureFilter |  |  |
|  |  | SetTextureWrap |  |  |
|  |  | UnloadImage |  |  |
|  |  | UnloadImageColors |  |  |
|  |  | UnloadImagePalette |  |  |
|  |  | UnloadRenderTexture |  |  |
|  |  | UnloadTexture |  |  |
|  |  | UpdateTexture |  |  |
|  |  | UpdateTextureRec |  |  |

---

*Generated on Thu Nov 13 08:23:17 MST 2025*
