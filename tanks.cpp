#include "sketch.h"

#include <math.h>

#ifdef ESP8266
#define ANIM_DELAY 8

#elif defined(ESP32)
#define ANIM_DELAY 25

#elif defined(__AVR_ATmega2560__)
#define ANIM_DELAY 0

#endif

typedef enum {
  MOVE,
  AIM,
  SHOOT
} TankGameState;


typedef void (*ptrCollide)(int16_t, int8_t);
typedef void (*ptrShoot)();

typedef struct {
  char name[12];
  int8_t size = 1;
  uint8_t damage;
  ptrShoot Shoot;
  ptrCollide OnCollision;
} Weapon;

Weapon none, Bomb, Medium_Bomb, _Stream, Pepper, Wall, Jetpack, Mole;



typedef struct {
  int16_t score = 0;
  int8_t fuel = 20;
  int8_t x;
  int8_t y;
  int16_t angle;
  int8_t power = 60;
  int8_t currentWeapon = 0;
  int8_t p1x = 0;
  int8_t p1y = 0;
  int8_t p2x = 0;
  int8_t p2y = 0;
  Weapon* weapons[9];
} Tank;


     
int16_t* terrain = nullptr;     
Weapon** WeaponList = nullptr; 

Tank players[2];
uint8_t Input = NONE;
TankGameState GState;
int8_t turn = 0;
int8_t Windforce = 0;


#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.0174
#endif

#define player players[turn % 2]
#define otherplayer players[(turn + 1) % 2]
#define weapon player.weapons[player.currentWeapon]
#define GRAVITY 3


void strcopy(char* arr, const char* str, uint8_t len);
void DrawTank(uint8_t x, uint8_t y);
void DrawTerrain(uint8_t x, uint8_t lim, uint8_t h);
void Shoot();
void TankMove(uint8_t direction);
void DrawAim(bool erase);
void TankSetup();
int8_t Map1(int16_t x);
int8_t Map2(int16_t x);
int8_t Map3(int16_t x);
void Render();
void fBomb();
void fPepper();
void fStream();
void fWall();
void fJetpack();
void fMole();
void Explode(int16_t x, int8_t y);
void SummonWall(int16_t x, int8_t y);
void ReDrawTank();
void WeaponSelect();
void DrawOptions(char* sel1, char* sel2, char* sel3, uint8_t selection);
void WeaponSetup();
void DrawWeaponList();
void UnDrawWeaponList();
void DrawInfo();
void TankExplode(uint8_t x, uint8_t y);
void Intro();
void TankGameOver();

bool AllocateMemory();
void FreeMemoryTanks();

bool AllocateMemory() {
  terrain = new int16_t[SCREEN_WIDTH];
  WeaponList = new Weapon*[18];

  if (!players || !terrain || !WeaponList) {
    debugPrint(F("Memory allocation failed!"));
    FreeMemoryTanks();
    return false;
  }

  debugPrint(F("Memory allocated successfully"));
  return true;
}

void FreeMemoryTanks() {
  delete[] terrain;
  delete[] WeaponList;


  terrain = nullptr;
  WeaponList = nullptr;

  debugPrint(F("Memory freed"));
}

void strcopy(char* arr, const char* str, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    arr[i] = str[i];
  }
}

void DrawTank(uint8_t x, uint8_t y) {


  if (x >= otherplayer.x) {
    display.fillCircle(x, y - 5, 2, OLED_WHITE);
    display.drawPixel(x - 3, y - 7, OLED_WHITE);
    display.drawPixel(x - 2, y - 8, OLED_WHITE);

  } else {
    display.fillCircle(x - 2, y - 5, 2, OLED_WHITE);
    display.drawPixel(x + 1, y - 7, OLED_WHITE);
    display.drawPixel(x, y - 8, OLED_WHITE);
  }
  display.fillRoundRect(x - 5, y - 5, 9, 5, 1, OLED_WHITE);
  display.fillRoundRect(x - 4, y - 4, 7, 3, 1, OLED_BLACK);
}


