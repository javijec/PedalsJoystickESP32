#include "ST7789_Graphics.h"
#include <math.h>

// Fuente bitmap 5x8 pixels - EXTENDIDA CON MINÚSCULAS
const uint8_t ST7789_Graphics::font5x8[128][8] = {
  // Caracteres 0-31 (caracteres de control - espacios en blanco)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 0
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 1
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 2
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 3
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 4
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 5
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 6
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 7
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 8
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 9
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 10
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 11
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 12
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 13
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 14
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 15
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 16
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 17
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 18
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 19
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 20
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 21
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 22
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 23
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 24
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 25
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 26
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 27
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 28
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 29
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 30
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // 31

  // Caracteres ASCII 32-126 (originales)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // ' ' (32)
  { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00 },  // '!' (33)
  { 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 },  // '"' (34)
  { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A, 0x00 },  // '#' (35)
  { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04, 0x00 },  // '$' (36)
  { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03, 0x00 },  // '%' (37)
  { 0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D, 0x00 },  // '&' (38)
  { 0x0C, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },  // '\'' (39)
  { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02, 0x00 },  // '(' (40)
  { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00 },  // ')' (41)
  { 0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00, 0x00 },  // '*' (42)
  { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00 },  // '+' (43)
  { 0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08, 0x00 },  // ',' (44)
  { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00 },  // '-' (45)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00 },  // '.' (46)
  { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 },  // '/' (47)
  { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E, 0x00 },  // '0' (48)
  { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00 },  // '1' (49)
  { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F, 0x00 },  // '2' (50)
  { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E, 0x00 },  // '3' (51)
  { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02, 0x00 },  // '4' (52)
  { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E, 0x00 },  // '5' (53)
  { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E, 0x00 },  // '6' (54)
  { 0x1F, 0x11, 0x01, 0x02, 0x04, 0x04, 0x04, 0x00 },  // '7' (55)
  { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00 },  // '8' (56)
  { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C, 0x00 },  // '9' (57)
  { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00, 0x00 },  // ':' (58)
  { 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x04, 0x08, 0x00 },  // ';' (59)
  { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 },  // '<' (60)
  { 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 },  // '=' (61)
  { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 },  // '>' (62)
  { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04, 0x00 },  // '?' (63)
  { 0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E, 0x00 },  // '@' (64)
  { 0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00 },  // 'A' (65)
  { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E, 0x00 },  // 'B' (66)
  { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E, 0x00 },  // 'C' (67)
  { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C, 0x00 },  // 'D' (68)
  { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F, 0x00 },  // 'E' (69)
  { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10, 0x00 },  // 'F' (70)
  { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F, 0x00 },  // 'G' (71)
  { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00 },  // 'H' (72)
  { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00 },  // 'I' (73)
  { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C, 0x00 },  // 'J' (74)
  { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 },  // 'K' (75)
  { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00 },  // 'L' (76)
  { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00 },  // 'M' (77)
  { 0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x00 },  // 'N' (78)
  { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00 },  // 'O' (79)
  { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10, 0x00 },  // 'P' (80)
  { 0x0E, 0x11, 0x11, 0x15, 0x12, 0x11, 0x0F, 0x00 },  // 'Q' (81)
  { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11, 0x00 },  // 'R' (82)
  { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E, 0x00 },  // 'S' (83)
  { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 },  // 'T' (84)
  { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00 },  // 'U' (85)
  { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00 },  // 'V' (86)
  { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11, 0x00 },  // 'W' (87)
  { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00 },  // 'X' (88)
  { 0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00 },  // 'Y' (89)
  { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F, 0x00 },  // 'Z' (90)
  { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E, 0x00 },  // '[' (91)
  { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 },  // '\' (92)
  { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E, 0x00 },  // ']' (93)
  { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 },  // '^' (94)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00 },  // '_' (95)
  { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },  // '`' (96)

  // LETRAS MINÚSCULAS (97-122)
  { 0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00 },  // 'a' (97)
  { 0x10, 0x10, 0x16, 0x19, 0x11, 0x19, 0x16, 0x00 },  // 'b' (98)
  { 0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F, 0x00 },  // 'c' (99)
  { 0x01, 0x01, 0x0D, 0x13, 0x11, 0x13, 0x0D, 0x00 },  // 'd' (100)
  { 0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00 },  // 'e' (101)
  { 0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08, 0x00 },  // 'f' (102)
  { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E, 0x00 },  // 'g' (103)
  { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 },  // 'h' (104)
  { 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E, 0x00 },  // 'i' (105)
  { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C, 0x00 },  // 'j' (106)
  { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00 },  // 'k' (107)
  { 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00 },  // 'l' (108)
  { 0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15, 0x00 },  // 'm' (109)
  { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00 },  // 'n' (110)
  { 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00 },  // 'o' (111)
  { 0x00, 0x00, 0x16, 0x19, 0x16, 0x10, 0x10, 0x00 },  // 'p' (112)
  { 0x00, 0x00, 0x0D, 0x13, 0x0D, 0x01, 0x01, 0x00 },  // 'q' (113)
  { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 },  // 'r' (114)
  { 0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E, 0x00 },  // 's' (115)
  { 0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06, 0x00 },  // 't' (116)
  { 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D, 0x00 },  // 'u' (117)
  { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00 },  // 'v' (118)
  { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00 },  // 'w' (119)
  { 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00 },  // 'x' (120)
  { 0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E, 0x00 },  // 'y' (121)
  { 0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F, 0x00 },  // 'z' (122)

  // Caracteres finales (123-127)
  { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02, 0x00 },  // '{' (123)
  { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 },  // '|' (124)
  { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08, 0x00 },  // '}' (125)
  { 0x00, 0x00, 0x00, 0x0C, 0x12, 0x06, 0x00, 0x00 },  // '~' (126)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }   // DEL (127)
};
// Instancia global
ST7789_Graphics display;

