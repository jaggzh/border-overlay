#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

//using namespace std;
#include <chrono>
#include <thread>
#include <iostream>

#include <getopt.h>
#include <unistd.h>

// sizepos='764x410 -i :0+1052,166'
int px=50, py=150;
int sx=300, sy=200;
int thick=3;

Region CreateRegion(int x, int y, int w, int h);
Region CreateFrameRegion(int bound);

void draw(cairo_t *cr) {
	// ----------
	// |		|
	// ----------
	//cairo_rectangle(cr, 0, 0, sx, sy);
	cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.4);
	cairo_rectangle(cr, 0, 0, thick, sy+thick);				 // left
	cairo_rectangle(cr, 0, 0, sx+thick*2, thick);		 // top
	cairo_rectangle(cr, 0, sy+thick, sx+thick*2, thick);	  // bottom
	cairo_rectangle(cr, sx+thick, 0, thick, sy+thick); // right
	cairo_fill(cr);
}

#if 0 // yuck. we'll just do it by hand (possibly crashing on invalid input)
void opts(int argc, char *argv[]) {
	static struct option long_options[] = {
		  /* These options set a flag. */
		  //{"verbose", no_argument,	   &verbose_flag, 1},
		  //{"brief",   no_argument,	   &verbose_flag, 0},
		  /* These options don’t set a flag.
			 We distinguish them by their indices. */
		  {"x",  required_argument, &px, 'x'},
		  {"y",  required_argument, &py, 'y'},
		  {"width",  required_argument, &sx, 'w'},
		  {"height",  required_argument, &sy, 'h'},
		  {"border",  required_argument, &thick, 'b'},
		  {0, 0, 0, 0}
		};
	getopt_long(argc, argv, "x:y:w:h:b"
		   const struct option *longopts, int *longindex);
}
#endif

int main(int argc, char *argv[]) {
	//opts(argc, argv);
	if (argc<6) {
		printf("Using default geometry\n");
		printf("(Syntax: sx sy px py thickness)\n");
		printf("*Note: If you don't get the arguments right I'll probably break.\n");
		printf("%dx%d+%d+%d Thickness %d\n", sx,sy,px,py, thick);
	} else {
		sx = strtol(argv[1], NULL, 10);
		sy = strtol(argv[2], NULL, 10);
		px = strtol(argv[3], NULL, 10);
		py = strtol(argv[4], NULL, 10);
		thick = strtol(argv[5], NULL, 10);
		printf("%dx%d+%d+%d Thickness %d\n", sx,sy,px,py, thick);
	}

	Display *d = XOpenDisplay(NULL);
	if (!d) {
		fprintf(stderr, "Couldn't XOpenDisplay(); it returned NULL\n");
		exit(1);
	}
	Window root = DefaultRootWindow(d);
	//int default_screen = XDefaultScreen(d);

	// these two lines are really all you need
	XSetWindowAttributes attrs;
	attrs.override_redirect = true;

	XVisualInfo vinfo;
	if (!XMatchVisualInfo(d, DefaultScreen(d), 32, TrueColor, &vinfo)) {
		printf("No visual found supporting 32 bit color, terminating\n");
		exit(EXIT_FAILURE);
	}
	// these next three lines add 32 bit depth, remove if you dont need and change the flags below
	attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
	attrs.background_pixel = 0;
	attrs.border_pixel = 0;

	// Window XCreateWindow(
	//	 Display *display, Window parent,
	//	 int x, int y, unsigned int wid, unsigned int he, unsigned int bord_width,
	//	 int depth, unsigned int class, 
	//	 Visual *visual,
	//	 unsigned long valuemask, XSetWindowAttributes *attributes
	// );
	Window win = XCreateWindow(
		d, root,
		px-thick, py-thick, sx+thick*2, sy+thick*2, 0,
		vinfo.depth, InputOutput, 
		vinfo.visual,
		CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs
	);

	XMapWindow(d, win);

	//long eventMask = NoEventMask;
  	//XSelectInput(d, win, eventMask );

	cairo_surface_t* surf = cairo_xlib_surface_create(d, win,
								  vinfo.visual,
								  sx+thick*2, sy+thick*2);
	cairo_t* cr = cairo_create(surf);

	draw(cr);

	Region region = CreateRegion(0,0,1,1);
	XShapeCombineRegion(d, win, ShapeInput, 0, 0, region, ShapeSet);
	XDestroyRegion(region);
	XFlush(d);

	//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	printf("Hit CTRL-C to kill me\n");
	while (1) {
		sleep(1);
	}
	//getchar();

	cairo_destroy(cr);
	cairo_surface_destroy(surf);

	XUnmapWindow(d, win);
	XCloseDisplay(d);
	return 0;
}

Region CreateRegion(int x, int y, int w, int h) {
	Region region = XCreateRegion();
	XRectangle rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.width = w;
	rectangle.height = h;
	XUnionRectWithRegion(&rectangle, region, region);

	return region;
}


/* Not using this. It was from the example at:
   https://gist.github.com/mertyildiran/b33c6b2df85d26981358aad3854d9d15#file-transregion-c
 */
#if 0
Region CreateFrameRegion(int bound, int XWinSize, int YWinSize) {
	Region region = XCreateRegion();
	XRectangle rectangle;

	/* top */
	rectangle.x = 0;
	rectangle.y = 0;
	rectangle.width = XWinSize;
	rectangle.height = bound;
	XUnionRectWithRegion(&rectangle, region, region);

	/* bottom */
	rectangle.x = 0;
	rectangle.y = YWinSize - bound;
	rectangle.width = XWinSize;
	rectangle.height = bound;
	XUnionRectWithRegion(&rectangle, region, region);

	/* left side */
	rectangle.x = 0;
	rectangle.y = 0;
	rectangle.width = bound;
	rectangle.height = YWinSize;
	XUnionRectWithRegion(&rectangle, region, region);

	/* right side */
	rectangle.x = XWinSize - bound;
	rectangle.y = 0;
	rectangle.width = bound;
	rectangle.height = YWinSize;
	XUnionRectWithRegion(&rectangle, region, region);

	return region;
}
#endif
