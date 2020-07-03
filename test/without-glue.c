#include <stdbool.h>

#define BOUNDS 255
#define RECT_SIDE 50
#define BOUNCE_POINT (BOUNDS - RECT_SIDE)

extern int jsClearRect();
extern int jsFillRect(int x, int y, int width, int height);

bool isRunning = true;

typedef struct Rect {
    int x;
    int y;
    char direction;
} Rect;

struct Rect rect;

void updateRectLocation() {
    if (rect.x == BOUNCE_POINT) rect.direction = 'L';

    if (rect.x < 1) rect.direction = 'R';

    int incrementer = 1;
    if (rect.direction == 'L') incrementer = -1;
    rect.x += incrementer;
    rect.y += incrementer;
}

void moveRect() {
    jsClearRect();
    updateRectLocation();
    jsFillRect(rect.x, rect.y, RECT_SIDE, RECT_SIDE);
}

bool getIsRunning() {
    return isRunning;
}

void setIsRunning(bool newIsRunning) {
    isRunning = newIsRunning;
}

void init() {
    rect.x = 0;
    rect.y = 0;
    rect.direction = 'R';
    setIsRunning(true);
}