#include <iostream>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include <list>
#include <math.h>
#include <string.h>
#include <sstream>

/*
 * Header files for X functions
 */
//#include "/opt/X11/include/X11/Xlib.h"
//#include "/opt/X11/include/X11/Xutil.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"



using namespace std;

const int FPS = 30;
const int PAD_LENGTH = 50;
const int BASE_HEIGHT = 250;
const int Border = 5;
const int BufferSize = 10;
const int window_width = 800;
const int window_height = 600;

int game_start;
int game_restart;
int score;
int score_plus;
int high_score;
int blink;
int width_diff;
int height_diff;
int level;

/*
 * Information to draw on the window.
 */
struct XInfo {
    Display   *display;
    int		 screen;
    Window	 window;
    Pixmap   pixmap;
    GC		 gc[3];
    int      width;
    int      height;
};

struct point {
    int x;
    int y;
    int type;    // 0 indicates Terrain, 1 indicates Pad, 2 indicates ship points
};


/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
    cerr << str << endl;
    exit(0);
}

class Displayable {
public:
    virtual void paint(XInfo &xinfo) = 0;
};


class Ship : public Displayable {
    int x;
    int y;
    int width;
    int feetLength;
    int diameter;
    int count_x;
    int count_y;
    int gravity;
    int air;
    /* status[0] indicates if the ship stopped (crash or land);
     status[1] indicates if the ship is paused;
     status[2] to status[3] indicates v for up, down, left and right
     status[4] indicates the fuel the ship has
     */
    int status[5];
    
    int thruster_fire[4];
    
public:
    
    Ship(int x, int y, int width, int fuel):x(x), y(y), width(width) {
        diameter = width * 0.6;
        feetLength = width * 0.4;
        count_x = 3;
        count_y = 3;
        gravity = 7;
        air = 30;
        for (int i = 0; i < 4; i++) {
            status[i] = 0;
        }
        for (int j = 0; j < 4; j++) {
            thruster_fire[j] = 0;
        }
        status[2] = 5;
        status[3] = 5;
        status[4] = fuel;
    }
    
    virtual void paint(XInfo &xinfo) {
        Display *display = xinfo.display;
        Window win = xinfo.pixmap;
        GC gc1 = xinfo.gc[1];
        GC gc2 = xinfo.gc[2];
        
        float fuel_bar = (float)status[4]/950;
        
        /* draw the body of the ship */
        XDrawRectangle(display, win, gc1, x, y, width, width);
        
        XDrawArc(display, win, gc1, x + (width/2) - (diameter/2), y + (width/2) - (diameter/2), diameter, diameter, 0, 360*64);
        
        /* draw the feet of the ship */
        XDrawLine(display, win, gc1, x + (feetLength/2), y + (width*1.5), x + (width/2), y + width);
        XDrawLine(display, win, gc1, x + width - (feetLength/2), y + (width*1.5), x + (width/2), y + width);
        XDrawLine(display, win, gc1, x, y + (width*1.5), x + feetLength, y + (width*1.5));
        XDrawLine(display, win, gc1, x + width, y + (width*1.5), x + width - feetLength, y + (width*1.5));
        
        XDrawString(display, win, gc2, 670, 10, "Fuel: ", strlen("Fuel: "));
        XDrawString(display, win, gc2, 670, 20, "Vertical Speed: ", strlen("Vertical Speed: "));
        XDrawString(display, win, gc2, 670, 30, "Horizontal Speed: ", strlen("Horizontal Speed: "));
        XDrawString(display, win, gc2, 670, 40, "Score: ", strlen("Score: "));
        XDrawString(display, win, gc2, 670, 50, "Level: ", strlen("Level: "));
        
        // draw the thruster fire
        for (int i = 0; i < 4; i++) {
            if (thruster_fire[i] > 0 && (thruster_fire[i] % 2) == 1) {
                switch (i) {
                    case 0:
                        XDrawLine(display, win, gc1, x, y + (width*1.5), x + (feetLength/2), y + (width*1.5) + (width*0.4));
                        XDrawLine(display, win, gc1, x + feetLength, y + (width*1.5), x + (feetLength/2), y + (width*1.5) + (width*0.4));
                        
                        XDrawLine(display, win, gc1, x + width, y + (width*1.5), x + width - (feetLength/2), y + (width*1.5) + (width*0.4));
                        XDrawLine(display, win, gc1, x + width - feetLength, y + (width*1.5), x + width - (feetLength/2), y + (width*1.5) + (width*0.4));
                        
                        XDrawLine(display, win, gc1, x + (0.1*width), y + (width*1.5), x + (feetLength/2), y + (width*1.5) + (width*0.2));
                        XDrawLine(display, win, gc1, x + feetLength - (0.1*width), y + (width*1.5), x + (feetLength/2), y + (width*1.5) + (width*0.2));
                        
                        XDrawLine(display, win, gc1, x + width - (0.1*width), y + (width*1.5), x + width - (feetLength/2), y + (width*1.5) + (width*0.2));
                        XDrawLine(display, win, gc1, x + width - feetLength + (0.1*width), y + (width*1.5), x + width - (feetLength/2), y + (width*1.5) + (width*0.2));
                        thruster_fire[i]--;
                        break;
                    case 1:
                        XDrawLine(display, win, gc1, x + (width*0.2), y, x + (width*0.5), y - (width*0.6));
                        XDrawLine(display, win, gc1, x + width - (width*0.2), y, x + (width*0.5), y - (width*0.6));
                        
                        XDrawLine(display, win, gc1, x + (width*0.3), y, x + (width*0.5), y - (width*0.4));
                        XDrawLine(display, win, gc1, x + width - (width*0.3), y, x + (width*0.5), y - (width*0.4));
                        thruster_fire[i]--;
                        break;
                    case 2:
                        XDrawLine(display, win, gc1, x + width, y + (width*0.2), x + width + (width*0.6), y + (width*0.5));
                        XDrawLine(display, win, gc1, x + width, y + width - (width*0.2), x + width + (width*0.6), y + (width*0.5));
                        
                        XDrawLine(display, win, gc1, x + width, y + (width*0.3), x + width + (width*0.4), y + (width*0.5));
                        XDrawLine(display, win, gc1, x + width, y + width - (width*0.3), x + width + (width*0.4), y + (width*0.5));
                        thruster_fire[i]--;
                        break;
                    case 3:
                        XDrawLine(display, win, gc1, x, y + (width*0.2), x - (width*0.6), y + (width*0.5));
                        XDrawLine(display, win, gc1, x, y + width - (width*0.2), x - (width*0.6), y + (width*0.5));
                        
                        XDrawLine(display, win, gc1, x, y + (width*0.3), x - (width*0.4), y + (width*0.5));
                        XDrawLine(display, win, gc1, x, y + width - (width*0.3), x - (width*0.4), y + (width*0.5));
                        thruster_fire[i]--;
                        break;
                        
                    default:
                        break;
                }
            }
        }
        
        /* draw the fuel bar */
        XFillRectangle(display, win, gc1, x, y - 10, width * fuel_bar, 5);
        
        // show the status of the ship
        stringstream ss1, ss2, ss3, ss4, ss5;
        string buffer1, buffer2, buffer3, buffer4, buffer5;
        
        ss1 << status[4];
        ss1 >> buffer1;
        ss2 << status[2];
        ss2 >> buffer2;
        ss3 << status[3];
        ss3 >> buffer3;
        ss4 << score;
        ss4 >> buffer4;
        ss5 << level;
        ss5 >> buffer5;
        
        const char* s1 = buffer1.c_str();
        const char* s2 = buffer2.c_str();
        const char* s3 = buffer3.c_str();
        const char* s4 = buffer4.c_str();
        const char* s5 = buffer5.c_str();
        
        XDrawString(display, win, gc2, 720, 10, s1, strlen(s1));
        XDrawString(display, win, gc2, 780, 20, s2, strlen(s2));
        XDrawString(display, win, gc2, 780, 30, s3, strlen(s3));
        XDrawString(display, win, gc2, 720, 40, s4, strlen(s4));
        XDrawString(display, win, gc2, 720, 50, s5, strlen(s5));
        
    }
    