void TerrainGenerate() {

  int8_t (*RandomMap[3])(int16_t) = { Map1, Map2, Map3 };

  int8_t i = random(0, 3);
  float y;

  display.clearDisplay();
  for (uint8_t x = 0; x <= 127; x++) {
    y = RandomMap[i](x);
    terrain[x] = y;
    display.drawFastVLine(x, y, 65 - y, OLED_WHITE);
    if (x % 3 == 0) display.display();
  }
  display.display();
}


void DrawTerrain(uint8_t x, uint8_t lim, uint8_t h) {
  for (x; x <= lim; x++) {
    display.drawFastVLine(x, terrain[x], h, OLED_WHITE);
  }
  debugPrint("f");
}

void TankMove(uint8_t direction) {



  display.fillRect(player.x - 5, player.y - 9, 9, 9, OLED_BLACK);
  DrawTerrain(player.x - 6, player.x + 5, 10);
  player.x = player.x + (direction - 1);
  player.y = terrain[player.x];
  if (abs(player.x - otherplayer.x) < 9) {
    DrawTank(players[0].x, players[0].y);
    DrawTank(players[1].x, players[1].y);
  } else {
    DrawTank(player.x, player.y);
  }
  display.display();
}



void Tanks() {

  if (!AllocateMemory()) {
    debugPrint(F("Skipping Tanks due to memory allocation failure"));
    return;
  }


  Intro();
  WeaponSetup();
  TerrainGenerate();
  TankSetup();

  Windforce = random(-15, 16);
  DrawInfo();
  display.display();
  turn = 0;


  GState = AIM;
  DrawAim(0);
  uint8_t wall = 0;

  while (1) {
    switch (GState) {
      case MOVE:
        Input = ReadButton();

        if (Input == BUTTON1 || player.fuel <= 0) {
          GState = AIM;
          delay(200);
          DrawAim(1);
        } else if (Input == LEFT || Input == RIGHT) {
          if ((abs(terrain[player.x + (Input - 1)] - terrain[player.x]) < 4)) {
            TankMove(Input);
            player.fuel -= 1;
            if (wall) {
              DrawTerrain(player.x - 5, player.x + 4, 50);
              display.display();
              wall--;
            }
          } else {
            wall = 3;
            DrawTerrain(player.x - 5, player.x + 4, 50);
            display.display();
          }
        } else if (Input == BUTTON3) {
          DrawWeaponList();
          break;
        }
        break;

      case AIM:

        Input = ReadButton();

        switch (Input) {
          case LEFT:
            player.angle++;
            DrawAim(0);
            break;

          case RIGHT:
            player.angle--;
            DrawAim(0);
            break;

          case UP:
            if (player.power < 100) player.power++;
            DrawAim(0);
            break;

          case DOWN:
            if (player.power > 1) player.power--;
            DrawAim(0);
            break;

          case BUTTON1:
            if (player.fuel > 0) {
              DrawAim(1);
              GState = MOVE;
              delay(400);
            }
            break;

          case BUTTON2:
            DrawAim(1);
            weapon->Shoot();
            weapon = &none;
            while ((weapon->size == -1) && !(turn >= 16)) player.currentWeapon = (player.currentWeapon + 1) % 9;
            delay(400);
            player.fuel = 20;
            Windforce = random(-15, 16);
            DrawInfo();
            ReDrawTank();
            turn++;
            ReDrawTank();
            if (turn == 18) TankGameOver();
            DrawAim(1);
            break;

          case BUTTON3:
            DrawWeaponList();
            break;

          case NONE:
            break;
        }
        break;

      case SHOOT:
        break;
    }

    delay(30);
  }
}




