#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;
map<string, int*> mapset;
ofstream logfile;

int append(
		  int from_x
		, int from_y
		, int from_width
		, int from_height
		, string direction
		, int to_x
		, int to_y
		, int to_width
		, int to_height
	) {
	ostringstream ostring;
	ostring << from_x << "." << from_y << "." << from_width << "." << from_height << "." << direction;
	string key = ostring.str();
	int *value = mapset[key];
	if (value) {
		logfile
			<< "already exist " << key
			<< " = " << to_x
			<< "." << to_y
			<< "." << to_width
			<< "." << to_height
	    	<< ", from(" << from_x << ", " << from_y << ", " << from_width << ", " << from_height << ")"
	    	<< ", previous to(" << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << ")"
	    	<< ", replaced new to(" << to_x << ", " << to_y << ", " << to_width << ", " << to_height << ")"
			<< endl;
	}
	int *to = new int[4]{to_x, to_y, to_width, to_height};
	mapset[key] = to;

	return 0;
}

int initiaize() {
	append(0, 0, 1, 1, "right",	1, 0, 1, 1);
	append(0, 0, 1, 1, "down",	0, 1, 1, 1);

	append(0, 0, 1, 4, "left",	0, 0, 4, 4);
	append(0, 0, 1, 4, "right",	1, 0, 1, 4);

	append(0, 0, 2, 1, "left",	0, 0, 1, 1);

	append(0, 0, 2, 2, "left",	0, 0, 1, 2);
	append(0, 0, 2, 2, "right",	2, 0, 2, 2);
	append(0, 0, 2, 2, "up",	0, 0, 2, 1);
	append(0, 0, 2, 2, "down",	0, 2, 2, 2);

	append(0, 0, 2, 4, "left",	0, 0, 1, 4);
	append(0, 0, 2, 4, "right",	0, 0, 3, 4);
	append(0, 0, 2, 4, "up",	0, 0, 2, 2);
	append(0, 0, 2, 4, "down",	0, 2, 2, 2);

	append(0, 0, 4, 2, "left",	0, 0, 2, 2);
	append(0, 0, 4, 2, "right",	2, 0, 2, 2);
	append(0, 0, 4, 2, "up",	0, 0, 4, 1);
	append(0, 0, 4, 2, "down",	0, 0, 4, 3);

	append(1, 0, 1, 4, "right",	2, 0, 1, 4);

	append(2, 0, 1, 4, "right",	3, 0, 1, 4);

	return 0;
}

int get_width(Display *x_open_display, int x_default_screen) {
	if(!x_open_display) {
		errx(1, "cannot open display '%s'", XDisplayName(0));
		return 1024;
	}

	int width = DisplayWidth(x_open_display, x_default_screen);

	return width;
}
int get_height(Display *x_open_display, int x_default_screen) {
	if(!x_open_display) {
		errx(1, "cannot open display '%s'", XDisplayName(0));
		return 768;
	}

	int height = DisplayHeight(x_open_display, x_default_screen);

	return height;
}

Window get_parent_window(Display* display, Window window) {
     Window parent;
     Window root;
     Window * children;
     unsigned int num_children;

     while (1) {
         if (0 == XQueryTree(display, window, &root,
                   &parent, &children, &num_children)) {
             fprintf(stderr, "XQueryTree error\n");
             abort(); //change to whatever error handling you prefer
         }
         if (children) { //must test for null
             XFree(children);
         }
         if (window == root || parent == root) {
             return window;
         }
         else {
             window = parent;
         }
     }
}
Window get_focus_window(Display* display){
	Window w;
	int revert_to;

	XGetInputFocus(display, &w, &revert_to); // see man

	return get_parent_window(display, w);
}
int get_rect_window(Display* display, Window activeWindow, int *x, int *y, unsigned int *width, unsigned int *height) {
	Window root_return;
	unsigned int border_width_return;
	unsigned int depth_return;

	Status status = XGetGeometry(display, (Drawable)activeWindow, &root_return
		, x, y
		, width, height
		, &border_width_return, &depth_return);

	return 0;
}

Display *display;
Window rootWindow;
int screen;
Window focusedWindow;
int screen_width;
int screen_height;
int client_x, client_y;
unsigned int client_width, client_height;
int index_x = -1;
int index_y = -1;
int index_width = -1;
int index_height = -1;

int quarter_width = 128;
int quarter_height = 0;

Window deskbarWindow;
int deskbar_x, deskbar_y;
unsigned int deskbar_width, deskbar_height;
int mleft = 0, mright = 0, mtop = 0, mbottom = 0;	//	margin

