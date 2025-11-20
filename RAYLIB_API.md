# MSRLWeb Raylib API Reference

This document lists all Raylib functions available in MSRLWeb, organized by module.

Generated from: src/R*.cpp module files

## Summary

- **rcore**: 88 functions
- **rshapes**: 69 functions
- **rtextures**: 114 functions
- **rtext**: 51 functions
- **raudio**: 60 functions
- **Total**: 382 functions

## Functions by Module

| rcore | rshapes | rtextures | rtext | raudio |
|-------|---------|-----------|-------|--------|
| BeginBlendMode | CheckCollisionCircleLine | BeginTextureMode | CodepointToUTF8 | CloseAudioDevice |
| BeginDrawing | CheckCollisionCircleRec | ColorAlpha | DrawFPS | CreateWave |
| BeginMode2D | CheckCollisionCircles | ColorAlphaBlend | DrawText | GetMasterVolume |
| BeginScissorMode | CheckCollisionLines | ColorBrightness | DrawTextCodepoint | GetMusicTimeLength |
| ClearBackground | CheckCollisionPointCircle | ColorContrast | DrawTextCodepoints | GetMusicTimePlayed |
| DisableCursor | CheckCollisionPointLine | ColorFromHSV | DrawTextEx | InitAudioDevice |
| EnableCursor | CheckCollisionPointPoly | ColorFromNormalized | DrawTextPro | IsAudioDeviceReady |
| EncodeDataBase64 | CheckCollisionPointRec | ColorIsEqual | GenImageFontAtlas | IsAudioStreamPlaying |
| EndBlendMode | CheckCollisionPointTriangle | ColorLerp | GetCodepoint | IsAudioStreamProcessed |
| EndDrawing | CheckCollisionRecs | ColorNormalize | GetCodepointCount | IsAudioStreamValid |
| EndMode2D | DrawCircle | ColorTint | GetCodepointNext | IsMusicStreamPlaying |
| EndScissorMode | DrawCircleGradient | ColorToHSV | GetCodepointPrevious | IsMusicValid |
| GetCameraMatrix2D | DrawCircleLines | ColorToInt | GetFontDefault | IsSoundPlaying |
| GetCharPressed | DrawCircleLinesV | DrawTexture | GetGlyphAtlasRec | IsSoundValid |
| GetClipboardImage | DrawCircleSector | DrawTextureEx | GetGlyphIndex | IsWaveValid |
| GetFPS | DrawCircleSectorLines | DrawTextureNPatch | GetGlyphInfo | LoadAudioStream |
| GetFrameTime | DrawCircleV | DrawTexturePro | GetTextBetween | LoadMusicStream |
| GetGamepadAxisCount | DrawEllipse | DrawTextureRec | IsFontValid | LoadMusicStreamFromMemory |
| GetGamepadAxisMovement | DrawEllipseLines | DrawTextureV | LoadCodepoints | LoadSound |
| GetGamepadButtonPressed | DrawEllipseLinesV | EndTextureMode | LoadFont | LoadSoundAlias |
| GetGamepadName | DrawEllipseV | Fade | LoadFontData | LoadSoundFromWave |
| GetGestureDetected | DrawLine | GenImageCellular | LoadFontEx | LoadWave |
| GetGestureDragAngle | DrawLineBezier | GenImageChecked | LoadFontFromImage | LoadWaveFromMemory |
| GetGestureDragVector | DrawLineDashed | GenImageColor | LoadFontFromMemory | LoadWaveSamples |
| GetGestureHoldDuration | DrawLineEx | GenImageGradientLinear | LoadTextLines | PauseAudioStream |
| GetGesturePinchAngle | DrawLineStrip | GenImageGradientRadial | LoadUTF8 | PauseMusicStream |
| GetGesturePinchVector | DrawLineV | GenImageGradientSquare | MeasureText | PauseSound |
| GetKeyPressed | DrawPixel | GenImagePerlinNoise | MeasureTextEx | PlayAudioStream |
| GetMouseDelta | DrawPixelV | GenImageText | SetTextLineSpacing | PlayMusicStream |
| GetMousePosition | DrawPoly | GenImageWhiteNoise | TextAppend | PlaySound |
| GetMouseWheelMove | DrawPolyLines | GenTextureMipmaps | TextCopy | ResumeAudioStream |
| GetMouseWheelMoveV | DrawPolyLinesEx | GetColor | TextFindIndex | ResumeMusicStream |
| GetMouseX | DrawRectangle | GetImageAlphaBorder | TextFormat | ResumeSound |
| GetMouseY | DrawRectangleGradientEx | GetImageColor | TextInsert | SeekMusicStream |
| GetRandomValue | DrawRectangleGradientH | GetPixelColor | TextIsEqual | SetAudioStreamBufferSizeDefault |
| GetRenderHeight | DrawRectangleGradientV | GetPixelDataSize | TextJoin | SetAudioStreamPan |
| GetRenderWidth | DrawRectangleLines | ImageAlphaClear | TextLength | SetAudioStreamPitch |
| GetScreenHeight | DrawRectangleLinesEx | ImageAlphaCrop | TextReplace | SetAudioStreamVolume |
| GetScreenToWorld2D | DrawRectanglePro | ImageAlphaMask | TextReplaceBetween | SetMasterVolume |
| GetScreenWidth | DrawRectangleRec | ImageAlphaPremultiply | TextSplit | SetMusicPan |
| GetTime | DrawRectangleRounded | ImageBlurGaussian | TextToCamel | SetMusicPitch |
| GetTouchPointCount | DrawRectangleRoundedLines | ImageClearBackground | TextToFloat | SetMusicVolume |
| GetTouchPointId | DrawRectangleRoundedLinesEx | ImageColorBrightness | TextToInteger | SetSoundPan |
| GetTouchPosition | DrawRectangleV | ImageColorContrast | TextToLower | SetSoundPitch |
| GetTouchX | DrawRing | ImageColorGrayscale | TextToPascal | SetSoundVolume |
| GetTouchY | DrawRingLines | ImageColorInvert | TextToSnake | StopAudioStream |
| GetWorldToScreen2D | DrawSplineBasis | ImageColorReplace | TextToUpper | StopMusicStream |
| HideCursor | DrawSplineBezierCubic | ImageColorTint | UnloadCodepoints | StopSound |
| IsCursorHidden | DrawSplineBezierQuadratic | ImageCopy | UnloadFont | UnloadAudioStream |
| IsCursorOnScreen | DrawSplineCatmullRom | ImageCrop | UnloadFontData | UnloadMusicStream |
| IsFileExtension | DrawSplineLinear | ImageDither | UnloadUTF8 | UnloadSound |
| IsGamepadAvailable | DrawSplineSegmentBasis | ImageDraw |  | UnloadSoundAlias |
| IsGamepadButtonDown | DrawSplineSegmentBezierCubic | ImageDrawCircle |  | UnloadWave |
| IsGamepadButtonPressed | DrawSplineSegmentBezierQuadratic | ImageDrawCircleLines |  | UnloadWaveSamples |
| IsGamepadButtonReleased | DrawSplineSegmentCatmullRom | ImageDrawCircleLinesV |  | UpdateAudioStream |
| IsGamepadButtonUp | DrawSplineSegmentLinear | ImageDrawCircleV |  | UpdateMusicStream |
| IsGestureDetected | DrawTriangle | ImageDrawLine |  | UpdateSound |
| IsKeyDown | DrawTriangleFan | ImageDrawLineEx |  | WaveCopy |
| IsKeyPressed | DrawTriangleLines | ImageDrawLineV |  | WaveCrop |
| IsKeyPressedRepeat | DrawTriangleStrip | ImageDrawPixel |  | WaveFormat |
| IsKeyReleased | GetCollisionRec | ImageDrawPixelV |  |  |
| IsKeyUp | GetShapesTexture | ImageDrawRectangle |  |  |
| IsMouseButtonDown | GetShapesTextureRectangle | ImageDrawRectangleLines |  |  |
| IsMouseButtonPressed | GetSplinePointBasis | ImageDrawRectangleRec |  |  |
| IsMouseButtonReleased | GetSplinePointBezierCubic | ImageDrawRectangleV |  |  |
| IsMouseButtonUp | GetSplinePointBezierQuad | ImageDrawText |  |  |
| IsWindowFocused | GetSplinePointCatmullRom | ImageDrawTextEx |  |  |
| IsWindowReady | GetSplinePointLinear | ImageDrawTriangle |  |  |
| LoadFileText | SetShapesTexture | ImageDrawTriangleEx |  |  |
| LoadRandomSequence |  | ImageDrawTriangleFan |  |  |
| OpenURL |  | ImageDrawTriangleLines |  |  |
| SetClipboardText |  | ImageDrawTriangleStrip |  |  |
| SetExitKey |  | ImageFlipHorizontal |  |  |
| SetGamepadMappings |  | ImageFlipVertical |  |  |
| SetGamepadVibration |  | ImageFormat |  |  |
| SetGesturesEnabled |  | ImageFromChannel |  |  |
| SetMouseCursor |  | ImageFromImage |  |  |
| SetMouseOffset |  | ImageKernelConvolution |  |  |
| SetMousePosition |  | ImageMipmaps |  |  |
| SetMouseScale |  | ImageResize |  |  |
| SetRandomSeed |  | ImageResizeCanvas |  |  |
| SetTargetFPS |  | ImageResizeNN |  |  |
| SetTraceLogLevel |  | ImageRotate |  |  |
| SetWindowIcon |  | ImageRotateCCW |  |  |
| SetWindowTitle |  | ImageRotateCW |  |  |
| ShowCursor |  | ImageText |  |  |
| TakeScreenshot |  | ImageTextEx |  |  |
| WaitTime |  | ImageToPOT |  |  |
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

*Generated on Sat Nov 15 10:49:54 MST 2025*