void WeaponSetup() {


  strcopy(none.name, "NONE", 5);
  none.size = -1;


  strcopy(Bomb.name, "Bomb", 5);
  Bomb.size = 2;
  Bomb.damage = 15;
  Bomb.Shoot = &fBomb;
  Bomb.OnCollision = &Explode;

  strcopy(Medium_Bomb.name, "TNT", 12);
  Medium_Bomb.size = 4;
  Medium_Bomb.damage = 25;
  Medium_Bomb.Shoot = &fBomb;
  Medium_Bomb.OnCollision = &Explode;


  strcopy(_Stream.name, "Stream", 7);
  _Stream.size = 2;
  _Stream.damage = 3;
  _Stream.Shoot = &fStream;


  strcopy(Pepper.name, "Pepper", 7);
  Pepper.size = 1;
  Pepper.damage = 4;
  Pepper.Shoot = &fPepper;
  Pepper.OnCollision = &Explode;

  strcopy(Wall.name, "Wall", 5);
  Wall.size = 3;
  Wall.damage = 0;
  Wall.Shoot = &fWall;
  Wall.OnCollision = &SummonWall;

  strcopy(Jetpack.name, "Jetpck", 7);
  Jetpack.Shoot = &fJetpack;
  Jetpack.size = 1;

  strcopy(Mole.name, "Mole", 5);
  Mole.size = 3;
  Mole.damage = 20;
  Mole.Shoot = &fMole;
  Mole.OnCollision = &Explode;

  display.clearDisplay();
  display.setCursor(50, 10);
  display.setTextSize(2);
  display.println(F("P1"));
  display.setTextSize(1);
  display.print(" Choose your weapons      with RIGHT");
  display.display();
  delay(2000);
  WeaponSelect();

  display.clearDisplay();
  display.setCursor(50, 10);
  display.setTextSize(2);
  display.println(F("P2"));
  display.setTextSize(1);
  display.print(" Choose your weapons      with RIGHT");
  display.display();
  turn++;
  delay(3000);
  WeaponSelect();
  display.clearDisplay();
  display.display();
}



void TankSetup() {



  players[0].x = 20;
  players[1].x = 108;

  players[0].y = terrain[players[0].x];
  players[1].y = terrain[players[1].x];

  players[0].angle = 0;
  players[1].angle = 120;

  players[0].score = 0;
  players[1].score = 0;

  players[0].fuel = 20;
  players[1].fuel = 20;

  players[0].currentWeapon = 0;
  players[1].currentWeapon = 0;



  DrawTank(player.x, player.y);
  DrawTank(otherplayer.x, otherplayer.y);
}

void DrawAim(bool erase) {

  display.drawTriangle(player.x, player.y - 5,
                       player.x + player.p1x, player.y + player.p1y,
                       player.x + player.p2x, player.y + player.p2y, OLED_INVERSE);

  if (erase) {
    display.display();
    return;
  }
  player.p1x = player.power * 0.5 * cos((player.angle + 4) * DEG_TO_RAD);
  player.p2x = player.power * 0.5 * cos((player.angle - 4) * DEG_TO_RAD);
  player.p1y = -player.power * 0.5 * sin((player.angle + 4) * DEG_TO_RAD) - 5;
  player.p2y = -player.power * 0.5 * sin((player.angle - 4) * DEG_TO_RAD) - 5;

  display.drawTriangle(player.x, player.y - 5,
                       player.x + player.p1x, player.y + player.p1y,
                       player.x + player.p2x, player.y + player.p2y, OLED_INVERSE);
  display.display();
}


//
// Some weapon functions are not yet optimized
//