int find_deskbar_window(Display *display, Window window, int depth, Window *deskbar) {
	string target ("xfce4-panel");
	Window parent;
	Window root;
	Window *children;
	unsigned int num_children;

	char *windowName;
	Status status = XFetchName(display, window, &windowName);
	int window_x, window_y;
	unsigned int window_width, window_height;
	get_rect_window(display, window, &window_x, &window_y, &window_width, &window_height);
	if (window_width <= 1 || window_height <= 1) {
		return 0;
	}

	if (window_width <= screen_width / 2 && window_height <= screen_height / 2) {
		return 0;
	}

	if (windowName) {
		if (target.compare(windowName) == 0) {
			logfile
				<< "depth: " << depth
				<< ", windowName: " << windowName
				<< " window(" << window_x << ", " << window_y << ", " << window_width << ", " << window_height << ")"
				<< endl;
			*deskbar = window;
			return 1;
		} else {
//			logfile
//				<< "depth: " << depth
//				<< ", windowName: " << windowName
//				<< " window(" << window_x << ", " << window_y << ", " << window_width << ", " << window_height << ")"
//				<< endl;
		}
	} else {
//		logfile
//			<< "depth: " << depth
//			<< ", windowName: " << "no name"
//			<< " window(" << window_x << ", " << window_y << ", " << window_width << ", " << window_height << ")"
//			<< endl;
	}
	XFree(windowName);

	if (0 == XQueryTree(display, window, &root, &parent, &children, &num_children)) {
		logfile << "-----------------------return 0" << endl;
		return 0;
	}

	for (int cx = 0; cx < num_children; cx++) {
		if (find_deskbar_window(display, children[cx], depth + 1, deskbar) > 0) {
			return 1;
		}
	}
	return 0;
}
int get_index(int unit, int x) {
	int margin = 10;
    for (int cx = 0; cx < 5; cx++) {
		if (unit * cx <= (x + margin) && (x - margin) <= unit * cx) {
			return cx;
		}
	}
	
	return -1;
}
int open() {
	display = XOpenDisplay(NULL);
	screen = XDefaultScreen(display);
	focusedWindow = get_focus_window(display);
	screen_width = get_width(display, screen);
	screen_height = get_height(display, screen);
	get_rect_window(display, focusedWindow, &client_x, &client_y, &client_width, &client_height);
	rootWindow = XDefaultRootWindow(display);
	int root_x, root_y;
	unsigned int root_width, root_height;
	get_rect_window(display, rootWindow, &root_x, &root_y, &root_width, &root_height);
    
	find_deskbar_window(display, rootWindow, 0, &deskbarWindow);
	get_rect_window(display, deskbarWindow, &deskbar_x, &deskbar_y, &deskbar_width, &deskbar_height);
	if (deskbar_width > screen_width / 2) {
		if (deskbar_x > 0) {
			mbottom = deskbar_x;
		} else {
			mtop = deskbar_height;
		}
	} else if (deskbar_height > screen_height / 2) {
		if (deskbar_y > 0) {
			mright = deskbar_y;
		} else {
			mleft = deskbar_width;
		}
	} else {
		//	deskbar hidden
	}

	screen_width -= (mleft + mright);
	screen_height -= (mtop + mbottom);
	quarter_width = screen_width / 4;
	quarter_height = screen_height / 4;
	client_x -= mleft;
	client_x--;
	client_y -= 0;
    //client_y--;
    
	index_x = get_index(quarter_width, client_x);
	index_y = get_index(quarter_height, client_y);
	index_width = get_index(quarter_width, client_width);
	index_height = get_index(quarter_height, client_height);

	return 0;
}
int close() {
    XCloseDisplay(display);

	return 0;
}
int set_window(int ix, int iy, int iw, int ih) {
	if (iw == 4 && ih == 4 && false) {
		//	maximize
		XEvent x_event;
		Atom wm_fullscreen;
		
		x_event.type = ClientMessage;
		x_event.xclient.window = focusedWindow;
		x_event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
		x_event.xclient.format = 32;
		x_event.xclient.data.l[0] = 1;  /* 0 = Windowed, 1 = Fullscreen */
		wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
		x_event.xclient.data.l[1] = wm_fullscreen;
		x_event.xclient.data.l[2] = wm_fullscreen;
		XSendEvent(display, XRootWindow(display, XDefaultScreen(display)), False, ClientMessage, &x_event);
		return 0;
	}

	int x = quarter_width * ix;
	int y = quarter_height * iy;
	int w = quarter_width * iw;
	int h = quarter_height * ih;

    XMoveResizeWindow(display, focusedWindow, x + mleft, y + mtop, w, h);

   	logfile
    	<< "screen(" << screen_width << ", " << screen_height << ")"
    	<< ", client(" << client_x << ", " << client_y << ", " << client_width << ", " << client_height << ")"
    	<< ", index(" << index_x << ", " << index_y << ", " << index_width << ", " << index_height << ")"
    	<< ", quarter(" << quarter_width << ", " << quarter_height << ")"
    	<< ", next index(" << ix << ", " << iy << ", " << iw << ", " << ih << ")"
    	<< ", next coord(" << x + mleft << ", " << y + mtop << ", " << w << ", " << h << ")"
		<< ", deskbar(" << deskbar_x << ", " << deskbar_y << ", " << deskbar_width << ", " << deskbar_height << ")"
		<< ", margin(" << mleft << ", " << mtop << ", " << mright << ", " << mbottom << ")"
   		<< endl;

	return 0;
}
int is_index(int ix, int iy, int iw, int ih) {
	return index_x == ix && index_y == iy && index_width == iw && index_height == ih;
}
int *get_map_way(int *result, int ix, int iy, int iw, int ih, string direction) {
	int max_ix = 4;
	int max_iy = 4;

	ostringstream ostring;
	string arrow = direction;
	string key;
	int *value;

	ostring.str("");
	ostring << index_x << "." << index_y << "." << index_width << "." << index_height << "." << arrow;
	key = ostring.str();
	value = mapset[key];
	if (value) {
		result[0] = value[0];
		result[1] = value[1];
		result[2] = value[2];
		result[3] = value[3];

	   	logfile
	    	<< "대칭 없음("  << ix << ", " << iy << ", " << iw << ", " << ih << ")"
	    	<< ", " << ostring.str()
	    	<< ", return index(" << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << ")"
	    	<< ", next index(" << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3] << ")"
	   		<< endl;

		return result;
	} else {
	   	logfile << "NONE 대칭 없음("  << ostring.str() << ")" << endl;
	}
	
	//	y축 대칭
	if (direction.compare("left") == 0) {
		arrow = "right";
	} else if (direction.compare("right") == 0) {
		arrow = "left";
	} else {
		arrow = direction;
	}
	ostring.str("");
	ostring
		<< max_ix - index_x - iw
		<< "." << index_y
		<< "." << index_width
		<< "." << index_height
		<< "." << arrow;
	key = ostring.str();
	value = mapset[key];
	if (value) {
		result[0] = max_ix - value[0] - value[2];
		result[1] = value[1];
		result[2] = value[2];
		result[3] = value[3];

	   	logfile
	    	<< "y축 대칭("  << ix << ", " << iy << ", " << iw << ", " << ih << ")"
	    	<< ", " << ostring.str()
	    	<< ", return index(" << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << ")"
	    	<< ", next index(" << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3] << ")"
	   		<< endl;
	
		return result;
	} else {
	   	logfile << "y축 대칭 없음("  << ostring.str() << ")" << endl;
	}

	//	x축 대칭(y = 2)
	if (direction.compare("up") == 0) {
		arrow = "down";
	} else if (direction.compare("down") == 0) {
		arrow = "up";
	} else {
		arrow = direction;
	}
	ostring.str("");
	ostring
		<< index_x
		<< "." << max_iy - index_y - ih
		<< "." << index_width
		<< "." << index_height
		<< "." << arrow;
	key = ostring.str();
	value = mapset[key];
	if (value) {
		result[0] = value[0];
		result[1] = max_iy - value[1] - value[3];
		result[2] = value[2];
		result[3] = value[3];

	   	logfile
	    	<< "x축 대칭("  << ix << ", " << iy << ", " << iw << ", " << ih << ")"
	    	<< ", " << ostring.str()
	    	<< ", return index(" << value[0] << ", " << value[1] << ", " << value[2] << ", " << value[3] << ")"
	    	<< ", next index(" << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3] << ")"
	   		<< endl;

		return result;
	} else {
	   	logfile << "x축 대칭 없음("  << ostring.str() << ")" << endl;
	}

	//	xy축 대칭
	if (direction.compare("left") == 0) {
		arrow = "right";
	} else if (direction.compare("right") == 0) {
		arrow = "left";
	} else if (direction.compare("up") == 0) {
		arrow = "down";
	} else if (direction.compare("down") == 0) {
		arrow = "up";
	} else {
		arrow = direction;
	}
	ostring.str("");
	ostring
		<< max_ix - index_x - iw
		<< "." << max_iy - index_y - ih
		<< "." << index_width
		<< "." << index_height
		<< "." << arrow;
	key = ostring.str();
	value = mapset[key];
	if (value) {
		result[0] = max_ix - value[0] - value[2];
		result[1] = max_iy - value[1] - value[3];
		result[2] = value[2];
		result[3] = value[3];

	   	logfile
	    	<< "xy축 대칭("  << ix << ", " << iy << ", " << iw << ", " << ih << ")"
	    	<< ", " << ostring.str()
	    	<< ", next index(" << result[0] << ", " << result[1] << ", " << result[2] << ", " << result[3] << ")"
	   		<< endl;

		return result;
	} else {
	   	logfile << "xy축 대칭 없음("  << ostring.str() << ")" << endl;
	}

	if (direction.compare("left") == 0) {
		result[0] = 0;
		result[1] = 0;
		result[2] = 2;
		result[3] = 4;
	} else if (direction.compare("right") == 0) {
		result[0] = 2;
		result[1] = 0;
		result[2] = 2;
		result[3] = 4;
	} else if (direction.compare("up") == 0) {
		result[0] = 0;
		result[1] = 0;
		result[2] = 4;
		result[3] = 2;
	} else if (direction.compare("down") == 0) {
		result[0] = 0;
		result[1] = 2;
		result[2] = 4;
		result[3] = 2;
	} else {
		result[0] = 0;
		result[1] = 0;
		result[2] = 4;
		result[3] = 4;
	}

	return result;
}

