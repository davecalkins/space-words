/*
 * GENERIC ALL-PURPOSE SPRITE CLASS
 *
 * Author: Jonathan Harbour
 * Date: Early 21st century
 *
 */

#include <allegro.h>
#include "sprite.h"

#define PI 3.1415926535
#define PI_div_180 0.017453292519444

sprite::sprite() {
    this->image = NULL;
    this->frame = NULL;
    this->alive = 1;
    this->direction = 0;
    this->animcolumns = 1;
    this->animstartx = 0; 
    this->animstarty = 0;
    this->x = 0.0f; 
    this->y = 0.0f;
    this->width = 0; 
    this->height = 0;
    this->xdelay = 0; 
    this->ydelay = 0;
    this->xcount = 0; 
    this->ycount = 0;
    this->velx = 0.0; 
    this->vely = 0.0;
    this->speed = 0.0;
    this->curframe = 0; 
    this->totalframes = 1;
    this->framecount = 0; 
    this->framedelay = 10;
    this->animdir = 1;
    this->faceAngle = 0; 
    this->moveAngle = 0;
    this->counter1 = 0;
    this->counter2 = 0;
    this->counter3 = 0;
    this->threshold1 = 0;
    this->threshold2 = 0;
    this->threshold3 = 0;
}

sprite::~sprite()  {
    this->destroy();
}

void sprite::destroy() {
    if (this->image != NULL)
        destroy_bitmap(this->image);
}

int sprite::load(char *filename) {
	this->image = load_bitmap(filename, NULL);
	if (!this->image) {
		allegro_message("Error loading sprite file");
		return 0;
	}
	this->width = image->w;
	this->height = image->h;
    return 1;
}

void sprite::draw(BITMAP *dest) {
    if (this->image)
	    draw_sprite(dest, this->image, (int)this->x, (int)this->y);
}

//draw normally
void sprite::drawframe(BITMAP *dest)  {
    if (!this->image) return;

    int fx = this->animstartx + (this->curframe % this->animcolumns) * this->width;
    int fy = this->animstarty + (this->curframe / this->animcolumns) * this->height;
    masked_blit(this->image, dest, fx, fy, (int)this->x, (int)this->y, this->width, this->height);
}

//draw with scaling
void sprite::drawframe(BITMAP *dest, int dest_w, int dest_h)  {
    if (!this->image) return;

    int fx = this->animstartx + (this->curframe % this->animcolumns) * this->width;
    int fy = this->animstarty + (this->curframe / this->animcolumns) * this->height;
    masked_stretch_blit(this->image, dest, fx, fy, this->width, this->height, (int)this->x, (int)this->y, dest_w, dest_h);
}

//draw with rotation
void sprite::drawframe(BITMAP *dest, int angle)  {
    if (!this->image) return;

    //create scratch frame if necessary
    if (!this->frame) {
        this->frame = create_bitmap(this->width, this->height);
    }


    //first, draw frame normally but send it to the scratch frame image
    int fx = this->animstartx + (this->curframe % this->animcolumns) * this->width;
    int fy = this->animstarty + (this->curframe / this->animcolumns) * this->height;
    blit(this->image, this->frame, fx, fy, 0, 0, this->width, this->height);

    //draw rotated image in scratch frame onto dest 
    //adjust for Allegro's 16.16 fixed trig (256 / 360 = 0.7) then divide by 2 radians
    rotate_sprite(dest, this->frame, (int)this->x, (int)this->y, 
        itofix((int)(angle / 0.7f / 2.0f)));

}

void sprite::move()
{
    //update x position
    if (++xcount > xdelay)  {
        xcount = 0;
        x += velx;
    }

    //update y position
    if (++ycount > ydelay)  {
        ycount = 0;
        y += vely;
    }
}

void sprite::animate() 
{
    //update frame based on animdir
    if (++framecount > framedelay)  {
        framecount = 0;
		curframe += animdir;

		if (curframe < 0) {
            curframe = totalframes-1;
		}
		if (curframe > totalframes-1) {
            curframe = 0;
        }
    }
}

int sprite::inside(int x,int y,int left,int top,int right,int bottom)
{
    if (x > left && x < right && y > top && y < bottom)
        return 1;
    else
        return 0;
}

int sprite::pointInside(int px,int py)
{
	return inside(px, py, (int)x, (int)y, (int)x+width, (int)y+height);
}

int sprite::collided(sprite *other, int shrink)
{
    int wa = (int)x + width;
    int ha = (int)y + height;
    int wb = (int)other->x + other->width;
    int hb = (int)other->y + other->height;

    if (inside((int)x, (int)y, (int)other->x+shrink, (int)other->y+shrink, wb-shrink, hb-shrink) ||
        inside((int)x, ha, (int)other->x+shrink, (int)other->y+shrink, wb-shrink, hb-shrink) ||
        inside(wa, (int)y, (int)other->x+shrink, (int)other->y+shrink, wb-shrink, hb-shrink) ||
        inside(wa, ha, (int)other->x+shrink, (int)other->y+shrink, wb-shrink, hb-shrink))
        return 1;
    else
        return 0;
}

double sprite::calcAngleMoveX(int angle) {
   //calculate X movement value based on direction angle
    return (double) cos(angle * PI_div_180);
}

//calculate Y movement value based on direction angle
double sprite::calcAngleMoveY(int angle) {
    return (double) sin(angle * PI_div_180);
}
