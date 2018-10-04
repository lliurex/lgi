
#ifndef _LGI_X11_WINDOW_
#define _LGI_X11_WINDOW_

/* including window before xlib avoids some name collision*/
#include "window.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <pthread.h>
#include <cairo-xlib.h>
#include <stdint.h>
#include <vector>
#include <queue>


/*! X11Window extended flags */
#define LGI_FLAG_EXT_RENDER_SOFTWARE		0x010000
#define LGI_FLAG_EXT_RENDER_OPENGL		0x020000

#define LGI_MESSAGE_DND_ENTER	0x00010000
#define LGI_MESSAGE_DND_LEAVE	0x00020000
#define LGI_MESSAGE_DND_DROP	0x00030000


namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			
			struct click_t
			{
				int x;
				int y;
				int count;
				long time;
			};

			typedef struct
			{
				unsigned long	flags;
				unsigned long	functions;
				unsigned long	decorations;
				long		inputMode;
				unsigned long	status;
			} wm_hints;
			
			
			class X11Window : public BaseWindow
			{
				private:

				int width;
				int height;
				
				Display * display;
				Window xwindow;
				int screen;
				GC gc;
				timeval init_time;

				/* render method */
				int render_method;
			
				/* OpenGL render */
				XVisualInfo * visual;
				GLuint framebuffer_texture;
				GLXContext glc;
			
				Atom wm_delete_window;
				Atom wm_protocols;
				
				
				/* keyboard mapping */
				KeySym * keymap;
				int keysym_per_code;
				XComposeStatus kcompose_status;
				
				/* x11 render */
				Pixmap pix_buffer;
				XImage* back_buffer;
				
				/* mouse click detection */
				click_t clicks[6];
				
				std::vector<Layer * > layers;
				std::queue<RawEvent *> event_queue;
				pthread_mutex_t queue_mutex;
				
				Widget * dragging;
				
				int double_click_ms;
				
				/* cursor mode, you know, none, x11 or custom one */
				int cursor_x;
				int cursor_y;
				int cursor_mode;
				int cursor_type;
				BaseCursor * custom_cursor;
				
				/* X11 dnd */
				std::vector<std::string>dnd_targets;
				bool dnd_allowed;
				
				/* current focused layer */
				Layer * focus_layer;
				
				
				void Log(std::string msg);
				
				void Resize(int w,int h);
				/* draws widgets bounding box, for debugging purposes */
				void DrawBB();
				
				int ReadProperty(Atom property,unsigned char * data);
				int MapKey(KeySym k);
				std::string GetX11CursorName(int type);
				void Latin1ToUTF8(unsigned char * in,unsigned char * out);
				float GetDist(float x1,float y1,float x2,float y2);
				
				void SetFocus(Layer * layer,Widget * widget);
					
				public:
				uint8_t * framebuffer;
				cairo_t * cairo;
				cairo_surface_t * cairo_surface;
				
				/* Inherited methods */
				
				/*!
					Creates the window with specified size and flags
					\param w width
					\param h height
					\param flags Any combination of window flag constants
				*/
				X11Window(int w,int h,int flags=0);
				
				/*!
					Gets X11 Window xid
				*/
				unsigned long GetXWindow();
				
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
				
				void SetSize(int width,int height);
					
				void AddLayer(Layer * layer);
				void RemoveLayer(Layer * layer);
				void ShowLayer(Layer * layer);
				void HideLayer(Layer * layer);
				
				void SetCursor(int type);
				void SetCursorMode(int mode);
				void SetCustomCursor(BaseCursor * cursor);
				
				void SetDoubleClickTime(int ms);
				
				void SendMessage(Layer * layer,Widget * widget,Message * msg);
					
				int GetTicks();
			
				/* ******* */
				virtual ~X11Window();
				
				cairo_t * GetCairo();
				
				void GetScreenSize(int * width,int * height);
				
				void FullScreen();
				
				bool GetCollision(int x,int y,Widget ** widget,Layer ** layer);
				
				void SetDndTargets(std::vector<std::string> targets);

				Widget * GetFocus();
				void NextFocus(Layer * layer=nullptr);
			
			};
		}
	}
}


#endif
