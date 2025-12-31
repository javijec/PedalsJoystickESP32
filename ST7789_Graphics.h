#ifndef ST7789_GRAPHICS_H
#define ST7789_GRAPHICS_H

#include <Arduino.h>
#include "Display_ST7789.h"

/**
 * @file ST7789_Graphics.h
 * @brief Interfaz de alto nivel para dibujar en el display ST7789.
 *
 * Provee primitivas de dibujo, funciones de texto, utilidades de color y
 * control de brillo. Internamente usa las funciones de bajo nivel definidas
 * en Display_ST7789.h.
 */

// Configuración de la pantalla (proveniente de Display_Config.h)
#include "Display_Config.h"

// Colores básicos RGB565
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20
#define PINK    0xFC18
#define PURPLE  0x8010
#define BROWN   0xA145
#define GRAY    0x8410
#define LIGHTGRAY 0xC618
#define DARKGRAY  0x4208

// Alineación de texto
#define ALIGN_LEFT    0
#define ALIGN_CENTER  1
#define ALIGN_RIGHT   2

/**
 * @class ST7789_Graphics
 * @brief Clase que encapsula operaciones gráficas sobre el display.
 *
 * Uso típico:
 *   ST7789_Graphics display; // (ya existe instancia global 'display')
 *   display.begin(80);
 *   display.clearScreen(BLACK);
 */
class ST7789_Graphics {
private:
    bool initialized;
    
    // Fuente bitmap 5x8 - ACTUALIZADA CON SOPORTE COMPLETO ASCII (0-127)
    static const uint8_t font5x8[128][8];
    
public:
    ST7789_Graphics();

    /**
     * @brief Habilita/deshabilita suavizado ligero (anti-alias) para texto escalado.
     *
     * El suavizado que implementamos es una técnica ligera que dibuja píxeles
     * intermedios en los bordes de los bloques escalados para reducir el efecto
     * de dientes (aliasing) cuando se usa `size > 1` con la fuente bitmap.
     * @param enable true para habilitar, false para deshabilitar (por defecto: false)
     */
    void enableTextAA(bool enable) { textAAEnabled = enable; }
    
    // Inicialización
    /**
     * @brief Inicializa el driver de la pantalla y ajusta el brillo.
     * @param brightness Valor 0..100 para la retroiluminación.
     * @return true si la inicialización fue exitosa.
     */
    bool begin(uint8_t brightness = 80);
    /** @brief Indica si la pantalla ya fue inicializada. */
    bool isReady();
    
    // Control de brillo
    /** @brief Ajusta el brillo de la retroiluminación (0..100). */
    void setBrightness(uint8_t brightness);
    
    // Funciones básicas de dibujo
    /** @brief Rellena toda la pantalla con un color. */
    void clearScreen(uint16_t color = BLACK);
    /** @brief Fuerza un refresco completo del display (si la implementación lo requiere). */
    void forceRefresh(); // Forzar refresco completo del display
    /** @brief Dibuja un píxel en coordenadas x,y. */
    void drawPixel(int x, int y, uint16_t color);
    /** @brief Dibuja una línea entre dos puntos. */
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color);
    void drawHLine(int x, int y, int width, uint16_t color);
    void drawVLine(int x, int y, int height, uint16_t color);
    
    // Rectángulos
    void drawRect(int x, int y, int width, int height, uint16_t color);
    void fillRect(int x, int y, int width, int height, uint16_t color);
    void drawRoundRect(int x, int y, int width, int height, int radius, uint16_t color);
    void fillRoundRect(int x, int y, int width, int height, int radius, uint16_t color);
    
    // Círculos
    void drawCircle(int x, int y, int radius, uint16_t color);
    void fillCircle(int x, int y, int radius, uint16_t color);
    
    // Triángulos
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
    
    // Funciones de texto
    void drawChar(int x, int y, char c, uint16_t textColor, uint16_t bgColor = BLACK, uint8_t size = 1);
    void drawText(int x, int y, const String& text, uint16_t textColor, uint16_t bgColor = BLACK, uint8_t size = 1);
    void drawText(int x, int y, const char* text, uint16_t textColor, uint16_t bgColor = BLACK, uint8_t size = 1);
    
    // Texto con alineación
    void drawAlignedText(int y, const String& text, uint8_t alignment, uint16_t textColor, uint16_t bgColor = BLACK, uint8_t size = 1);
    void drawCenteredText(int y, const String& text, uint16_t textColor, uint16_t bgColor = BLACK, uint8_t size = 1);
    
    // Utilidades de texto
    int getTextWidth(const String& text, uint8_t size = 1);
    int getTextWidth(const char* text, uint8_t size = 1);
    int getCharWidth(uint8_t size = 1);
    int getCharHeight(uint8_t size = 1);
    /**
     * @brief Borra una franja horizontal centrada en la coordenada y.
     *
     * Útil para limpiar una línea donde se dibuja texto centrado.
     * @param y Coordenada vertical central de la franja a borrar.
     * @param size Tamaño de fuente (usa getCharHeight para calcular la altura).
     * @param bgColor Color de fondo para rellenar (por defecto BLACK).
     */
    void clearCenteredLine(int y, uint8_t size = 3, uint16_t bgColor = BLACK);
    
    // Funciones avanzadas
    void drawBitmap(int x, int y, const uint8_t* bitmap, int width, int height, uint16_t color, uint16_t bgColor = BLACK);
    void drawProgressBar(int x, int y, int width, int height, int progress, int maxProgress, 
                        uint16_t fillColor = GREEN, uint16_t bgColor = DARKGRAY, uint16_t borderColor = WHITE);
    
    // Efectos visuales
    void fadeScreen(uint16_t toColor, int steps = 50, int delayMs = 20);
    void scrollText(int y, const String& text, uint16_t textColor, uint16_t bgColor = BLACK, 
                   uint8_t size = 1, int speed = 100);
    
    // Utilidades de color
    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
    void getRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b);
    uint16_t interpolateColor(uint16_t color1, uint16_t color2, float ratio);
    
    // Información
    int getWidth() { return SCREEN_WIDTH; }
    int getHeight() { return SCREEN_HEIGHT; }
    
private:
    // Funciones auxiliares
    void drawCircleHelper(int x0, int y0, int r, uint8_t cornername, uint16_t color);
    void fillCircleHelper(int x0, int y0, int r, uint8_t cornername, int delta, uint16_t color);
    void swap(int& a, int& b);
    // Habilita suavizado ligero para texto escalado
    bool textAAEnabled;
};

// Instancia global para facilitar el uso
extern ST7789_Graphics display;

#endif // ST7789_GRAPHICS_H