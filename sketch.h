#pragma once

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

// Uncomment the correct display type

//#define USE_SSD1306   // Use SSD1306
#define USE_SH1106  // Use SH1106

#ifdef USE_SSD1306
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;

#define OLED_WHITE SSD1306_WHITE
#define OLED_BLACK SSD1306_BLACK
#define OLED_INVERSE SSD1306_INVERSE

#endif

#ifdef USE_SH1106
#include <Adafruit_SH110X.h>
extern Adafruit_SH1106G display;

#define OLED_WHITE SH110X_WHITE
#define OLED_BLACK SH110X_BLACK
#define OLED_INVERSE SH110X_INVERSE

#endif

#define DEBUG  // Uncomment this for debugging

#ifdef DEBUG
#define debugPrint(x) Serial.println(x)
#else
#define debugPrint(x)
#endif



#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


#ifdef ESP8266
#define OLED_SDA D2
#define OLED_SCL D1
#define RESET_PIN -1


#define OLED_MOSI  D7  
#define OLED_CLK   D5  
#define OLED_DC    D2 
#define OLED_CS    D1 
#define OLED_RESET D0 



#elif defined(ESP32)

#define OLED_SDA 23
#define OLED_SCL 18
#define RESET_PIN 17

#define OLED_MOSI 23  
#define OLED_CLK  18  
#define OLED_DC   16 
#define OLED_CS   5   
#define OLED_RESET 17 


#elif defined(__AVR_ATmega2560__)
#define OLED_SDA 20
#define OLED_SCL 21
#define RESET_PIN 17

#define OLED_MOSI 51  
#define OLED_CLK  52 
#define OLED_DC   16 
#define OLED_CS   5   
#define OLED_RESET 17 


#endif



extern const uint8_t buttons[7];

typedef enum {
  LEFT,
  UP,
  RIGHT,
  DOWN,
  BUTTON1,
  BUTTON2,
  BUTTON3,
  NONE
} Button;

uint8_t ReadButton();

#if defined(ESP8266) || defined(ESP32)
void resetFunc();
#elif defined(__AVR_ATmega2560__)
extern void(* resetFunc) (void);
#endif

