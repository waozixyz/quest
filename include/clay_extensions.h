#ifndef CLAY_EXTENSIONS_H
#define CLAY_EXTENSIONS_H

// Global extensions for Clay configurations
#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer; bool shadowEnabled; Clay_Color shadowColor; Clay_Vector2 shadowOffset; float shadowBlurRadius; float shadowSpread;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;

#endif // CLAY_EXTENSIONS_H