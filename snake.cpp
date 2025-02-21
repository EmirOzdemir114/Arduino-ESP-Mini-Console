#include "sketch.h"

#define WIDTH_S 42
#define HEIGHT_S 21
#define INITIAL_SIZE 6
#define INITIAL_SPEED 18
#define MAX_SPEED 3
#define MAX_SIZE 127
#define PIXEL_SIZE 3

typedef enum {
  START,
  RUNNING
} SnakeGameState;

typedef enum {
  sLEFT,
  sUP,
  sRIGHT,
  sDOWN
} Direction;

Direction dir, new_dir;
SnakeGameState state;
uint8_t snake_length = INITIAL_SIZE;
int8_t** snake = nullptr;  
int8_t* fruit = nullptr;   
uint8_t timer;
uint8_t speed = INITIAL_SPEED;

void GameSetup();
void FreeMemorySnake();
bool Move();
bool SnakeButtonPress();
void GameOver();
void FruitGenerate();
void DrawMap();
void DrawScore();
void DrawStart();

void Snake() {
  delay(100);
  display.clearDisplay();
  state = START;
  DrawStart();
  display.display();

  while (true) {
    switch (state) {
      case RUNNING:
        timer++;
        SnakeButtonPress();
        if (timer >= speed) {
          dir = new_dir;
          display.clearDisplay();
          if (Move()) {
            GameOver();
            return;
          }
          DrawMap();
          DrawScore();
          display.display();
          timer = 0;
        }
        break;

      case START:
        uint8_t temp = ReadButton();
        if (temp == UP) {
          GameSetup();
        } else if (temp == RIGHT) {
          FreeMemorySnake();
          resetFunc();
        }
        break;
    }
    delay(10);
  }
}


void GameSetup() {
  FreeMemorySnake(); 

  snake = new int8_t*[MAX_SIZE];
  fruit = new int8_t[2];

  if (!snake || !fruit) {
    debugPrint("Memory allocation failed!");
    FreeMemorySnake();
    return;
  }

  for (uint8_t i = 0; i < MAX_SIZE; i++) {
    snake[i] = new int8_t[2];
    if (!snake[i]) {
      debugPrint("Memory allocation failed!");
      FreeMemorySnake();
      return;
    }
  }

  display.clearDisplay();
  state = RUNNING;
  dir = sLEFT;
  new_dir = sLEFT;


  snake_length = INITIAL_SIZE;
  for (uint8_t i = 0; i < snake_length; i++) {
    snake[i][0] = WIDTH_S / 2 - i;
    snake[i][1] = HEIGHT_S / 2;
  }

  FruitGenerate();
  DrawMap();
  DrawScore();
  display.display();
  timer = 0;
}


void FreeMemorySnake() {
  if (snake) {
    for (uint8_t i = 0; i < MAX_SIZE; i++) {
      delete[] snake[i];
    }
    delete[] snake;
    snake = nullptr;
  }

  delete[] fruit;
  fruit = nullptr;

  debugPrint("Memory freed.");
}


bool SnakeButtonPress() {
  for (uint8_t i = 0; i < 4; i++) {
    if (digitalRead(buttons[i]) == LOW && i != ((int)dir + 2) % 4) {
      new_dir = (Direction)i;
      return true;
    }
  }
  return false;
}


bool Move() {
  int8_t x = snake[0][0];
  int8_t y = snake[0][1];

  switch (dir) {
    case sLEFT:  x--; break;
    case sRIGHT: x++; break;
    case sUP:    y--; break;
    case sDOWN:  y++; break;
  }


  if (x <= 0 && (8 <= y && y <= 12)) x = WIDTH_S - 1;
  else if (x >= WIDTH_S && (8 <= y && y <= 12)) x = 1;
  else if (y <= 0 && (17 <= x && x <= 23)) y = HEIGHT_S - 1;
  else if (y >= HEIGHT_S && (17 <= x && x <= 23)) y = 1;
  else if (!(x >= 0 && x < WIDTH_S && y >= 0 && y < HEIGHT_S)) {
    return true;
  }


  for (uint8_t i = snake_length - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = x;
  snake[0][1] = y;


  if (x == fruit[0] && y == fruit[1]) {
    snake_length++;
    if (speed >= MAX_SPEED && snake_length % 2 == 0) speed--;
    FruitGenerate();
  }


  for (uint8_t i = 1; i < snake_length; i++) {
    if (x == snake[i][0] && y == snake[i][1]) {
      return true;
    }
  }
  return false;
}

void FruitGenerate() {
  if (!fruit) return;

  fruit[0] = random(3, WIDTH_S - 3);
  fruit[1] = random(3, HEIGHT_S - 3);


  for (int i = 0; i < snake_length; i++) {
    if (fruit[0] == snake[i][0] && fruit[1] == snake[i][1]) {
      FruitGenerate();
      return;
    }
  }
}


void GameOver() {
  display.setTextSize(1);
  display.setTextColor(OLED_WHITE);
  display.setCursor(2, 2);
  display.println(F("     GAME OVER"));
  display.println(F("  Press UP to retry"));
  display.print(F("\n     Score: "));
  display.println(snake_length - INITIAL_SIZE);
  display.println(F("\nPress RIGHT to exit"));

  display.display();
  delay(300);
  state = START;
  new_dir = sLEFT;

  FreeMemorySnake();  
}


void DrawMap() {
  display.drawRect(0, 0, PIXEL_SIZE * WIDTH_S, PIXEL_SIZE * HEIGHT_S, OLED_WHITE);
  display.drawRect(WIDTH_S * PIXEL_SIZE - 1, 8 * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE * 5, OLED_BLACK);
  display.drawRect(0, 8 * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE * 5, OLED_BLACK);
  display.drawRect(17 * PIXEL_SIZE, HEIGHT_S * PIXEL_SIZE - 1, PIXEL_SIZE * 7, PIXEL_SIZE, OLED_BLACK);
  display.drawRect(17 * PIXEL_SIZE, 0, PIXEL_SIZE * 7, PIXEL_SIZE, OLED_BLACK);

  display.drawRect(fruit[0] * PIXEL_SIZE, fruit[1] * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, OLED_INVERSE);
  for (uint8_t i = 0; i < snake_length; i++) {
    display.fillRect(snake[i][0] * PIXEL_SIZE, snake[i][1] * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, OLED_WHITE);
  }
}


void DrawScore() {
  display.setTextSize(1);
  display.setTextColor(OLED_WHITE);
  display.setCursor(2, 2);
  display.println(snake_length - INITIAL_SIZE);
}


void DrawStart() {
  display.setTextSize(1);
  display.setTextColor(OLED_WHITE);
  display.setCursor(2, 2);
  display.println(F("Press UP to start! \n or RIGHT to exit"));
  delay(200);
}
