/* Student: Edward Kyles
 * xproj3.c
 * written during class Nov 14, 2022
 * OG: gordon griesel
 *
 * you may start with this program for your project phase-3.
 *
*/
/*
  general order of ball activity
  1. starts at top of window
  2. drops to the catching hand
  3. moves to the right, to throwing hand
  4. shoots upward on trajectory that will put it back in catching hand
  5. go to step 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define rnd() (double)rand() / (double)RAND_MAX

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	int xres, yres;
	int animation;
} g;

void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void *movement(void *arg);
void render(void);

// ball states
// 0 - no state yet - ball is entering the scene
// 1 - in catching hand
// 2 - moving from catching hand to throwing hand
// 3 - in throwing hand
// 4 - in the air

struct ball_t {
	double pos[2];
	double vel[2];
	int w, h;
	unsigned int color;
	int state;
} ball, balls[2];


sem_t start_hand; 
int catching_hand_pos[2] = { 200, 180 };
int throwing_hand_pos[2] = { 310, 180 };


int main(int argc, char *argv[], char *envp[])
{
	XEvent e;
	int done = 0;
	x11_init_xwindows();
	//
	
    sem_init(&start_hand, 0, 1); //semaphore starts here
    pthread_t thread1, thread2; 
    pthread_create(&thread1, NULL, movement, (void *)0);
    pthread_create(&thread2, NULL, movement, (void *)1);

    srand((unsigned)time(NULL));
	g.animation = 0;
	//initialize ball state
	int i;
	balls[0].color = 0x00114499;
	balls[1].color = 0x00ff0000;
	balls[0].w = balls[0].h = 24;
	balls[1].w = balls[1].h = 16;
	balls[0].state = 0;
	balls[1].state = 0;
	printf("%ix%i\n", g.xres, g.yres);
	catching_hand_pos[1] = g.yres - balls[0].h*0.5 - 10;
	throwing_hand_pos[1] = g.yres - balls[0].h*0.5 - 10;
	for (i=0; i<2; i++) {
		balls[i].pos[0] = 0.0;
		balls[i].pos[1] = 0.0;
		balls[i].vel[0] = 0.0;
		balls[i].vel[1] = 0.0;
	}
    balls[0].pos[0] = catching_hand_pos[0];
    balls[0].pos[1] = catching_hand_pos[1];
    printf("%i %f\n", catching_hand_pos[1], balls[0].pos[1]);
    fflush(stdout);
    balls[1].pos[0] = throwing_hand_pos[0];
    balls[1].pos[1] = throwing_hand_pos[1];
    printf("%i %f\n", throwing_hand_pos[1], balls[1].pos[1]);
    fflush(stdout);
    
    while (!done) {
		/* Check the event queue */
		while (XPending(g.dpy)) {
			XNextEvent(g.dpy, &e);
			check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
			//render();
		}
		usleep(4000);
		//movement();
		render();
	}
	x11_cleanup_xwindows();
	return 0;
}

void x11_cleanup_xwindows(void)
{
	XDestroyWindow(g.dpy, g.win);
	XCloseDisplay(g.dpy);
}

void x11_init_xwindows(void)
{
	int scr;

	if (!(g.dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display!\n");
		exit(EXIT_FAILURE);
	}
	scr = DefaultScreen(g.dpy);
	g.xres = 400;
	g.yres = 200;
	g.win = XCreateSimpleWindow(g.dpy, RootWindow(g.dpy, scr), 1, 1,
							g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
	XStoreName(g.dpy, g.win, "cs3600 f22 - sem project phase-3");
	g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
	XMapWindow(g.dpy, g.win);
	XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
								PointerMotionMask | ButtonPressMask |
								ButtonReleaseMask | KeyPressMask);
}

void check_resize(XEvent *e)
{
	//Respond to ConfigureNotify.
	//The type of event sent by the server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//size changed
		g.xres = xce.width;
		g.yres = xce.height;
	}
}

void drawRect(int x, int y, int w, int h)
{
	XFillRectangle(g.dpy, g.win, g.gc, x, y, w, h);	
}

void drawText(int x, int y, const char *str)
{
	XDrawString(g.dpy, g.win, g.gc, x, y, str, strlen(str));	
}