    void drift(XInfo &xinfo) {
        
        // if the ship crash or land than skip drifting
        if (status[0] == 1) {
            return;
        }
        
        // x-vector
        if (count_x == 0) {
            if (status[3] > 0) {
                x = x + (status[3]/10 + 1);
            } else if (status[3] < 0) {
                x = x - ((- status[3])/10 + 1);
            }
            count_x = 3;
        } else {
            count_x--;
        }
        
        // y-vector
        if (count_y == 0) {
            if (status[2] > 0) {
                y = y + (status[2]/10 + 1);
            } else {
                y = y - ((- status[2])/10 + 1);
            }
            count_y = 3;
        } else {
            count_y--;
        }
        
        // gravity acceleration
        if (gravity == 0) {
            status[2]++;
            gravity = 7;
        } else {
            gravity--;
        }
        
        // air friction
        if (air == 0) {
            if (status[3] > 0) {
                status[3]--;
            } else if (status[3] < 0) {
                status[3]++;
            }
            air = 30;
        } else {
            air--;
        }
        
        // boundary checking, reflect if reach the bound
        if (y <= 0) {
            if (status[2] < 0) {
                status[2] = - status[2];
            }
        }
        
        if ((x + width) > 800 ) {
            x = 0;
        }
        if (x < 0) {
            x = 800 - width;
        }
        
        
    }
    
    void thruster(int direction) {
        
        int v_thruster = 3;
        if (status[4] > 0) {
            status[4] = status[4] - 2;
        }
        
        switch (direction) {
            case 1:
                status[2] = status[2] - v_thruster;
                thruster_fire[direction - 1] = 5;
                break;
            case 2:
                status[2] = status[2] + v_thruster;
                thruster_fire[direction - 1] = 5;
                break;
            case 3:
                status[3] = status[3] - v_thruster;
                thruster_fire[direction - 1] = 5;
                break;
            case 4:
                status[3] = status[3] + v_thruster;
                thruster_fire[direction - 1] = 5;
                break;
            default:
                break;
        }
    }
    
    void land() {
        if (tooFast() == 1 && status[4] > 100) {
            game_restart = 3;
        } else if (lowFuel() == 2){
            game_restart = 4;
        } else {
            game_restart = 1;
        }
        
        status[0] = 1;
        switch (level) {
            case 1:
                score_plus = 50;
                break;
            case 2:
                score_plus = 70;
                break;
            case 3:
                score_plus = 100;
                break;
            case 4:
                score_plus = 150;
                break;
            case 5:
                score_plus = 200;
                break;
            default:
                break;
        }
        score = score + score_plus;
    }
    
