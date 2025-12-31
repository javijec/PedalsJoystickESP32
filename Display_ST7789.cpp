#include "Display_ST7789.h"

/**
 * @file Display_ST7789.cpp
 * @brief Controlador básico para pantallas con controlador ST7789 usando SPI.
 *
 * Este archivo contiene funciones de bajo nivel para inicializar la interfaz SPI,
 * enviar comandos y datos al display, y controlar la retroiluminación.
 * Los nombres de pines y configuraciones como SPIFreq, EXAMPLE_PIN_* y HORIZONTAL
 * se definen en el archivo de cabecera o en la configuración del proyecto.
 *
 * Documentación escrita en español (Doxygen-compatible).
 */

/**
 * Instancia de la clase SPI usada para el LCD.
 * Se construye con el bus FSPI (modo de hardware ESP32/ESP families).
 */
SPIClass LCDspi(FSPI);

/**
 * Macros de conveniencia para transferencias SPI.
 * SPI_WRITE envía un byte, SPI_WRITE_Word envía 16 bits.
 */
#define SPI_WRITE(_dat) LCDspi.transfer(_dat)
#define SPI_WRITE_Word(_dat) LCDspi.transfer16(_dat)

/**
 * @brief Inicializa la interfaz SPI para el LCD.
 *
 * Configura los pines SCLK, MISO y MOSI usados por la instancia SPI. Los
 * identificadores de pin se toman de las constantes EXAMPLE_PIN_NUM_*.
 */
void SPI_Init() {
  LCDspi.begin(EXAMPLE_PIN_NUM_SCLK, EXAMPLE_PIN_NUM_MISO, EXAMPLE_PIN_NUM_MOSI);
}

// Offsets y orientación en tiempo de ejecución (antes eran macros)
// Inicializar offsets desde la configuración central
int Offset_X = OFFSET_X_VERT;
int Offset_Y = OFFSET_Y_VERT;
uint8_t CurrentOrientation = ORIENTATION; // valor por defecto (desde Display_Config.h)

/**
 * @brief Ajusta la orientación del display en tiempo de ejecución.
 *
 * Escribe el registro MADCTL apropiado y ajusta Offset_X/Offset_Y para que
 * las coordenadas enviadas por las funciones de dibujo cuadren con el panel.
 */
void SetOrientation(uint8_t orient) {
  CurrentOrientation = orient;
  LCD_WriteCommand(0x36);
  if (orient == HORIZONTAL) {
    // Para horizontal usar MADCTL = 0xA0 según preferencia del usuario
    LCD_WriteData(0xA0);
    Offset_X = OFFSET_X_HORIZ;
    Offset_Y = OFFSET_Y_HORIZ;
  } else {
    // Para vertical usar MADCTL = 0x20
    LCD_WriteData(0x20);
    Offset_X = OFFSET_X_VERT;
    Offset_Y = OFFSET_Y_VERT;
  }
}

/**
 * @brief Envía un comando al controlador ST7789.
 *
 * El pin DC se pone en bajo para indicar que lo enviado es un comando.
 * La función abre una transacción SPI con las configuraciones definidas en SPIFreq.
 *
 * @param Cmd Byte que contiene el comando a enviar.
 */
void LCD_WriteCommand(uint8_t Cmd) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, LOW);
  SPI_WRITE(Cmd);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);
  LCDspi.endTransaction();
}

/**
 * @brief Envía un byte de datos al controlador ST7789.
 *
 * El pin DC se pone en alto para indicar que lo enviado es dato (no comando).
 *
 * @param Data Byte de datos a enviar.
 */
void LCD_WriteData(uint8_t Data) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH);
  SPI_WRITE(Data);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);
  LCDspi.endTransaction();
}

/**
 * @brief Envía 16 bits (un color en formato RGB565) por SPI.
 *
 * Utilizado habitualmente para escritura de píxeles en modo palabra.
 *
 * @param Data Valor de 16 bits a enviar.
 */
void LCD_WriteData_Word(uint16_t Data) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH);
  SPI_WRITE_Word(Data);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);
  LCDspi.endTransaction();
}

/**
 * @brief Transferencia de múltiples bytes por SPI (lectura/escritura simultánea).
 *
 * Esta función usa transferBytes para enviar un bloque de datos y opcionalmente
 * leer la respuesta en ReadData.
 *
 * @param SetData Puntero al buffer de datos a enviar.
 * @param ReadData Puntero al buffer donde almacenar datos leídos (puede ser NULL si no se requiere lectura).
 * @param Size Número de bytes a transferir.
 */
