// ConsoleApplication1.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <Windows.h>
#include <iostream>
#include <thread>
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <random>
#include "Source.h"
#include <time.h>

using namespace std;

struct point
{
	int x, y;
	char c = 'o';
};

const int width = 120, height = 30;
// char* screenBuffer is an array with width * height chars mapped to the console window
CHAR_INFO* screenBuffer;

HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */

COORD characterBufferSize = { width, height };
COORD characterPosition = { 0, 0 };

// SMALL_RECT consoleWriteArea = { 0, 0, 1, 1 };

/* A CHAR_INFO structure containing data about a single character

* we use this to iterate over the matrix
*/
CHAR_INFO character;

char mainChar = 0;
char oldChar;

char headChar = 'U';
char tailChar = 9;
char* obstcleChar = &mainChar;
const int obstacleNumber = 25;
char* wallChar = &mainChar;
const char gateChar = '@';
vector<point> tail;
int minTailSize = 8; 

int speedLevels = 15;
int level = 1;

int eatenEggRecently = 0;

int score;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection direction;
point head, egg, gate1, gate2;
bool gateActivated = false;
string dirStr;

enum
{
	KEY_ESC = 27,
	ARROW_UP = 256 + 72,
	ARROW_DOWN = 256 + 80,
	ARROW_LEFT = 256 + 75,
	ARROW_RIGHT = 256 + 77
};


static int get_code(void)
{
	int ch = _getch();
	if (ch == 0 || ch == 224)
		ch = 256 + _getch();
	return ch;
}

void toScreen(char*& screen, const int index, const char* text) {
	for (int i = 0; i < strlen(text); i++)
		screen[index + i] = text[i];
}

static int get_code(int ch)
{

	if (ch == 0 || ch == 224)
		ch = 256 + _getch();
	return ch;
}

enum {
	colorWhite = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
	colorGrey = FOREGROUND_INTENSITY,
	colorRed = FOREGROUND_RED | FOREGROUND_INTENSITY,
	colorGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	colorBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	colorYellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	colorCyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	colorLightGrey = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	colorLightRed = FOREGROUND_RED,
	colorLightGreen = FOREGROUND_GREEN,
	colorLightBlue = FOREGROUND_BLUE,
	colorLightYellow = FOREGROUND_RED | FOREGROUND_GREEN
};

void putChar(point position, UINT16 color, char character) {
	screenBuffer[position.x + position.y * width].Attributes = color;
	screenBuffer[position.x + position.y * width].Char.AsciiChar = character;
}

void putColor(point position, UINT16 color) {
	screenBuffer[position.x + position.y * width].Attributes = color;
}

int getInBoundary(int value, int min, int max) {
	if (value >= max) {
		return max;
	}
	else if (value < min) {
		return min;
	}
	else {
		return value;
	}
}

int getX(int x) {
	return getInBoundary(x, 0, width);
}

int getY(int y) {
	if (y >= height) {
		return height;
	}
	else if (y < 0) {
		return 0;
	}
	else {
		return y;
	}
}

void putObstacle1() {
	point obstacleRoot;
	obstacleRoot.x = rand() % width;
	obstacleRoot.y = rand() % height;

	int obstaclePartMax;
	obstaclePartMax = rand() % 40 + 10;
	float x = 0;
	float y = 0;
	for (int obstacleBranchCount = 0; obstacleBranchCount < obstaclePartMax; obstacleBranchCount++) {
		point obstacleBranch;
		x = x + 0.1f * float((rand() % 20) - 10);
		y = y + 0.1f * float((rand() % 20) - 10);
		putChar({ getX(obstacleRoot.x + static_cast<int>(floor(x))),getY(obstacleRoot.y + static_cast<int>(floor(y))) }, colorGrey, *obstcleChar);
	}
}