ST7789_Graphics::ST7789_Graphics() {
  initialized = false;
  textAAEnabled = false;
}

bool ST7789_Graphics::begin(uint8_t brightness) {
  try {
    LCD_Init();
    setBrightness(brightness);
    clearScreen();
    initialized = true;
    return true;
  } catch (...) {
    initialized = false;
    return false;
  }
}

bool ST7789_Graphics::isReady() {
  return initialized;
}

void ST7789_Graphics::setBrightness(uint8_t brightness) {
  if (brightness > 100) brightness = 100;
  Set_Backlight(brightness);
}

void ST7789_Graphics::clearScreen(uint16_t color) {
  if (!initialized) return;

  uint16_t colorBuffer[SCREEN_WIDTH];
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    colorBuffer[i] = color;
  }

  // Limpiar línea por línea para asegurar borrado completo
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    LCD_addWindow(0, y, SCREEN_WIDTH - 1, y, colorBuffer);
    if (y % 50 == 0) delayMicroseconds(100);  // Pausa cada 50 líneas
  }

  // Pausa adicional para asegurar que se complete
  delay(10);
}

void ST7789_Graphics::forceRefresh() {
  if (!initialized) return;

  // Resetear el display completamente
  LCD_Reset();
  delay(50);

  // Limpiar con negro
  clearScreen(BLACK);
}

void ST7789_Graphics::drawPixel(int x, int y, uint16_t color) {
  if (!initialized) return;
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
  LCD_addWindow(x, y, x, y, &color);
}

void ST7789_Graphics::fillRect(int x, int y, int width, int height, uint16_t color) {
  if (!initialized) return;
  if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
  if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
  if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
  if (width <= 0 || height <= 0) return;

  uint16_t colorBuffer[width];
  for (int i = 0; i < width; i++) {
    colorBuffer[i] = color;
  }

  for (int i = 0; i < height; i++) {
    LCD_addWindow(x, y + i, x + width - 1, y + i, colorBuffer);
  }
}

void ST7789_Graphics::drawHLine(int x, int y, int width, uint16_t color) {
  fillRect(x, y, width, 1, color);
}

