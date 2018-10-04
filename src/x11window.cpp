
#include "x11window.hpp"
#include "events.hpp"
#include "exceptions.hpp"

#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include <unistd.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>

using namespace std;
using namespace net::lliurex::lgi;

/* I should find a better place for them */
Atom XdndAware;
Atom XdndEnter;
Atom XdndLeave;
Atom XdndDrop;
Atom XdndStatus;
Atom XdndPosition;
Atom XdndActionCopy;
Atom XdndFinished;
Atom XdndSelection;
Atom XdndTypeList;

Atom dropdata;

/*!
 * sort operator 
 */
bool sort_func(const Layer * l1,const Layer * l2)
{
	return (l1->depth<l2->depth);
}


/*!
* stdout messages
*/
void X11Window::Log(string msg)
{
	cout<<"X11Window.cpp: "<<msg<<endl;
}

/*!
 * Destructor
 */ 
X11Window::~X11Window()
{
	/* there is room for improvement here */
	pthread_mutex_destroy(&queue_mutex);
	XCloseDisplay(display);
}

/*!
* Gets current cairo object
* WARNING: cairo object is volatile, as it is destroyed and re-created each time the window gets resized
*/
cairo_t * X11Window::GetCairo()
{
	return cairo;
}

unsigned long X11Window::GetXWindow()
{
	return xwindow;
}

/*!
 * Gets screen dimensions
 */ 
void X11Window::GetScreenSize(int * width,int * height)
{
	*width=WidthOfScreen(ScreenOfDisplay(display,screen));
	*height=HeightOfScreen(ScreenOfDisplay(display,screen));
}

/*!
 * Creates a X11 window with given dimension and flags
 */ 
X11Window::X11Window(int w,int h,int flags)
{
	
	display = XOpenDisplay(nullptr);
	screen = DefaultScreen(display);

	memset(clicks, 0, 6*sizeof(click_t));
	
	dragging = nullptr;
	double_click_ms=200;
	
	gettimeofday(&init_time,nullptr);
	
	cursor_mode=LGI_CURSOR_MODE_SYSTEM;
	cursor_type=LGI_CURSOR_DEFAULT;

	width=w;
	height=h;
	
	focus_layer=nullptr;
	
	int scr_w,scr_h;
	
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	
	
	/* queue mutex */
	pthread_mutex_init(&queue_mutex,nullptr);
	
	//default render mode
	this->render_method=LGI_FLAG_EXT_RENDER_SOFTWARE;
	
	if(flags & LGI_FLAG_EXT_RENDER_OPENGL)
	{
		this->render_method=LGI_FLAG_EXT_RENDER_OPENGL;
		Log("Using OpenGL render method");
	}
		
			
	/* Create window */
	xwindow = XCreateSimpleWindow(display, XDefaultRootWindow(display), 0, 0,width, height, 0, WhitePixel(display, screen), WhitePixel(display, screen));
	
	/* Set window title */
	XStoreName(display,xwindow,"LGI Window");
	
	/*Non-resizable windows*/
	if( !( flags & LGI_FLAG_RESIZABLE))
	{
		
		XSizeHints * hints = XAllocSizeHints();
		
		hints->min_width=width;
		hints->max_width=width;
		hints->min_height=height;
		hints->max_height=height;
		hints->flags=PMinSize | PMaxSize;
				
		XSetWMNormalHints(display, xwindow, hints);
		XFree(hints);
	}
	
	if( flags & LGI_FLAG_NO_BORDER)
	{
		wm_hints wm_hints;
		Atom motif_noborder;
		wm_hints.flags = 2; //set we want to change window decor
		wm_hints.decorations = 0; //we want window decor to turn off
		motif_noborder = XInternAtom(display, "_MOTIF_WM_HINTS", true);
		XChangeProperty(display,xwindow,motif_noborder,motif_noborder,32,PropModeReplace,(unsigned char *)&wm_hints,5);
	}
	
	/* Catch close event */
	wm_protocols = XInternAtom(display, "WM_PROTOCOLS", false);
	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
	
	
	/* Keyboard mapping */
	int min_keycode,max_keycode;
	XDisplayKeycodes(display,&min_keycode,&max_keycode);
	keymap = XGetKeyboardMapping(display,min_keycode,max_keycode-min_keycode,&keysym_per_code);
	
	/* DND */
	XdndAware=XInternAtom(display, "XdndAware", false);
	unsigned char dnd_version=3;
	XChangeProperty(display, xwindow, XdndAware, XA_ATOM,32,PropModeReplace,&dnd_version,1);
	                         
	XdndEnter=XInternAtom(display, "XdndEnter", false);
	XdndDrop=XInternAtom(display, "XdndDrop", false);
	XdndStatus=XInternAtom(display, "XdndStatus", false);
	XdndPosition=XInternAtom(display, "XdndPosition", false);
	XdndActionCopy=XInternAtom(display, "XdndActionCopy", false);
	XdndFinished=XInternAtom(display, "XdndFinished", false);
	XdndSelection=XInternAtom(display, "XdndSelection", false);
	XdndLeave=XInternAtom(display,"XdndLeave",false);
	XdndTypeList=XInternAtom(display,"XdndTypeList",false);
	
	 
	
	/* Select input filter */
	XSelectInput(display,xwindow,StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | SubstructureRedirectMask | ExposureMask);
	
	XMapWindow(display, xwindow);
			
	
	/* ToDo: check cairo stride setup before allocating memory */
	
	framebuffer = new uint8_t[width*height*4];
	cairo_surface = cairo_image_surface_create_for_data(framebuffer,CAIRO_FORMAT_ARGB32,width,height,width*4);
	if(cairo_surface_status(cairo_surface)!=CAIRO_STATUS_SUCCESS)
		throw Exception("Failed to create cairo surface");
	

	if(this->render_method==LGI_FLAG_EXT_RENDER_OPENGL)
	{
		visual=glXChooseVisual(display, 0, att);
		if(visual==NULL)throw Exception("Error with GLX visual");
		
		glc = glXCreateContext(display, visual, nullptr, GL_TRUE);
		glXMakeCurrent(display, xwindow, glc);
		
		
		glDisable(GL_DEPTH_TEST);
		
		
		glClearColor(1.0f,0.0f,0.0f,1.0f);
		glViewport(0,0,width,height);
		glMatrixMode(GL_PROJECTION);
		
		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, 10);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		
		/* texture creation */
					
		glEnable(GL_TEXTURE_2D);
		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		
		glGenTextures(1, &framebuffer_texture);
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);

		
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		
		
	}
	else
	{
		/* Create XImage back_buffer*/
		back_buffer = XCreateImage(display,XDefaultVisual(display,screen),24,ZPixmap,0,(char *)framebuffer,width,height,32,4*width);
		gc = XCreateGC(display, xwindow, 0, nullptr);
	}
	/* Create cairo object
	 * this may be re-created whenever window is resized, as
	 * current xlib cairo backend doesn't support 
	 * target surface resize
	 * */
	cairo=cairo_create(cairo_surface);
	
	XFlush(display);
			
}

