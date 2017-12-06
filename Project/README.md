# Project 3: LCD Game Pong
## Introduction

Tasked with making a game on the msp430 auch as pong or other games pending approval from the instructor. I chose to do the game pong. To get started if the game is not already loaded onto the msp430 just go into the folder that contains the game and run the command make load. Once the game is loaded onto the msp430 the game will start on it's own. Either player needs to score 5 points to win the game. A score is aquired if the moving ball hits the wall your opponent is defending. The two buttons on the left control the up and down movement of the blue paddle (left) and the two buttons on the right control the movements of the orange paddle (right). I collaborated with Daniel Maynez and Bianca Alvarado on this project.

## Requirements
Your game should meet the following minimum criteria:

- dynamically render graphical elements that move
-- including one original algorithmically rendered graphical element 
- handle colisions properly with other shapes and area boundaries
- produce sounds triggered by game events
-- in a manner that does not cause the game to pause
- communicate with the player using text
- include a state machine written in assembly language
- respond to user input (from buttons)


## Libraries

Several libraries are provided.  
They can be installed by the default production of Makefile in the repostiory's 
root directory, or by a "$make install" in each of their subdirs.

- timerLib: Provides code to configure Timer A to generate watchdog timer interrupts at 250 Hz

- p2SwLib: Provides an interrupt-driven driver for the four switches on the LCD board and a demo program illustrating its intended functionality.

- lcdLib: Provides low-level lcd control primitives, defines several fonts, 
and a simple demo program that uses them.

- shapeLib: Provides an translatable model for shapes that can be translated 
and rendered as layers.

- circleLib: Provides a circle model as a vector of demi-chord lengths,
pre-computed circles as layers with a variety of radii, 
and a demonstration program that renders a circle.