void ST7789_Graphics::drawVLine(int x, int y, int height, uint16_t color) {
  fillRect(x, y, 1, height, color);
}

void ST7789_Graphics::drawRect(int x, int y, int width, int height, uint16_t color) {
  drawHLine(x, y, width, color);               // Top
  drawHLine(x, y + height - 1, width, color);  // Bottom
  drawVLine(x, y, height, color);              // Left
  drawVLine(x + width - 1, y, height, color);  // Right
}

void ST7789_Graphics::drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
  if (!initialized) return;

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    drawPixel(x0, y0, color);

    if (x0 == x1 && y0 == y1) break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void ST7789_Graphics::drawCircle(int x, int y, int radius, uint16_t color) {
  if (!initialized) return;

  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x1 = 0;
  int y1 = radius;

  drawPixel(x, y + radius, color);
  drawPixel(x, y - radius, color);
  drawPixel(x + radius, y, color);
  drawPixel(x - radius, y, color);

  while (x1 < y1) {
    if (f >= 0) {
      y1--;
      ddF_y += 2;
      f += ddF_y;
    }
    x1++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x + x1, y + y1, color);
    drawPixel(x - x1, y + y1, color);
    drawPixel(x + x1, y - y1, color);
    drawPixel(x - x1, y - y1, color);
    drawPixel(x + y1, y + x1, color);
    drawPixel(x - y1, y + x1, color);
    drawPixel(x + y1, y - x1, color);
    drawPixel(x - y1, y - x1, color);
  }
}

void ST7789_Graphics::fillCircle(int x, int y, int radius, uint16_t color) {
  drawVLine(x, y - radius, 2 * radius + 1, color);
  fillCircleHelper(x, y, radius, 3, 0, color);
}

void ST7789_Graphics::drawChar(int x, int y, char c, uint16_t textColor, uint16_t bgColor, uint8_t size) {
  if (!initialized) return;
  if (c < 0 || c > 127) return;  // Solo caracteres imprimibles

  int charIndex = c;  // Ajustar índice

  for (int row = 0; row < 8; row++) {
    uint8_t line = font5x8[charIndex][row];
    for (int col = 0; col < 5; col++) {
      if (line & (0x10 >> col)) {
        // Pixel del carácter
        if (size == 1) {
          drawPixel(x + col, y + row, textColor);
        } else {
          // Dibujar bloque escalado
          int bx = x + col * size;
          int by = y + row * size;
          fillRect(bx, by, size, size, textColor);
          // Suavizado ligero: si está habilitado, dibujar píxeles vecinos semi-translúcidos
          if (textAAEnabled) {
            // Ajuste simple: dibujar un contorno de 1 píxel mezclado alrededor del bloque
            // mezclando textColor y bgColor al 50% para simular anti-alias
            uint16_t blend = interpolateColor(textColor, bgColor, 0.5f);
            // izquierda
            fillRect(bx - 1, by, 1, size, blend);
            // derecha
            fillRect(bx + size, by, 1, size, blend);
            // arriba
            fillRect(bx, by - 1, size, 1, blend);
            // abajo
            fillRect(bx, by + size, size, 1, blend);
            // esquinas (opcional)
            fillRect(bx - 1, by - 1, 1, 1, blend);
            fillRect(bx + size, by - 1, 1, 1, blend);
            fillRect(bx - 1, by + size, 1, 1, blend);
            fillRect(bx + size, by + size, 1, 1, blend);
          }
        }
      } else if (bgColor != textColor) {
        // Fondo del carácter
        if (size == 1) {
          drawPixel(x + col, y + row, bgColor);
        } else {
          fillRect(x + col * size, y + row * size, size, size, bgColor);
        }
      }
    }
  }
}

void ST7789_Graphics::drawText(int x, int y, const String& text, uint16_t textColor, uint16_t bgColor, uint8_t size) {
  drawText(x, y, text.c_str(), textColor, bgColor, size);
}

