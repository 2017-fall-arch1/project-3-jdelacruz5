/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"

#define GREEN_LED BIT6
/*Variables to help keep track of the score*/
int lscore = 0;
int rscore = 0;
char score[3];


AbRect rect10 = {abRectGetBounds, abRectCheck, {3,15}}; /**< 5x20 rectangle */


AbRectOutline fieldOutline = {	/* playing field */
		abRectOutlineGetBounds, abRectOutlineCheck,   
		{(screenWidth/2)-1.5, (screenHeight/2)-10}
};

Layer fieldLayer = {		//playing field as a layer 
		(AbShape *) &fieldOutline,
		{screenWidth/2, screenHeight/2},//< center
		{0,0}, {0,0},				    // last & next pos 
		COLOR_BLACK,
		0
};

Layer layer2 = {          // Layer with a blue paddle
		(AbShape *)&rect10,
		{5, (screenHeight/2)}, // far left and centered
		{0,0}, {0,0},				    /* last & next pos */
		COLOR_BLUE,
		&fieldLayer
};

Layer layer1 = {		// Layer with a orange paddle
		(AbShape *)&rect10,
		{screenWidth-5, (screenHeight/2)}, // far right and centered
		{0,0}, {0,0},				    // last & next pos
		COLOR_ORANGE,
		&layer2,
};

Layer layer0 = {		// Layer with a black circle 
		(AbShape *)&circle4,
		{(screenWidth/2), (screenHeight/2)}, // center of screen
		{0,0}, {0,0},				    //last & next pos 
		COLOR_BLACK,
		&layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
	Layer *layer;
	Vec2 velocity;
	struct MovLayer_s *next;
} MovLayer;
/* initial value of {0,0} will be overwritten */
// Blue paddle
MovLayer ml2 = { &layer2, {0,0}, 0 };
// Orange paddle
MovLayer ml1 = { &layer1, {0,0}, &ml2 };
// Ball
MovLayer ml0 = { &layer0, {2,2}, &ml1 };

void movLayerDraw(MovLayer *movLayers, Layer *layers){
	int row, col;
	MovLayer *movLayer;
	and_sr(~8);			/**< disable interrupts (GIE off) */
	for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
		Layer *l = movLayer->layer;
		l->posLast = l->pos;
		l->pos = l->posNext;
	}
	or_sr(8);			/**< disable interrupts (GIE on) */
	for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
		Region bounds;
		layerGetBounds(movLayer->layer, &bounds);
		lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
				bounds.botRight.axes[0], bounds.botRight.axes[1]);
		for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
			for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
				Vec2 pixelPos = {col, row};
				u_int color = bgColor;
				Layer *probeLayer;
				for (probeLayer = layers; probeLayer; 
						probeLayer = probeLayer->next) { /* probe all layers, in order */
					if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
						color = probeLayer->color;
						break; 
					} /* if probe check */
				} // for checking all layers at col, row
				lcd_writeColor(color); 
			} // for col
		} // for row
	} // for moving layer being updated
}	  
/** Advances a moving shape within a fence
 *  
 *  \param ml The moving ball to be advanced
 *  \param fence The region which will serve as a boundary for ml
 *  \param orange The orange paddle to be moved
 *  \param blue The blue paddle to be moved
 *  \param orangeBound The boundary of the orange paddle
 *  \param blueBound The boundary of the blue paddle
 */