int map_way(string direction) {
	int result[4];
	get_map_way(result, index_x, index_y, index_width, index_height, direction);
	set_window(result[0], result[1], result[2], result[3]);
	return 1;
}

int left() {
	if (index_x < 0 || index_y < 0 || index_width < 0 || index_height < 0) {
		set_window(0, 0, 2, 4);
		return 0;
	}
	
	if (map_way("left")) {
		return 0;
	}

	if (is_index(0, 0, 2, 4)) {
		set_window(0, 0, 1, 4);
		return 0;
	}

	set_window(0, 0, 2, 4);
	return 0;
}

int right() {
	if (index_x < 0 || index_y < 0 || index_width < 0 || index_height < 0) {
		set_window(2, 0, 2, 4);
		return 0;
	}
	
	if (map_way("right")) {
		return 0;
	}

	if (is_index(2, 0, 2, 4)) {
		set_window(3, 0, 1, 4);
		return 0;
	}

	set_window(2, 0, 2, 4);
	return 0;
}

int up() {
	if (index_x < 0 || index_y < 0 || index_width < 0 || index_height < 0) {
		set_window(0, 0, 4, 2);
		return 0;
	}
	
	if (map_way("up")) {
		return 0;
	}

	if (is_index(0, 0, 4, 2)) {
		set_window(0, 0, 4, 1);
		return 0;
	}

	if (is_index(2, 0, 2, 4)) {
		set_window(2, 0, 2, 2);
		return 0;
	}

	if (is_index(2, 0, 2, 2)) {
		set_window(2, 0, 2, 1);
		return 0;
	}

	set_window(0, 0, 4, 2);
	return 0;
}