void LCD_WriteData_nbyte(uint8_t* SetData, uint8_t* ReadData, uint32_t Size) {
  LCDspi.beginTransaction(SPISettings(SPIFreq, MSBFIRST, SPI_MODE0));
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_DC, HIGH);
  LCDspi.transferBytes(SetData, ReadData, Size);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, HIGH);
  LCDspi.endTransaction();
}

/**
 * @brief Reinicia el display mediante los pines de control.
 *
 * Realiza una secuencia de pulsos en los pines CS y RST con pequeños delays
 * para asegurar el reset correcto del controlador.
 */
void LCD_Reset(void) {
  digitalWrite(EXAMPLE_PIN_NUM_LCD_CS, LOW);
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, LOW);
  delay(50);
  digitalWrite(EXAMPLE_PIN_NUM_LCD_RST, HIGH);
  delay(50);
}

/**
 * @brief Inicializa el controlador ST7789 y configura el display.
 *
 * Configura los pines como salidas, inicializa la retroiluminación, SPI y
 * envía la secuencia de comandos recomendada para poner el display en modo
 * operativo. Los valores y comandos usados siguen la hoja de datos del ST7789.
 */
void LCD_Init(void) {
  pinMode(EXAMPLE_PIN_NUM_LCD_CS, OUTPUT);
  pinMode(EXAMPLE_PIN_NUM_LCD_DC, OUTPUT);
  pinMode(EXAMPLE_PIN_NUM_LCD_RST, OUTPUT);
  Backlight_Init();
  SPI_Init();

  LCD_Reset();
  //************* Start Initial Sequence **********//
  // 0x11: Sleep OUT — despierta el controlador del estado de sueño
  LCD_WriteCommand(0x11);
  delay(120);

  // Configurar orientación por defecto usando la función runtime
  LCD_WriteCommand(0x36); // MADCTL será escrito por SetOrientation
  SetOrientation(CurrentOrientation);

  // 0x3A: Interface Pixel Format
  // 0x05 = 16 bits/píxel (RGB565). Es el formato usado por el resto del código.
  LCD_WriteCommand(0x3A);
  LCD_WriteData(0x05);

  // Comandos B0/B2/B7/BB: configuración de modos de panel, porches y timings
  // Estos valores son típicos para módulos ST7789 y ajustan parámetros internos
  // como frecuencia de cuadro, porches front/back y funciones específicas del panel.
  LCD_WriteCommand(0xB0);
  LCD_WriteData(0x00);
  LCD_WriteData(0xE8);

  LCD_WriteCommand(0xB2);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x0C);
  LCD_WriteData(0x00);
  LCD_WriteData(0x33);
  LCD_WriteData(0x33);

  LCD_WriteCommand(0xB7);
  LCD_WriteData(0x35);

  LCD_WriteCommand(0xBB);
  LCD_WriteData(0x35);

  // Comandos Cx: control de voltajes y ajustes de driving (vcom, gammas, etc.)
  LCD_WriteCommand(0xC0);
  LCD_WriteData(0x2C);

  LCD_WriteCommand(0xC2);
  LCD_WriteData(0x01);

  LCD_WriteCommand(0xC3);
  LCD_WriteData(0x13);

  LCD_WriteCommand(0xC4);
  LCD_WriteData(0x20);

  LCD_WriteCommand(0xC6);
  LCD_WriteData(0x0F);

  // D0/D6: ajustes de power control / VGH/VGL según el panel
  LCD_WriteCommand(0xD0);
  LCD_WriteData(0xA4);
  LCD_WriteData(0xA1);

  LCD_WriteCommand(0xD6);
  LCD_WriteData(0xA1);

  // E0 / E1: tablas de corrección de gamma (positive / negative)
  // Los múltiples bytes a continuación configuran la curva de gamma para
  // mejorar contraste y reproducción tonal del panel.
  LCD_WriteCommand(0xE0);
  LCD_WriteData(0xF0);
  LCD_WriteData(0x00);
  LCD_WriteData(0x04);
  LCD_WriteData(0x04);
  LCD_WriteData(0x04);
  LCD_WriteData(0x05);
  LCD_WriteData(0x29);
  LCD_WriteData(0x33);
  LCD_WriteData(0x3E);
  LCD_WriteData(0x38);
  LCD_WriteData(0x12);
  LCD_WriteData(0x12);
  LCD_WriteData(0x28);
  LCD_WriteData(0x30);

  LCD_WriteCommand(0xE1);
  LCD_WriteData(0xF0);
  LCD_WriteData(0x07);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x0B);
  LCD_WriteData(0x07);
  LCD_WriteData(0x28);
  LCD_WriteData(0x33);
  LCD_WriteData(0x3E);
  LCD_WriteData(0x36);
  LCD_WriteData(0x14);
  LCD_WriteData(0x14);
  LCD_WriteData(0x29);
  LCD_WriteData(0x32);

  // 0x21: Display Inversion ON — mejora uniformidad/contraste para algunos paneles
  LCD_WriteCommand(0x21);

  // Re-send Sleep OUT / Display ON sequence: asegurar que el controlador salga
  // del modo sleep y que la pantalla quede activada.
  LCD_WriteCommand(0x11);
  delay(120);
  LCD_WriteCommand(0x29); // 0x29: DISPLAY ON
}
/******************************************************************************/
/**
 * @brief Coloca el cursor (ventana de escritura) en el display.
 *
 * Según la orientación (HORIZONTAL) intercambia las coordenadas X/Y y aplica
 * offsets definidos en Offset_X / Offset_Y.
 *
 * @param Xstart Coordenada X inicial (pixel).
 * @param Ystart Coordenada Y inicial (pixel).
 * @param Xend   Coordenada X final (pixel).
 * @param Yend   Coordenada Y final (pixel).
 */
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend) {
  if (CurrentOrientation == HORIZONTAL) {
    // set the X coordinates
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);

    // set the Y coordinates
    LCD_WriteCommand(0x2B);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);
  } else {
    // set the X coordinates
    LCD_WriteCommand(0x2A);
    LCD_WriteData(Ystart >> 8);
    LCD_WriteData(Ystart + Offset_Y);
    LCD_WriteData(Yend >> 8);
    LCD_WriteData(Yend + Offset_Y);
    // set the Y coordinates
    LCD_WriteCommand(0x2B);
    LCD_WriteData(Xstart >> 8);
    LCD_WriteData(Xstart + Offset_X);
    LCD_WriteData(Xend >> 8);
    LCD_WriteData(Xend + Offset_X);
  }
  LCD_WriteCommand(0x2C);
}
/******************************************************************************/
/**
 * @brief Escribe un buffer de colores en una ventana del display.
 *
 * Calcula el número de bytes a enviar en base al ancho y alto de la ventana
 * y llama a LCD_WriteData_nbyte para transferir el buffer en formato RGB565
 * (uint16_t por píxel).
 *
 * @warning Esta implementación reserva un buffer local `Read_D` en la pila
 * cuya longitud depende de la ventana. Para ventanas grandes esto puede agotar
 * la pila. Considerar usar un buffer estático o enviar por partes si procede.
 *
 * @param Xstart Coordenada X inicial (pixel).
 * @param Ystart Coordenada Y inicial (pixel).
 * @param Xend   Coordenada X final (pixel).
 * @param Yend   Coordenada Y final (pixel).
 * @param color  Puntero a un buffer de uint16_t con los colores (RGB565) a escribir.
 */
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t* color) {
  uint16_t Show_Width = Xend - Xstart + 1;
  uint16_t Show_Height = Yend - Ystart + 1;
  uint32_t numBytes = Show_Width * Show_Height * sizeof(uint16_t);
  uint8_t Read_D[numBytes];
  LCD_SetCursor(Xstart, Ystart, Xend, Yend);
  LCD_WriteData_nbyte((uint8_t*)color, Read_D, numBytes);
}
// backlight
/**
 * @brief Nivel por defecto de la retroiluminación (0-100).
 */
uint8_t LCD_Backlight = 90;

/**
 * @brief Inicializa el controlador de PWM para la retroiluminación y aplica
 * el nivel por defecto.
 *
 * Usa ledcAttach/ledcWrite (API ESP32) con las constantes Frequency/Resolution
 * y el pin EXAMPLE_PIN_NUM_BK_LIGHT.
 */
void Backlight_Init(void) {
  ledcAttach(EXAMPLE_PIN_NUM_BK_LIGHT, PWM_FREQUENCY, Resolution);
  Set_Backlight(LCD_Backlight);  //0~100
}

/**
 * @brief Ajusta la intensidad de la retroiluminación.
 *
 * Convierte un valor 0..100 a la escala usada por ledcWrite. Si el valor
 * excede Backlight_MAX se imprime un aviso por consola.
 *
 * @param Light Nivel deseado de retroiluminación (0..100).
 */
void Set_Backlight(uint8_t Light)  //
{

  if (Light > Backlight_MAX)
    printf("Set Backlight parameters in the range of 0 to 100 \r\n");
  else {
    uint32_t Backlight = Light * 10;
    if (Backlight == 1000)
      Backlight = 1024;
    ledcWrite(EXAMPLE_PIN_NUM_BK_LIGHT, Backlight);
  }
}