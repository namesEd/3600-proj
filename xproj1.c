/*
 cs3600 Fall 2022
 original author:  Gordon Griesel
 Student Editor:   Edward Kyles
 File: xproj.c 

      Instructions:

      1. If you make changes to this file, put your name at the top of
	     the file. Use one C style multi-line comment to hold your full
		 name. Do not remove the original author's name from this or 
		 other source files please.

      2. Build and run this program by using the provided Makefile.

	     At the command-line enter make.
		 Run the program by entering ./a.out
		 Quit the program by pressing Esc.

         The compile line will look like this:
            gcc xwin89.c -Wall -Wextra -Werror -pedantic -ansi -lX11

		 To run this program on the Odin server, you will have to log in
		 using the -YC option. Example: ssh myname@odin.cs.csub.edu -YC

      3. See the assignment page associated with this program for more
	     instructions.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	int xres, yres;
} g;


void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void x11_clear_window(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void render(void);

char **my_argv;
char **my_envp;
static int child = 0;

int pos[2];

int main(int argc, char *argv[], char *envp[])
{
    
    //int pipe; 

    my_argv = argv;
    my_envp = envp;
    if (argc > 1) { 
        printf("I am the Child!\n");
        child = 1; 
    }
    XEvent e;
	int done = 0;
	x11_init_xwindows();
	while (!done) {
		/* Check the event queue */
		while (XPending(g.dpy)) {
			XNextEvent(g.dpy, &e);
			check_mouse(&e);
			done = check_keys(&e);
            render();
		}
		usleep(4000);
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
	XStoreName(g.dpy, g.win, "cs3600 xwin sample");
	g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
	XMapWindow(g.dpy, g.win);
	XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
								PointerMotionMask | ButtonPressMask |
								ButtonReleaseMask | KeyPressMask);
}


void drawRect(int x, int y, int w, int h)
{
    XFillRectangle(g.dpy, g.win, g.gc, x, y, w, h);
}


void check_mouse(XEvent *e)
{
    static int count = 0;
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
            pos[0] = mx;
            pos[1] = my; 
            if (++ count >20) {
                (child) ? printf("c") : printf("m");
                fflush(stdout);
                count = 0;
            }
		}
	}
}


void make_child_process() 
{
    static int nchildren = 0; 
    if (nchildren == 0) {
        printf("fork\n"); 
        fflush(stdout);
        ++nchildren;
        int pid = fork();
        if (pid ==0) {
            //child
            //strcpy(my_argv[1], "C"); 
            char *argv[3] = {my_argv[0],"C", NULL};
            execve(my_argv[0], argv, my_envp);
            //main();
            //printf("Child ended\n"); fflush(stdout);
            //exit(0);
        } else {
            //parent
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
			case XK_c:
                make_child_process();
	            break;		
			case XK_Escape:
				return 1;
		}
	}
	return 0;
}

void render(void)
{
    XSetForeground(g.dpy, g.gc, 0x003594);
    if (child) 
        XSetForeground(g.dpy, g.gc, 0x00FFC72C);
    //XFillRectangle(g.dpy, g.win, g.gc, 0, 0, g.xres, g.yres);
    drawRect( 0, 0, g.xres, g.yres);
    XSetForeground(g.dpy, g.gc, 0x0000ff);
    drawRect(pos[0] - 15, pos[1] - 15, 20, 20);
}