    void crash() {
        if (lowFuel() == 2) {
            game_restart = 4;
        } else if (status[4] > 100) {
            game_restart = 2;
            status[4] = status[4] - 100;
        } else if (status[4] >= 0 && status[4] <= 100){
            status[4] = 0;
            game_restart = 4;
        }
        status[0] = 1;
    }
    
    int tooFast() {
        if (status[2] > 30) {
            return 1;
        } else {
            return 0;
        }
    }
    
    int lowFuel() {
        if (status[4] <= 200 && status[4] > 100) {
            return 1;
        } else if (status[4] > 200) {
            return 0;
        } else if (status[4] > 0 && status[4] <= 100) {
            return 3;
        } else {
            return 2;
        }
    }
    
    void pause() {
        if (status[1] == 0) {
            status[1] = 1;
        } else if (status[1] == 1) {
            status[1] = 0;
        }
    }
    
    void restart() {
        x = 10;
        y = 50;
        status[0] = 0;
        status[1] = 0;
        status[2] = 5;
        status[3] = 5;
        
        if (status[4] <= 0) {
            score = 0;
            status[4] = 950;
        }
        
        game_restart = 0;
    }
    
    int isPause() {
        return status[1];
    }
    
    int isStop() {
        return status[0];
    }
    
    int getX() {
        return x;
    }
    
    int getY() {
        return y;
    }
    
    int getWidth() {
        return width;
    }
    
    int getHeight() {
        return width * 1.5;
    }
    
    point getLeftCorner() {
        point leftCorner;
        leftCorner.x = x;
        leftCorner.y = y + width * 1.5;
        leftCorner.type = 2;
        
        return leftCorner;
    }
    
    point getRightCorner() {
        point rightCorner;
        rightCorner.x = x + width;
        rightCorner.y = y + width * 1.5;
        rightCorner.type = 2;
        
        return rightCorner;
    }
};

class Meteor: public Displayable {
    int x;
    int y;
    int direction;
    int init_direction;
    int speed;
    int diameter;
    int init_x;
    int init_y;
    int period;
public:
    Meteor(int x, int y, int direction, int speed): x(x), y(y), direction(direction), speed(speed) {
        diameter = 10;
        init_x = x;
        init_y = y;
        init_direction = direction;
        period = rand() % 100 + 100;
    }
    virtual void paint(XInfo &xinfo) {
        Display* display = xinfo.display;
        Window win = xinfo.pixmap;
        GC gc1 = xinfo.gc[2];
        GC gc2 = xinfo.gc[1];
        XDrawArc(display, win, gc1, x - (diameter/2), y - (diameter/2), diameter, diameter, 0, 360*64);
        
        if (direction == 1 && blink == 2) {
            XDrawLine(display, win, gc2, x - diameter/2 + 2, y - 2, x - diameter/2 - 50, y);
            XDrawLine(display, win, gc2, x - diameter/2 + 2, y + 2, x - diameter/2 - 50, y);
        } else if (direction == 0 && blink == 2) {
            XDrawLine(display, win, gc2, x + diameter/2 - 2, y - 2, x + diameter/2 + 50, y);
            XDrawLine(display, win, gc2, x + diameter/2 - 2, y + 2, x + diameter/2 + 50, y);
        }
    }
    
    void move() {
        if (level == 2 || level == 4 || level == 5) {
            if (period <= 0) {
                if (direction == 0) {
                    direction = 1;
                } else {
                    direction = 0;
                }
                period = rand() % 100 + 100;
                
            }
        }
        
        if (direction == 0) {
            if (x <= -50) {
                x = window_width;
            }
            x = x - speed/10;
        } else {
            if (x >= window_width + 50) {
                x = 0;
            }
            x = x + speed/10;
        }
        
        period--;
    }
    
    void changeSpeed(int newSpeed) {
        speed = newSpeed;
    }
    
    int leftSide() {
        return x - diameter/2;
    }
    
    int rightSide() {
        return x + diameter/2;
    }
    
    int topSide() {
        return y - diameter/2;
    }
    
    int botSide() {
        return y + diameter/2;
    }
    
    void restart() {
        x = init_x;
        y = init_y;
        direction = init_direction;
    }
};


Ship ship(10, 50, 30, 950);
Meteor meteor1(100, 100, 1, 10);
Meteor meteor2(700, 150, 0, 10);
Meteor meteor3(400, 200, 1, 10);

list<point> tPoints;
list<point> pPoints;

point *generatePads(int numPads) {
    point *pad = new point[numPads];
    
    for (int i = 0; i < numPads; i++) {
        if (numPads == 2) {
            pad[i].x = rand() % (400 - PAD_LENGTH + 1) + i * 400;
        } else {
            pad[i].x = rand() % (266 - PAD_LENGTH + 1) + i * 266;
        }
        
        pad[i].y = rand() % 200 + BASE_HEIGHT;
    }
    
    return pad;
}


