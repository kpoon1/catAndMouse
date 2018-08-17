# Cat And Mouse 
This Repo was aim to improve the networking part of the original cat and mouse.

0. Downloading the game:

Place the game folder into an easy to find directory.


1. Building the Game:

For running in Linux:

Libraries required: SFML

Instructions for installing SFML:

In terminal, input:

sudo apt -get install libsfml-dev

(If error occurs)

sudo apt -get update

If g++ is present on system, then run makefile. Otherwise, update g++.


2. Running the Game:

In terminal, get to the game directory, type ‘make’ to compile and build.
 
Type ‘./catAndMouse’ to run the game. 

At this point the terminal will pop up another screen, switching focus to this new screen. 

Return to the terminal again and enter the IP4 address (ex: 172.16.254.1). 

Then enter ‘m’ or ‘a’ for the appropriate characters: ‘m’ for Marine and ‘a’ for Alien. 
 
For testing proposes, we recommend running the game two consoles to represent two players. Also use the same IP address for both players.


3. Playing the Game:

Basic controls:

W = up
S = down
A = left
D = right
Mouse = 360 directional view
Mouse click = shooting
E = Interact with objects in game.
1 = Use item in inventory


BUGS:

If running, it's possible to run through the walls