void ST7789_Graphics::drawText(int x, int y, const char* text, uint16_t textColor, uint16_t bgColor, uint8_t size) {
  if (!initialized || !text) return;

  int currentX = x;
  int charWidth = getCharWidth(size);

  for (int i = 0; text[i] != '\0'; i++) {
    if (currentX + charWidth > SCREEN_WIDTH) break;

    drawChar(currentX, y, text[i], textColor, bgColor, size);
    currentX += charWidth;
  }
}

void ST7789_Graphics::drawAlignedText(int y, const String& text, uint8_t alignment, uint16_t textColor, uint16_t bgColor, uint8_t size) {
  int x = 0;
  int textWidth = getTextWidth(text, size);

  switch (alignment) {
    case ALIGN_LEFT:
      x = 0;
      break;
    case ALIGN_CENTER:
      x = (SCREEN_WIDTH - textWidth) / 2;
      break;
    case ALIGN_RIGHT:
      x = SCREEN_WIDTH - textWidth;
      break;
  }

  drawText(x, y, text, textColor, bgColor, size);
}

void ST7789_Graphics::drawCenteredText(int y, const String& text, uint16_t textColor, uint16_t bgColor, uint8_t size) {
  drawAlignedText(y, text, ALIGN_CENTER, textColor, bgColor, size);
}

int ST7789_Graphics::getTextWidth(const String& text, uint8_t size) {
  return getTextWidth(text.c_str(), size);
}

int ST7789_Graphics::getTextWidth(const char* text, uint8_t size) {
  if (!text) return 0;
  int len = strlen(text);
  return len * getCharWidth(size) - size;  // Quitar el último espacio
}

int ST7789_Graphics::getCharWidth(uint8_t size) {
  return 5 * size + size;  // 5 pixels + 1 de espacio
}

int ST7789_Graphics::getCharHeight(uint8_t size) {
  return 8 * size;
}

void ST7789_Graphics::drawProgressBar(int x, int y, int width, int height, int progress, int maxProgress,
                                      uint16_t fillColor, uint16_t bgColor, uint16_t borderColor) {
  if (!initialized) return;

  // Dibujar borde
  drawRect(x, y, width, height, borderColor);

  // Fondo
  fillRect(x + 1, y + 1, width - 2, height - 2, bgColor);

  // Calcular progreso
  if (maxProgress <= 0) return;
  int fillWidth = ((width - 2) * progress) / maxProgress;
  if (fillWidth < 0) fillWidth = 0;
  if (fillWidth > width - 2) fillWidth = width - 2;

  // Llenar progreso
  if (fillWidth > 0) {
    fillRect(x + 1, y + 1, fillWidth, height - 2, fillColor);
  }
}

void ST7789_Graphics::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void ST7789_Graphics::clearCenteredLine(int y, uint8_t size, uint16_t bgColor) {
  if (!initialized) return;
  int h = getCharHeight(size);
  int padding = 6;
  int top = y - h / 2 - padding;
  if (top < 0) top = 0;
  int height = h + padding * 2;
  fillRect(0, top, SCREEN_WIDTH, height, bgColor);
}

void ST7789_Graphics::drawRoundRect(int x, int y, int width, int height, int radius, uint16_t color) {
  // Líneas principales
  drawHLine(x + radius, y, width - 2 * radius, color);               // Top
  drawHLine(x + radius, y + height - 1, width - 2 * radius, color);  // Bottom
  drawVLine(x, y + radius, height - 2 * radius, color);              // Left
  drawVLine(x + width - 1, y + radius, height - 2 * radius, color);  // Right

  // Esquinas
  drawCircleHelper(x + radius, y + radius, radius, 1, color);
  drawCircleHelper(x + width - radius - 1, y + radius, radius, 2, color);
  drawCircleHelper(x + width - radius - 1, y + height - radius - 1, radius, 4, color);
  drawCircleHelper(x + radius, y + height - radius - 1, radius, 8, color);
}