void drawTerrain(XInfo &xinfo, int gcIndex, point pad[], int numPads) {
    Display *display = xinfo.display;
    Window win = xinfo.pixmap;
    GC gc = xinfo.gc[gcIndex];
    
    point curPoint;
    point distance;
    
    curPoint.x = 0;
    curPoint.y = BASE_HEIGHT + 200;
    curPoint.type = 0;
    tPoints.push_front(curPoint);
    
    for (int i = 0; i < numPads; i++) {
        while (pad[i].x - curPoint.x > 40) {
            distance.x = rand() % 20 + 20;
            distance.y = rand() % 100;
            
            if (curPoint.y >  BASE_HEIGHT + 100) {
                distance.y = 0 - distance.y;
            }
            
            XDrawLine(display, win, gc, curPoint.x, curPoint.y, curPoint.x + distance.x, curPoint.y + distance.y);
            curPoint.x = curPoint.x + distance.x;
            curPoint.y = curPoint.y + distance.y;
            curPoint.type = 0;
            tPoints.push_front(curPoint);
        }
        XDrawLine(display, win, gc, curPoint.x, curPoint.y, pad[i].x, pad[i].y);
        pad[i].type = 1;
        tPoints.push_front(pad[i]);
        pPoints.push_front(pad[i]);
        
        curPoint.x = pad[i].x + PAD_LENGTH;
        curPoint.y = pad[i].y;
        curPoint.type = 1;
        tPoints.push_front(curPoint);
        pPoints.push_front(curPoint);
        
        
        if (i == numPads - 1) {
            while (800 - curPoint.x > 40) {
                distance.x = rand() % 20 + 20;
                distance.y = rand() % 100;
                
                if (curPoint.y >  BASE_HEIGHT + 100) {
                    distance.y = 0 - distance.y;
                }
                
                XDrawLine(display, win, gc, curPoint.x, curPoint.y, curPoint.x + distance.x, curPoint.y + distance.y);
                curPoint.x = curPoint.x + distance.x;
                curPoint.y = curPoint.y + distance.y;
                curPoint.type = 0;
                tPoints.push_front(curPoint);
                
            }
            
            distance.y = rand() % 100;
            
            if (curPoint.y >  BASE_HEIGHT + 100) {
                distance.y = 0 - distance.y;
            }
            
            XDrawLine(display, win, gc, curPoint.x, curPoint.y, 800, curPoint.y + distance.y);
            curPoint.x = 800;
            curPoint.y = curPoint.y + distance.y;
            curPoint.type = 0;
            tPoints.push_front(curPoint);
            
        }
    }
}

void repaintTerrain(XInfo &xinfo) {
    list<point>::const_iterator begin = tPoints.begin();
    list<point>::const_iterator end = tPoints.end();
    point p1,p2;
    int lastPad;
    
    while (begin != end) {
        
        p1 = *begin;
        begin++;
        p2 = *begin;
        
        if (p1.type == 1 && p2.type == 1) {
            if (p1.x != lastPad) {
                XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[2], p1.x, p1.y, p2.x, p2.y);
                lastPad = p2.x;
                //                cout << p1.x << "," << p1.y << "+" << p2.x << "," << p2.y << endl;
            }
        } else {
            XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], p1.x, p1.y, p2.x, p2.y);
            //            cout << p1.x << "," << p1.y << "+" << p2.x << "," << p2.y << endl;
        }
        
        if (p2.x == 0 && p2.y == 450) {
            break;
        }
        
        if (p2.x == 1 && p2.y == 0) {
            continue;
        }
        
    }
    
    p1 = p2;
    p2 = *begin;
    XDrawLine(xinfo.display, xinfo.pixmap, xinfo.gc[1], p1.x, p1.y, p2.x, p2.y);
    //    cout << p1.x << "," << p1.y << "+" << p2.x << "," << p2.y << endl;
    
}

void drawPads(XInfo &xinfo, int gcIndex, point pad[], int numPads) {
    Display *display = xinfo.display;
    Window win = xinfo.pixmap;
    GC gc = xinfo.gc[gcIndex];
    
    for (int i = 0; i < numPads; i++) {
        XDrawLine(display, win, gc, pad[i].x, pad[i].y, pad[i].x + PAD_LENGTH, pad[i].y);
    }
}

/*
 * Initialize X and create a window
 */
void initX(int argc, char *argv[], XInfo &xInfo) {
    XSizeHints hints;
    unsigned long white, black;
    
    /*
     * Display opening uses the DISPLAY	environment variable.
     * It can go wrong if DISPLAY isn't set, or you don't have permission.
     */
    xInfo.display = XOpenDisplay( "" );
    if ( !xInfo.display )	{
        error( "Can't open display." );
    }
    
    /*
     * Find out some things about the display you're using.
     */
    xInfo.screen = DefaultScreen( xInfo.display );
    
    white = XWhitePixel( xInfo.display, xInfo.screen );
    black = XBlackPixel( xInfo.display, xInfo.screen );
    
    hints.x = 100;
    hints.y = 100;
    hints.width = 800;
    hints.height = 600;
    hints.flags = PPosition | PSize;
    
    xInfo.width = 800;
    xInfo.height = 600;
    
    xInfo.window = XCreateSimpleWindow(
                                       xInfo.display,				// display where window appears
                                       DefaultRootWindow( xInfo.display ), // window's parent in window tree
                                       hints.x, hints.y,			// upper left corner location
                                       hints.width, hints.height,	// size of the window
                                       Border,						// width of window's border
                                       white,						// window border colour
                                       white );					// window background colour
    
    XSetStandardProperties(
                           xInfo.display,		// display containing the window
                           xInfo.window,		// window whose properties are set
                           "XLander",	// window's title
                           "SD",				// icon's title
                           None,				// pixmap for the icon
                           argv, argc,			// applications command line args
                           &hints );			// size hints for the window
    
    /*
     * Create Graphics Contexts
     */
    int i = 0;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       1, LineSolid, CapButt, JoinRound);
    
    i = 1;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       1, LineSolid, CapRound, JoinMiter);
    
    XFontStruct* font_info;
    const char* font_name = "*-helvetica-*-24-*";
    font_info = XLoadQueryFont(xInfo.display, font_name);
    
    XSetFont(xInfo.display, xInfo.gc[1], font_info->fid);
    
    i = 2;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
    XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                       7, LineOnOffDash, CapButt, JoinBevel);
    
    int depth = DefaultDepth(xInfo.display, DefaultScreen(xInfo.display));
	xInfo.pixmap = XCreatePixmap(xInfo.display, xInfo.window, hints.width, hints.height, depth);
    
    XSelectInput(xInfo.display, xInfo.window,
                 ButtonPressMask | KeyPressMask |
                 PointerMotionMask |
                 EnterWindowMask | LeaveWindowMask |
                 StructureNotifyMask);  // for resize events
    
    
    /*
     * Put the window on the screen.
     */
    XMapRaised( xInfo.display, xInfo.window );
    
    XFlush(xInfo.display);
    sleep(2);	// let server get set up before sending drawing commands
}