void putObstacle() {
	point obstacleRoot;
	obstacleRoot.x = rand() % width;
	obstacleRoot.y = rand() % height;

	int obstacleBranchNumber;
	obstacleBranchNumber = rand() % 10 + 1;
	for (int obstacleBranchCount = 0; obstacleBranchCount < obstacleBranchNumber; obstacleBranchCount++) {
		;
		point obstacleBranch;
		for (int obstacleBranchPart = 0; obstacleBranchPart < 20; obstacleBranchPart++) {
			int dir = rand() % 4;
			if (dir == 0) {
				obstacleBranch.x = getX(obstacleRoot.x--);
				obstacleBranch.y = obstacleRoot.y;
			}
			else if (dir == 1) {
				obstacleBranch.x = obstacleRoot.x;
				obstacleBranch.y = getY(obstacleRoot.y--);
			}
			else if (dir == 2) {
				obstacleBranch.x = getX(obstacleRoot.x++);
				obstacleBranch.y = obstacleRoot.y;
			}
			else {
				obstacleBranch.x = obstacleRoot.x;
				obstacleBranch.y = getY(obstacleRoot.y++);
			}

			putChar(obstacleBranch, colorGrey, *obstcleChar);
		}
	}
}

void drawWalls() {
	point cursor;
	for (cursor.y = 0; cursor.y < height; cursor.y++) {
		for (cursor.x = 0; cursor.x < width; cursor.x++) {
			if (cursor.y == 0 || cursor.x == 0 || cursor.x == width - 1 || cursor.y == height - 1) {
				putChar(cursor, colorGrey, *wallChar);
			}
			else {
				putChar(cursor, NULL, ' ');
			}
		}
	}
}

void redrawScreen() {
	point cursor;
	for (cursor.y = 0; cursor.y < height; cursor.y++) {
		for (cursor.x = 0; cursor.x < width; cursor.x++) {
			if (screenBuffer[cursor.x + cursor.y * width].Char.AsciiChar == oldChar) putChar(cursor, colorGrey, mainChar);
		}
	}
}

void growObstacles() {
	for (int obstacleCount = 0; obstacleCount < obstacleNumber; obstacleCount++) {
		putObstacle1();
	}
}

void Setup() {
	srand(time(NULL));
	direction = STOP;
	head.x = width / 2;
	head.y = height / 2;
	head.c = headChar;
	egg.x = rand() % width;
	egg.y = rand() % height;
	score = 0;
	for (int i = 0; i < minTailSize; i++) {
		point o(head);
		o.c = tailChar;
		tail.push_back(o);
	}
	drawWalls();
	growObstacles();
}

void Draw() {
	//	Egg
	screenBuffer[egg.x + egg.y * width].Attributes = colorLightRed;
	screenBuffer[egg.x + egg.y * width].Char.AsciiChar = '0';
	//	Snake
	// Tail	
	for (int t = 0; t < tail.size(); t++) {
		if (eatenEggRecently > 0) {
			if (t % 2) {
				putColor({ tail[t].x, tail[t].y }, colorLightRed);
			}
			else {
				putColor({ tail[t].x, tail[t].y }, colorRed);
			}
		}
		else if (t % minTailSize == 0) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_BLUE;
		}
		else if (t % minTailSize == 1) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		}
		else if (t % minTailSize == 2) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN;
		}
		else if (t % minTailSize == 3) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		}
		else if (t % minTailSize == 4) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_GREEN;
		}
		else if (t % minTailSize == 5) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		}
		else if (t % minTailSize == 6) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_RED | FOREGROUND_GREEN;
		}
		else if (t % minTailSize == 7) {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		}
		else {
			screenBuffer[tail[t].x + tail[t].y * width].Attributes = FOREGROUND_RED;
		}
		/*		if(eatenEggRecently)
					screenBuffer[tail[t].x + tail[t].y * width].Attributes = BACKGROUND_GREEN;
		*/
		screenBuffer[tail[t].x + tail[t].y * width].Char.AsciiChar = tail[t].c;
	}
	// Head
	if (eatenEggRecently > 0) {
		eatenEggRecently--;
		for (SHORT x = 0; x < width; x++) {
			if (x < eatenEggRecently) {
				putChar({ x,  height - 1 }, colorRed, *wallChar);
			}
			else {
				// putChar({ x,  height - 1 }, colorGrey, ); // char(screenBuffer[x + (height -1) * width].Char)
				screenBuffer[(height - 1) * width + x].Attributes = colorGrey;
			}
		}
		putChar({ head.x,head.y }, colorRed, 'W');
	}
	else {
		putChar({ head.x,head.y }, colorCyan, head.c);
	}

	// Level
	for (SHORT x = 0; x < width; x++) {
		for (int levelIndicator = 0; levelIndicator < level; levelIndicator++) {
			if (x == width / 2 + levelIndicator || x == width / 2 - levelIndicator)
				putChar({ x,0 }, colorYellow, 10);
			else {
				putColor({ x,0 }, colorGrey);
			}
		}
	}
}

