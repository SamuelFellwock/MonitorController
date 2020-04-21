//compile: g++ -std=c++11 -o MonitorControl MonitorControl.cpp -lwiringPi
/*
GPIO Connections:
pin 1 > left button > pin 12
pin 17 > right button > pin 13
pin 7 > up button > pin 15
pin 3 > down button > pin 16
pin 5 > game button > pin 18
pin 24 > ppt button > pin 22
*/

#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>
#include <ratio>
#include <ctime>
using namespace std;
using namespace std::chrono;

void startGame();
void openPPT();
void leftButtonPress();
void rightButtonPress();
void upButtonPress();
void downButtonPress();
void gameButtonPress();
void pptButtonPress();

//WiringPi inputs
const int LEFT_BUTTON = 1;
const int RIGHT_BUTTON = 2;
const int UP_BUTTON = 3;
const int DOWN_BUTTON = 4;
const int GAME_BUTTON = 5;
const int PPT_BUTTON = 6;

int mousePosition;

bool gameRunning = false;
bool alreadyRan = false;
bool difficultySelect = false;
bool DEBUG = false;  //set to false when ready to launch (launching from startup)

high_resolution_clock::time_point t1;
high_resolution_clock::time_point t2;


int main(){
	wiringPiSetup();
	
	//setting GPIO outputs to use for 3.3V power sources
	pinMode(7, OUTPUT);
	digitalWrite(7, HIGH);
	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH);
	pinMode(9, OUTPUT);
	digitalWrite(9, HIGH);
	pinMode(10, OUTPUT);
	digitalWrite(10, HIGH);
	
	//setting GPIO inputs with internal pull down resistors
	pinMode(LEFT_BUTTON, INPUT);
	pullUpDnControl(LEFT_BUTTON, PUD_DOWN);
	pinMode(RIGHT_BUTTON, INPUT);
	pullUpDnControl(RIGHT_BUTTON, PUD_DOWN);
	pinMode(UP_BUTTON, INPUT);
	pullUpDnControl(UP_BUTTON, PUD_DOWN);
	pinMode(DOWN_BUTTON, INPUT);
	pullUpDnControl(DOWN_BUTTON, PUD_DOWN);
	pinMode(GAME_BUTTON, INPUT);
	pullUpDnControl(GAME_BUTTON, PUD_DOWN);
	pinMode(PPT_BUTTON, INPUT);
	pullUpDnControl(PPT_BUTTON, PUD_DOWN);
	
	//assuming normally closed switch
	wiringPiISR(LEFT_BUTTON, INT_EDGE_BOTH, *leftButtonPress);
	wiringPiISR(RIGHT_BUTTON, INT_EDGE_BOTH, *rightButtonPress);
	wiringPiISR(UP_BUTTON, INT_EDGE_BOTH, *upButtonPress);
	wiringPiISR(DOWN_BUTTON, INT_EDGE_BOTH, *downButtonPress);
	wiringPiISR(GAME_BUTTON, INT_EDGE_BOTH, *gameButtonPress);
	wiringPiISR(PPT_BUTTON, INT_EDGE_FALLING, *pptButtonPress);
	
	//initializes program to be running in PPT state
	openPPT();
	while(true){
		//the game runs for a maximum of 2 minutes, so this loop checks for time to
		//pass and automatically closes the game and treturns to PPT after a set time
		if(gameRunning){
			t2 = high_resolution_clock::now();
			
			duration<double> time_span = duration_cast<duration<double>>(t2-t1);
			
			if(time_span.count() > 130){//two minutes and ten seconds have passed
				system("pkill chromium-browse");
				gameRunning = false;
				difficultySelect = false;
				openPPT();
			}
		}
	}
}

//function that starts game
void startGame(){
	system("xdg-open /home/pi/Code/NASA_Game/PowerupGame.html");
	//chrome takes longer to load for first time on startup, so
	//this decides how long to wait
	if(alreadyRan || DEBUG){
		usleep(4000000);
	}
	else{
		usleep(8000000);
		alreadyRan = true;
	}
	system("/home/pi/Code/bin/open_game");
}

//function that opens powerpoint
void openPPT(){
	system("xdg-open /home/pi/Code/info.odp");
	if(alreadyRan || DEBUG){
		usleep(4500000);
	}
	else{
		usleep(10000000);
	}
	system("xdotool key F5");
	system("xdotool mousemove 2000 2000");
}

//when selecting difficulty, moves cursor. otherwise, holds down left key
void leftButtonPress(){
	usleep(10000);
	//assuming normally closed switch
	if(!digitalRead(LEFT_BUTTON)){ //high to low, pressing button
		if(difficultySelect){
			if(mousePosition > 400){
				mousePosition -= 300;
			}
			stringstream statement;
			statement << "xdotool mousemove " << mousePosition << " 530";
			system(statement.str().c_str());
		}
		else{
			system("xdotool keydown Left");
		}
	}
	else{ //low to high, releasing button
		if(!difficultySelect){
			system("xdotool keyup Left");
		}
	}
}

//when selecting difficulty, moves cursor. otherwise, holds down right key
void rightButtonPress(){
	usleep(10000);
	//assuming normally closed switch
	if(!digitalRead(RIGHT_BUTTON)){ //high to low, pressing button
		if(difficultySelect){
			if(mousePosition < 1000){
				mousePosition += 300;
			}
			stringstream statement;
			statement << "xdotool mousemove " << mousePosition << " 530";
			system(statement.str().c_str());
		}
		else{
			system("xdotool keydown Right");
		}
	}
	else{ //low to high, releasing button
		if(!difficultySelect){
			system("xdotool keyup Right");
		}
	}
}

//holds down up key
void upButtonPress(){
	usleep(10000);
	//assuming normally closed switch
	if(!digitalRead(UP_BUTTON)){ //high to low, pressing button
		system("xdotool keydown Up");
	}
	else{ //low to high, releasing button
		system("xdotool keyup Up");
	}
}

//holds down down key
void downButtonPress(){
	usleep(10000);
	//assuming normally closed switch
	if(!digitalRead(DOWN_BUTTON)){ //high to low, pressing button
		system("xdotool keydown Down");
	}
	else{ //low to high, releasing button
		system("xdotool keyup Down");
	}
}

//on first press, loads game and lets user choose difficulty
//on second press, selects highlighted difficulty
void gameButtonPress(){
	usleep(10000);
	system("xdotool key g"); //presses a key to exit sleep mode
	if(!digitalRead(GAME_BUTTON)){ //high to low, pressing button
		if(!difficultySelect && !gameRunning){
		system("xdotool key Escape");
		system("xdotool key ctrl+q");
		startGame();
		mousePosition = 400;
		difficultySelect = true;
		}
		else if(difficultySelect && !gameRunning){
			system("xdotool click 1");
			system("xdotool mousemove 2000 2000");
			gameRunning = true;
			difficultySelect = false;
			t1 = high_resolution_clock::now();
		}
	}
	
}

//exits out of game and loads powerpoint
void pptButtonPress(){
	usleep(10000);
	system("xdotool key g"); //presses a key to exit sleep mode
	if(gameRunning || difficultySelect){
		system("pkill chromium-browse");
		gameRunning = false;
		difficultySelect = false;
		openPPT();
	}
}