void repaint(XInfo &xinfo) {
    
    if (game_start == 0) {
        return;
    }
    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0],
                   0, 0, xinfo.width, xinfo.height);
    
    
    ship.paint(xinfo);
    repaintTerrain(xinfo);
    
    meteor1.paint(xinfo);
    meteor2.paint(xinfo);
    if (level >= 3) {
        meteor3.paint(xinfo);
    }
    
    
    if ((ship.lowFuel() == 1 || ship.lowFuel() == 3) && blink == 2 && ship.isStop() ==0 && ship.isPause() == 0) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 100, "Low Fuel !", strlen("Low Fuel !"));
    }
    if (ship.lowFuel() == 2 && blink == 2 && ship.isStop() ==0 && ship.isPause() == 0) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 360, 100, "No Fuel !", strlen("No Fuel !"));
    }
    
    if (ship.isPause() == 1 && ship.isStop() == 0) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 100, "Game paused.", strlen("Game paused."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 280, 150, "Press Space to continue.", strlen("Press Space to continue."));
    }
    //    cout << "restart: " << game_restart << endl;
    
    if (game_restart == 1) {
        stringstream ss;
        string buffer;
        ss << score_plus;
        ss >> buffer;
        const char* s = buffer.c_str();
        
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 310, 100, "Land successfully.", strlen("Land successfully."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 270, 150, "Press SPACE to continue.", strlen("Press SPACE to continue."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 200, "Score + ", strlen("Score + "));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 450, 200, s, strlen(s));
    } else if (game_restart == 2) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 300, 100, "Crash into terrain.", strlen("Crash into terrain."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 270, 150, "Press SPACE to continue.", strlen("Press SPACE to continue."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 200, "Fuel - 100", strlen("Fuel - 100"));
    } else if (game_restart == 3) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 280, 100, "Crash into landing pad.", strlen("Crash into landing pad."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 270, 150, "Press SPACE to continue.", strlen("Press SPACE to continue."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 200, "Fuel - 100", strlen("Fuel - 100"));
    } else if (game_restart == 4) {
        stringstream ss;
        string buffer;
        ss << score;
        ss >> buffer;
        const char* s = buffer.c_str();
        
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 310, 100, "GAME OVER.", strlen("GAME OVER."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 280, 150, "Your score is: ", strlen("Your score is: "));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 470, 150, s, strlen(s));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 250, 200, "Press SPACE to try again.", strlen("Press SPACE to try again."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 300, 250, "Press Q to quit", strlen("Press Q to quit"));
        if (score > high_score) {
            high_score = score;
            if (blink == 2) {
                XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 280, 300, "New HIGH SCORE !!!.", strlen("New HIGH SCORE !!!"));
            }
        }
        
    } else if (game_restart == 5) {
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 300, 100, "Crash into meteor.", strlen("Crash into meteor."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 270, 150, "Press SPACE to continue.", strlen("Press SPACE to continue."));
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 350, 200, "Fuel - 100", strlen("Fuel - 100"));
    }
    
    XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0],
              0, 0, window_width, window_height,  // region of pixmap to copy
              width_diff/2, height_diff/2); // position to put top left corner of pixmap in window
    
    if (blink == 0 || blink == 1) {
        blink++;
    } else if (blink == 2) {
        blink = 0;
    }
    
    XFlush(xinfo.display);
}

void handleButtonPress(XInfo &xinfo, XEvent &event) {
    printf("Got button press!\n");
    // dList.push_front(new Text(event.xbutton.x, event.xbutton.y, "Urrp!"));
    // repaint( dList, xinfo );
    
}

