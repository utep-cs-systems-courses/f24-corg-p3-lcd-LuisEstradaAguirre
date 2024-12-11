/** \file lcddemo.c
 *  \brief A simple demo that draws a string and square
 */

#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
static int rightTriangle(int size, int x, int y, int color);
static int leftTriangle(int size, int x, int y, int color);
static int rightTriangleInverse(int size, int x, int y, int color);
static int leftTriangleInverse(int size, int x, int y, int color);
static int square(int size, int x, int y, int color);
static int hectagon(int size, int x, int y, int color);

/** Initializes everything, clears the screen, draws "hello" and a square */
int
main()
{
  configureClocks();
  lcd_init();
  u_char width = screenWidth, height = screenHeight;
  int centerCol = width/2, centerRow = height/2;
  clearScreen(COLOR_BLUE);

  // leftTriangle(20, centerCol, centerRow, COLOR_PINK);
  // square(20, centerCol, centerRow, COLOR_PINK);
  // rightTriangle(20, centerCol+20, centerRow, COLOR_PINK);
  // fillRectangle(centerCol-20, centerRow+20, 60, 20, COLOR_PINK);
  // leftTriangleInverse(20, centerCol, centerRow+40, COLOR_PINK);
  // square(20, centerCol, centerRow+40, COLOR_PINK);
  // rightTriangleInverse(20, centerCol+20-1, centerRow+40, COLOR_PINK);
  octagon(20, centerCol, centerRow, COLOR_PINK);

 // drawString5x7(20,20, "hello", COLOR_GREEN, COLOR_RED);

 // fillRectangle(30,30, 60, 60, COLOR_ORANGE);
  
}

int octagon(int size, int x, int y, int color){
  leftTriangle(size, x, y, color);
  square(size, x, y, color);
  rightTriangle(size, x+size, y, color);
  fillRectangle(x-size, y+size, size*3, size, color);
  leftTriangleInverse(size, x, y+(size*2), color);
  square(size, x, y+(size*2), color);
  rightTriangleInverse(size, x+(size-1), y+(size*2), color);
}



int rightTriangle(int size, int x, int y, int color){
  for (int row = 0; row < size; row ++) {
    for (int col = 0; col <= row; col ++) {
      drawPixel(x + col, y + row, color);
    }
  }
}

int rightTriangleInverse(int size, int x, int y, int color){
  for (int row = 0; row < size; row ++) {
    for (int col = size - row; col > 0; col --) {
      drawPixel(x + col, y + row, color);
    }
  }
}


int leftTriangle(int size, int x, int y, int color){
  for (int row = 0; row < size; row ++) {
    for (int col = -row; col <= 0; col ++) {
      drawPixel(x + col, y + row, color);
    }
  }
}

int leftTriangleInverse(int size, int x, int y, int color){
  for (int row = 0; row < size; row ++) {
    for (int col = -size + row; col < 0; col ++) {
      drawPixel(x + col, y + row, color);
    }
  }
}


int square(int size, int x, int y, int color){
  for (int row = 0; row < size; row ++) {
    for (int col = 0; col < size; col ++) {
      drawPixel(x + col, y + row, color);
    }
  }
}
