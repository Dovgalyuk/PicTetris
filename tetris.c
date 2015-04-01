#include <pic18.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"

#define	DelayUs(x)	do { unsigned char _dcnt; \
			  /*if(x>=4)*/ _dcnt=x*2; \
			  /*else _dcnt=1;*/ \
			  while(--_dcnt > 0) \
				{\
				asm("nop");\
				asm("nop");\
				continue; }\
		} while(0)

#define SetTimer do { unsigned char tmp = TMR0L + 0x87; TMR0H = 0xfd; TMR0L = tmp; } while(0)
#define JustWaitTimer do { ZERO = 1; asm("dw 0e0ffh"); } while(0)
#define WaitTimer do { JustWaitTimer; SetTimer; } while(0)

#define RGB(r,g,b) (1 + (b << 2) + (g << 4) + (r << 6))

static const char WIDTH = 10;
static const char HEIGHT = 20;

static const char CELL = RGB(3,3,3);
//static const char GRAY = RGB(2,2,2);
static const char MAGENTA = RGB(2,0,2);
static const char EMPTY = RGB(0,0,0);
unsigned char level[20][10];

unsigned char figureX, figureY, figureW, figureH, figureColor;
unsigned char figureN;

unsigned char timeout;
unsigned char moveTimeout;

#define SCORE_LEN 6
unsigned char score[SCORE_LEN];
unsigned char hiscore[SCORE_LEN];

enum { MENU, TETRIS } gameState;

unsigned char txtImage[8][SCORE_LEN * 8];


typedef struct cell { unsigned char x, y; } cell;
cell figure[4];

unsigned char FIRE, LEFT, RIGHT, DOWN;

void shortSync(void)
{
  PORTD = 0;
  DelayUs(5);
  PORTD = EMPTY;
  DelayUs(26);
}

void shortShortSync(void)
{
  PORTD = 0;
  DelayUs(5);
  PORTD = EMPTY;
}

void longSync(void)
{
  PORTD = 0;
  DelayUs(26);
  PORTD = EMPTY;
  DelayUs(5);
}

void skipLine(void)
{
  WaitTimer;
  PORTD = 0;
  DelayUs(5);
  PORTD = EMPTY;
}


void InitLevel(void)
{
  unsigned char x, y;
  for (y = 0 ; y < HEIGHT ; ++y)
    for (x = 0 ; x < WIDTH ; ++x)
      level[y][x] = EMPTY;
}

void InitLevelPart(unsigned char n)
{
  unsigned char *lev, i;
  lev = level[n * 4];
  for (i = WIDTH * (HEIGHT / 4) + 1 ; --i ; )
  {
    *lev = EMPTY;
    ++lev;
  }
}


void ClearLine(unsigned char n)
{
  unsigned char *lev, i;
  lev = level[n];
  for (i = WIDTH + 1 ; --i ; )
  {
    *lev = EMPTY;
    ++lev;
  }
}