/*!
 * When screen is resizable and the event happens, this function
 * allocates another cairo surface and creates a new cairo object
 * in order to fit new dimensions
 */ 
void X11Window::Resize(int w,int h)
{
	width = w;
	height = h;
	
	if(render_method==LGI_FLAG_EXT_RENDER_OPENGL)
	{
		cairo_surface_destroy(cairo_surface);
		delete framebuffer;
		cairo_destroy(cairo);
	}
	else
	{
		cairo_surface_destroy(cairo_surface);
		XDestroyImage(back_buffer);
		/* framebuffer is already freed by XDestroyImage */
		cairo_destroy(cairo);
	}
	
	

	framebuffer = new uint8_t[width*height*4];
	cairo_surface = cairo_image_surface_create_for_data(framebuffer,CAIRO_FORMAT_ARGB32,width,height,width*4);
	if(cairo_surface_status(cairo_surface)!=CAIRO_STATUS_SUCCESS)
		throw Exception("Failed to create cairo surface");
	
	if(render_method==LGI_FLAG_EXT_RENDER_OPENGL)
	{
		glClearColor(1.0f,0.0f,0.0f,1.0f);
		glViewport(0,0,width,height);
		glMatrixMode(GL_PROJECTION);
		
		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, 10);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glBindTexture(GL_TEXTURE_2D, framebuffer_texture);

		
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0,GL_BGRA, GL_UNSIGNED_BYTE, framebuffer);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		
	}
	else
	{
		back_buffer = XCreateImage(display,XDefaultVisual(display,screen),24,ZPixmap,0,(char *)framebuffer,width,height,32,4*width);
	}
	
	cairo=cairo_create(cairo_surface);

}

/*!
 * Destroys cairo and x-window
 */ 
void X11Window::Destroy()
{
	
		
	if(render_method==LGI_FLAG_EXT_RENDER_OPENGL)
	{
		glXMakeCurrent(display, None, nullptr);
		glXDestroyContext(display, glc);
		
		cairo_surface_destroy(cairo_surface);
		delete framebuffer;
		cairo_destroy(cairo);
	}
	else
	{
		XDestroyImage(back_buffer);
		
		cairo_surface_destroy(cairo_surface);
		cairo_destroy(cairo);
	}
	
	
	XDestroyWindow(display, xwindow);
}

/*!
 * Sets window title name
 */ 
void X11Window::SetTitle(const char * title)
{
	XStoreName(display,xwindow,title);
}

/*!
 * checks for an event
 */ 
