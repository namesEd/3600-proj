all: xproj1 xproj2 xproj3

xproj1: xproj1.c
	gcc -Wall xproj1.c -oxproj1 -lX11 -lpthread

xproj2: xproj2.c
	gcc -Wall xproj2.c -oxproj2 -lX11 -lpthread

xproj3: xproj3.c
	gcc -Wall xproj3.c -oxproj3 -lX11 -lpthread

clean:
	rm -f xproj1 xproj2 xproj3