void GetFigInBuffer(void)
{
  switch (figureN)
  {
  case 0:
// {{0,0},{1,0},{2,0},{3,0}}, // ****
//                            //
    figure[0].y = 0;
    figure[0].x = 0;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 2;
    figure[2].x = 0;
    figure[3].y = 3;
    figure[3].x = 0;
    figureW = 1;
    figureH = 4;
    figureColor = RGB(0,2,2);//dark cyan
    break;
  case 1:
// {{0,1},{1,1},{2,1},{2,0}}, // ***
//                            //   *
    figure[0].y = 0;
    figure[0].x = 1;
    figure[1].y = 1;
    figure[1].x = 1;
    figure[2].y = 2;
    figure[2].x = 1;
    figure[3].y = 2;
    figure[3].x = 0;
    figureW = 2;
    figureH = 3;
    figureColor = RGB(0,3,0);//green
    break;
  case 2:
// {{0,0},{1,0},{2,0},{2,1}}, //   *
//                            // ***
    figure[0].y = 0;
    figure[0].x = 0;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 2;
    figure[2].x = 0;
    figure[3].y = 2;
    figure[3].x = 1;
    figureW = 2;
    figureH = 3;
    figureColor = RGB(2,2,0);//dark yellow
    break;
  case 3:
// {{0,0},{1,0},{2,0},{1,1}}, // ***
//                            //  *
    figure[0].y = 0;
    figure[0].x = 0;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 2;
    figure[2].x = 0;
    figure[3].y = 1;
    figure[3].x = 1;
    figureW = 2;
    figureH = 3;
    figureColor = RGB(3,3,0);//yellow
    break;
  case 4:
// {{0,1},{1,0},{1,1},{2,0}}, // **
//                            //  **
    figure[0].y = 0;
    figure[0].x = 1;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 1;
    figure[2].x = 1;
    figure[3].y = 2;
    figure[3].x = 0;
    figureW = 2;
    figureH = 3;
    figureColor = RGB(0,3,3);//cyan
    break;
  case 5:
// {{0,0},{1,0},{1,1},{2,1}}, //  **
//                            // **
    figure[0].y = 0;
    figure[0].x = 0;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 1;
    figure[2].x = 1;
    figure[3].y = 2;
    figure[3].x = 1;
    figureW = 2;
    figureH = 3;
    figureColor = RGB(2,2,2);//gray
    break;
  case 6:
// {{0,0},{1,0},{1,1},{0,1}}  // **
//                            // **
    figure[0].y = 0;
    figure[0].x = 0;
    figure[1].y = 1;
    figure[1].x = 0;
    figure[2].y = 1;
    figure[2].x = 1;
    figure[3].y = 0;
    figure[3].x = 1;
    figureW = 2;
    figureH = 2;
    figureColor = RGB(0,2,0);//dark green
    break;
  };
}



void DrawFigure(unsigned char color)
{
  unsigned char i;
  for (i = 0 ; i < 4 ; ++i)
    level[(unsigned char)(figureY + figure[i].y)]
         [(unsigned char)(figureX + figure[i].x)] = color;
}


unsigned char CheckFigure()
{
  unsigned char i;
  for (i = 0 ; i < 4 ; ++i)
    if (level[(unsigned char)(figureY + figure[i].y)]
             [(unsigned char)(figureX + figure[i].x)] > EMPTY)
      return 0;

  return 1;
}



void CreateFigure(void)
{
  figureN = (rand() & 7);

  if (figureN)
    --figureN;
  else
    figureN = 3; // T-shape

  figureX = 4;
  figureY = 0;
  GetFigInBuffer();
}




void RotateFigureRight(void)
{
  unsigned char t;
  unsigned char i;
  t = figureH;
  figureH = figureW;
  figureW = t;

  for (i = 0 ; i < 4 ; ++i)
  {
    t = figure[i].x;
    figure[i].x = figure[i].y;
    figure[i].y = figureH - 1 - t;
  }
}

void RotateFigureLeft(void)
{
  unsigned char t;
  unsigned char i;
  t = figureH;
  figureH = figureW;
  figureW = t;

  for (i = 0 ; i < 4 ; ++i)
  {
    t = figure[i].y;
    figure[i].y = figure[i].x;
    figure[i].x = figureW - 1 - t;
  }
}




unsigned char CheckLine(unsigned char y)
{
  unsigned char x/*, *line*/;
  int t1 = *(int*)&FSR1L;

  *(int*)&FSR1L = (int)level[(unsigned char)(y + figureY)];

  for (x = WIDTH + 1 ; --x ; )
    if (POSTINC1 <= EMPTY)
    {
      *(int*)&FSR1L = (int)t1;
      return 0;
    }

  *(int*)&FSR1L = (int)t1;
  return 1;
}



void LineCpy(unsigned char x, unsigned char y)
{
  int t1 = *(int*)&FSR1L;
  int t2 = *(int*)&FSR2L;

  *(int*)&FSR1L = (int)level[x];
  *(int*)&FSR2L = (int)level[y];
  POSTINC1 = POSTINC2; // 1
  POSTINC1 = POSTINC2; // 2
  POSTINC1 = POSTINC2; // 3
  POSTINC1 = POSTINC2; // 4
  POSTINC1 = POSTINC2; // 5
  POSTINC1 = POSTINC2; // 6
  POSTINC1 = POSTINC2; // 7
  POSTINC1 = POSTINC2; // 8
  POSTINC1 = POSTINC2; // 9
  POSTINC1 = POSTINC2; // 10

  *(int*)&FSR1L = (int)t1;
  *(int*)&FSR2L = (int)t2;
}