void X11Window::GetEvent()
{
	RawEvent * lgi_event = nullptr;
	XEvent event;
	XEvent out_event;
	Widget *  widget;
	Layer * layer;
	
	int key_index;
	
	bool is_autorepeat;
	
	
	
	if(XCheckTypedWindowEvent(display,xwindow,ClientMessage,&event))
	{
		/* Window is destroyed */
		if(event.xclient.message_type==wm_protocols)
		{
			if(event.xclient.data.l[0] == wm_delete_window)
			{
				lgi_event=new RawEvent();
				lgi_event->event=new DestroyEvent();
				PushEvent(lgi_event);
			}
		}
		
		/* DnD Enter */
		if(event.xclient.message_type==XdndEnter)
		{
						
			vector<string> droptypes;
			
			Window srcwin = event.xclient.data.l[0];
			int version = event.xclient.data.l[1]>>24;
			bool three_types= (event.xclient.data.l[1] & 0x1UL)==0;
			
			if(three_types)
			{
					Log("Simple dnd");
					
					for(int n=0;n<3;n++)
					{
						if(event.xclient.data.l[2+n]!=0)
						{
							string target = XGetAtomName(display,event.xclient.data.l[2+n]);
							droptypes.push_back(target);
							Log("- target:"+target);
						
						}
					}
			}
			else
			{
				Log("Enhanced dnd");
				
				/* code taken and adapted from Blender xdnd.c */
				int format;
				unsigned long count, remaining;
				unsigned char *data = nullptr;
				Atom type;
				
				XGetWindowProperty(display,srcwin,XdndTypeList,0,0x8000000L,False,XA_ATOM,&type,&format,&count,&remaining,&data);
				
				if (type != XA_ATOM || format != 32 || count == 0 || !data)
				{
					if(data)
					{
						XFree(data);
						Log("* Failed to get dnd type list");
						return;
					}
				}
				
				
				Atom * list = reinterpret_cast<Atom * >(data);
				
				for(int n=0;n<count;n++)
				{
					Atom a = list[n];
					string target = XGetAtomName(display,a);
					Log("- target:"+target);
					droptypes.push_back(target);
				}
				
				XFree(data);
			}
			
			
						
			
			
			dnd_allowed=false;
			string target_name;
			
			/* looking for a target match */
			for(string s : dnd_targets)
			{
				for(string q : droptypes)
				{
					if(q==s)
					{
						target_name=q;
						dnd_allowed=true;
						break;
					}
				}
				
				if(dnd_allowed)break;
								
			}
			
			if(dnd_allowed)
			{
				dropdata = XInternAtom(display,target_name.c_str(),true);
				
				lgi_event = new RawEvent();
				lgi_event->event = new DndEnterEvent();
								
				static_cast<DndEnterEvent *>(lgi_event->event)->target=target_name;
				
				lgi_event->widget=nullptr;
				lgi_event->layer=nullptr;
		 
				PushEvent(lgi_event);
			}
			
			
		}
		
		/* Dnd position */
		if(event.xclient.message_type==XdndPosition)
		{
			int root_x,root_y;
			int src_x,src_y;
			Window wchild;
			
			root_x=event.xclient.data.l[2] >> 16;
			root_y=event.xclient.data.l[2] & 0xFFFFUL;
			
			XTranslateCoordinates(display,XDefaultRootWindow(display),xwindow,root_x,root_y,&src_x,&src_y,&wchild);
			
			lgi_event = new RawEvent();
			lgi_event->event = new DndMoveEvent();
						
			lgi_event->widget=nullptr;
			lgi_event->layer=nullptr;
			
			static_cast<DndMoveEvent*>(lgi_event->event)->x=src_x;
			static_cast<DndMoveEvent*>(lgi_event->event)->y=src_y;
			 
			PushEvent(lgi_event);					
			
			XClientMessageEvent m;
			memset(&m, 0,sizeof(m));
			m.type = ClientMessage;
			m.display = event.xclient.display;
			m.window = event.xclient.data.l[0];
			m.message_type = XdndStatus;
			m.format=32;
			m.data.l[0] = xwindow;
			m.data.l[1] = dnd_allowed; //1- we accept, 0- we not
			m.data.l[2] = 0; //Specify an empty rectangle
			m.data.l[3] = 0;
			m.data.l[4] = XdndActionCopy; //We only accept copying anyway.

			XSendEvent(display, event.xclient.data.l[0], False, NoEventMask, (XEvent*)&m);
			XFlush(display);

			
		}
		
		
		/* Dnd Leave */
		if(event.xclient.message_type==XdndLeave)
		{
						
			lgi_event = new RawEvent();
			lgi_event->event = new DndLeaveEvent();
						
			lgi_event->widget=nullptr;
			lgi_event->layer=nullptr;
			 
			PushEvent(lgi_event);
		}
		
		/* Dnd Drop */
		if(event.xclient.message_type==XdndDrop)
		{
			Window srcwin = event.xclient.data.l[0];
			
			XConvertSelection(display, XdndSelection, dropdata, XInternAtom(display, "PRIMARY", 0), xwindow, event.xclient.data.l[2]);
			
			//Reply OK.
			XClientMessageEvent m;
			memset(&m, 0,sizeof(m));
			m.type = ClientMessage;
			m.display = display;
			m.window = srcwin; // ??????? Who we are ?????
			m.message_type = XdndFinished;
			m.format=32;
			m.data.l[0] = xwindow;
			m.data.l[1] = 1;
			m.data.l[2] = XdndActionCopy; //We only ever copy.

			//Reply that all is ok.
			XSendEvent(display, srcwin, False, NoEventMask, (XEvent*)&m);	
			
		}
		
		
	}
	
	/* Getting DND drop from clipboard */
	if(XCheckTypedWindowEvent(display,xwindow,SelectionNotify,&event))
	{
		/*
		cout << "A selection notify has arrived!"<<endl;
		cout << hex << "Requestor = 0x" << event.xselectionrequest.requestor << dec << endl;
		cout << "Selection atom = " << XGetAtomName(display, event.xselection.selection) << endl;	
		cout << "Target atom    = " << XGetAtomName(display, event.xselection.target)    << endl;	
		cout << "Property atom  = " << XGetAtomName(display, event.xselection.property) << endl;
		*/
		
		/* actually limited to 64K of data */
		unsigned char data[65536];
		int len = ReadProperty(XInternAtom(display, "PRIMARY", 0),data);
		
				
		/* Reply OK */
		XClientMessageEvent m;
		memset(&m, 0,sizeof(m));
		m.type = ClientMessage;
		m.display = display;
		m.window = event.xclient.data.l[0];
		m.message_type = XdndFinished;
		m.format=32;
		m.data.l[0] = xwindow;
		m.data.l[1] = 1;
		m.data.l[2] = XdndActionCopy; //We only ever copy.

		/* Reply that all is well */
		XSendEvent(display, event.xclient.data.l[0], False, NoEventMask, (XEvent*)&m);
				
		
		lgi_event= new RawEvent();
		lgi_event->event = new DndDropEvent();
		
		static_cast<DndDropEvent*>(lgi_event->event)->target = string(XGetAtomName(display, event.xselection.property));
		static_cast<DndDropEvent*>(lgi_event->event)->size=len;
		static_cast<DndDropEvent*>(lgi_event->event)->data=new unsigned char[len];
		memcpy(static_cast<DndDropEvent*>(lgi_event->event)->data,data,len);
		lgi_event->widget=nullptr;
		lgi_event->layer=nullptr;
		 
		PushEvent(lgi_event);
	}
		
	/* Check for common events */
	if(XCheckWindowEvent(display,xwindow,StructureNotifyMask | KeymapStateMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask,&event))
	{
		switch(event.type)
		{
			
			case ConfigureNotify:
				//cout<<"Configure Notify!"<<endl;
				if(event.xconfigure.width!=width || event.xconfigure.height!=height)
				{
					Resize(event.xconfigure.width,event.xconfigure.height);
					lgi_event=new RawEvent();
					lgi_event->event=new ResizeEvent();
					
					static_cast<ResizeEvent *>(lgi_event->event)->width=event.xconfigure.width;
					static_cast<ResizeEvent *>(lgi_event->event)->height=event.xconfigure.height;
					PushEvent(lgi_event);
				}
			break;
			
			case MotionNotify:
				lgi_event=new RawEvent();
				lgi_event->event=new MouseMoveEvent();
				static_cast<MouseMoveEvent *>(lgi_event->event)->x=event.xmotion.x;
				static_cast<MouseMoveEvent *>(lgi_event->event)->y=event.xmotion.y;
				
				cursor_x=event.xmotion.x;
				cursor_y=event.xmotion.y;
			
				/* Check for Mouse enter/exit from widget*/
				widget=nullptr;
				layer=nullptr;
				GetCollision(event.xmotion.x,event.xmotion.y,&widget,&layer);
				
				lgi_event->widget=widget;
				lgi_event->layer=layer;
			
				PushEvent(lgi_event);
				
				for(int m=0;m<layers.size();m++)
				{
					if(layers[m]->visible)
					{
						for(int n=0;n<layers[m]->widgets.size();n++)
						{
							if(layers[m]->widgets[n]==widget)
							{
								if(layers[m]->widgets[n]->mouse_over==false)
								{
									
									layers[m]->widgets[n]->mouse_over=true;
									/* Push mouse enter */
									
									lgi_event = new RawEvent();
									lgi_event->event=new MouseEnterEvent();
									lgi_event->widget=layers[m]->widgets[n];
									lgi_event->layer=layers[m];
									
									PushEvent(lgi_event);
								}
							}
							else
							{
								if(layers[m]->widgets[n]->mouse_over==true)
								{
									if(layers[m]->widgets[n]->mouse_press==true)
									{
										//cout<<"[Window]: Dragging widget "<<layers[m]->widgets[n]->name<<endl;
										dragging=layers[m]->widgets[n];
										layers[m]->widgets[n]->mouse_press=false;
										
										/* push drag event */
										lgi_event = new RawEvent();
										lgi_event->event = new DragEvent();
										lgi_event->widget = layers[m]->widgets[n];
										lgi_event->layer = layers[m];
										
										PushEvent(lgi_event);
										
									}
									
									layers[m]->widgets[n]->mouse_over=false;
									
									/* Push mouse exit */
									lgi_event = new RawEvent();
									lgi_event->event=new MouseExitEvent();
									lgi_event->widget=layers[m]->widgets[n];
									lgi_event->layer=layers[m];
									
									PushEvent(lgi_event);
									
									
								}
							}
						}
					}
				}
					
				
				
			break;
				
			case ButtonPress:
				lgi_event=new RawEvent();
				lgi_event->event=new MouseDownEvent();

				static_cast<MouseDownEvent *>(lgi_event->event)->button=event.xbutton.button;
				static_cast<MouseDownEvent *>(lgi_event->event)->x=event.xbutton.x;
				static_cast<MouseDownEvent *>(lgi_event->event)->y=event.xbutton.y;
						
				if(GetCollision(event.xbutton.x,event.xbutton.y,&widget,&layer))
				{
					widget->mouse_press=true;
					SetFocus(layer,widget);
				}
				
				lgi_event->widget=widget;
				lgi_event->layer=layer;
				
				PushEvent(lgi_event);
				
				clicks[event.xbutton.button].x = event.xbutton.x;
				clicks[event.xbutton.button].y = event.xbutton.y;
				
				
			break;
			
			case ButtonRelease:
				lgi_event=new RawEvent();
				lgi_event->event=new MouseUpEvent();

				static_cast<MouseUpEvent *>(lgi_event->event)->button=event.xbutton.button;
				static_cast<MouseUpEvent *>(lgi_event->event)->x=event.xbutton.x;
				static_cast<MouseUpEvent *>(lgi_event->event)->y=event.xbutton.y;
								
				
				
				widget=nullptr;
				if(GetCollision(event.xbutton.x,event.xbutton.y,&widget,&layer))
				{
					widget->mouse_press=false;
				}
				
				lgi_event->widget=widget;
				lgi_event->layer=layer;
				
				PushEvent(lgi_event);
				
				/* Drop */				
				if(dragging!=nullptr)
				{
					if(widget==nullptr)
					{
						//cout<<"[Window]: Drop widget: "<<dragging->name<<endl;
					}
					else
					{
						//cout<<"[Window]: Drop widget: "<<dragging->name<<" over "<<widget->name<<endl;
					}
					
					/* push drop event */
					lgi_event = new RawEvent();
					lgi_event->event = new DropEvent();
					static_cast<DropEvent *>(lgi_event->event)->x=event.xbutton.x;
					static_cast<DropEvent *>(lgi_event->event)->y=event.xbutton.y;
					static_cast<DropEvent *>(lgi_event->event)->data=(void *)dragging;
					
					lgi_event->widget = widget;
					lgi_event->layer = layer;
					PushEvent(lgi_event);
					
					dragging=nullptr;
				}
				
				
				if(GetDist(clicks[event.xbutton.button].x,clicks[event.xbutton.button].y,event.xbutton.x,event.xbutton.y)<5)
				{
					
					/* double click  */
					if( (event.xbutton.time - clicks[event.xbutton.button].time)<double_click_ms  ) 
					{
						clicks[event.xbutton.button].time = 0;
						
						/* Push double click event */
						
						lgi_event = new RawEvent();
						lgi_event->event = new MouseDoubleClickEvent();
						static_cast<MouseDoubleClickEvent *>(lgi_event->event)->x=event.xbutton.x;
						static_cast<MouseDoubleClickEvent *>(lgi_event->event)->y=event.xbutton.y;
						static_cast<MouseDoubleClickEvent *>(lgi_event->event)->button=event.xbutton.button;
					
						lgi_event->widget = widget;
						lgi_event->layer = layer;
						PushEvent(lgi_event);
						
					}
					else
					{
						/* single click */
						clicks[event.xbutton.button].time = event.xbutton.time;
				
						/* Push Click event */
						
						lgi_event = new RawEvent();
						lgi_event->event = new MouseClickEvent();
						static_cast<MouseClickEvent *>(lgi_event->event)->x=event.xbutton.x;
						static_cast<MouseClickEvent *>(lgi_event->event)->y=event.xbutton.y;
						static_cast<MouseClickEvent *>(lgi_event->event)->button=event.xbutton.button;
					
						lgi_event->widget = widget;
						lgi_event->layer = layer;
						PushEvent(lgi_event);
						
					}
					
										
				}	
				
			break;
			
			case KeymapNotify:
				XRefreshKeyboardMapping(&event.xmapping);
			break;
			
			case KeyPress:

				key_index = (keymap[((event.xkey.keycode-8)*keysym_per_code)+0]);
				
				
				lgi_event=new RawEvent();
				lgi_event->event=new KeyDownEvent();
				static_cast<KeyDownEvent *>(lgi_event->event)->key=MapKey(key_index);
				lgi_event->widget=nullptr;
				lgi_event->layer=nullptr;
				PushEvent(lgi_event);
			
				lgi_event=new RawEvent();
				lgi_event->event=new KeyPressEvent();
				key_index = (keymap[((event.xkey.keycode-8)*keysym_per_code)+0]);
				static_cast<KeyPressEvent *>(lgi_event->event)->key=MapKey(key_index);
				lgi_event->widget=nullptr;
				lgi_event->layer=nullptr;
						
				PushEvent(lgi_event);
								
				int len;
				KeySym keysym;
				unsigned char str[4];
				len = XLookupString(&event.xkey, (char *)str, 4, &keysym, nullptr);
				if(len>0)
				{
					
					lgi_event=new RawEvent();
					lgi_event->event=new CharacterEvent();
					lgi_event->widget=nullptr;
					lgi_event->layer=nullptr;
					Latin1ToUTF8(str,((CharacterEvent *)lgi_event->event)->str);
					PushEvent(lgi_event);					
					
				}
					
				
				
			break;
			
			case KeyRelease:
				
				is_autorepeat=false;
			
				if(XPending(display))
				{
					XEvent next;
					XPeekEvent(display,&next);
					if( next.type == KeyPress && next.xkey.keycode == event.xkey.keycode && (next.xmotion.time - event.xmotion.time < 2 ))
					{
						is_autorepeat=true;
						//We don't want next event on the queue, so it is readed and discarded
						XNextEvent(display,&next);
						
						lgi_event=new RawEvent();
						lgi_event->event=new KeyPressEvent();
						key_index = (keymap[((event.xkey.keycode-8)*keysym_per_code)+0]);
						static_cast<KeyPressEvent *>(lgi_event->event)->key=MapKey(key_index);
						lgi_event->widget=nullptr;
						lgi_event->layer=nullptr;
								
						PushEvent(lgi_event);
	
					}
				}
				
				if(!is_autorepeat)
				{
					lgi_event=new RawEvent();
					lgi_event->event=new KeyUpEvent();
					key_index = (keymap[((event.xkey.keycode-8)*keysym_per_code)+0]);
					static_cast<KeyUpEvent *>(lgi_event->event)->key=MapKey(key_index);
					lgi_event->widget=nullptr;
					lgi_event->layer=nullptr;
					
					PushEvent(lgi_event);
				
				}
				
			break;
			
			case MapNotify:
				//cout<<"[Window::GetEvent]: MapNotify"<<endl;
			break;
			
			case DestroyNotify:
				//cout<<"[Window::Wait]: DestroyNotify"<<endl;
			break;
			
			case Expose:
				if(event.xexpose.count==0)
				{
					lgi_event= new RawEvent();
					lgi_event->event=new lgi::ExposeEvent();
					lgi_event->layer=nullptr;
					lgi_event->widget=nullptr;
					
					PushEvent(lgi_event);
				}
			break;
		}
	}

	
	
}


