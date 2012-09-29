/*
 * GENERIC ALL-PURPOSE SPRITE CLASS
 *
 * Author: Jonathan Harbour
 * Date: Early 21st century
 *
 * This class was not intended to be a textbook example of OOP design.
 * It's a simple-to-use class with no accessor/mutator crap cluttering up the code.
 * If you don't like it, feel free to sub-class it and make your own nicely-formed class.
 */

#ifndef _SPRITE_H
#define _SPRITE_H 1

#include <allegro.h>
#include <math.h>

class sprite {
// PROPERTIES
private:
    BITMAP *frame;
	BITMAP *image;
public:
    int alive;
	int state;
	int objecttype;
	int direction;
    double x,y;
    int width,height;
    double speed;
    double velx, vely;
    int xdelay,ydelay;
    int xcount,ycount;
    int curframe,totalframes,animdir;
    int framecount,framedelay;
	int animcolumns;
	int animstartx, animstarty;
	float faceAngle, moveAngle;
    int counter1, threshold1;
    int counter2, threshold2;
    int counter3, threshold3;

// METHODS
private:
	~sprite();
	int inside(int x,int y,int left,int top,int right,int bottom);

public:
	sprite();
    void destroy();
	int load(char *filename);
	void move();
	void animate();
	int pointInside(int px,int py);
	int collided(sprite *other = NULL, int shrink = 5);
    double calcAngleMoveX(int angle);
    double calcAngleMoveY(int angle);

	void draw(BITMAP *dest);
	void drawframe(BITMAP *dest);
    void drawframe(BITMAP *dest, int angle);
    void drawframe(BITMAP *dest, int dest_w, int dest_h);

};

#endif
