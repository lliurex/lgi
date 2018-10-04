#include "fbwindow.hpp"
#include "events.hpp"
#include "exceptions.hpp"
#include "widget.hpp"
#include "events.hpp"
#include "layer.hpp"
#include "message.hpp"
#include "cursor.hpp"

#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

using namespace net::lliurex::lgi;


bool sort_func(const Layer * l1,const Layer * l2)
{
	return (l1->depth<l2->depth);
}


fbWindow::fbWindow(int width,int height,const char * filename,int flags)
{
	/* var state */
	
	dragging=NULL;
	memset(buttons,0,6*sizeof(int));
	
	gettimeofday(&init_time,NULL);
	
	/* open device */
	fb_fd = open(filename,O_RDWR);
	
	if(fb_fd==-1)throw Exception(string("Failed to open device:")+filename);
	
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) == -1)throw Exception("Failed to retrieve fscreen info");
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == -1)throw Exception("Failed to retrieve vscreen info");
	
	cout<<"* FrameBuffer info:"<<vinfo.xres<<"x"<<vinfo.yres<<" bpp:"<<vinfo.bits_per_pixel<<endl;
	
	this->width = vinfo.xres;
	this->height = vinfo.yres;
	
	if(vinfo.bits_per_pixel!=32)
	{
		throw Exception("Unsupported color depth");
	}
	
	/* mapping shared mem */
	framesize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	
	framebuffer = (uint8_t *)mmap(0, framesize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (framebuffer == MAP_FAILED)
	{
		throw  Exception("Failed to map framebuffer device to memory");
	}
	
	cout<<"* creating cairo surface"<<endl;
	
	/* backbuffer and cairo surface */
	backbuffer = new uint8_t[vinfo.xres * vinfo.yres * 4];
	
	cairo_surface = cairo_image_surface_create_for_data(backbuffer,CAIRO_FORMAT_ARGB32,this->width,this->height,this->width*4);
	cairo=cairo_create(cairo_surface);
	
	cout<<"* opening evdev devices"<<endl;
	
	/* input devices */
	mouse_fd = open("/dev/input/event3", O_RDWR);
	int ioflags = fcntl(mouse_fd, F_GETFL, 0);
	fcntl(mouse_fd, F_SETFL, ioflags | O_NONBLOCK);
}

fbWindow::~fbWindow()
{
}

void fbWindow::Resize(int w,int h)
{
	/* Nothing to do */
}

void fbWindow::Destroy()
{
	munmap(framebuffer, framesize);
	
	close(fb_fd);
	close(mouse_fd);
	
	cairo_surface_destroy(cairo_surface);
	cairo_destroy(cairo);
	delete backbuffer;
}

void fbWindow::SetTitle(const char * title)
{
	/* Nothing to do */
}


void fbWindow::GetEvent()
{
	struct input_event event[64];
	RawEvent * lgi_event = NULL;
	Widget * widget;
	Layer * layer;

	/* read up to 64 events */
	int sz=sizeof(struct input_event);
	int n = read(mouse_fd,event,sz*64);
	
	if(n>0)
	{
		for(int i=0;i<(n/sz);i++)
		{
			if(event[i].type==EV_REL)
			{
				if(event[i].code==REL_X)
				{
					mx+=event[i].value;
									
					if(mx<0)mx=0;
					if(mx>=width)mx=width-1;			
								
				}
				
				if(event[i].code==REL_Y)
				{
					my+=event[i].value;
										
					if(my<0)my=0;
					if(my>=height)my=height-1;	
													
				}
				
				lgi_event = new RawEvent();
				lgi_event->event=new MouseMoveEvent();
				((MouseMoveEvent *)lgi_event->event)->x=mx;
				((MouseMoveEvent *)lgi_event->event)->y=my;
				
				widget=NULL;
				layer=NULL;
				
				GetCollision(mx,my,&widget,&layer);
				
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
				
				
			}//EV_REL
			
			if(event[i].type==EV_KEY)
			{
				if(event[i].code==BTN_LEFT)
				{
					buttons[0]=event[i].value;
				}
			}//EV_KEY
			
			
		}//for
	}//if
}