void gate() {
	if (!gateActivated) {
		gateActivated = true;
	}
	else {
		putChar(gate1, colorRed, *obstcleChar);
		putChar(gate2, colorBlue, *obstcleChar);
	}

	point min, max;
	if (head.x < egg.x) {
		min.x = head.x;
		max.x = egg.x;
	}
	if (head.x >= egg.x) {
		min.x = egg.x;
		max.x = head.x;
	}
	if (head.y >= egg.y) {
		min.y = egg.y;
		max.y = head.y;
	}
	if (head.y < egg.y) {
		min.y = head.y;
		max.y = egg.y;
	}

	if (max.x == min.x) {
		gate1.x = max.x;
		gate2.x = max.x;
	}
	else {
		gate1.x = int(rand() % (max.x - min.x)) + min.x;
		gate2.x = int(rand() % (max.x - min.x)) + min.x;
	}

	if (max.y == min.y) {
		gate1.y = min.y;
		gate2.y = min.y;
	}
	else {
		gate1.y = int(rand() % (max.y - min.y)) + min.y;
		gate2.y = int(rand() % (max.y - min.y)) + min.y;
	}

	if (gateActivated) {
		putChar({ gate1.x , gate1.y }, colorRed, gateChar);
		putChar({ gate2.x , gate2.y }, colorBlue, gateChar);
	}
}

void crunch() {
	point crunch;
	crunch.x = head.x;
	crunch.y = head.y;
	if (direction == LEFT) crunch.x = getX(head.x - 2);
	if (direction == UP) crunch.y = getY(head.y - 2);
	if (direction == RIGHT) crunch.x = getX(head.x + 2);
	if (direction == DOWN) crunch.y = getY(head.y + 2);
	putChar(crunch, colorRed, '#');
	Sleep(500);
	putChar(crunch, colorGrey, ' ');
}

void Input() {


	int ch;
	if (_kbhit()) {
		int key = _getch();
		switch (ch = get_code(key)) {
		case ARROW_LEFT:
			direction = LEFT;
			dirStr = "left";
			break;
		case ARROW_RIGHT:
			direction = RIGHT;
			dirStr = "right";
			break;
		case ARROW_UP:
			direction = UP;
			dirStr = "up";
			break;
		case ARROW_DOWN:
			direction = DOWN;
			dirStr = "down";
			break;
		case '+':
			level = getInBoundary(++level, 0, speedLevels);
			break;
		case '-':
			if (level > 1)
				level--;
			break;
		case 'r':
			head.x = rand() % width;
			head.y = rand() % height;
			putObstacle1();
			break;
		case 'a':
			crunch();
			break;
		case 'c':
			oldChar = mainChar;
			mainChar = getInBoundary(++mainChar, 0, 255);
			redrawScreen();
			putObstacle1();
			break;
		case 'd':
			gate();
			dirStr = "gate";
			break;
		}
	}
}

bool compareChar(point position, char character) {
	return screenBuffer[position.x + position.y * width].Char.AsciiChar == character;
}