uint16_t ST7789_Graphics::color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void ST7789_Graphics::getRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
  r = (color >> 8) & 0xF8;
  g = (color >> 3) & 0xFC;
  b = (color << 3) & 0xF8;
}

uint16_t ST7789_Graphics::interpolateColor(uint16_t color1, uint16_t color2, float ratio) {
  if (ratio <= 0) return color1;
  if (ratio >= 1) return color2;

  uint8_t r1, g1, b1, r2, g2, b2;
  getRGB(color1, r1, g1, b1);
  getRGB(color2, r2, g2, b2);

  uint8_t r = r1 + (r2 - r1) * ratio;
  uint8_t g = g1 + (g2 - g1) * ratio;
  uint8_t b = b1 + (b2 - b1) * ratio;

  return color565(r, g, b);
}

void ST7789_Graphics::fadeScreen(uint16_t toColor, int steps, int delayMs) {
  // Simplificado - llenar con color destino
  clearScreen(toColor);
  delay(delayMs * steps / 10);  // Simular fade
}

void ST7789_Graphics::scrollText(int y, const String& text, uint16_t textColor, uint16_t bgColor, uint8_t size, int speed) {
  int textWidth = getTextWidth(text, size);
  int charHeight = getCharHeight(size);

  for (int x = SCREEN_WIDTH; x > -textWidth; x -= 2) {
    fillRect(0, y, SCREEN_WIDTH, charHeight, bgColor);
    drawText(x, y, text, textColor, bgColor, size);
    delay(speed);
  }
}

// Funciones auxiliares privadas
void ST7789_Graphics::drawCircleHelper(int x0, int y0, int r, uint8_t cornername, uint16_t color) {
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void ST7789_Graphics::fillCircleHelper(int x0, int y0, int r, uint8_t cornername, int delta, uint16_t color) {
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x1) {
      drawVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
      drawVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
    }
    if (cornername & 0x2) {
      drawVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
      drawVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
    }
  }
}

void ST7789_Graphics::fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
  int a, b, y, last;

  // Ordenar coordenadas por Y
  if (y0 > y1) {
    swap(y0, y1);
    swap(x0, x1);
  }
  if (y1 > y2) {
    swap(y2, y1);
    swap(x2, x1);
  }
  if (y0 > y1) {
    swap(y0, y1);
    swap(x0, x1);
  }

  if (y0 == y2) {  // Handle awkward all-on-same-line case
    a = b = x0;
    if (x1 < a) a = x1;
    else if (x1 > b) b = x1;
    if (x2 < a) a = x2;
    else if (x2 > b) b = x2;
    drawHLine(a, y0, b - a + 1, color);
    return;
  }

  int dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
      dx12 = x2 - x1, dy12 = y2 - y1;
  int sa = 0, sb = 0;

  if (y1 == y2) last = y1;
  else last = y1 - 1;

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    if (a > b) swap(a, b);
    drawHLine(a, y, b - a + 1, color);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    if (a > b) swap(a, b);
    drawHLine(a, y, b - a + 1, color);
  }
}

void ST7789_Graphics::fillRoundRect(int x, int y, int width, int height, int radius, uint16_t color) {
  fillRect(x + radius, y, width - 2 * radius, height, color);
  fillCircleHelper(x + width - radius - 1, y + radius, radius, 1, height - 2 * radius - 1, color);
  fillCircleHelper(x + radius, y + radius, radius, 2, height - 2 * radius - 1, color);
}

void ST7789_Graphics::drawBitmap(int x, int y, const uint8_t* bitmap, int width, int height, uint16_t color, uint16_t bgColor) {
  if (!initialized || !bitmap) return;

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      int byteIndex = (j * width + i) / 8;
      int bitIndex = (j * width + i) % 8;

      if (bitmap[byteIndex] & (0x80 >> bitIndex)) {
        drawPixel(x + i, y + j, color);
      } else if (bgColor != color) {
        drawPixel(x + i, y + j, bgColor);
      }
    }
  }
}

void ST7789_Graphics::swap(int& a, int& b) {
  int temp = a;
  a = b;
  b = temp;
}