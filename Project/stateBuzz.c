
#include <msp430.h>
#include "stateBuzz.h"
#include "buzzer.h"

void Bstate(char hit){
  switch (hit) {  
  /*Ball collides with wall for score*/
  case 1:
    buzzer_set_period(1000);
    break;
  /*Ball collides with paddles*/
  case 2:
    buzzer_set_period(5000);
    break;    
  /*Ball collides with upper and lower wall*/
  case 3:
      buzzer_set_period(4000);
      break;
  }
}
