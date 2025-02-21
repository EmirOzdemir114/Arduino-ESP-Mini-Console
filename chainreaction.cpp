#include "sketch.h"
#include "chainreaction.h"

#define X 8
#define Y 5


typedef enum{
  CIRCLE,
  TRIANGLE
}Team;

struct cell{
  uint8_t val=0;
  uint8_t max;
  uint8_t team = 0;
};


uint8_t Turn;
cell** board;


void Unstable();
cell** BoardInit();
void FreeBoard();
void React();
void CheckGameOver();


void DrawCell(uint8_t j ,uint8_t i){
      display.fillRect(15*j+1,13*i,13,12,OLED_BLACK);
      if(board[i][j].val == 1){
        if(board[i][j].team == 0){
          display.fillCircle(15*j + 7 , 13*i + 6,3,OLED_WHITE);
        }
        else{
          display.fillCircle(15*j + 7 , 13*i + 6,3,OLED_WHITE);
          display.fillCircle(15*j + 7 , 13*i + 6,2,OLED_BLACK);
        }
      }

      else if(board[i][j].val == 2){
        if(board[i][j].team == 0){
          display.fillCircle(15*j + 5 , 13*i + 7,2,OLED_WHITE);
          display.fillCircle(15*j + 10 , 13*i + 5,2,OLED_WHITE);
          
        }
        else{
          display.fillCircle(15*j + 5 , 13*i + 5,2,OLED_WHITE);
          display.fillCircle(15*j + 10 , 13*i + 7,2,OLED_WHITE);
          display.fillRect(15*j + 4 , 13*i + 4,3,3,OLED_BLACK);
          display.fillRect(15*j + 9 , 13*i + 6,3,3,OLED_BLACK);
        }

      }

      else if(board[i][j].val == 3){
        if(board[i][j].team == 0){
          display.fillCircle(15*j + 5 , 13*i + 8,2,OLED_WHITE);
          display.fillCircle(15*j + 8 , 13*i + 3,2,OLED_WHITE);
          display.fillCircle(15*j + 10 , 13*i + 7,2,OLED_WHITE);

          
        }
        else{
          display.fillCircle(15*j + 4 , 13*i + 4,2,OLED_WHITE);
          display.fillCircle(15*j + 7 , 13*i + 8,2,OLED_WHITE);
          display.fillCircle(15*j + 11 , 13*i + 6,2,OLED_WHITE);
          display.fillRect(15*j + 3 , 13*i + 3,3,3,OLED_BLACK);
          display.fillRect(15*j + 6 , 13*i + 7,3,3,OLED_BLACK);
          display.fillRect(15*j + 10 , 13*i + 5,3,3,OLED_BLACK);
          
        }
      }
}

void DrawBoard(){
  for(uint8_t i = 0; i<Y;i++){
    for(uint8_t j = 0; j<X; j++){
      DrawCell(j,i);
  }
}
 display.display();
}

void Unstable(){
  uint8_t Input, px = 0, py = 0;
  Turn = 0;

  board = BoardInit();
  if (!board) {
    debugPrint("Memory allocation failed!");
    return;
  }

  

  display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
  display.display();


  while(1){
    

    Input = ReadButton();
    switch(Input){
      case LEFT:
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        px = (px + X -1 ) % X;
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        display.display();
        delay(100);
        break;

      case RIGHT:
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        px = (px+1) % X;
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        display.display();
        delay(100);
        break;

      case UP:
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        py = (py + Y -1) % Y;
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        display.display();
        delay(100);
        break;

      case DOWN:
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        py = (py+1) % Y;
        display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
        display.display();
        delay(100);
        break;

      case NONE:
        break;

      default:
        if((board[py][px].team == Turn%2) || board[py][px].val == 0){
          display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
          board[py][px].val++;
          board[py][px].team = Turn%2;
          if(board[py][px].val >= board[py][px].max){ 
            React();
          }
          else{
          
          DrawCell(px,py);
          }
          display.drawRect(15*px+1,13*py,14,12,OLED_INVERSE);
          display.display();
          Turn++;
          
        }

    }
    delay(50);
  }
}