void X11Window::DispatchEvents(int mode)
{
	RawEvent * raw_event;
	int t;	
	int dp_events;
	
	start:
	
	t = GetTicks();
	dp_events=0;
	
	while(1)
	{
		GetEvent();
		raw_event = PopEvent();
		if(raw_event==nullptr)break;
		dp_events++;
		
		
		/*
		 * Only 10 ms are allowed for event dispatching, after timeout
		 *  we go on rendering, expecting the event queue to be fully dispatched on next frame
		 *  but this may never happen!
		  */
		if((GetTicks()-t) > 10)
		{
			Log("Event dispatch timeout! (>10ms)");
			break;
		}
		
		ProcessEvent(raw_event);
		
		//Free event
		delete raw_event->event;
		delete raw_event;
		
	}
	
	if(mode==LGI_DISPATCH_EVENTS_WAIT && dp_events==0)
	{
		
		//If there are no available events we just take a rest of 15ms, maybe this sleep time should be customizable
		
		usleep(15000);//15ms wait
		goto start;//hell yes, this is a f*cking goto! That one goes for you UPV guys!
	}
}

/**
* Pushes a synthethized event from outside into the event queue
**/
void X11Window::PushEvent(RawEvent * raw_event)
{
	pthread_mutex_lock(&queue_mutex);
	event_queue.push(raw_event);
	pthread_mutex_unlock(&queue_mutex);
}

