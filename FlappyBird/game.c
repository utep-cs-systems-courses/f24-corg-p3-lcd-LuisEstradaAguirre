#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"
#include <stdio.h>

extern void switch_interrupt_handler();


// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1     //P2.0     0001
#define SW2 2     //P2.1     0010
#define SW3 4     //P2.3     0100
#define SW4 8     //P2.4     1000

#define SWITCHES 15    //Means from SW1 to SW4     1111

char blue = 31, green = 0, red = 31;
unsigned char step = 0;

unsigned short BACKGROUND_COLOR = COLOR_BLACK;
unsigned short BIRD_COLOR = COLOR_GREEN;
unsigned short PIPE_COLOR = COLOR_WHITE;

short gameRunning = 1; // 1 if game is running, 0 if game is over
short score = 0;
short lastScore = -1;
char scoreText[20];
    
short drawPos[2] = {15,10}, controlPos[2] = {2, 10};
short colVelocity = 0.1, colLimits[2] = {1, screenHeight};
short jumpVelocity = -2; // Negative value for upward movement
short topBannerSize = 15;
int switches = 0;
short redrawScreen = 1;
short buzzerEnabled = 1; // 0 = off, 1 = on
short gapPosition = 30;


short rectCol = screenWidth - 10; // Start the rectangle on the right side
short rectRow = screenHeight / 2 - 10; // Center vertically
short rectWidth = 20;  // Width of the rectangle
short rectHeight = 10; // Height of the rectangle
short rectSpeed = -2;  // Speed of movement (negative for leftward)
short playScreenHeight = screenHeight - 10;


