/**
 * @file Display_ST7789.h
 * @brief Definiciones y prototipos para el controlador ST7789 (SPI).
 */
#pragma once
#include <Arduino.h>
#include <SPI.h>



/** Frecuencia SPI usada para transferencias al LCD. */
#define SPIFreq 80000000

// Pines SPI y control del LCD (ajustar si se usan otros pines)
#define EXAMPLE_PIN_NUM_MISO -1
#define EXAMPLE_PIN_NUM_MOSI 45
#define EXAMPLE_PIN_NUM_SCLK 40
#define EXAMPLE_PIN_NUM_LCD_CS 42
#define EXAMPLE_PIN_NUM_LCD_DC 41
#define EXAMPLE_PIN_NUM_LCD_RST 39
#define EXAMPLE_PIN_NUM_BK_LIGHT 46

/** PWM para control de retroiluminación. Frequency = Hz, Resolution = bits. */
#define PWM_FREQUENCY 1000  // PWM frequencyconst
#define Resolution 10
#define Backlight_MAX 100

// Use central configuration for dimensions and orientation
#include "Display_Config.h"




/** Offsets aplicados a las coordenadas para ajustar el área visible del panel.
 *  Ahora son variables para permitir cambiar la orientación en tiempo de ejecución.
 */
extern int Offset_X;
extern int Offset_Y;

/** Orientación actual en tiempo de ejecución (usar HORIZONTAL o VERTICAL). */
extern uint8_t CurrentOrientation;

/** Establece la orientación del display y ajusta MADCTL + offsets.
 *  @param orient HORIZONTAL (1) o VERTICAL (0)
 */
void SetOrientation(uint8_t orient);

/**
 * Nota: la detección automática fue eliminada. Use SetOrientation(HORIZONTAL)
 * o SetOrientation(VERTICAL) para configurar la pantalla.
 */

/** Nivel actual de retroiluminación (0..100). */
extern uint8_t LCD_Backlight;

// Prototipos de funciones públicas (documentadas en Display_ST7789.cpp)
/** Inicializa pines, SPI, retroiluminación y realiza la secuencia de arranque del display. */
void LCD_Init(void);
/** Realiza un reset físico del controlador mediante el pin RST. */
void LCD_Reset(void);
/** Establece la ventana (cursor) para la siguiente escritura de píxeles.
 *  x1,y1 - esquina superior/izquierda; x2,y2 - esquina inferior/derecha. */
void LCD_SetCursor(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
/** Escribe un buffer de colores (RGB565) en la ventana especificada. */
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t* color);

/** Enviar comando (8-bit) al controlador. */
void LCD_WriteCommand(uint8_t Cmd);
/** Enviar dato (8-bit) al controlador. */
void LCD_WriteData(uint8_t Data);
/** Enviar dato de 16-bit (usado para colores RGB565). */
void LCD_WriteData_Word(uint16_t Data);
/** Transferencia de múltiples bytes (envío/lectura simultánea). */
void LCD_WriteData_nbyte(uint8_t* SetData, uint8_t* ReadData, uint32_t Size);

/** Inicializa la retroiluminación (PWM) y aplica el nivel por defecto. */
void Backlight_Init(void);
/** Ajusta la intensidad de la retroiluminación (0..100). */
void Set_Backlight(uint8_t Light);