void handleKeyPress(XInfo &xinfo, XEvent &event) {
    KeySym key;
    char text[BufferSize];
    
    /*
     * Exit when 'q' is typed.
     * This is a simplified approach that does NOT use localization.
     */
    int i = XLookupString(
                          (XKeyEvent *)&event, 	// the keyboard event
                          text, 					// buffer when text will be written
                          BufferSize, 			// size of the text buffer
                          &key, 					// workstation-independent key symbol
                          NULL );					// pointer to a composeStatus structure (unused)
    
    if (i == 1) {
        if (key == XK_q) {
            error("Terminating normally.");
        }else if (key == XK_1) {
            level = 1;
            meteor1.changeSpeed(10);
            meteor2.changeSpeed(10);
        }else if (key == XK_2) {
            level = 2;
            meteor1.changeSpeed(20);
            meteor2.changeSpeed(20);
        }else if (key == XK_3) {
            level = 3;
            meteor1.changeSpeed(20);
            meteor2.changeSpeed(20);
            meteor3.changeSpeed(20);
        }else if (key == XK_4) {
            level = 4;
            meteor1.changeSpeed(30);
            meteor2.changeSpeed(30);
            meteor3.changeSpeed(30);
        }else if (key == XK_5) {
            level = 5;
            meteor1.changeSpeed(40);
            meteor2.changeSpeed(40);
            meteor3.changeSpeed(40);
        }else if (key == XK_space) {
            if (game_start == 0) {
                game_start = 1;
            } else if (game_restart != 0){
                tPoints.clear();
                pPoints.clear();
                
                int numPads = rand() % 2 + 2;
                
                point* pad = generatePads(numPads);
                
                drawTerrain(xinfo, 0, pad, numPads);
                
                drawPads(xinfo, 2, pad, numPads);
                
                ship.restart();
                
                meteor1.restart();
                meteor2.restart();
                if (level >= 3) {
                    meteor3.restart();
                }
                
                XFlush(xinfo.display);
            } else {
                ship.pause();
            }
        }
    }
    
    
    if (game_start == 0) {
        return;
    }
    
    if ( i != 1) {
        //		printf("Got key press -- %c\n", text[0]);
        if (key == XK_Up) {
            if (ship.isPause() == 0 && ship.isStop() == 0 && ship.lowFuel() != 2) {
                ship.thruster(1);
            }
        } else if (key == XK_Down) {
            if (ship.isPause() == 0 && ship.isStop() == 0 && ship.lowFuel() != 2) {
                ship.thruster(2);
            }
        } else if (key == XK_Left) {
            if (ship.isPause() == 0 && ship.isStop() == 0 && ship.lowFuel() != 2) {
                ship.thruster(3);
            }
        } else if (key == XK_Right) {
            if (ship.isPause() == 0 && ship.isStop() == 0 && ship.lowFuel() != 2) {
                ship.thruster(4);
            }
        }
    }
}