void fBomb() {
  double xVel = player.power * cos(player.angle * DEG_TO_RAD) * 0.2;
  double yVel = -player.power * sin(player.angle * DEG_TO_RAD) * 0.2;
  float dt = 0.1, t = 0.2;
  int16_t x;
  int8_t y;

  do {
    x = round(player.x + (xVel * t) + (0.125 * Windforce / 5 * t * t));
    y = round(player.y - 6 + (yVel * t) + (0.5 * GRAVITY * t * t));
    t += 0.1;
    if (abs(otherplayer.x - x) <= 5 && otherplayer.y >= y && otherplayer.y - y <= 9) {
      break;
    }
    if (abs(player.x - x) <= 5 && player.y >= y && player.y - y <= 9 && t > 1) {
      break;
    }

  } while (y < terrain[x] && x >= 0 && x <= SCREEN_WIDTH);

  while (dt <= t) {
    int16_t dx = round(player.x + (xVel * dt) + (0.125 * Windforce / 5 * dt * dt));
    int16_t dy = round(player.y - 5 + (yVel * dt) + (0.5 * GRAVITY * dt * dt));
    display.fillCircle(dx, dy, 2, OLED_INVERSE);
    display.display();
    display.fillCircle(dx, dy, 2, OLED_INVERSE);
    dt += 0.35;
    delay(ANIM_DELAY);
  }
  display.display();

  Explode(x, y);
}


void fPepper() {
  double xVel[7];
  double yVel[7];
  int16_t x[7];
  int16_t y[7];


  for (uint8_t i = 0; i < 7; i++) {
    int16_t angle = player.angle + random(-10, 10);
    xVel[i] = player.power * cos(angle * DEG_TO_RAD) * 0.2;
    yVel[i] = -player.power * sin(angle * DEG_TO_RAD) * 0.2;
  }
  float dt = 0;
  float t[7] = { 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2 };

  for (uint8_t i = 0; i < 7; i++) {
    do {
      x[i] = round(player.x + (xVel[i] * t[i]) + (0.125 * Windforce / 5 * t[i] * t[i]));
      y[i] = round(player.y - 6 + (yVel[i] * t[i]) + (0.5 * GRAVITY * t[i] * t[i]));
      t[i] += 0.1;
      if (abs(otherplayer.x - x[i]) <= 5 && otherplayer.y - 6 < y[i]) {
        continue;
      }
      if (abs(player.x - x[i]) <= 5 && player.y - 6 < y[i]) {
        break;
      }
    } while (y[i] < terrain[x[i]] && x[i] >= 0 && x[i] <= SCREEN_WIDTH);
  }

  bool flag = 1;
  while (flag) {
    flag = 0;
    for (uint8_t i = 0; i < 7; i++) {
      if (dt >= t[i]) {
        continue;
      }
      x[i] = round(player.x + (xVel[i] * dt) + (0.125 * Windforce / 5 * dt * dt));
      y[i] = round(player.y - 5 + (yVel[i] * dt) + (0.5 * GRAVITY * dt * dt));
      display.drawPixel(x[i], y[i], OLED_INVERSE);

      flag = 1;
    }
    display.display();
    for (uint8_t i = 0; i < 7; i++) {
      display.drawPixel(x[i], y[i], OLED_INVERSE);
    }
    dt += 0.3;
    delay(ANIM_DELAY);
  }
  display.display();
  for (uint8_t i = 0; i < 7; i++) {
    Explode(round(player.x + (xVel[i] * t[i]) + (0.125 * Windforce / 5 * t[i] * t[i])), round(player.y - 5 + (yVel[i] * t[i]) + (0.5 * GRAVITY * t[i] * t[i])));
  }
  delay(300);
  display.display();
}

void fWall() {
  double xVel = player.power * cos(player.angle * DEG_TO_RAD) * 0.2;
  double yVel = -player.power * sin(player.angle * DEG_TO_RAD) * 0.2;
  float dt = 0.1, t = 0.2;
  int16_t x;
  int16_t y;


  do {
    x = round(player.x + (xVel * t) + (0.125 * Windforce / 5 * t * t));
    y = round(player.y - 6 + (yVel * t) + (0.5 * GRAVITY * t * t));
    t += 0.1;
  } while (y < terrain[x] && x >= 0 && x <= SCREEN_WIDTH);

  while (dt <= t) {
    int16_t dx = round(player.x + (xVel * dt) + (0.125 * Windforce / 5 * dt * dt));
    int16_t dy = round(player.y - 5 + (yVel * dt) + (0.5 * GRAVITY * dt * dt));
    display.fillRoundRect(dx, dy, 2 * weapon->size - 1, 3 * weapon->size - 1, weapon->size - 1, OLED_INVERSE);
    display.display();
    display.fillRoundRect(dx, dy, 2 * weapon->size - 1, 3 * weapon->size - 1, weapon->size - 1, OLED_INVERSE);
    dt += 0.25;
    delay(ANIM_DELAY);
  }
  display.display();

  SummonWall(x, y);
}

