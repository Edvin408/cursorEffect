extern int printf(const char*__restrict __format, ...);
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#define min(a, b) (a<b?a:b)
#define max(a, b) (a>b?a:b)
#define sqr(a) (a*a)

//X11 core
#include <X11/Xlib.h>
#include <X11/Xatom.h>

//X11 extensions
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xdbe.h>

void XFillCircle(Display *display, Window window, GC gc, int centerX, int centerY, int radius) {
    int diameter = radius * 2;
    XFillArc(display, window, gc, centerX - radius, centerY - radius, diameter, diameter, 0, 360 * 64);
}

void XFillTriangle(Display *display, Window window, GC gc, int x1, int y1, int x2, int y2, int x3, int y3) {
    XPoint p[3] = {{x1, y1}, {x2, y3}, {x3, y3}};
    XFillPolygon(display, window, gc, p, 3, Convex, CoordModeOrigin);
}
// don't any of the lines that are before this comment

#define fps 240


//click

struct clickAnimationInfo {
    int x, y;
    int frameNr; // max 1 second animation (fps * 1) zero indexed
                 // you can add more info in this struct
    int ld11, ld21, ld12, ld22;
    int ltd1, ltd2;
};
void clickAnimation(Display* display, GC gc, XdbeBackBuffer buffer, struct clickAnimationInfo *info) {
    if (info->frameNr <= fps / 5) {
        XSetForeground(display, gc, (((fps / 5) - info->frameNr) * 5 * 0x3f / fps) * 0x01010100 + (((fps / 5) - info->frameNr) * 5 * 0xff / fps));
        XFillCircle(display, buffer, gc, info->x, info->y, 75);
    }
    if (info->frameNr == 0) {
        info->ltd1 = rand() % (360 * 64);
        info->ltd2 = info->ltd1 + rand() % (360 * 64);
        info->ld21 = 90 * 64 + rand() % (180 * 64);
        info->ld22 = (360 * 64) - info->ld21;
        info->ld11 = (rand() % (360 * 64)) - (180 * 64);
        info->ld12 = info->ltd1 + info->ld11 + info->ld21 - info->ltd2;
    }
    int frameNrDown = fps - 2 * info->frameNr;
    if (frameNrDown > 0) {
        XSetLineAttributes(display, gc, 10, LineSolid, CapButt, JoinMiter);
        XSetForeground(display, gc, 0xffafafff);
        XDrawArc(display, buffer, gc, info->x - 75, info->y - 75, 150, 150, info->ltd1 + info->ld11 * frameNrDown / fps, info->ld21 * frameNrDown / fps);
        XDrawArc(display, buffer, gc, info->x - 80, info->y - 80, 160, 160, info->ltd2 + info->ld12 * frameNrDown / fps, info->ld22 * frameNrDown / fps);
        XSetLineAttributes(display, gc, 4, LineSolid, CapButt, JoinMiter);
        XSetForeground(display, gc, 0xffffffff);
        XDrawArc(display, buffer, gc, info->x - 75, info->y - 75, 150, 150, info->ltd1 + info->ld11 * frameNrDown / fps, info->ld21 * frameNrDown / fps);
        XDrawArc(display, buffer, gc, info->x - 80, info->y - 80, 160, 160, info->ltd2 + info->ld12 * frameNrDown / fps, info->ld22 * frameNrDown / fps);
    }
    return;
}

void holdAnimation(Display* display, GC gc, XdbeBackBuffer buffer, int x, int y) {
    for (int i = 0; i < 49; i++) {
        XSetForeground(display, gc, 0x01000000 + min(i * 0x040000, 0xff0000) + min(i * 0x0800, 0xff00) + min(i * 0x10, 0xff));
        XFillCircle(display, buffer, gc, x, y, 50 - i);
    }
} // currently glow effect

//particles
#define particlesExists
#ifdef particlesExists
#define particlesOnClick 4
#define particlesDistrubution 20000 // the higher the better but laggier
#define particleOffset 50
struct particleAnimationInfo {
    int mouseX, mouseY; // maybe move the particle
    int x, y;
    int frameNr; // max 1 second animation (fps * 1) zero indexed
    // you can add more info in this struct
    bool upSideDown;
    unsigned int col;
    float speedX, speedY;
    float size;
};
void particleAnimation(Display* display, GC gc, XdbeBackBuffer buffer, struct particleAnimationInfo *info) {
    if (info->frameNr == 0) {
        info->upSideDown = rand() & 1;
        info->speedX = info->x - info->mouseX;
        info->speedY = info->y - info->mouseY;
        info->speedX /= 20;
        info->speedY /= 20;
        info->size = 4;
        switch (rand() % 4) {
            case 0:
                info->col = 0x3f3030ff;
                break;
            case 1:
                info->col = 0x3f4060ff;
                break;
            case 2:
                info->col = 0x3f00b0ff;
                break;
            case 3:
                info->col = 0x3f9080ff;
                break;
        }
    }
    XSetForeground(display, gc, (((info->frameNr * 30)/fps) & 1) ? info->col: 0x3fffffff);
    if (info->size > 0) {
        XFillTriangle(display, buffer, gc, info->x, info->y + (int)(info->size * (info->upSideDown ? -5:5)), info->x - (int)(info->size * 6), info->y + (int)(info->size * (info->upSideDown ? 5:-5)), info->x + (int)(info->size * 6), info->y + (int)(info->size * (info->upSideDown ? 5:-5)));
    }
    info->size -= 8.0 / fps;
    info->x += (int)info->speedX;
    info->y += (int)info->speedY;
    info->speedX *= 0.98;
    info->speedY *= 0.98;
    return;
}
#endif

//trail

//comment this if you dont want trail
#define trailExist
#ifdef trailExist
#define trailLen (int)(25 * ((float)fps / 120))
#define trailCol 0x46fcf9
// This variable sets if the trail appears when you dont click. if its 1 no trail when not clicked. if its 0 then theres always a trail
#define clickOnlyTrail 1

// calculates from the end
int trailWidthCalc(int atSegment) {
    return trailLen * 3 / (trailLen - atSegment + 10);
}
#endif
