
#ifndef _LGI_FB_WINDOW_
#define _LGI_FB_WINDOW_

#include "window.hpp"

#include <linux/fb.h>
#include <unistd.h>
#include <stdint.h>
#include <cairo.h>

#include <vector>
#include <queue>

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class fbWindow: public BaseWindow
			{
				private:
				
				int width;
				int height;
				
				int fb_fd;
				struct fb_var_screeninfo vinfo;
				struct fb_fix_screeninfo finfo;
				
				int framesize;
				uint8_t * framebuffer;
				uint8_t * backbuffer;
				
				std::vector<Layer * > layers;
				std::queue<RawEvent *> event_queue;
				
				int mouse_fd;
				int mx,my;
				int buttons[6];
				Widget * dragging;
				
				timeval init_time;
				
				
				void DrawMouse();
				
				
				
				
				public:
				
				
				
				cairo_t * cairo;
				cairo_surface_t * cairo_surface;
				
				fbWindow(int width,int height,const char * devname,int flags);
				virtual ~fbWindow();
				
				/* inherited methods */
				
				void Resize(int w,int h);
								
				void Destroy();
				void SetTitle(const char * title);
				
				void GetEvent();
				void DispatchEvents(int mode);
				void PushEvent(RawEvent * raw_event);
				RawEvent * PopEvent();
				void ProcessEvent(RawEvent * raw_event);
				
				void Flip();
				
				int GetWidth();
				int GetHeight();
							
				void AddLayer(Layer * layer);
				void RemoveLayer(Layer * layer);
				void ShowLayer(Layer * layer);
				void HideLayer(Layer * layer);
				
				void SetCursor(int type);
				void SetCursorMode(int mode);
				void SetCustomCursor(BaseCursor * cursor);
				
				void SetDoubleClickTime(int ms);
				
				void SendMessage(Layer * layer,Widget * widget,Message * msg);
				
				/* custom functs */
				int GetTicks();
				bool GetCollision(int x,int y,Widget ** widget,Layer ** layer);
			}; 
		}
	}
}


#endif