void fStream() {

  double xVel = player.power * cos(player.angle * DEG_TO_RAD) * 0.2;
  double yVel = -player.power * sin(player.angle * DEG_TO_RAD) * 0.2;
  float dt = 0.1, t = 0.1;
  int16_t x;
  int16_t y;
  bool hitself = 0, hitenemy = 0;

  do {
    x = round(player.x + (xVel * t) + (0.125 * Windforce / 5 * t * t));
    y = round(player.y - 5 + (yVel * t) + (0.5 * GRAVITY * t * t));
    t += 0.1;
    if (abs(otherplayer.x - x) <= 5 && otherplayer.y >= y && otherplayer.y - y <= 9) {
      hitenemy = 1;
    }
    if (abs(player.x - x) <= 5 && player.y >= y && player.y - y <= 9 && t > 1) {
      hitself = 1;
    }

  } while (y < terrain[x] && x >= 0 && x <= SCREEN_WIDTH);

  while (dt <= t) {
    x = round(player.x + (xVel * dt) + (0.125 * Windforce / 5 * dt * dt));
    y = round(player.y - 5 + (yVel * dt) + (0.5 * GRAVITY * dt * dt));
    display.drawCircle(x, y, 2, OLED_INVERSE);
    display.display();
    dt += 0.35;
    delay(ANIM_DELAY);
  }
  delay(100);
  dt = 0.1;
  while (dt <= t) {
    x = round(player.x + (xVel * dt) + (0.125 * Windforce / 5 * dt * dt));
    y = round(player.y - 5 + (yVel * dt) + (0.5 * GRAVITY * dt * dt));
    display.drawCircle(x, y, 2, OLED_INVERSE);
    display.display();
    dt += 0.35;
    delay(ANIM_DELAY);
  }

  otherplayer.score += hitself * t * weapon->damage * 0.7;
  player.score += hitenemy * t * weapon->damage * 0.7;
}

void fJetpack() {
  double xVel = player.power * cos(player.angle * DEG_TO_RAD) * 0.2;
  double yVel = -player.power * sin(player.angle * DEG_TO_RAD) * 0.2;
  float t = 0.2;
  display.fillRect(player.x - 5, player.y - 9, 9, 9, OLED_BLACK);
  int16_t x = player.x;
  player.y -= 5;
  int16_t y = player.y;


  do {
    display.fillRect(x - 5, y - 9, 9, 9, OLED_BLACK);
    DrawTerrain(x - 6, x + 5, 10);
    x = round(player.x + (xVel * t) + (0.125 * Windforce / 5 * t * t));
    y = round(player.y + (yVel * t) + (0.5 * GRAVITY * t * t));
    t += 0.15;
    DrawTank(x, y);
    display.display();
    if (!(x > 0 && x < SCREEN_WIDTH)) {
      display.fillRect(x - 5, y - 9, 9, 9, OLED_BLACK);
      return;
    }
    delay(ANIM_DELAY);
  } while (y < terrain[x]);

  player.x = x;
  player.y = y;
}