void handleAnimation(XInfo &xinfo) {
    
    if (game_start == 0) {
        return;
    }
    
    if (game_restart != 0) {
        return;
    }
    
    if (ship.isPause() == 1) {
        return;
    }
    
    
    list<point>::const_iterator begin1 = pPoints.begin();
    list<point>::const_iterator end1 = pPoints.end();
    
    point p1, p2;
    int land_crash = 0;
    
    
    ship.drift(xinfo);
    meteor1.move();
    if (meteor1.rightSide() >= ship.getLeftCorner().x && meteor1.rightSide() <= ship.getLeftCorner().x && ((meteor1.topSide() <= ship.getLeftCorner().y && meteor1.topSide() >= ship.getY()) || (meteor1.botSide() >= ship.getY() && meteor1.botSide() <= ship.getLeftCorner().y))) {
        int temp;
        temp = ship.lowFuel();
        ship.crash();
        land_crash = 1;
        if (temp == 3) {
            game_restart = 4;
        } else {
            game_restart = 5;
        }
        return;
    }
    
    if (meteor1.leftSide() <= ship.getRightCorner().x && meteor1.rightSide() >= ship.getLeftCorner().x && ((meteor1.topSide() <= ship.getLeftCorner().y && meteor1.topSide() >= ship.getY()) || (meteor1.botSide() >= ship.getY() && meteor1.botSide() <= ship.getLeftCorner().y))) {
        int temp;
        temp = ship.lowFuel();
        ship.crash();
        land_crash = 1;
        if (temp == 3) {
            game_restart = 4;
        } else {
            game_restart = 5;
        }
        return;
    }
    
    
    meteor2.move();
    if (meteor2.rightSide() >= ship.getLeftCorner().x && meteor2.rightSide() <= ship.getLeftCorner().x && ((meteor2.topSide() <= ship.getLeftCorner().y && meteor2.topSide() >= ship.getY()) || (meteor2.botSide() >= ship.getY() && meteor2.botSide() <= ship.getLeftCorner().y))) {
        int temp;
        temp = ship.lowFuel();
        ship.crash();
        land_crash = 1;
        if (temp == 3) {
            game_restart = 4;
        } else {
            game_restart = 5;
        }
        return;
    }
    
    if (meteor2.leftSide() <= ship.getRightCorner().x && meteor2.rightSide() >= ship.getLeftCorner().x && ((meteor2.topSide() <= ship.getLeftCorner().y && meteor2.topSide() >= ship.getY()) || (meteor2.botSide() >= ship.getY() && meteor2.botSide() <= ship.getLeftCorner().y))) {
        int temp;
        temp = ship.lowFuel();
        ship.crash();
        land_crash = 1;
        if (temp == 3) {
            game_restart = 4;
        } else {
            game_restart = 5;
        }
        return;
    }
    
    if (level >= 3) {
        meteor3.move();
        if (meteor3.rightSide() >= ship.getLeftCorner().x && meteor3.rightSide() <= ship.getLeftCorner().x && ((meteor3.topSide() <= ship.getLeftCorner().y && meteor3.topSide() >= ship.getY()) || (meteor3.botSide() >= ship.getY() && meteor3.botSide() <= ship.getLeftCorner().y))) {
            int temp;
            temp = ship.lowFuel();
            ship.crash();
            land_crash = 1;
            if (temp == 3) {
                game_restart = 4;
            } else {
                game_restart = 5;
            }
            return;
        }
        
        if (meteor3.leftSide() <= ship.getRightCorner().x && meteor3.rightSide() >= ship.getLeftCorner().x && ((meteor3.topSide() <= ship.getLeftCorner().y && meteor3.topSide() >= ship.getY()) || (meteor3.botSide() >= ship.getY() && meteor3.botSide() <= ship.getLeftCorner().y))) {
            int temp;
            temp = ship.lowFuel();
            ship.crash();
            land_crash = 1;
            if (temp == 3) {
                game_restart = 4;
            } else {
                game_restart = 5;
            }
            return;
        }
        
    }
    
    
    
    /* the judgements between ship and pads */
    
    while (begin1 != end1) {
        p1 = *begin1;
        begin1++;
        p2 = *begin1;
        
        // judge if the ship land correctly
        if ((ship.getX() >= p2.x && ship.getX() <= p1.x && (ship.getX() + ship.getWidth()) >= p1.x && (ship.getY() + ship.getHeight()) >= p2.y) ||
            (ship.getX() <= p2.x && (ship.getX() + ship.getWidth()) >= p2.x && (ship.getX() + ship.getWidth()) <= p1.x && (ship.getY() + ship.getHeight()) >= p2.y)
            && land_crash == 0)
        {
            ship.crash();
            land_crash = 1;
            break;
        }
        
        // judge if the ship land
        if (ship.getX() >= p2.x && (ship.getX() + ship.getWidth()) <= p1.x && land_crash == 0) {
            if (ship.getY() + ship.getHeight() >= p2.y) {
                ship.land();
                land_crash = 1;
                break;
            } else {
                return;
            }
        }
        begin1++;
    }
    
    /* the judgements between ship and terrains */
    
    list<point>::const_iterator begin2 = tPoints.begin();
    list<point>::const_iterator end2 = tPoints.end();
    
    point t1, t2;
    point x1, x2, x3, x4;
    float slope1, slope2, constant1, constant2;
    int left_right;       // 0 indicates left corner, 1 indicates right corner
    
    if (land_crash == 0) {
        while (begin2 != end2) {
            t1 = *begin2;
            begin2++;
            t2 = *begin2;
            
            
            if (t2.x <= ship.getX() && t1.x >= ship.getX() && t1.x < ship.getX() + ship.getWidth()) {
                x3.x = t2.x;
                x3.y = t2.y;
                x4.x = t1.x;
                x4.y = t1.y;
                break;
                
            }
            if (t2.x <= (ship.getX() + ship.getWidth()) && t1.x >= (ship.getX() + ship.getWidth())) {
                if (t2.x <= ship.getX()) {
                    return;
                }
                x1.x = t2.x;
                x1.y = t2.y;
                x2.x = t1.x;
                x2.y = t1.y;
            }
        }
        //        cout << x1.x << "," << x1.y << "," << x2.x << "," << x2.y << endl;
        //        cout << x3.x << "," << x3.y << "," << x4.x << "," << x4.y << endl;
        
        if (x1.y == x2.y) {
            slope1 = 0;
        } else {
            slope1 = ((float)x2.y - (float)x1.y) / ((float)x2.x - (float)x1.x);
        }
        constant1 = (float)x1.y - (float)x1.x * (slope1);
        
        if (x3.y == x4.y) {
            slope2 = 0;
        } else {
            slope2 = ((float)x3.y - (float)x4.y) / ((float)x3.x - (float)x4.x);
        }
        constant2 = (float)x3.y - (float)x3.x * (slope2);
        
        
        float y1, y2;
        
        y1 = ship.getRightCorner().x * slope1 + constant1;
        y2 = ship.getLeftCorner().x * slope2 + constant2;
        
        //        cout << "slope1: " << slope1 << "," <<"slope2: "<< slope2 << endl;
        //        cout << "y1: " << y1 << "," <<"y2: "<< y2 << endl;
        
        if (slope1 > 0) {
            if (slope2 <= 0) {
                if (x1.y > x4.y) {
                    if (ship.getLeftCorner().y >= x4.y) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "1" << endl;
                    }
                } else {
                    if (x2.x == x3.x && x2.y == x3.y) {
                        if (ship.getRightCorner().y >= y1) {
                            ship.crash();
                            land_crash = 1;
                            cout << "crash: " << "2a" << endl;
                        }
                    }
                    if (ship.getRightCorner().y >= x1.y) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "2b" << endl;
                    }
                }
            } else {
                if (x1.x == x3.x && x1.y == x3.y) {
                    if (ship.getRightCorner().y >= y2) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "3a" << endl;
                    }
                }
                else {
                    if (ship.getRightCorner().y >= x1.y || ship.getLeftCorner().y >= y2) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "3b" << endl;
                    }
                }
            }
        } else if (slope1 < 0) {
            if (slope2 < 0) {
                if (ship.getLeftCorner().y >= x3.y || ship.getRightCorner().y >= y1) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "3" << endl;
                    //                    cout << x1.x << "," << x1.y << "," << x2.x << "," << x2.y << endl;
                    //                    cout << x3.x << "," << x3.y << "," << x4.x << "," << x4.y << endl;
                    //                    cout << ship.getLeftCorner().y << x3.y << "," << y1 << endl;
                }
            } else if (slope2 > 0) {
                if (ship.getRightCorner().y >= y1 || ship.getLeftCorner().y >= y2) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "4" << endl;
                    //                    cout << x1.x << "," << x1.y << "," << x2.x << "," << x2.y << endl;
                    //                    cout << x3.x << "," << x3.y << "," << x4.x << "," << x4.y << endl;
                    //                    cout << ship.getLeftCorner().y <<","<< y1 << "," << y2 << endl;
                }
            } else {
                if (ship.getRightCorner().y >= y1) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "5" << endl;
                }
                
                if (ship.getLeftCorner().y >= x4.y) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "6" << endl;
                }
            }
        } else {
            if (slope2 > 0) {
                if (ship.getLeftCorner().y >= y2) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "7" << endl;
                }
                
                if (ship.getRightCorner().y >= x1.y) {
                    ship.crash();
                    land_crash = 1;
                    cout << "crash: " << "8" << endl;
                }
            } else if (slope2 < 0) {
                if (x4.y <= x1.y) {
                    if (ship.getLeftCorner().y >= x4.y) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "9" << endl;
                    }
                } else {
                    if (ship.getRightCorner().y >= x1.y) {
                        ship.crash();
                        land_crash = 1;
                        cout << "crash: " << "10" << endl;
                    }
                }
            } else {
                if (ship.getLeftCorner().y >= x1.y || ship.getLeftCorner().y >= x4.y) {
                    //                    ship.crash();
                    //                    land_crash = 1;
                    return;
                }
            }
        }
    }
}