void fbWindow::DispatchEvents(int mode)
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
		if(raw_event==NULL)break;
		dp_events++;
		
		
		/*
		 * Only 10 ms are allowed for event dispatching, after timeout
		 *  we go on rendering, expecting the event queue to be fully dispatched on next frame
		 *  but this may never happen!
		  */
		if((GetTicks()-t) > 10)
		{
			cout<<"Event dispatch timeout! (>10ms)"<<endl;
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

void fbWindow::PushEvent(RawEvent * raw_event)
{
	event_queue.push(raw_event);
}

RawEvent * fbWindow::PopEvent()
{
	RawEvent * ret = NULL;
		
	
	if(!event_queue.empty())
	{
		ret = event_queue.front();
		event_queue.pop();
	}
			
	return ret;
}

void fbWindow::ProcessEvent(RawEvent * raw_event)
{
	switch(raw_event->event->type)
	{
		case LGI_EVENT_EXPOSE:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnExpose((ExposeEvent *)raw_event->event);
			}
		break;
		
		case LGI_EVENT_RESIZE:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnResize((ResizeEvent *)raw_event->event);
			}
		break;
		
		case LGI_EVENT_DESTROY:
			for(int n=0;n<layers.size();n++)
			{
				layers[n]->OnDestroy((DestroyEvent *)raw_event->event);
			}
		break;
		
		case LGI_EVENT_KEY_PRESS:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyPress((KeyPressEvent *)raw_event->event);
			}
		break;
		
		case LGI_EVENT_KEY_DOWN:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyDown((KeyDownEvent *)raw_event->event);
			}
		break;
		
		case LGI_EVENT_KEY_UP:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnKeyUp((KeyUpEvent *)raw_event->event);
			}
		break;
			
		case LGI_EVENT_CHARACTER:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible)
					layers[n]->OnCharacter((CharacterEvent *)raw_event->event);
			}
		break;
			
		case LGI_EVENT_MOUSE_CLICK:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseClick(raw_event->widget,(MouseClickEvent *)raw_event->event);
					raw_event->widget->OnMouseClick((MouseClickEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_DOUBLE_CLICK:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseDoubleClick(raw_event->widget,(MouseDoubleClickEvent *)raw_event->event);
					raw_event->widget->OnMouseDoubleClick((MouseDoubleClickEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_DOWN:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseDown(raw_event->widget,(MouseDownEvent *)raw_event->event);
					raw_event->widget->OnMouseDown((MouseDownEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_UP:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseUp(raw_event->widget,(MouseUpEvent *)raw_event->event);
					raw_event->widget->OnMouseUp((MouseUpEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_ENTER:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseEnter(raw_event->widget,(MouseEnterEvent *)raw_event->event);
					raw_event->widget->OnMouseEnter((MouseEnterEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_EXIT:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseExit(raw_event->widget,(MouseExitEvent *)raw_event->event);
					raw_event->widget->OnMouseExit((MouseExitEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_MOUSE_MOVE:
			for(int n=0;n<layers.size();n++)
			{
				/*ToDo: if widget/layer = null we must send event to all visible layers */
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnMouseMove(raw_event->widget,(MouseMoveEvent *)raw_event->event);
					raw_event->widget->OnMouseMove((MouseMoveEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_DRAG:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnDrag(raw_event->widget,(DragEvent *)raw_event->event);
					raw_event->widget->OnDrag((DragEvent *)raw_event->event);
				}
			}
		break;
		
		case LGI_EVENT_DROP:
			for(int n=0;n<layers.size();n++)
			{
				if(layers[n]->visible && raw_event->layer==layers[n])
				{
					layers[n]->OnDrop(raw_event->widget,(DropEvent *)raw_event->event);
					raw_event->widget->OnDrop((DropEvent *)raw_event->event);
				}
			}
		break;
			
		case LGI_EVENT_MESSAGE:
			/* Broadcast message 
			* This message is sent to all layers, but it does not go down to their widgets
			*/
			if(raw_event->layer==NULL)
			{
				for(int n=0;n<layers.size();n++)
				{
					layers[n]->OnMessage(raw_event->widget,(MessageEvent *)raw_event->event);
					
				}		
			}
			else /* direct layer:widget message */
			{
				for(int n=0;n<layers.size();n++)
				{
					if(layers[n]==raw_event->layer)
					{
						layers[n]->OnMessage(raw_event->widget,(MessageEvent *)raw_event->event);
						
						if(raw_event->widget!=NULL)
						{
							raw_event->widget->OnMessage((MessageEvent *)raw_event->event);
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
					layers[n]->OnAdd((LayerAddEvent *)raw_event->event);
				}
			}
		break;
			
		case LGI_EVENT_LAYER_REMOVE:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnRemove((LayerRemoveEvent *)raw_event->event);
				}
			}
		break;
			
		case LGI_EVENT_LAYER_SHOW:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnShow((LayerShowEvent *)raw_event->event);
				}
			}
		break;
			
		case LGI_EVENT_LAYER_HIDE:
			for(int n=0;n<layers.size();n++)
			{
				if(raw_event->layer==layers[n])
				{
					layers[n]->OnHide((LayerHideEvent *)raw_event->event);
				}
			}
		break;
	}
}

void fbWindow::Flip()
{
	
	std::sort(layers.begin(),layers.end(),sort_func);
	
	for(int m=layers.size()-1;m>=0;m--)
	{
		if(layers[m]->visible)
		{
			layers[m]->Draw(cairo);		
		}	
	}
	
	DrawMouse();
	
	memcpy(framebuffer,backbuffer,framesize);
}
				
int fbWindow::GetWidth()
{
	return width;
}

int fbWindow::GetHeight()
{
	return height;
}
							
void fbWindow::AddLayer(Layer * layer)
{
	layers.push_back(layer);
	
	RawEvent * lgi_event;
	lgi_event= new RawEvent();
	lgi_event->event=new LayerAddEvent();
	lgi_event->layer=layer;
	lgi_event->widget=NULL;
	
	PushEvent(lgi_event);
}

void fbWindow::RemoveLayer(Layer * layer)
{
}

void fbWindow::ShowLayer(Layer * layer)
{
}

void fbWindow::HideLayer(Layer * layer)
{
}

void fbWindow::SetCursor(int type)
{
}

void fbWindow::SetCursorMode(int mode)
{
}

void fbWindow::SetCustomCursor(BaseCursor * cursor)
{
}
				
void fbWindow::SetDoubleClickTime(int ms)
{
}
				
void fbWindow::SendMessage(Layer * layer,Widget * widget,Message * msg)
{
}

void fbWindow::DrawMouse()
{
	cairo_set_source_rgba(cairo,0.2,0.2,0.2,0.5);
	cairo_move_to(cairo,mx+2,my+2);
	
	cairo_line_to(cairo,mx+20+2,my+15+2);
	cairo_line_to(cairo,mx+2,my+20+2);
	
	cairo_fill(cairo);
	
	
	cairo_set_source_rgba(cairo,0.9,0.9,0.9,1.0);
	cairo_move_to(cairo,mx,my);
	
	cairo_line_to(cairo,mx+20,my+15);
	cairo_line_to(cairo,mx,my+20);
	
	cairo_fill(cairo);
	
}

int fbWindow::GetTicks()
{
	timeval current_time;
	gettimeofday(&current_time,NULL);
	return (current_time.tv_sec - init_time.tv_sec)*1000 + (current_time.tv_usec - init_time.tv_usec)/1000.0;
}

bool fbWindow::GetCollision(int x,int y,Widget ** widget,Layer ** layer)
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