int down() {
	if (index_x < 0 || index_y < 0 || index_width < 0 || index_height < 0) {
		set_window(0, 2, 4, 2);
		return 0;
	}
	
	if (map_way("down")) {
		return 0;
	}

	if (is_index(0, 2, 4, 2)) {
		set_window(0, 3, 4, 1);
		return 0;
	}

	if (is_index(2, 0, 2, 4)) {
		set_window(2, 2, 2, 2);
		return 0;
	}

	if (is_index(2, 2, 2, 2)) {
		set_window(2, 3, 2, 1);
		return 0;
	}

	set_window(0, 2, 4, 2);
	return 0;
}

int get_direction(int argc, char** argv) {
	if (argc < 2 || argv == NULL) {
		return 0;
	};
	
	char *first = argv[1];
	if (!first) {
		return 0;
	}
	
	if (strcmp("left", first) == 0) {
		return 1;
	}

	if (strcmp("right", first) == 0) {
		return 2;
	}

	if (strcmp("up", first) == 0) {
		return 3;
	}

	if (strcmp("down", first) == 0) {
		return 4;
	}

	return 0;
}

int main(int argc, char** argv) {
	logfile.open("/home/andold/logs/test-x11/hotkey.log", ios_base::app);

	initiaize();
    open();

	switch (get_direction(argc, argv)) {
		case 0:
			break;
		case 1:
			left();
			break;
		case 2:
			right();
			break;
		case 3:
			up();
			break;
		case 4:
			down();
			break;
		default:
			break;
	}
	
	close();
    return 0;
}