// update width and height when window is resized
void handleResize(XInfo &xinfo, XEvent &event) {
    XConfigureEvent xce = event.xconfigure;
    fprintf(stderr, "Handling resize  w=%d  h=%d\n", xce.width, xce.height);
    if (xce.width >= xinfo.width && xce.height >= xinfo.height) {
        width_diff = xce.width - window_width;
        height_diff = xce.height - window_height;
        if (game_start == 0) {
            XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0],
                           0, 0, xinfo.width, xinfo.height);
            
            repaintTerrain(xinfo);
            
            XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[1], 360, 100, "XLander", strlen("XLander"));
            XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[2], 350, 150, "Press SPACE to start", strlen("Press SPACE to start"));
            XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[2], 350, 170, "Press Q to quit", strlen("Press Q to quit"));
            XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[2], 350, 190, "Use arrow key to control", strlen("Use arrow key to control"));
            
            
            XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0],
                      0, 0, window_width, window_height,  // region of pixmap to copy
                      width_diff/2, height_diff/2); // position to put top left corner of pixmap in window
            
        }
    } else if (xce.width < xinfo.width || xce.height < xinfo.height) {
        width_diff = xce.width - window_width;
        height_diff = xce.height - window_height;
        game_start = 0;
        XClearWindow(xinfo.display, xinfo.window);
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[0], 330 + width_diff/2, 240 + height_diff/2, "Too Small !!!", strlen("Too Small !!!"));
        XDrawString(xinfo.display, xinfo.window, xinfo.gc[0], 320 + width_diff/2, 260 + height_diff/2, "Press Q to quit", strlen("Press Q to quit"));
    }
}

// get microseconds
unsigned long now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}


void eventLoop(XInfo &xinfo) {
    XEvent event;
    unsigned long lastRepaint = 0;
    int inside = 0;
    
    while( true ) {
        
        if (XPending(xinfo.display) > 0) {
            
            XNextEvent( xinfo.display, &event );
            switch( event.type ) {
                case ButtonPress:
                    handleButtonPress(xinfo, event);
                    break;
                case KeyPress:
                    handleKeyPress(xinfo, event);
                    break;
                case ConfigureNotify:
                    handleResize(xinfo, event);
                    break;
            }
        }
        
        // animation timing
        unsigned long end = now();
        if (end - lastRepaint > 1000000/FPS) {
            handleAnimation(xinfo);
            repaint(xinfo);
            lastRepaint = now();
        }
        // give the system time to do other things
        if (XPending(xinfo.display) == 0) {
            usleep(1000000/FPS - (end - lastRepaint));
        }
    }
    
}


/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 *	 Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char *argv[] ) {
    XInfo xInfo;
    
    srand(time(NULL));
    
    width_diff = 0;
    height_diff = 0;
    
    initX(argc, argv, xInfo);
    
    int numPads = rand() % 2 + 2;
    point *pad = generatePads(numPads);
    
    XFillRectangle(xInfo.display, xInfo.pixmap, xInfo.gc[0],
                   0, 0, xInfo.width, xInfo.height);
    
    drawTerrain(xInfo, 1, pad, numPads);
    
    drawPads(xInfo, 2, pad, numPads);
    
    
    XDrawString(xInfo.display, xInfo.pixmap, xInfo.gc[1], 360, 100, "XLander", strlen("XLander"));
    XDrawString(xInfo.display, xInfo.pixmap, xInfo.gc[1], 270, 150, "Press SPACE to start", strlen("Press SPACE to start"));
    XDrawString(xInfo.display, xInfo.pixmap, xInfo.gc[1], 270, 200, "Use arrow key to control direction", strlen("Use arrow key to control direction"));
    XDrawString(xInfo.display, xInfo.pixmap, xInfo.gc[1], 270, 250, "Use number key 1-5 to change level", strlen("Use number key 1-5 to change level"));
    
    XCopyArea(xInfo.display, xInfo.pixmap, xInfo.window, xInfo.gc[0],
              0, 0, window_width, window_height,  // region of pixmap to copy
              width_diff/2, height_diff/2); // position to put top left corner of pixmap in window
    
    game_start = 0;
    game_restart = 0;
    level = 1;
    score = 0;
    score_plus = 50;
    blink = 0;
    
    /* flush all pending requests to the X server. */
    XFlush(xInfo.display);
    
    eventLoop(xInfo);
    
    XCloseDisplay(xInfo.display);
}
