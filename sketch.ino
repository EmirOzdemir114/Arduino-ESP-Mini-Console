#include "sketch.h"


#if defined(ESP8266) || defined(ESP32)
void resetFunc(){
  ESP.restart();
}

#elif  defined(__AVR_ATmega2560__)
void(* resetFunc) () = 0;
#endif

enum {SNAKE, UNSTABLE, TANKS};

#ifdef USE_SSD1306
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RESET_PIN);
#endif

#ifdef USE_SH1106

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64,OLED_MOSI, OLED_CLK, OLED_DC, RESET_PIN, OLED_CS);
#endif

//GPIO pins corresponding to
//left, up, right, down, move, place/shoot , weapon list in that order.
const uint8_t buttons[7] = {33,34,35,32,25,26,27}; 
//const uint8_t buttons[7] = {5,4,3,6,7,14,15}; 

uint8_t selection;
unsigned long start_time = millis();

void setup() {
  #if defined(ESP8266) || defined(ESP32)
  Wire.begin(23,18);
  #endif
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  debugPrint("b");

  #ifdef USE_SSD1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    debugPrint(F("SSD1306 allocation failed"));
    for (;;);
  }
  #elif defined(USE_SH1106)

  if (!display.begin(0x3C, RESET_PIN)) { 
    debugPrint(F("SH1106 initialization failed"));
    for (;;);
  }
  #endif

  debugPrint("a");

  for (uint8_t i = 0; i < 7; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  debugPrint("a");

  //to turn the screen upside down
  //display.setRotation(2); 

  randomSeed(analogRead(A0));

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(OLED_WHITE);
  display.setCursor(0, 20);

  debugPrint("c");


  for (uint8_t i = 32; i > 5; i -= 3) {
    display.fillCircle(4, 32, i, OLED_INVERSE);
    display.display();
    delay(10);
  }
  debugPrint("d");

  selection = UNSTABLE;
  DrawMenu();
  debugPrint("a");
}



void loop(){

  uint8_t input = ReadButton();
  switch(input){
    case LEFT:
      break;

    case RIGHT:
        switch (selection) {
        case SNAKE:
        #include "snake.h" 
        Snake(); 
        break;
        
        case UNSTABLE: 
        #include "chainreaction.h"
        Unstable(); 
        break;
        
        case TANKS: 
        #include "tanks.h"
        Tanks(); 
        break;
       }
      break;

    case UP:
      selection = (selection+2)%3;
      DrawMenu();
      delay(100);
      break;

    case DOWN:
      selection = (selection+1)%3;
      DrawMenu();
      delay(100);

  }
  
  delay(50);
}

uint8_t ReadButton(){
  for (uint8_t i = 0 ; i < 7 ; i++) {
    if (digitalRead(buttons[i]) == LOW) return i;
  }
  return NONE;
}

void DrawMenu() {
  display.fillRect(40, 8, 84, 50, OLED_BLACK);
  display.setCursor(0, 12);
  display.println(F("       >Snake"));
  display.println(F("\n       >Unstable"));
  display.println(F("\n       >Pocket Tanks"));

  switch (selection) {
    case SNAKE: display.fillRect(40, 10, 50, 10, OLED_INVERSE); break;
    case UNSTABLE: display.fillRect(40, 26, 70, 10, OLED_INVERSE); break;
    case TANKS: display.fillRect(40, 42, 84, 10, OLED_INVERSE); break;
  }

  display.display();
}