static near bit	GAMEPAD_CYCLE @ ((unsigned)&PORTA*8)+0;
static near bit	GAMEPAD_SYNC @ ((unsigned)&PORTA*8)+1;
static near bit	GAMEPAD_DATA @ ((unsigned)&PORTA*8)+2;



void ReadGamepad()
{
    // 1
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;
 
    // start reading data from gamepad
    GAMEPAD_CYCLE = 0;
    GAMEPAD_SYNC = 1;
    DelayUs(50);
    GAMEPAD_SYNC = 0;

    // 2
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read A
    FIRE = GAMEPAD_DATA;

    // 3
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read B
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 4
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // 5
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read SELECT
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 6
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // 7
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read START
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 8
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // 9
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read UP
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 10
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // 11
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read DOWN
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 12
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    DOWN = GAMEPAD_DATA;

    // 13
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read LEFT
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 14
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    LEFT = GAMEPAD_DATA;

    // 15
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    // read RIGHT
    GAMEPAD_CYCLE = 1;
    DelayUs(50);
    GAMEPAD_CYCLE = 0;

    // 16
    WaitTimer;
    PORTD = 0;
    DelayUs(5);
    PORTD = EMPTY;

    RIGHT = GAMEPAD_DATA;
}




void DrawTetris(void)
{
  unsigned char y, cnt, i, c;
  unsigned char *pLine, *ptr;
  unsigned char canFall, x, checkRotate;
  unsigned char linesCnt, lines[4];

    for (cnt = 25 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    // level
    for (y = 0 ; y < HEIGHT ; ++y)
    {
      if (y < SCORE_LEN)
        ptr = score + SCORE_LEN - y - 1;
      for (cnt = 11 ; --cnt ; )
      {
        pLine = level[y];
        WaitTimer;
        PORTD = 0;
        DelayUs(5);
        PORTD = EMPTY;
        DelayUs(18);

        // border
        PORTD = MAGENTA;
        DelayUs(1);
        PORTD = EMPTY;

        // cell 1
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 2
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 3
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 4
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 5
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 6
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 7
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 8
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 9
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // cell 10
        PORTD = *(pLine++);
        DelayUs(1);
        PORTD = EMPTY;

        // border
        PORTD = MAGENTA;
        DelayUs(1);

        PORTD = EMPTY;

        // prepare score image
        if (y < SCORE_LEN && cnt <= 7)
        {
          pLine = txtImage[cnt] + (y * 8);
          c = GetDigitLine(*ptr, 7 - cnt);
          for (i = 9 ; --i ; )
          {
            *pLine = (c & 0x80) ? CELL : EMPTY;
            c <<= 1;
            ++pLine;
          }
        }

        // draw score
        if (y == 10 && cnt <= 7)
        {
          pLine = txtImage[cnt];
          // char 0
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 1
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 2
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 3
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 4
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 5
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          //
          PORTD = EMPTY;
        }
      }

      // black line
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    // bottom
    for (cnt = 11 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
      DelayUs(18);

      // border
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 1
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 2
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 3
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 4
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 5
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 6
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 7
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 8
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 9
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // cell 10
      PORTD = MAGENTA;
      DelayUs(1);
      PORTD = EMPTY;

      // border
      PORTD = MAGENTA;
      DelayUs(1);

      PORTD = EMPTY;
    }

    ReadGamepad();

    for (cnt = 19 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    JustWaitTimer;

    for (cnt = 6 ; --cnt ; )
      shortSync();
    for (cnt = 6 ; --cnt ; )
      longSync();
    for (cnt = 5 ; --cnt ; )
      shortSync();

    SetTimer;

    shortSync();
    shortShortSync();

    if (timeout)
      --timeout;
    if (moveTimeout)
      --moveTimeout;

    canFall = 1;
    checkRotate = 0;
    linesCnt = 0;

    skipLine(); //1
    DrawFigure(EMPTY);

    skipLine(); //2
    // falling
    if (!timeout)
    {
      if (figureH + figureY >= HEIGHT)
        canFall = 0;
      else
      {
        ++figureY;
        canFall = CheckFigure();
        if (!canFall)
          --figureY;
      }
    }

    skipLine(); //3
    if (!moveTimeout && !DOWN)
      moveTimeout = 1;
    if (timeout && !moveTimeout && !LEFT && figureX > 0)
    {
      --figureX;
      if (!CheckFigure())
        ++figureX;
      moveTimeout = 10;
    }

    skipLine(); //4
    if (timeout && !moveTimeout && !RIGHT && figureX + figureW < WIDTH)
    {
      ++figureX;
      if (!CheckFigure())
        --figureX;
      moveTimeout = 10;
    }

    skipLine(); //5
    checkRotate = 0;
    if (timeout && !moveTimeout && !FIRE && figureY + figureW <= HEIGHT)
    {
      x = figureX;
      RotateFigureRight();
      if (figureX + figureW > WIDTH)
        figureX = WIDTH - figureW;
      checkRotate = 1;
      moveTimeout = 10;
    }

    skipLine(); //6
    if (checkRotate && !CheckFigure())
    {
      figureX = x;
      RotateFigureLeft();
    }
    checkRotate = 0;

    skipLine(); //7
    DrawFigure(figureColor);

    skipLine(); //8
    // check destroyed
    if (!timeout && !canFall && figureY)
    {
      for (y = 0 ; y < figureH ; ++y)
        linesCnt += (lines[y] = CheckLine(y));
    }


    skipLine(); //9
    if (linesCnt)
    {
      //score += linesCnt;
      score[0] += linesCnt;
      ptr = score;
      for (cnt = SCORE_LEN ; --cnt ; )
        if (*ptr > 9)
        {
          *ptr -= 10;
          ++ptr;
          ++*ptr;
        }
        else
          ++ptr;

      x = figureY + figureH - 1;
      for (y = figureH ; y-- ; )
        if (!lines[y])
        {
          LineCpy(x, y + figureY);
          //memcpy(level[x], level[figureY + y], WIDTH);
          --x;
        }
    }


    skipLine(); //10
    if (linesCnt)
    {
      y = figureY;
      cnt = 5;
      while (--cnt && y)
      {
        --y;
        LineCpy(x, y);
        --x;
      }
    }

    skipLine(); //11
    if (!timeout && !canFall)
    {
      if (!figureY)
      {
        gameState = MENU;
/*
        ptr = score;
        for (cnt = SCORE_LEN + 1 ; --cnt ; )
        {
          *ptr = 0;
          ++ptr;
        }
        InitLevelPart(0);
*/
      }
      else if (linesCnt)
      {
        cnt = 5;
        while (--cnt && y)
        {
          --y;
          LineCpy(x, y);
          --x;
        }
      }
    }

    skipLine(); //12
    if (!timeout && !canFall)
    {
      if (!figureY)
        /*InitLevelPart(1)*/;
      else if (linesCnt)
      {
        cnt = 5;
        while (--cnt && y)
        {
          --y;
          LineCpy(x, y);
          --x;
        }
      }
    }

    skipLine(); //13
    if (!timeout && !canFall)
    {
      if (!figureY)
        /*InitLevelPart(2)*/;
      else if (linesCnt)
      {
        cnt = 5;
        while (--cnt && y)
        {
          --y;
          LineCpy(x, y);
          --x;
        }
      }
    }

    skipLine(); //14
    if (!timeout && !canFall)
    {
      if (!figureY)
        /*InitLevelPart(3)*/;
      else if (linesCnt)
      {
        cnt = 5;
        while (--cnt && y)
        {
          --y;
          LineCpy(x, y);
          --x;
        }
      }
    }

    skipLine(); //15
    if (!timeout && !canFall)
    {
      if (!figureY)
        /*InitLevelPart(4)*/;
      else if (linesCnt)
      {
        while (y)
        {
          --y;
          ClearLine(y);
        }
      }
    }

    skipLine(); //16
    if (!timeout && !canFall)
    {
      CreateFigure();
    }

    skipLine(); //17
    if (!timeout)
      timeout = DOWN ? 25 : 5;
}



void DrawMenu(void)
{
  unsigned char cnt, i, c, y;
  unsigned char *pLine, *ptr;

    for (cnt = 25 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    // level
    for (y = 0 ; y < HEIGHT ; ++y)
    {
      if (y < SCORE_LEN)
        ptr = hiscore + SCORE_LEN - y - 1;
      for (cnt = 11 ; --cnt ; )
      {
        WaitTimer;
        PORTD = 0;
        DelayUs(5);
        PORTD = EMPTY;

        // prepare score image
        if (y < SCORE_LEN && cnt <= 7)
        {
          pLine = txtImage[cnt] + (y * 8);
          c = GetDigitLine(*ptr, 7 - cnt);
          for (i = 9 ; --i ; )
          {
            *pLine = (c & 0x80) ? CELL : EMPTY;
            c <<= 1;
            ++pLine;
          }
        }

        // draw score
        if (y == 10 && cnt <= 7)
        {
          DelayUs(25);

          pLine = txtImage[cnt];
          // char 0
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 1
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 2
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 3
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 4
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          // char 5
          PORTD = *pLine++; //0
          PORTD = *pLine++; //1
          PORTD = *pLine++; //2
          PORTD = *pLine++; //3
          PORTD = *pLine++; //4
          PORTD = *pLine++; //5
          PORTD = *pLine++; //6
          PORTD = *pLine++; //7
          //
          PORTD = EMPTY;
        }
      }

      // black line
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    // bottom
    for (cnt = 11 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    ReadGamepad();

    for (cnt = 19 ; --cnt ; )
    {
      WaitTimer;
      PORTD = 0;
      DelayUs(5);
      PORTD = EMPTY;
    }

    JustWaitTimer;

    for (cnt = 6 ; --cnt ; )
      shortSync();
    for (cnt = 6 ; --cnt ; )
      longSync();
    for (cnt = 5 ; --cnt ; )
      shortSync();

    SetTimer;

    shortSync();
    shortShortSync();
    skipLine(); //1
    skipLine(); //2
    skipLine(); //3
    skipLine(); //4

    skipLine(); //5
    if (!FIRE)
    {
      //start !!!
      gameState = TETRIS;
    }

    skipLine(); //6
    skipLine(); //7
    skipLine(); //8
    skipLine(); //9
    skipLine(); //10
    skipLine(); //11
    skipLine(); //12
    skipLine(); //13
    skipLine(); //14
    skipLine(); //15
    skipLine(); //16
    skipLine(); //17
}


void main(void)
{
  unsigned char cnt, t;
  signed char c;
  unsigned char *ptr;

  timeout = 0;
  moveTimeout = 0;

  LATD = PORTD = 0;
  TRISD = 0x00; // output

  /*RCONbits.*/IPEN = 1;
  /*INTCONbits.*/GIEH = 1;
  /*INTCONbits.*/GIEL = 1;

  TMR0IP = 0; // low priority interrupt
  T08BIT = 0; // 16 bit
  T0CS = 0; // internal source
  TMR0IF = 0; // Clear Timer0 overflow flag
  TMR0IE = 1; // Enable Timer0 overflow interrupt

  // PORTA
  ADCON1 = 7; // digital inputs
  TRISA = 0x4; // 2 - input; 0,1 - output

  GAMEPAD_SYNC = 0;
  GAMEPAD_CYCLE = 0;

  gameState = MENU;

  TMR0ON = 0; // stop timer
  for (cnt = SCORE_LEN ; cnt-- ;)
  {
    t = EEPROM_READ(cnt);
    hiscore[cnt] = t;
  }
  TMR0ON = 1; // start timer

  while (1)
  {
    while (gameState == MENU)
      DrawMenu();

    ptr = score;
    for (cnt = SCORE_LEN + 1 ; --cnt ; )
    {
      *ptr = 0;
      ++ptr;
    }

    InitLevel();
    CreateFigure();

    while (gameState == TETRIS)
      DrawTetris();

    c = 0;
    for (cnt = SCORE_LEN ; cnt-- ;)
    {
      c = (signed char)hiscore[cnt] - (signed char)score[cnt];
      if (c)
        break;
    }
    if (c < 0)
    {
      TMR0ON = 0; // stop timer
      for (cnt = SCORE_LEN ; cnt-- ;)
      {
        EEPROM_WRITE(cnt, hiscore[cnt] = score[cnt]);
      }
      TMR0ON = 1; // start timer
    }
  }
}