/**
* Pops event from the queue
**/
RawEvent * X11Window::PopEvent()
{
	RawEvent * ret = nullptr;
	
	pthread_mutex_lock(&queue_mutex);
	
	if(!event_queue.empty())
	{
		ret = event_queue.front();
		event_queue.pop();
	}
	
	pthread_mutex_unlock(&queue_mutex);
	
	return ret;
}

/**
* Process event and destroys it
**/
void X11Window::ProcessEvent(RawEvent * raw_event)
{
	switch(raw_event->event->type)
	{
		//You know what? this bunch of loops can be replaced with iterators
		case LGI_EVENT_EXPOSE:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnExpose(static_cast<ExposeEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_RESIZE:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnResize(static_cast<ResizeEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_DESTROY:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnDestroy(static_cast<DestroyEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_KEY_PRESS:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyPress(static_cast<KeyPressEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_KEY_DOWN:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyDown(static_cast<KeyDownEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_KEY_UP:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyUp(static_cast<KeyUpEvent *>(raw_event->event));
			}
		break;
			
		case LGI_EVENT_CHARACTER:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnCharacter(static_cast<CharacterEvent *>(raw_event->event));
			}
		break;
			
		case LGI_EVENT_MOUSE_CLICK:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseClick(raw_event->widget,static_cast<MouseClickEvent *>(raw_event->event));
					raw_event->widget->OnMouseClick(static_cast<MouseClickEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_DOUBLE_CLICK:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseDoubleClick(raw_event->widget,static_cast<MouseDoubleClickEvent *>(raw_event->event));
					raw_event->widget->OnMouseDoubleClick(static_cast<MouseDoubleClickEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_DOWN:
		
			if(raw_event->layer==nullptr)
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]->visible)
					{
						layers[n]->OnMouseDown(nullptr,static_cast<MouseDownEvent *>(raw_event->event));
					}
				}
			}
			else
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]->visible && raw_event->layer==layers[n])
					{
						layers[n]->OnMouseDown(raw_event->widget,static_cast<MouseDownEvent *>(raw_event->event));
						raw_event->widget->OnMouseDown(static_cast<MouseDownEvent *>(raw_event->event));
					}
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_UP:
		
			if(raw_event->layer==nullptr)
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]->visible)
					{
						layers[n]->OnMouseUp(nullptr,static_cast<MouseUpEvent *>(raw_event->event));
					}
				}
			}
			else
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]->visible && raw_event->layer==layers[n])
					{
						layers[n]->OnMouseUp(raw_event->widget,static_cast<MouseUpEvent *>(raw_event->event));
						raw_event->widget->OnMouseUp(static_cast<MouseUpEvent *>(raw_event->event));
					}
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_ENTER:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseEnter(raw_event->widget,static_cast<MouseEnterEvent *>(raw_event->event));
					raw_event->widget->OnMouseEnter(static_cast<MouseEnterEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_EXIT:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseExit(raw_event->widget,static_cast<MouseExitEvent *>(raw_event->event));
					raw_event->widget->OnMouseExit(static_cast<MouseExitEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_MOVE:
		
			/* this happens when there is no widget collision,
			 in that case, we push a mousemove event with nullptr widget target
			*/
			if(raw_event->layer==nullptr)
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]->visible)
						layers[n]->OnMouseMove(nullptr,static_cast<MouseMoveEvent *>(raw_event->event));
				}
			}
			else
			{
				for(int n=0;n<layers.size();n++)
				{
			

					if(layers[n]->visible && raw_event->layer==layers[n])
					{
						layers[n]->OnMouseMove(raw_event->widget,static_cast<MouseMoveEvent *>(raw_event->event));					raw_event->widget->OnMouseMove(static_cast<MouseMoveEvent *>(raw_event->event));
					}
				}
			}
		break;
		
		case LGI_EVENT_DRAG:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnDrag(raw_event->widget,static_cast<DragEvent *>(raw_event->event));
					raw_event->widget->OnDrag(static_cast<DragEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_DROP:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnDrop(raw_event->widget,static_cast<DropEvent *>(raw_event->event));
					raw_event->widget->OnDrop(static_cast<DropEvent *>(raw_event->event));
				}
			}
		break;
			
		case LGI_EVENT_MESSAGE:
			/* Broadcast message 
			* This message is sent to all layers, but it does not go down to their widgets
			*/
			if(raw_event->layer==nullptr)
			{
				for(int n=0;n<layers.size();n++)
				{
					layers[n]->OnMessage(raw_event->widget,static_cast<MessageEvent *>(raw_event->event));
					
				}		
			}
			else /* direct layer:widget message */
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]==raw_event->layer)
					{
						layers[n]->OnMessage(raw_event->widget,static_cast<MessageEvent *>(raw_event->event));
						
						if(raw_event->widget!=nullptr)
						{
							raw_event->widget->OnMessage(static_cast<MessageEvent *>(raw_event->event));
						}
						break;
					}
				}	
			}
		break;
			
		case LGI_EVENT_LAYER_ADD:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnAdd(static_cast<LayerAddEvent *>(raw_event->event));
				}
			}
		break;
			
		case LGI_EVENT_LAYER_REMOVE:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnRemove(static_cast<LayerRemoveEvent *>(raw_event->event));
				}
			}
		break;
			
		case LGI_EVENT_LAYER_SHOW:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnShow(static_cast<LayerShowEvent *>(raw_event->event));
				}
			}
		break;
			
		case LGI_EVENT_LAYER_HIDE:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnHide(static_cast<LayerHideEvent *>(raw_event->event));
				}
			}
		break;
		
		case LGI_EVENT_DND_ENTER:
			for(int n=0;n<layers.size();n++)
			{
					layers[n]->OnDndEnter(static_cast<DndEnterEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_DND_LEAVE:
			for(int n=0;n<layers.size();n++)
			{
					layers[n]->OnDndLeave(static_cast<DndLeaveEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_DND_DROP:
			for(int n=0;n<layers.size();n++)
			{
					layers[n]->OnDndDrop(static_cast<DndDropEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_DND_MOVE:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnDndMove(static_cast<DndMoveEvent *>(raw_event->event));
			}
		break;
		
		case LGI_EVENT_GOT_FOCUS:
			raw_event->layer->OnGotFocus(raw_event->widget,static_cast<GotFocusEvent *>(raw_event->event));
			raw_event->widget->OnGotFocus(static_cast<GotFocusEvent *>(raw_event->event));
		break;
		
		case LGI_EVENT_LOST_FOCUS:
			raw_event->layer->OnLostFocus(raw_event->widget,static_cast<LostFocusEvent *>(raw_event->event));
			raw_event->widget->OnLostFocus(static_cast<LostFocusEvent *>(raw_event->event));
		break;
		
	}
}

/**
 * Blits cairo target surface into xwindow
 **/ 
void X11Window::Flip()
{
	std::sort(layers.begin(),layers.end(),sort_func);
	
	for(int m=layers.size()-1;m>=0;m--)
	{
		if(layers[m]->visible)
		{
			layers[m]->Draw(cairo);		
		}	
	}
	
	if(cursor_mode==LGI_CURSOR_MODE_CUSTOM && custom_cursor!=nullptr)
	{
		cairo_save(cairo);
			cairo_translate(cairo,cursor_x,cursor_y);	
			custom_cursor->Draw(cairo);	
		cairo_restore(cairo);	
	}
	
	//DrawBB();
	if(render_method==LGI_FLAG_EXT_RENDER_OPENGL)
	{
		GLfloat vertices[] = {0,0,0,  (GLfloat)width,0,0,  (GLfloat)width,(GLfloat)height,0,  0,(GLfloat)height,0};
		GLfloat uv[] = {0,0, 1,0, 1,1, 0,1};
		
		glClear(GL_COLOR_BUFFER_BIT);

				
		glBindTexture(GL_TEXTURE_2D,framebuffer_texture);
		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,GL_BGRA,GL_UNSIGNED_BYTE,framebuffer);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glVertexPointer(3, GL_FLOAT, 0,vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, uv);
		
		glDrawArrays(GL_QUADS,0,4);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glXSwapBuffers(display,xwindow);
	}
	else
	{
		XPutImage(display,xwindow,gc,back_buffer,0,0,0,0,width,height);
		XFlush(display);	
	}
}

/**
 * Gets width dimension
 **/ 
int X11Window::GetWidth()
{
	return width;	
}

/**
 * Gets height dimension
 **/ 
int X11Window::GetHeight()
{
	return height;
}


void X11Window::SetSize(int width,int height)
{
	//XMoveResizeWindow(display,xwindow,0,0,width,height);
	XResizeWindow(display,xwindow,width,height);
	Resize(width,height);
}


/**
 * Toggles window fullscreen
 **/ 
void X11Window::FullScreen()
{
	int sw,sh;
	XEvent xev;
	Atom wm_state;
	Atom fullscreen;
	
	GetScreenSize(&sw,&sh);
	XMoveResizeWindow(display,xwindow,0,0,sw,sh);
	
	wm_state = XInternAtom(display, "_NET_WM_STATE", False);
	fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = xwindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 2;
	

	if(!XSendEvent(display, DefaultRootWindow(display), False,SubstructureNotifyMask | SubstructureRedirectMask, &xev))
	{
		Log("Failed to set Full Screen");
	}
}


void X11Window::AddLayer(Layer * layer)
{
	
	layers.push_back(layer);
	
	RawEvent * lgi_event;
	lgi_event= new RawEvent();
	lgi_event->event=new LayerAddEvent();
	lgi_event->layer=layer;
	lgi_event->widget=nullptr;
	
	PushEvent(lgi_event);
}


void X11Window::RemoveLayer(Layer * layer)
{
	vector<Layer *>tmp;
	
	for(int n=0;n<layers.size();n++)
	{
		if(layers[n]!=layer)
			tmp.push_back(layers[n]);
	}
	
	layers = tmp;
	
	RawEvent * lgi_event;
	lgi_event= new RawEvent();
	lgi_event->event=new LayerRemoveEvent();
	lgi_event->layer=layer;
	lgi_event->widget=nullptr;
	
	PushEvent(lgi_event);
}

void X11Window::ShowLayer(Layer * layer)
{
	layer->visible=true;
	
	RawEvent * lgi_event;
	lgi_event= new RawEvent();
	lgi_event->event=new LayerShowEvent();
	lgi_event->layer=layer;
	lgi_event->widget=nullptr;
	
	PushEvent(lgi_event);
}

void X11Window::HideLayer(Layer * layer)
{
	layer->visible=false;
	
	RawEvent * lgi_event;
	lgi_event= new RawEvent();
	lgi_event->event=new LayerHideEvent();
	lgi_event->layer=layer;
	lgi_event->widget=nullptr;
	
	PushEvent(lgi_event);
}


void X11Window::SetCursor(int type)
{
	this->cursor_type=type;
	
	switch(this->cursor_mode)
	{
		case LGI_CURSOR_MODE_CUSTOM:
			if(custom_cursor!=nullptr)
			{
				custom_cursor->SetCursor(type);
			}
		break;
		
		case LGI_CURSOR_MODE_SYSTEM:
			Cursor xlib_cursor;
			xlib_cursor = XcursorLibraryLoadCursor(display,GetX11CursorName(this->cursor_type).c_str());
			XDefineCursor(display,xwindow,xlib_cursor);
			XFreeCursor(display,xlib_cursor);
		break;
	}
	
}

void X11Window::SetCursorMode(int mode)
{
	this->cursor_mode=mode;
	
	switch(mode)
	{
		case LGI_CURSOR_MODE_CUSTOM:
			
		
		case LGI_CURSOR_MODE_NONE:
			Cursor invisibleCursor;
			Pixmap bitmapNoData;
			XColor black;
			static char noData[] = { 0,0,0,0,0,0,0,0 };
			black.red = black.green = black.blue = 0;

			bitmapNoData = XCreateBitmapFromData(display, xwindow, noData, 8, 8);
			invisibleCursor = XCreatePixmapCursor(display, bitmapNoData, bitmapNoData, &black, &black, 0, 0);
			XDefineCursor(display,xwindow, invisibleCursor);
			XFreeCursor(display, invisibleCursor);
		break;
		
		case LGI_CURSOR_MODE_SYSTEM:
			SetCursor(this->cursor_type);
		break;
		
		
	}
}

void X11Window::SetCustomCursor(BaseCursor * cursor)
{
	custom_cursor=cursor;
}

/**
* Sets double click time in milliseconds
**/
void X11Window::SetDoubleClickTime(int ms)
{
	double_click_ms=ms;
}

/**
* Sends a custom message down to the event queue
**/
void X11Window::SendMessage(Layer * layer,Widget * widget,Message * msg)
{
	RawEvent * event = new RawEvent();
	
	event->layer=layer;
	event->widget=widget;
	
	MessageEvent * msge = new MessageEvent();
	msge->msg=msg;
	event->event=msge;
	PushEvent(event);
}


/**
 * Gets the number of milliseconds since Class started
 **/ 
int X11Window::GetTicks()
{
	timeval current_time;
	gettimeofday(&current_time,nullptr);
	return (current_time.tv_sec - init_time.tv_sec)*1000 + (current_time.tv_usec - init_time.tv_usec)/1000.0;
}

/**
* Debug method, draws a red bounding box for all widgets
**/
void X11Window::DrawBB()
{
	for(int m=0;m<layers.size();m++)
	{
		if(layers[m]->visible)
		{
			cairo_save(cairo);
			cairo_translate(cairo,layers[m]->x,layers[m]->y);
					
				
			for(int n=0;n<layers[m]->widgets.size();n++)
			{
				cairo_set_source_rgb (cairo, 1.0, 0.0, 0.0);
				cairo_set_line_width (cairo, 1.0);
				cairo_rectangle(cairo,layers[m]->widgets[n]->x,layers[m]->widgets[n]->y ,layers[m]->widgets[n]->width,layers[m]->widgets[n]->height);
				cairo_stroke(cairo);
			}
			cairo_restore(cairo);
		}
	}
}

int X11Window::ReadProperty(Atom property,unsigned char * data)
{
	Atom actual_type;
	int actual_format;
	unsigned long nitems;
	unsigned long bytes_after;
	unsigned char *ret=0;

	int read_bytes = 4096;
	int len=0;
	int offset=0;
	int total_len=0;	

	//Keep trying to read the property until there are no
	//bytes unread.
	do
	{
		
		XGetWindowProperty(display, xwindow, property, 0, read_bytes, False, AnyPropertyType,&actual_type, &actual_format, &nitems, &bytes_after,&ret);
		
		len = (actual_format/8)*nitems;
		
		memcpy(data + total_len,ret,len);
		XFree(ret);
		
		total_len = total_len + len;
		
	}while(bytes_after != 0);

	
	//cout << "Actual type: " << XGetAtomName(display, actual_type) << endl;
	
	return total_len;
}

int X11Window::MapKey(KeySym k)
{
	int key=0xffff;
	
	switch(k)
	{
		case XK_a: key = LGI_KEY_a; break;
		case XK_b: key = LGI_KEY_b; break;
		case XK_c: key = LGI_KEY_c; break;
		case XK_d: key = LGI_KEY_d; break;
		case XK_e: key = LGI_KEY_e; break;
		case XK_f: key = LGI_KEY_f; break;
		case XK_g: key = LGI_KEY_g; break;
		case XK_h: key = LGI_KEY_h; break;
		case XK_i: key = LGI_KEY_i; break;
		case XK_j: key = LGI_KEY_j; break;
		case XK_k: key = LGI_KEY_k; break;
		case XK_l: key = LGI_KEY_l; break;
		case XK_m: key = LGI_KEY_m; break;
		case XK_n: key = LGI_KEY_n; break;
		case XK_o: key = LGI_KEY_o; break;
		case XK_p: key = LGI_KEY_p; break;
		case XK_q: key = LGI_KEY_q; break;
		case XK_r: key = LGI_KEY_r; break;
		case XK_s: key = LGI_KEY_s; break;
		case XK_t: key = LGI_KEY_t; break;
		case XK_u: key = LGI_KEY_u; break;
		case XK_v: key = LGI_KEY_v; break;
		case XK_w: key = LGI_KEY_w; break;
		case XK_x: key = LGI_KEY_x; break;
		case XK_y: key = LGI_KEY_y; break;
		case XK_z: key = LGI_KEY_z; break;

		case XK_0: key = LGI_KEY_0; break;
		case XK_1: key = LGI_KEY_1; break;
		case XK_2: key = LGI_KEY_2; break;
		case XK_3: key = LGI_KEY_3; break;
		case XK_4: key = LGI_KEY_4; break;
		case XK_5: key = LGI_KEY_5; break;
		case XK_6: key = LGI_KEY_6; break;
		case XK_7: key = LGI_KEY_7; break;
		case XK_8: key = LGI_KEY_8; break;
		case XK_9: key = LGI_KEY_9; break;
		
		case XK_Left: key = LGI_KEY_LEFT; break;
		case XK_Right: key = LGI_KEY_RIGHT; break;
		case XK_Up: key = LGI_KEY_UP; break;
		case XK_Down: key = LGI_KEY_DOWN; break;
		
		case XK_F1: key = LGI_KEY_F1; break;
		case XK_F2: key = LGI_KEY_F2; break;
		case XK_F3: key = LGI_KEY_F3; break;
		case XK_F4: key = LGI_KEY_F4; break;
		case XK_F5: key = LGI_KEY_F5; break;
		case XK_F6: key = LGI_KEY_F6; break;
		case XK_F7: key = LGI_KEY_F7; break;
		case XK_F8: key = LGI_KEY_F8; break;
		case XK_F9: key = LGI_KEY_F9; break;
		case XK_F10: key = LGI_KEY_F10; break;
		case XK_F11: key = LGI_KEY_F11; break;
		case XK_F12: key = LGI_KEY_F12; break;
		
		case XK_Return: key = LGI_KEY_ENTER; break;
		case XK_space: key = LGI_KEY_SPACE; break;
		
		case XK_KP_Add: key = LGI_KEY_PLUS; break;
		case XK_KP_Subtract: key = LGI_KEY_MINUS; break;
		
		case XK_Escape: key = LGI_KEY_ESCAPE; break;
		
		case XK_BackSpace: key = LGI_KEY_BACKSPACE; break;
		
		case XK_Home: key = LGI_KEY_HOME; break;
		case XK_End: key = LGI_KEY_END; break;
		case XK_Delete: key = LGI_KEY_DELETE; break;
		case XK_Insert: key = LGI_KEY_INSERT; break;
		
		case XK_Shift_L: key = LGI_KEY_LEFT_SHIFT; break;
		case XK_Shift_R: key = LGI_KEY_RIGHT_SHIFT; break;
		
		case XK_Control_L: key = LGI_KEY_LEFT_CTRL; break;
		case XK_Control_R: key = LGI_KEY_RIGHT_CTRL; break;
		
		case XK_Super_L: key = LGI_KEY_LEFT_SUPER; break;
		case XK_Super_R: key = LGI_KEY_RIGHT_SUPER; break;
		
		case XK_Alt_L: key = LGI_KEY_LEFT_ALT; break;
		case XK_Alt_R: key = LGI_KEY_RIGHT_ALT; break;
		
		case XK_Page_Up: key = LGI_KEY_PAGE_UP; break;
		case XK_Page_Down: key = LGI_KEY_PAGE_DOWN; break;
		
		case XK_Tab: key = LGI_KEY_TAB; break;
		
		default:
			/* take a look at keysymdef.h */
			stringstream ss;
			ss<<"Unmapped key:"<<hex<<k;
			Log(ss.str());
			key = 0xffff;
	}
	
	return key;
}

string X11Window::GetX11CursorName(int type)
{

	map<int,string> names={{LGI_CURSOR_DEFAULT,"left_ptr"},{LGI_CURSOR_BUSY,"watch"},{LGI_CURSOR_HALF_BUSY,"half-busy"},
	{LGI_CURSOR_OPEN_HAND,"openhand"},{LGI_CURSOR_CLOSE_HAND,"closehand"},{LGI_CURSOR_HAND,"hand"},
	{LGI_CURSOR_CROSS,"cross"},{LGI_CURSOR_FORBIDDEN,"forbidden"},{LGI_CURSOR_QUESTION_ARROW,"question_arrow"},
	{LGI_CURSOR_FLEUR,"fleur"}
	};
	
	
	return names[type];
}

/*!
* There is room for improvement here... :(
*/
void X11Window::Latin1ToUTF8(unsigned char * in, unsigned char * out)
{
		if (*in<128) *out++=*in++;
			else *out++=0xc2+(*in>0xbf), *out++=(*in++&0x3f)+0x80;
			
		
}

float X11Window::GetDist(float x1,float y1,float x2,float y2)
{
	float a = x2-x1;
	float b = y2-y1;
	
	return sqrtf((a*a)+(b*b));
}

/**
 * Checks for a bounding box vs point collision
 * No Z sorting implemented yet
 **/  
bool X11Window::GetCollision(int x,int y,Widget ** widget,Layer ** layer)
{
	bool found=false;
	float wx,wy;
	

	for(int m=0;m<layers.size();m++)
	{
		if(layers[m]->visible)
		{
			for(int n=0;n<layers[m]->widgets.size();n++)
			{
				wx = layers[m]->x + layers[m]->widgets[n]->x;
				wy = layers[m]->y + layers[m]->widgets[n]->y;
				
				
				if(x>wx && y>wy && x<(wx+layers[m]->widgets[n]->width)  && y<(wy+layers[m]->widgets[n]->height) )
				{
					*layer=layers[m];
					*widget=layers[m]->widgets[n];
					found=true;
					break;
				}
			}
		}
		
		if(found)break;
	}
	return found;
}

void X11Window::SetDndTargets(vector<string> targets)
{
	dnd_targets=targets;
}

Widget * X11Window::GetFocus()
{
	if(focus_layer==nullptr)
		return nullptr;
	else
		return focus_layer->focus;
}

void X11Window::NextFocus(Layer * layer)
{
	if(layer==nullptr)
	{
		if(focus_layer==nullptr)
		{
			/* nothing to do here :) */
			return;
		}
		else
		{
			layer=focus_layer;
		}
	}
	
	/* it has no sense to cycle with just one widget */
	if(layer->widgets.size()>1)
	{
		Widget * widget;
		int t;
	
		for(int n=0;n<layer->widgets.size();n++)
		{
			if(layer->widgets[n]==layer->focus)
			{
				t=n;
				break;
			}
		}
		
		t++;
		t=t%layer->widgets.size();
		
		widget=layer->widgets[t];
				
		SetFocus(layer,widget);
								
					
	}
}

void X11Window::SetFocus(Layer * layer,Widget * widget)
{
	RawEvent * lgi_event;
			
	Widget * focus_last=nullptr;
	
	if(focus_layer!=nullptr)
	{
		if(focus_layer->focus!=nullptr)
		{
			focus_last=focus_layer->focus;
			
			if(widget!=focus_last)
			{
				focus_layer->focus->focus=false;
		
				lgi_event = new RawEvent();
				lgi_event->event = new LostFocusEvent();
	
				lgi_event->widget=focus_layer->focus;
				lgi_event->layer=focus_layer;
	
				PushEvent(lgi_event);
			}
	
		}
	}
	
	if(widget!=focus_last)
	{
		focus_layer=layer;
		focus_layer->focus=widget;
		focus_layer->focus->focus=true;
	
		lgi_event = new RawEvent();
		lgi_event->event = new GotFocusEvent();

		lgi_event->widget=widget;
		lgi_event->layer=layer;
	
		PushEvent(lgi_event);
	}
}