void fMole() {

  double xVel = player.power * cos(player.angle * DEG_TO_RAD) * 0.2;
  double yVel = -player.power * sin(player.angle * DEG_TO_RAD) * 0.2;
  double xVel2 = 0, yVel2 = 0;
  float dt = 0.1, t = 0.1, t2 = 0;
  bool hit = 0;
  int16_t x = 0, x2 = 0;
  int16_t y = 0, y2 = 0;


  do {
    x = round(player.x + (xVel * t) + (0.125 * Windforce / 5 * t * t));
    y = round(player.y - 6 + (yVel * t) + (0.5 * GRAVITY * t * t));
    t += 0.1;
    if (abs(otherplayer.x - x) <= 5 && otherplayer.y >= y && otherplayer.y - y <= 9) {
      hit = 1;
      break;
    }
    if (abs(player.x - x) <= 5 && player.y >= y && player.y - y <= 9 && t > 1) {
      hit = 1;
      break;
    }
  } while (y < terrain[x] && x >= 0 && x <= SCREEN_WIDTH);

  if (!hit) {
    xVel2 = xVel + 0.25 * Windforce / 5 * t;
    yVel2 = yVel + GRAVITY * t;
    do {

      x2 = round(x + (xVel2 * t2));
      y2 = round(y + (yVel2 * t2) - (0.5 * GRAVITY * t2 * t2));
      t2 += 0.1;

    } while ((y2 >= terrain[x2] - 1) && x2 >= 0 && x2 <= SCREEN_WIDTH);
  }

  while (dt <= t) {
    int16_t dx = round(player.x + (xVel * dt) + (0.125 * Windforce / 5 * dt * dt));
    int16_t dy = round(player.y - 5 + (yVel * dt) + (0.5 * GRAVITY * dt * dt));
    display.fillCircle(dx, dy, 2, OLED_INVERSE);
    display.display();
    display.fillCircle(dx, dy, 2, OLED_INVERSE);
    dt += 0.35;
    delay(ANIM_DELAY);
  }
  if (!hit) {
    dt = 0.1;
    while (dt <= t2) {
      int16_t dx = round(x + (xVel2 * dt));
      int16_t dy = round(y + (yVel2 * dt) - (0.5 * GRAVITY * dt * dt));
      display.fillCircle(dx, dy, 2, OLED_INVERSE);
      display.display();
      display.fillCircle(dx, dy, 2, OLED_INVERSE);
      dt += 0.35;
      delay(ANIM_DELAY);
    }
    display.display();
    Explode(x2, y2);
  } else {
    display.display();
    Explode(x, y);
  }
}

void Explode(int16_t x, int8_t y) {
  uint8_t radius = 2 * weapon->size;
  display.fillCircle(x, y, radius, OLED_BLACK);
  display.display();
  if (abs(otherplayer.x - x) <= radius + 5 && abs(otherplayer.y - y) <= radius + 5) {
    player.score += weapon->damage;
  }
  if (abs(player.x - x) <= 5 + radius && abs(player.y - y) < radius + 5) {
    otherplayer.score += weapon->damage;
  }

  uint8_t height;
  for (int8_t i = -radius; i <= radius; i++) {
    if (x + i <= 0 || x + i >= 128) continue;
    height = round(sqrt(radius * radius - i * i + 0.5));
    if (terrain[x + i] >= y + height)
      ;
    else if (terrain[x + i] > y - height) terrain[x + i] = y + height;
    else {
      display.drawFastVLine(x + i, terrain[x + i], 2 * height, OLED_BLACK);
      terrain[x + i] += 2 * height;
      display.drawFastVLine(x + i, terrain[x + i], 64 - terrain[x + i], OLED_WHITE);
    }
  }
}

void SummonWall(int16_t x, int8_t y) {
  int8_t temp;
  if (x + weapon->size >= 128) x = 127 - weapon->size;
  else if (x - weapon->size <= 0) x = weapon->size + 1;
  if (terrain[x] - 15 <= 12) temp = 12;
  else temp = terrain[x] - 15;

  for (int8_t i = -weapon->size; i < weapon->size; i++) {
    terrain[x + i] = temp;
    display.drawFastVLine(x + i, terrain[x + i], 50, OLED_WHITE);
    display.display();
  }
}

