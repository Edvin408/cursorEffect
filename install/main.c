#include "~/.config/cusorEffect/config.h"
#define delay (1000000000 / fps)

int width, height;
int mouseX = 0, mouseY = 0, wx, wy, mouseDown = 0;
int lastX = 0, lastY = 0;
bool mouse;


struct llcai{
    struct clickAnimationInfo item;
    struct llcai *next;
};

struct llpai{
    struct particleAnimationInfo item;
    struct llpai *next;
};

void getcursor(Display* display, Window root) {
    lastX = mouseX;
    lastY = mouseY;
    Window root_ret, child_ret;
    unsigned int mask;
    XQueryPointer(display, root, &root_ret, &child_ret, &mouseX, &mouseY, &wx, &wy, &mask);
    mouse = (mask & Button1Mask) != 0;
}

void clearScreen(Display * display, Window window, GC gc) {
    XSetForeground(display, gc, 0x00000000);
    XFillRectangle(display, window, gc, 0, 0, width, height);
}


void sleepTilNextFrame(struct timespec *lastFrameTime) {
    struct timespec currentTime, sleepTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    long elapsedNs = (currentTime.tv_sec - lastFrameTime->tv_sec) * 1e9 +
    (currentTime.tv_nsec - lastFrameTime->tv_nsec);

    if (elapsedNs < delay) {
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = delay - elapsedNs;
        nanosleep(&sleepTime, NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, lastFrameTime);
}

struct mouseInfo{
    int x,y;
    bool down;
};

int main() {
    srand((unsigned int)time(NULL));
    Display *display;
    Window window;
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("Cannot open display\n");
        exit(1);
    }
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    width = DisplayWidth(display, screen);
    height = DisplayHeight(display, screen);
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo)) {
        printf("No ARGB visual found\n");
        return 1;
    }
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.colormap = XCreateColormap(display, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0x00000000;
    attrs.border_pixel = 0;
    window = XCreateWindow(display, root, 0, 0, width, height, 0, vinfo.depth, InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);
    XShapeCombineRectangles(display, window, ShapeInput, 0, 0, NULL, 0, ShapeSet, 0);
    Atom above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    Atom state = XInternAtom(display, "_NET_WM_STATE", False);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&above, 1);
    XStoreName(display, window, "vedal is the greatest");
    XMapWindow(display, window);
    GC gc = XCreateGC(display, window, 0, NULL);
    XdbeBackBuffer buffer = XdbeAllocateBackBufferName(display, window, XdbeBackground);
    XdbeSwapInfo swapinfo;
    swapinfo.swap_window = window;
    swapinfo.swap_action = XdbeBackground;
    
    struct llcai *firstClick = 0, *lastClick = 0;
#   ifdef particlesExists
    struct llpai *firstParticle = 0, *lastParticle = 0;
#   endif

#   ifdef trailExist
    int on = 0;
    struct mouseInfo* trail = calloc(trailLen, sizeof(struct mouseInfo));
#   endif

    struct timespec lastFrameTime;
    clock_gettime(CLOCK_MONOTONIC, &lastFrameTime);
    while(1) {
        clearScreen(display, buffer, gc);
        getcursor(display, root);

        XSetForeground(display, gc, 0x01ff0000); //ARGB
        XFillRectangle(display, buffer, gc, 100, 100, 250, 30);
        XSetForeground(display, gc, 0x01ffffff);
        XDrawString(display, buffer, gc, 110, 120, "this is a water mark And Vedal is cool", 38);

        if (mouse) {
            holdAnimation(display, gc, buffer, mouseX, mouseY);
            if (mouseDown == 0) {
                struct llcai *temp = calloc(1, sizeof(struct llcai));
                temp->item.x = mouseX;
                temp->item.y = mouseY;
                if (firstClick == 0) {
                    firstClick = temp;
                    lastClick = temp;
                } else {
                    lastClick->next = temp;
                    lastClick = temp;
                }
#               ifdef particlesExists
                for (int i = 0; i < particlesOnClick; i++) {
                    struct llpai *temp = calloc(1, sizeof(struct llpai));
                    temp->item.mouseX = mouseX;
                    temp->item.mouseY = mouseY;
                    int degrees = rand() % 360;
                    temp->item.x = mouseX + (int)(sin(degrees) * particleOffset);
                    temp->item.y = mouseY + (int)(cos(degrees) * particleOffset);
                    if (firstParticle == 0) {
                        firstParticle = temp;
                        lastParticle = temp;
                    } else {
                        lastParticle->next = temp;
                        lastParticle = temp;
                    }
                }
#               endif
            }
#           ifdef particlesExists
#           define particlesPerFrame (particlesDistrubution / fps)
            for (int i = 0; i < particlesPerFrame; i++) {
                if (rand() % ((particlesPerFrame) * 120) < sqrt((mouseX - lastX) * (mouseX - lastX) + (mouseY - lastY) * (mouseY - lastY))) {
                    struct llpai *temp = calloc(1, sizeof(struct llpai));
                    temp->item.mouseX = mouseX;
                    temp->item.mouseY = mouseY;
                    int degrees = rand() % 360;
                    temp->item.x = mouseX + (int)(sin(degrees) / 5 * particleOffset);
                    temp->item.y = mouseY + (int)(cos(degrees) / 5 * particleOffset);
                    if (firstParticle == 0) {
                        firstParticle = temp;
                        lastParticle = temp;
                    } else {
                        lastParticle->next = temp;
                        lastParticle = temp;
                    }
                }
            }
#           endif
            mouseDown = 1;
        } else {
            mouseDown = 0;
        }

#       ifdef trailExist
        trail[on].down = clickOnlyTrail ? mouse:1;
        trail[on].x = mouseX;
        trail[on].y = mouseY;
        on += 1;
        on %= trailLen;
        for (int i = 0; i < trailLen - 1; i++) {
            int temp = (on + i) % trailLen;
            if (trail[temp].down && trail[(temp + 1) % trailLen].down) {
                XSetLineAttributes(display, gc, trailWidthCalc(i), LineSolid, CapRound, JoinRound);
                XSetForeground(display, gc, trailCol);
                XDrawLine(display, buffer, gc, trail[temp].x, trail[temp].y, trail[(temp + 1) % trailLen].x, trail[(temp + 1) % trailLen].y);
            }
        }
#       endif

#       ifdef particlesExists
        if (firstParticle != nullptr) {
            for (struct llpai* i = firstParticle; i != nullptr;) {
                if (i->item.frameNr < fps) {
                    particleAnimation(display, gc, buffer, &(i->item));
                    i->item.frameNr++;
                    i = i->next;
                } else if (i == firstParticle) {
                    firstParticle = firstParticle->next;
                    free(i);
                    i = firstParticle;
                }
            }
        }
#       endif
        if (firstClick != nullptr) {
            for (struct llcai* i = firstClick; i != nullptr;) {
                if (i->item.frameNr < fps) {
                    clickAnimation(display, gc, buffer, &(i->item));
                    i->item.frameNr++;
                    i = i->next;
                } else if (i == firstClick) {
                    firstClick = firstClick->next;
                    free(i);
                    i = firstClick;
                }
            }
        }
        XdbeSwapBuffers(display, &swapinfo, 1);
        XFlush(display);
        sleepTilNextFrame(&lastFrameTime);
    }
#   ifdef trailExist
    free(trail);
#   endif
    XFreePixmap(display, buffer);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
