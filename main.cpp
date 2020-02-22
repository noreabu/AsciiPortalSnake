#include <ncurses.h>
#include <unistd.h>
#include <random>

#define DELAY 35000

using namespace std;

struct point
{
	int x, y;
	char c = 'o';
};

int width, height;

char mainChar = 0;
char oldChar;

char headChar = 'U';
char tailChar = 9;
char* obstcleChar = &mainChar;
const int obstacleNumber = 25;
const char wallChar = 'O';
const char spaceChar = ' ';
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

void putChar(point position, short foregroudColor, short backgroundColor, const char* character) {
  start_color();
	init_pair(1, foregroudColor, backgroundColor);
	attron(COLOR_PAIR(1));
  mvprintw(position.y, position.x, character);
  attroff(COLOR_PAIR(1));
}

void drawWalls() {
	point cursor;
	for (cursor.y = 0; cursor.y < height; cursor.y++) {
		for (cursor.x = 0; cursor.x < width; cursor.x++) {
			if (cursor.y == 0 || cursor.x == 0 || cursor.x == width - 1 || cursor.y == height - 1) {
				putChar(cursor, COLOR_RED, COLOR_BLACK, &wallChar);
			}
			else {
				putChar(cursor, COLOR_BLACK, COLOR_BLACK, &spaceChar);
			}
		}
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
	// growObstacles();
}

void resize() {
  getmaxyx(stdscr, height, width);
}


int main(int argc, char *argv[]) {
  initscr();
  noecho();
  curs_set(FALSE);


  while (1) {
    resize();
    clear();
    Setup();
    usleep(DELAY);
    refresh();
  }

  endwin();

  return 0;
}