void mlAdvance(MovLayer *ml, MovLayer *orange, MovLayer *blue, Region *fence, Region *orangeBound, Region *blueBound){
	score[1] = '|';
	Vec2 newPos;
	u_char axis;
	Region shapeBoundary;
	for (; ml; ml = ml->next) {
        buzzer_set_period(0);
		vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
		abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
		for (axis = 0; axis < 2; axis ++) {
            //Checks if the ball hits the top or bottom walls and bounces the ball
            if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
				int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                buzzer_set_period(500);
				newPos.axes[axis] += (2*velocity);
			} /**< for axis */
			//Attempts at paddle bounderies
			if ((shapeBoundary.topLeft.axes[0] < (orangeBound->botRight.axes[0])) &&                (shapeBoundary.botRight.axes[1] > (orangeBound->topLeft.axes[1])) && 
                (shapeBoundary.topLeft.axes[1] < (orangeBound->botRight.axes[1]))) {
                int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                newPos.axes[axis] += (2*velocity);
                break;
            }
            //Attempts at paddle bounderis
            if ((shapeBoundary.botRight.axes[0] > blueBound->topLeft.axes[0]) &&               (shapeBoundary.botRight.axes[1] > blueBound->topLeft.axes[1]) &&               (shapeBoundary.topLeft.axes[1] < blueBound->botRight.axes[1])) {
                int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                newPos.axes[axis] += (2*velocity);
                break;
            }
            if (shapeBoundary.topLeft.axes[0] < fence->topLeft.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = 2;
                ml->layer->posNext = newPos;
                rscore++;
                drawString5x7(3,2, "Player 1", COLOR_GREEN, COLOR_WHITE);
                buzzer_set_period(1000);
                int redrawScreen = 1;
                break;
			}
			//checks if the ball hits the right wall and left player scores
			else if (shapeBoundary.botRight.axes[0] > fence->botRight.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = -2;
                ml->layer->posNext = newPos;
                lscore++;
                drawString5x7(80,2, "Player 2", COLOR_GREEN, COLOR_WHITE);
                buzzer_set_period(1000);
                int redrawScreen = 1;
                break;
			}
		} /**< for ml */
		int redrawScreen = 1;
		ml->layer->posNext = newPos;
		if ( lscore > 1 || rscore > 1 ){ //After maximum score it resets the game
            //layerDraw(&layer0);
            layerGetBounds(&fieldLayer, &layer0);
            
            lscore = 0;
            rscore = 0;
        }
        score[0] = '0' + lscore; //updates score
        score[2] = '0' + rscore; //updates score
	}
	int redrawScreen = 1;
	drawString5x7(55,2, score , COLOR_BLACK, COLOR_WHITE); // shows the score
    drawString5x7(3,2, "Player 1", COLOR_RED, COLOR_WHITE); // Player 1 side 
    drawString5x7(80,2, "Player 2", COLOR_RED, COLOR_WHITE); // Player 2 side
}


u_int bgColor = COLOR_WHITE;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */
Region fieldFence;		/**< fence around playing field  */
Region orange; //fence for orange paddle
Region blue; // fence for blue paddle
/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
	P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
	P1OUT |= GREEN_LED;

	configureClocks();
    buzzer_init();
	lcd_init();
	shapeInit();
	p2sw_init(1);

	shapeInit();

	layerInit(&layer0);
	layerDraw(&layer0);

	layerGetBounds(&fieldLayer, &fieldFence);
    layerGetBounds(&layer1,&orange);
    layerGetBounds(&layer2,&blue);
	drawString5x7(55,5, score, COLOR_BLACK, COLOR_WHITE); //update to use variables

	enableWDTInterrupts();      /**< enable periodic interrupt */
	or_sr(0x8);	              /**< GIE (enable interrupts) */

	for(;;) { 
		while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
			P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
			or_sr(0x10);	      /**< CPU OFF */
		}
		
		P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
		redrawScreen = 0;
        movLayerDraw(&ml1, &layer0);
        movLayerDraw(&ml0, &layer0);
        movLayerDraw(&ml2, &layer0);
	}
}
/*Moves the orange paddle down*/
void movLeftDown(Layer *layers){
    Vec2 nextPos;
    Vec2 velocity = {0,5};
    vec2Add(&nextPos, &layers->posNext, &velocity);
    layers->posNext = nextPos;
    layerGetBounds(&layer1, &orange);
}
/*Moves the orange paddle up*/
void movLeftUp(Layer *layers){
    Vec2 nextPos;
    Vec2 velocity = {0,-5};
    vec2Add(&nextPos, &layers->posNext, &velocity);
    layers->posNext = nextPos; 
    layerGetBounds(&layer1, &orange);
}
/*Moves the blue paddle down*/
void movRightDown(Layer *layers){
    Vec2 nextPos;
    Vec2 velocity = {0,5};
    vec2Add(&nextPos, &layers->posNext, &velocity);
    layers->posNext = nextPos;  
    layerGetBounds(&layer2,&blue);
}
/*Moves the blue paddle up*/
void movRightUp(Layer *layers){
    Vec2 nextPos;
    Vec2 velocity = {0,-5};
    vec2Add(&nextPos, &layers->posNext, &velocity);
    layers->posNext = nextPos; 
    layerGetBounds(&layer2,&blue);
}
/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
	static short count = 0;
	P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
	count ++;
	if (count == 15) {
		mlAdvance(&ml0, &ml1, &ml2, &fieldFence, &orange, &blue);
        switch(p2sw_read()){
            /*Switch cases to move the corrisponding paddle up or down for button presses*/
            case 1:
                movLeftDown(&layer2);
                break;
            case 2:
                movLeftUp(&layer2);
                break;
            case 3:
                movRightDown(&layer1);
                break;
            case 4:
                movRightUp(&layer1);
                break;
        }
        redrawScreen = 1;
		count = 0;
	} 
	P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