void ReDrawTank() {
  display.fillRect(player.x - 5, player.y - 9, 9, 9, OLED_BLACK);
  player.y = terrain[player.x];
  DrawTank(player.x, player.y);
  DrawTerrain(player.x - 6, player.x + 6, 50);
}

void WeaponSelect() {

  display.clearDisplay();
  display.display();
  Weapon* OptionList[27];
  DrawTank(14, 26);
  display.setTextSize(2);
  display.setCursor(5, 32);
  display.print(F("P"));
  display.print(turn % 2 + 1);
  display.setTextSize(1);

  Weapon* arr[10] = { &Bomb, &Bomb, &Bomb, &Pepper, &Pepper, &Medium_Bomb, &_Stream, &Wall, &Jetpack, &Mole };
  for (uint8_t i = 0; i < 27; i++) {
    uint8_t j = random(0, 10);
    OptionList[i] = arr[j];
  }

  uint8_t selection = 1;
  DrawOptions(OptionList[0]->name, OptionList[1]->name, OptionList[2]->name, selection);
  for (uint8_t i = 0; i < 9;) {

    Input = ReadButton();
    switch (Input) {
      case RIGHT:
        player.weapons[i] = OptionList[i + selection];
        i++;
        break;

      case UP:
        selection = (selection + 2) % 3;
        break;

      case DOWN:
        selection = (selection + 1) % 3;
        break;

      default:
        continue;
    }

    DrawOptions(OptionList[i]->name, OptionList[i + 1]->name, OptionList[i + 2]->name, selection);
    delay(200);
  }
}

void DrawOptions(char* sel1, char* sel2, char* sel3, uint8_t selection) {
  display.setCursor(0, 12);

  display.fillRect(32, 8, 84, 50, OLED_BLACK);
  display.print(F("      "));
  display.println(sel1);
  display.print(F("\n      "));
  display.println(sel2);
  display.print(F("\n      "));
  display.println(sel3);

  switch (selection) {
    case 0:
      display.fillRect(32, 10, 84, 10, OLED_INVERSE);
      break;

    case 1:
      display.fillRect(32, 26, 84, 10, OLED_INVERSE);
      break;

    case 2:
      display.fillRect(32, 42, 84, 10, OLED_INVERSE);
      break;
  }
  display.display();
}

void DrawWeaponList() {
  delay(100);
  display.clearDisplay();
  for (uint8_t i = 0; i < 3; i++) {
    for (uint8_t k = 0; k < 3; k++) {
      display.drawRect(k * 42 + 1, i * 21, 42, 21, OLED_WHITE);
      display.setCursor(k * 42 + 4, i * 21 + 2);
      display.print(player.weapons[3 * i + k]->name);
    }
  }
  display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
  display.display();
  delay(200);


  while (1) {
    Input = ReadButton();
    switch (Input) {
      case UP:
        display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
        player.currentWeapon = (player.currentWeapon + 6) % 9;
        delay(60);
        break;

      case LEFT:
        display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
        player.currentWeapon = (player.currentWeapon + 8) % 9;
        delay(60);
        break;

      case RIGHT:
        display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
        player.currentWeapon = (player.currentWeapon + 1) % 9;
        delay(60);
        break;

      case DOWN:
        display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
        player.currentWeapon = (player.currentWeapon + 3) % 9;
        delay(60);
        break;

      case NONE:
        continue;

      default:
        if (weapon->size != -1) {
          UnDrawWeaponList();
          delay(400);
          return;
        }
    }
    display.fillRect((player.currentWeapon % 3) * 42 + 1, (player.currentWeapon / 3) * 21, 42, 21, OLED_INVERSE);
    display.display();
    delay(100);
  }
}

void UnDrawWeaponList() {
  display.clearDisplay();
  DrawTerrain(0, 128, 64);
  DrawTank(player.x, player.y);
  DrawTank(otherplayer.x, otherplayer.y);
  DrawInfo();
  if (GState == AIM) DrawAim(1);
  display.display();
}