cell** BoardInit() {
  display.clearDisplay();
  for (uint8_t i = 0; i < 9; i++) {
    display.drawFastVLine(i * 15, 0, 64, OLED_WHITE);
  }
  for (uint8_t k = 0; k < 6; k++) {
    display.drawFastHLine(0, k * 13 - 1, 121, OLED_WHITE);
  }
  display.display();

  
  cell** board = new cell*[Y];
  if (!board) return nullptr;  

  for (uint8_t i = 0; i < Y; i++) {
    board[i] = new cell[X];
    if (!board[i]) {
      FreeBoard(); 
      return nullptr;
    }

    for (uint8_t j = 0; j < X; j++) {
      if ((i == Y - 1 || i == 0) && (j == X - 1 || j == 0)) board[i][j].max = 2;
      else if (i > 0 && i < Y - 1 && j > 0 && j < X - 1) board[i][j].max = 4;
      else board[i][j].max = 3;
    }
  }

  return board;
}

void FreeBoard() {
    if (board) {
        for (uint8_t i = 0; i < Y; i++) {
            delete[] board[i];
        }
        delete[] board;
        board = nullptr;
    }
    debugPrint("Memory freed.");
}


void React(){
  bool flag=0;
  uint8_t up[30][2],down[30][2],left[30][2],right[30][2];
  uint8_t i_up = 0, i_down = 0, i_left = 0, i_right = 0;

  for(uint8_t i = 0; i<Y;i++){
    for(uint8_t j = 0; j<X; j++){
      if(board[i][j].val >= board[i][j].max){

        if(i > 0){
          up[i_up][0] = j;
          up[i_up][1] = i;
          i_up++;
        }
        if(i < Y-1){
          down[i_down][0] = j;
          down[i_down][1] = i;
          i_down++;
        }
        if(j > 0){
          left[i_left][0] = j;
          left[i_left][1] = i;
          i_left++;
        }
        if(j < X-1){
          right[i_right][0] = j;
          right[i_right][1] = i;
          i_right++;
        }
        board[i][j].val -= board[i][j].max;
        display.fillRect(15*j+1,13*i,13,12,OLED_BLACK);
        flag = 1;  
      }
    }
  } 

  if(!flag) return;

  for(uint8_t k = 0; k<5 ; k++){
    for(uint8_t i = 0; i< i_up;i++){
      display.fillCircle(up[i][0]*15+7,up[i][1]*13+6 - 3*k,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_down;i++){
      display.fillCircle(down[i][0]*15+7,down[i][1]*13+6 + 3*k,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_left;i++){
      display.fillCircle(left[i][0]*15+7 -3*k ,left[i][1]*13+6,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_right;i++){
      display.fillCircle(right[i][0]*15+7 + 3*k,right[i][1]*13+6,2,OLED_INVERSE);
    }

    display.display();
    delay(40);

    for(uint8_t i = 0; i< i_up;i++){
      display.fillCircle(up[i][0]*15+7,up[i][1]*13+6 - 3*k,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_down;i++){
      display.fillCircle(down[i][0]*15+7,down[i][1]*13+6 + 3*k,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_left;i++){
      display.fillCircle(left[i][0]*15+7 -3*k ,left[i][1]*13+6,2,OLED_INVERSE);
    }

    for(uint8_t i = 0; i< i_right;i++){
      display.fillCircle(right[i][0]*15+7 + 3*k,right[i][1]*13+6,2,OLED_INVERSE);
    }
  }

  for(uint8_t i = 0; i< i_up;i++){
      board[up[i][1]-1][up[i][0]].val++;
      board[up[i][1]-1][up[i][0]].team = Turn%2;
    }

    for(uint8_t i = 0; i< i_down;i++){
      board[down[i][1]+1][down[i][0]].val++;
      board[down[i][1]+1][down[i][0]].team = Turn%2;
    }

    for(uint8_t i = 0; i< i_left;i++){
      board[left[i][1]][left[i][0]-1].val++;
      board[left[i][1]][left[i][0]-1].team = Turn%2;
    }

    for(uint8_t i = 0; i< i_right;i++){
      board[right[i][1]][right[i][0]+1].val++;
      board[right[i][1]][right[i][0]+1].team = Turn%2;
    }
  

  DrawBoard();
  CheckGameOver();
  React();

}

void CheckGameOver(){
  if(Turn <2) return;
  bool flag1 = 1, flag2 = 1;

  for(uint8_t i = 0; i<Y;i++){
    for(uint8_t j = 0; j<X; j++){
        if(board[i][j].val == 0) continue;
        else if(board[i][j].team != 0) flag1 = 0;
        else if(board[i][j].team != 1) flag2 = 0;

    }
  }

  if(flag1){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20,30);
    display.print(F("P1 Wins!"));
    display.display();

  }
  else if(flag2){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20,30);
    display.print(F("P2 Wins!"));
    display.display();
    
  }
  FreeBoard();
  while (ReadButton() == NONE);
  resetFunc();
}