void Logic() {
	//Head movement
	point prev, temp;
	prev.x = head.x;
	prev.y = head.y;
	//Tail movement
	for (int i = 0; i < tail.size(); i++) {
		screenBuffer[tail[i].x + tail[i].y * width].Char.AsciiChar = ' ';
		temp.x = tail[i].x;
		temp.y = tail[i].y;
		tail[i].x = prev.x;
		tail[i].y = prev.y;
		prev.x = temp.x;
		prev.y = temp.y;
	}
	//Teleporting through gates
	if (head.y == gate1.y && head.x == gate1.x) {
		head.y = gate2.y;
		head.x = gate2.x;
	}
	else if (head.y == gate2.y && head.x == gate2.x) {
		head.y = gate1.y;
		head.x = gate1.x;
	}
	//Bouncing of walls and obstacles
	switch (direction) {
	case LEFT:
		if (head.x <= 0) {
			head.x = width;
		}
		if (eatenEggRecently == 0 && (screenBuffer[head.x + head.y*width - 1].Char.AsciiChar == *obstcleChar || screenBuffer[head.x + head.y*width - 1].Char.AsciiChar == *wallChar))
		{
			direction = RIGHT;
			break;
		}
		head.x--;
		break;
	case RIGHT:
		if (head.x >= width)
			head.x = 0;
		if (eatenEggRecently == 0 &&
			(screenBuffer[head.x + head.y*width + 1].Char.AsciiChar == *obstcleChar || screenBuffer[head.x + head.y*width + 1].Char.AsciiChar == *wallChar))
		{
			direction = LEFT;
			break;
		}
		head.x++;
		break;
	case UP:
		if (head.y <= 0)
			head.y = height;
		if (eatenEggRecently == 0 && (screenBuffer[head.x + (head.y - 1)*width].Char.AsciiChar == *obstcleChar || screenBuffer[head.x + (head.y - 1)*width].Char.AsciiChar == *wallChar))
		{
			direction = DOWN;
			break;
		}
		head.y--;
		break;
	case DOWN:
		if (head.y >= height)
			head.y = 0;
		if (eatenEggRecently == 0 && (screenBuffer[head.x + (head.y + 1)*width].Char.AsciiChar == *obstcleChar || screenBuffer[head.x + (head.y + 1)*width].Char.AsciiChar == *wallChar))
		{
			direction = UP;
			break;
		}
		head.y++;
		break;
	case STOP:
		break;
	}
	//Egg intake
	if (head.y == egg.y && head.x == egg.x) {
		score++;
		putObstacle1();

		eatenEggRecently += level * level * level;
		if (eatenEggRecently > width) {
			eatenEggRecently = width;
		}
		point tailSeg;
		tailSeg.x = egg.x;
		tailSeg.y = egg.y;
		tailSeg.c = tailChar;
		tail.push_back(tailSeg);
		egg.x = rand() % width;
		egg.y = rand() % height;

		if (!egg.x) egg.x++;
		if (!egg.y) egg.y++;

		if (level > 5) {
			head.c = 'J';
			tail[0].c = 'u';
			tail[1].c = 'h';
			for (int i = 2; i < tail.size(); i++)
			{
				tail[i].c = 'u';
			}
			tailChar = 'u';
		}
	}
}

int main() {
	screenBuffer = new CHAR_INFO[width * height];

	SMALL_RECT windowSize = { 0, 0, width - 1, height - 1 };
	COORD bufferSize = { width, height };
	SMALL_RECT consoleWriteArea = { 0, 0, width, height };

	/* initialize handles */
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	/* Set the window size */
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	/* Set the screen's buffer size */
	SetConsoleScreenBufferSize(wHnd, bufferSize);

	SetConsoleTitle("Our shiny new snake!");

	// milliseconds
	int sleepDurationMax = 300;
	int sleepDurationFactor = 25;

	Setup();
	while (true) {

		Input();
		Logic();
		Draw();
		WriteConsoleOutputA(wHnd, screenBuffer, characterBufferSize, { 0, 0 }, &consoleWriteArea);
		Sleep(sleepDurationMax - level * sleepDurationFactor);
	}

	return 0;
}