void DrawInfo() {
  display.setTextSize(1);
  display.fillRect(55, 1, 22, 9, OLED_BLACK);
  if (Windforce > 0) {
    display.setCursor(60, 1);
    display.print(abs(Windforce));
    display.print(F(">"));
  } else if (Windforce < 0) {
    display.setCursor(55, 1);
    display.print(F("<"));
    display.print(abs(Windforce));
  } else {
    display.setCursor(57, 1);
    display.print(abs(Windforce));
  }

  display.setCursor(1, 1);
  display.fillRect(18, 1, 19, 9, OLED_BLACK);
  display.print(F("P1:"));
  display.print(players[0].score);
  display.setCursor(90, 1);
  display.fillRect(107, 1, 19, 9, OLED_BLACK);
  display.print(F("P2:"));
  display.print(players[1].score);
}

void TankExplode(uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < 17; i++) {
    if (i % 2) display.fillCircle(x, y, 2 * i + random(0, 2), OLED_BLACK);
    else display.fillCircle(x, y, 2 * i + random(0, 2), OLED_WHITE);
    display.display();
  }
  display.fillCircle(x, y, 36, OLED_BLACK);
}

void TankGameOver() {
  display.setTextSize(2);
  delay(100);
  if (players[0].score > players[1].score) {
    TankExplode(players[1].x, players[1].y);
    delay(400);
    display.clearDisplay();
    display.display();
    delay(100);
    display.setCursor(32, 20);
    display.print(F("P1 Wins!"));
    delay(100);
    DrawTank(14, 36);
    display.display();
  } else if (players[0].score < players[1].score) {
    TankExplode(players[0].x, players[0].y);
    delay(400);
    display.clearDisplay();
    display.display();
    delay(100);
    display.setCursor(10, 20);
    display.print(F("P2 Wins!"));
    delay(100);
    DrawTank(116, 36);
    display.display();
  } else {
    delay(400);
    display.clearDisplay();
    display.display();
    display.setCursor(36, 20);
    delay(100);
    display.print(F("Draw"));
    delay(100);
    DrawTank(20, 36);
    display.display();
    delay(100);
    DrawTank(100, 36);
    display.display();
  }

  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(1, 20);
  display.print(F("    Press RIGHT to        go back or UP          to replay"));
  display.display();

  while (1) {
    Input = ReadButton();
    switch (Input) {
      case UP:
        resetFunc();
        break;

      case RIGHT:
        resetFunc();
        break;
    }
  }
}

int8_t Map1(int16_t x) {
  float c = 9 / 128.0;
  return 7 * (c * x - 7) * (c * x - 4) * (c * x - 1) * (c * x / 100 - 0.09) * (-c * x) + 35;
}

int8_t Map2(int16_t x) {
  if (x <= 30) return 25 + x;
  else if (x >= 98) return 25 + (128 - x);
  else return 55;
}

int8_t Map3(int16_t x) {
  if (x <= 30 || x >= 98) return 55;
  else return ((x - 64) * (x - 64) + 500) / 30;
}

void Intro() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(1, 1);
  display.println(F("A,B,C,D: Angle/Power/Move"));
  display.println(F("F: Shoot"));
  display.println(F("E: Move/Aim mode"));
  display.println(F("K: Change Weapon"));
  display.println(F("     Press UP           to continue"));
  display.display();
  delay(200);
  while (ReadButton() != UP) delay(10);
  display.clearDisplay();
  DrawInfo();
  display.setCursor(1, 15);
  display.println(F(" ^        ^      ^"));
  display.println(F(" P1   Windforce  P2"));
  display.println(F("Score           Score"));
  display.println(F("After 18 turns higher    score wins"));
  display.println(F("Press UP to continue"));
  display.display();
  delay(200);
  while (ReadButton() != UP) delay(10);
}