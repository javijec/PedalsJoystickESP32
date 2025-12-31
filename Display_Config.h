// Display_Config.h
// Archivo de configuración central para tamaño de panel y orientación
#pragma once

// Tamaño físico del panel (según especificación / módulo)
#define LCD_PHYSICAL_WIDTH 320
#define LCD_PHYSICAL_HEIGHT 172

// Orientación posibles
#define HORIZONTAL 0
#define ORIENTATION 0




// Dimensiones lógicas (dependen de la orientación seleccionada)
#if ORIENTATION == HORIZONTAL
#define LCD_WIDTH  LCD_PHYSICAL_WIDTH
#define LCD_HEIGHT LCD_PHYSICAL_HEIGHT
#else
#define LCD_WIDTH  LCD_PHYSICAL_HEIGHT
#define LCD_HEIGHT LCD_PHYSICAL_WIDTH
#endif

// Alias usado por la capa gráfica
#define SCREEN_WIDTH  LCD_WIDTH
#define SCREEN_HEIGHT LCD_HEIGHT

// Offsets por orientación (ajustar si el módulo requiere desplazamiento)
#ifndef OFFSET_X_HORIZ
#define OFFSET_X_HORIZ 0
#endif
#ifndef OFFSET_Y_HORIZ
#define OFFSET_Y_HORIZ 34
#endif
#ifndef OFFSET_X_VERT
#define OFFSET_X_VERT 34
#endif
#ifndef OFFSET_Y_VERT
#define OFFSET_Y_VERT 0
#endif