void check_mouse(XEvent *e)
{
	//static int count = 0;
	static int savex = 0;
	static int savey = 0;
	int mx = e->xbutton.x;
	int my = e->xbutton.y;

	if (e->type != ButtonPress
		&& e->type != ButtonRelease
		&& e->type != MotionNotify)
		return;
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) { }
		if (e->xbutton.button==3) { }
	}
	if (e->type == MotionNotify) {
		if (savex != mx || savey != my) {
			/*mouse moved*/
			savex = mx;
			savey = my;
			//pos[0] = mx;
			//pos[1] = my;
		}
	}
}

int check_keys(XEvent *e)
{
	int key;
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				break;
			case XK_s:
				g.animation = 1;
				break;
			case XK_Escape:
				return 1;
		}
	}
	return 0;
}

void *movement(void* arg)
{
    int i = (long)(arg);
    int second_ball = !i;

    while(1) {
        usleep(4000);
	    if (!g.animation) {
            balls[i].pos[0] += balls[i].vel[0];
            balls[i].pos[1] += balls[i].vel[1];
            balls[i].vel[1] += 0.01;
        
        
            // ball states
            // 0 - no state yet - ball is entering the scene
            // 1 - in catching hand
            // 2 - moving from catching hand to throwing hand
            // 3 - in throwing hand
            // 4 - in the air

            if (balls[i].state == 0) {
                if (balls[i].pos[0] == catching_hand_pos[0] && balls[i].pos[1] == catching_hand_pos[1]) 
                    balls[i].state = 1;
                if (balls[i].pos[0] == throwing_hand_pos[0] && balls[i].pos[1] == throwing_hand_pos[1]) 
                    balls[i].state = 4;
            }

			if (balls[i].state == 1) {
					sem_wait(&start_hand);
                    printf("thread %i grabbed sem\n", i); fflush(stdout);
                    balls[i].state = 2;
			}

			if (balls[i].state == 2) {
				if (balls[second_ball].vel[1] >= 0.2) {
					sem_post(&start_hand);
					printf("thread %i released sem\n", i); fflush(stdout);
                    balls[i].vel[0] = 0.7;
					balls[i].state = 3;
				}
			}
		
			if (balls[i].state == 3) {
				if (balls[i].pos[1] > throwing_hand_pos[1])
					balls[i].pos[1] = throwing_hand_pos[1];
				if (balls[i].pos[0] >= throwing_hand_pos[0]) {
					balls[i].pos[0] = throwing_hand_pos[0];
					balls[i].state = 4;
				}
			}

			if (balls[i].state == 4) {
				balls[i].vel[0] = -0.4;
				balls[i].vel[1] = -1.5;
				balls[i].state = 5;
			}
		
			if (balls[i].state == 5) {
				if (balls[i].pos[1] > catching_hand_pos[1]) {
					balls[i].pos[1] = catching_hand_pos[1];
					balls[i].state = 1;
                }
            }
        }
    }
}

void render(void)
{
	XSetForeground(g.dpy, g.gc, 0x00ff9900); 
	drawRect(0, 0, g.xres, g.yres); 	   
	//
	XSetForeground(g.dpy, g.gc, 0x00553300); 
	int y = 16;
	drawText(12, y, "Gordon Griesel");
	y += 12;
	drawText(12, y, "Parent window");
	y += 12;
	char str[64];
	sprintf(str, "ball[0]_state: %i", balls[0].state);
	drawText(12, y, str);
    y += 12;
    sprintf(str, "ball[1]_state: %i", balls[1].state);
    drawText(12, y, str);

	//
	//draw the hands
	int xp, yp;
	//
	//catching hand...
	xp = catching_hand_pos[0];
	yp = catching_hand_pos[1];
	XSetForeground(g.dpy, g.gc, 0x00ff0000); 
	drawRect(xp-20, yp+8, 40, 8);
	//
	//throwing hand...
	xp = throwing_hand_pos[0];
	yp = throwing_hand_pos[1];
	XSetForeground(g.dpy, g.gc, 0x0022dd22); 
	drawRect(xp-20, yp+8, 40, 8);
	//
	//draw the ball
	int i;
	for (i=0; i<2; i++) {
		XSetForeground(g.dpy, g.gc, balls[i].color); 
		xp = balls[i].pos[0] - balls[i].w/2.0;
		yp = balls[i].pos[1] - balls[i].h/2.0;
		drawRect(xp, yp, balls[i].w, balls[i].h);
	}
}