static char switch_update_interrupt_sense() {
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void switch_init() {			/* setup switch */  
  P2REN |= SWITCHES;		  /* enables resistors for switches */
  P2IE |= SWITCHES;		    /* enable interrupts from switches */
  P2OUT |= SWITCHES;		  /* pull-ups for switches */
  P2DIR &= ~SWITCHES;		  /* set switches' bits for input */
  switch_update_interrupt_sense();
}

void switch_interrupt_handler() {
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;

  if (switches & SW1) { // If SW1 is pressed
    colVelocity = jumpVelocity; // Trigger a jump
  }

  if (switches & SW3) { // If SW3 is pressed
    buzzerEnabled = !buzzerEnabled; // Toggle buzzer state
  }

  if (switches & SW4) { // If SW4 is pressed
    gameRunning = !gameRunning; // Toggle game state
    if (gameRunning) {
      P1OUT |= LED; // Turn LED on (active)
      fillRectangle(90, 0, 50, 10, BACKGROUND_COLOR);   // Clear "PAUSED" message
    } 
    else {
      P1OUT &= ~LED; // Turn LED off (sleep)
      drawString5x7(90, 0, "PAUSED", COLOR_RED, BACKGROUND_COLOR);  // Display "PAUSED"
    }
  }
}

void play_game_over_song() {
  if(buzzerEnabled){
    short period;
    short minPeriod = 500;  // Lower frequency
    short maxPeriod = 2000; // Higher frequency

    // Loud (high frequency)
    for (period = minPeriod; period <= maxPeriod; period += 100) {
      buzzer_set_period(period);
      __delay_cycles(250000); // Short delay for each change
    }

    // Soft (low frequency)
    for (period = maxPeriod; period >= minPeriod; period -= 100) {
      buzzer_set_period(period);
      __delay_cycles(250000); // Short delay for each change
    }

    // Silence the buzzer after the song
    buzzer_set_period(0);
  }
    
}

// axis zero for col, axis 1 for row
void draw_bird(int col, int row, unsigned short color) {
  fillRectangle(col-1, row-1, 7, 7, color);
}

void screen_update_bird() {
  if (!gameRunning) return; // Don't update if the game is over

  // Check if bird touches the bottom of the screen
  if (controlPos[1] >= screenHeight) { // Bird size is 7x7
      gameRunning = 0; // Stop the game
      clearScreen(COLOR_RED); // Turn the screen red
      play_game_over_song();
      return;
  }

  // Check if bird hits the top of the screen
  if (controlPos[1] <= topBannerSize) {
      controlPos[1] = topBannerSize; // Prevent bird from going off-screen
      colVelocity = 0.1;   // Reset velocity to fall down
  }

  for (char axis = 0; axis < 2; axis++) {
      if (drawPos[axis] != controlPos[axis])
          goto redraw;
  }
  return;

redraw:
  draw_bird(drawPos[0], drawPos[1], BACKGROUND_COLOR); /* Erase */
  for (char axis = 0; axis < 2; axis++)
      drawPos[axis] = controlPos[axis];

  // Update the bird's position based on velocity
  controlPos[1] += colVelocity; // Vertical movement
  colVelocity += 1; // Gravity effect (increase downward velocity)

  draw_bird(drawPos[0], drawPos[1], BIRD_COLOR); /* Draw */
}
  
void create_pipe(short col, short gap, short position, short width, unsigned short color) {
  // Draw the top rectangle
  fillRectangle(col,topBannerSize, width, position, color);

  // Draw the bottom rectangle
  fillRectangle(col, position + gap + topBannerSize, width, screenHeight - (position + gap), color);
}

short check_collision(short birdCol, short birdRow, short pipeCol, short pipeWidth, short pipeGap, short pipePosition) {
  // Check horizontal overlap
  if (birdCol + 7 < pipeCol || birdCol > pipeCol + pipeWidth) {
    return 0; // No horizontal collision
  }

  // Check vertical overlap (bird hits top or bottom pipe)
  if (birdRow < pipePosition + topBannerSize || birdRow + 7 > pipePosition + pipeGap + topBannerSize) {
    return 1; // Collision detected
  }

  return 0; // No collision
}

void screen_update_pipe() {
  if (!gameRunning) return;
  // Erasing only the edge of the pipe
  fillRectangle(rectCol + rectWidth, topBannerSize, -rectSpeed, screenHeight, BACKGROUND_COLOR);
  
  // Update the pipe's position
  rectCol += rectSpeed;

  // Reset pipe when it goes off-screen
  if (rectCol + rectWidth < 0) {
    rectCol = screenWidth; // Move to the right edge
    score++; 
    play_game_over_song();
  }

  // Check for collision
  if (check_collision(drawPos[0], drawPos[1], rectCol, rectWidth, 30, 50)) {
    gameRunning = 0;        // Stop the game
    clearScreen(COLOR_RED); // Turn the screen red
    play_game_over_song();
    return;
  }

  // Draw the pipe with a gap of 30 and a position of 50
  create_pipe(rectCol, 30, 50, rectWidth, PIPE_COLOR);
  // gapPosition += 20;
  // if(gapPosition > 80){
  //   gapPosition = 30
  // }
}

void screen_update_score() {
  if (!gameRunning) return;
  if (score > lastScore){
    fillRectangle(15, 15, 60, topBannerSize, BACKGROUND_COLOR);
    sprintf(scoreText, "Score: %d", score);
    drawString5x7(0, 0, scoreText, COLOR_GREEN, BACKGROUND_COLOR);
    lastScore = score;
    if(buzzerEnabled == 1){
      drawString5x7(70, 0, "S", COLOR_BLUE, BACKGROUND_COLOR);
    }
  }
}

void screen_update_sound_indicator() {
  if (!gameRunning) return;
  fillRectangle(70, 0, 10, topBannerSize, BACKGROUND_COLOR);
  if(buzzerEnabled == 1){
    drawString5x7(70, 0, "S", COLOR_BLUE, BACKGROUND_COLOR);
  }
}

void update_game() {
  if (!gameRunning) {
    return; 
  }
  else{
    screen_update_bird();
    screen_update_score();
    screen_update_sound_indicator();
    screen_update_pipe();
  }
  
}

void wdt_c_handler() {
  static int secCount = 0;

  if (!gameRunning) return; // Do nothing if the game is paused

  secCount++;
  if (secCount >= 25) {		/* 10/sec */
    {	// Update bird's position
      short oldCol = controlPos[1];
      short newCol = oldCol + colVelocity;
      controlPos[1] = newCol;
    }

    if (step <= 30)
      step++;
    else
      step = 0;

    secCount = 0;
    redrawScreen = 1;
  }
}


void main() {
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();

  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  clearScreen(BACKGROUND_COLOR);
  while (1) {			
    if (redrawScreen && gameRunning) {
      redrawScreen = 0;
      update_game();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	 /**< CPU OFF */
    P1OUT |= LED;	 /* led on */
  }
}

void __interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
