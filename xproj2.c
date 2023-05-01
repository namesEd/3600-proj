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

#define NUM_THRDS 1
pthread_t callThd[NUM_THRDS];

struct Global {
	Display *dpy;
	Window win;
	GC gc;
	int xres, yres;
    int pos[2]; 
    int mbox; 
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

int pipefd[2]; 
int retval; 


void *thread_func(void *arg)
{   
    struct Global newStrct;
    while(read(pipefd[0], &newStrct, sizeof(newStrct))> 1) {     
        g.pos[0] = newStrct.pos[0];
        g.pos[1] = newStrct.pos[1];
        g.mbox = newStrct.mbox; 
    }
    pthread_exit((void *)0);
}



int main(int argc, char *argv[], char *envp[])
{
    
    //pthread_t thread1; 

    XEvent e;
    int done = 0;
    x11_init_xwindows();

    my_argv = argv;
    my_envp = envp;
    printf("argv =  %s", argv[0]);
    
    g.mbox = 0; 
    if (argc > 1) { 
        printf("I am the Child!\n");
        child = 1; 
        // set pipefd[0] = argv[2] which hold pipefd1 
        // set pipefd[0] = argv[3] which hold pipefd2
        // these are being passed into the child from the parent 
        // so
        // good luck
        pipefd[0] = atoi(argv[2]);
        pipefd[1] = atoi(argv[3]);

    } else { 
        //creating pipe before child
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    
    while (!done) {
		/* Check the event queue */
		while (XPending(g.dpy)) {
            XNextEvent(g.dpy, &e);
			check_mouse(&e);
			done = check_keys(&e);
            render();
		}
        render();
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
		if (e->xbutton.button==1) {
            if (g.mbox == 1) {
                g.mbox = 0;     
            } else {
                g.mbox = 1; 
            } 
                

        }
		if (e->xbutton.button==3) { }
	}
	if (e->type == MotionNotify) {
		if (savex != mx || savey != my) {
			/*mouse moved*/
			savex = mx;
			savey = my;
            if (child == 1) { 
                g.pos[0] = mx;
                g.pos[1] = my; 
                write(pipefd[1], &g, sizeof(g)); 
            }
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
            printf("%i %i", pipefd[0], pipefd[1]); 
            fflush(stdout); 
            
            char fd1[16];
            char fd2[16];
            sprintf(fd1, "%i", pipefd[0]);
            sprintf(fd2, "%i", pipefd[1]);
            char *child_argv[5] = {my_argv[0],"C", fd1, fd2, NULL};
            //read(pipefd[0], );
            //send command line arguments thru execve
            
            execve(my_argv[0], child_argv, my_envp);
            
            //main();
            //printf("Child ended\n"); fflush(stdout);
            //exit(0);
        } else {
            //parent
            for (long i = 0; i < NUM_THRDS; i ++) {
                pthread_create(&callThd[i], NULL, thread_func, (void *)i);
            }
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

void makeText(int x, int y, const char *str) 
{ 
    XDrawString(g.dpy, g.win, g.gc, x, y, str, strlen(str)); 
}

void render(void)
{   
    int y = 20; 
    if (child) {  
        XSetForeground(g.dpy, g.gc, 0xFF5733);     
        drawRect( 0, 0, g.xres, g.yres);
        XSetForeground(g.dpy, g.gc, 0xE4CAC3);
        makeText(12, y, "Edward Kyles");
        y += 10; 
        makeText(12, y, "This is the child");
        y += 10; 
        makeText(12, y, "Press C to make a new window");
    
   
    }
    else { 
        //XFillRectangle(g.dpy, g.win, g.gc, 0, 0, g.xres, g.yres);
        XSetForeground(g.dpy, g.gc, 0x98F576);     
        drawRect( 0, 0, g.xres, g.yres);
        XSetForeground(g.dpy, g.gc, 0x232522);
        makeText(12, y, "Edward Kyles");
        y += 10; 
        makeText(12, y, "This is the parent");
        y += 10; 
        makeText(12, y, "Press C to make a new window");
    
    
    }
        if (g.mbox ==1 && !child) {
            drawRect(g.pos[0] - 15, g.pos[1] - 15, 20, 20);
        }
}


