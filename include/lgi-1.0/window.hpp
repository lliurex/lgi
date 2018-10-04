
#ifndef _LGI_WINDOW_
#define _LGI_WINDOW_

#include "widget.hpp"
#include "events.hpp"
#include "layer.hpp"
#include "message.hpp"
#include "cursor.hpp"

/*! window flag constants */
#define LGI_FLAG_RESIZABLE			0x01
#define LGI_FLAG_NO_BORDER			0x02

/*! event dispatch method */
#define LGI_DISPATCH_EVENTS_FULL		0x01
#define LGI_DISPATCH_EVENTS_WAIT		0x02


/*! Mouse Buttons */
#define LGI_BUTTON_LEFT				0x01
#define LGI_BUTTON_MIDDLE			0x02
#define LGI_BUTTON_RIGHT			0x03
#define LGI_BUTTON_SCROLL_UP			0x04
#define LGI_BUTTON_SCROLL_DOWN 			0x05

/*! Cursor mode */
#define LGI_CURSOR_MODE_NONE			0x00
#define LGI_CURSOR_MODE_SYSTEM			0x01
#define LGI_CURSOR_MODE_CUSTOM			0x02

/*! Mouse Cursors */
#define LGI_CURSOR_DEFAULT			0x00
#define LGI_CURSOR_BUSY				0x01
#define LGI_CURSOR_HALF_BUSY			0x02
#define LGI_CURSOR_OPEN_HAND			0x03
#define LGI_CURSOR_CLOSE_HAND			0x04
#define LGI_CURSOR_HAND				0x05
#define LGI_CURSOR_CROSS			0x06
#define LGI_CURSOR_FORBIDDEN			0x07
#define LGI_CURSOR_QUESTION_ARROW		0x08
#define LGI_CURSOR_FLEUR			0x09

/*! Keys */
#define LGI_KEY_a	0x01
#define LGI_KEY_b	0x02
#define LGI_KEY_c	0x03
#define LGI_KEY_d	0x04
#define LGI_KEY_e	0x05
#define LGI_KEY_f	0x06
#define LGI_KEY_g	0x07
#define LGI_KEY_h	0x08
#define LGI_KEY_i	0x09
#define LGI_KEY_j	0x0a
#define LGI_KEY_k	0x0b
#define LGI_KEY_l	0x0c
#define LGI_KEY_m	0x0d
#define LGI_KEY_n	0x0e
#define LGI_KEY_o	0x0f
#define LGI_KEY_p	0x10
#define LGI_KEY_q	0x11
#define LGI_KEY_r	0x12
#define LGI_KEY_s	0x13
#define LGI_KEY_t	0x14
#define LGI_KEY_u	0x15
#define LGI_KEY_v	0x16
#define LGI_KEY_w	0x17
#define LGI_KEY_x	0x18
#define LGI_KEY_y	0x19
#define LGI_KEY_z	0x1a

#define LGI_KEY_0	0x1b
#define LGI_KEY_1	0x1c
#define LGI_KEY_2	0x1d
#define LGI_KEY_3	0x1e
#define LGI_KEY_4	0x1f
#define LGI_KEY_5	0x20
#define LGI_KEY_6	0x21
#define LGI_KEY_7	0x22
#define LGI_KEY_8	0x23
#define LGI_KEY_9	0x24

#define LGI_KEY_LEFT	0x25
#define LGI_KEY_RIGHT	0x26
#define LGI_KEY_UP	0x27
#define LGI_KEY_DOWN	0x28


#define LGI_KEY_F1	0x29
#define LGI_KEY_F2	0x2a
#define LGI_KEY_F3	0x2b
#define LGI_KEY_F4	0x2c
#define LGI_KEY_F5	0x2d
#define LGI_KEY_F6	0x2e
#define LGI_KEY_F7	0x2f
#define LGI_KEY_F8	0x30
#define LGI_KEY_F9	0x31
#define LGI_KEY_F10	0x32
#define LGI_KEY_F11	0x33
#define LGI_KEY_F12	0x34

#define LGI_KEY_ENTER	0x35
#define LGI_KEY_SPACE	0x36

#define LGI_KEY_PLUS	0x37
#define LGI_KEY_MINUS	0x38

#define LGI_KEY_ESCAPE	0x39

#define LGI_KEY_BACKSPACE 0x3a

#define LGI_KEY_HOME	0x3b
#define LGI_KEY_END	0x3c
#define LGI_KEY_INSERT	0x3d
#define LGI_KEY_DELETE	0x3e

#define LGI_KEY_LEFT_CTRL	0x3f
#define LGI_KEY_RIGHT_CTRL	0x40
#define LGI_KEY_LEFT_SHIFT	0x41
#define LGI_KEY_RIGHT_SHIFT	0x42
#define LGI_KEY_LEFT_SUPER	0x43
#define LGI_KEY_RIGHT_SUPER	0x44
#define LGI_KEY_LEFT_ALT	0x45
#define LGI_KEY_RIGHT_ALT	0x46

#define LGI_KEY_PAGE_UP	0x47
#define LGI_KEY_PAGE_DOWN	0x48

#define LGI_KEY_TAB		0x49

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class RawEvent
			{
				public:
					Widget * widget; /*!< Target widget */
					Layer * layer; /*!< Target layer */
					BaseEvent * event; /*!< Event delivered */
			};
			
			class BaseWindow
			{
				
				public:
				
						
				
				/*!
					Destructor
				*/
				virtual ~BaseWindow() {};
				
				
				/*!
					Destroys the window
					Trying to draw or process events before destroying the
					window may crash the application.
				*/
				virtual void Destroy()=0;
				
				/*!
					Sets windows title
				*/
				virtual void SetTitle(const char * title)=0;
				
				/*!
					Pulls an event from underlaying event stack
					and pushes it into lgi stack. Typically, each
					call pushes one event, but two, three or none
					it is also possible
				*/
				virtual void GetEvent()=0;
				
				/*!
					Dispatch events from stack
					\param mode Dispatch method, valid values are
					 LGI_DISPATCH_EVENTS_FULL and LGI_DISPATCH_EVENTS_WAIT
					 
				*/
				virtual void DispatchEvents(int mode)=0;
				
				/*!
					Push an event into the stack
					\param raw_event Source event, will be freed by DispatchEvents
					This method is thread safe
				*/
				virtual void PushEvent(RawEvent * raw_event)=0;
				
				/*!
					Pops and event from the stack
					This method should be thread safe
				*/
				virtual RawEvent * PopEvent()=0;
				
				/*!
					Process and event, this means, calling those layers
					and widgets hooks
				*/
				virtual void ProcessEvent(RawEvent * raw_event)=0;
				
				/*!
					Renders current frame and clean the screen
					Should be call at each iteration
				*/
				virtual void Flip()=0;
				
				virtual int GetWidth()=0;
				virtual int GetHeight()=0;
				
				
				virtual void SetSize(int width,int height)=0;
				
				
				virtual void AddLayer(Layer * layer)=0;
				virtual void RemoveLayer(Layer * layer)=0;
				virtual void ShowLayer(Layer * layer)=0;
				virtual void HideLayer(Layer * layer)=0;
				
				virtual void SetCursor(int type)=0;
				virtual void SetCursorMode(int mode)=0;
				virtual void SetCustomCursor(BaseCursor * cursor)=0;
				
				virtual void SetDoubleClickTime(int ms)=0;
				
				/*!
					Push a message into event stack
				*/
				virtual void SendMessage(Layer * layer,Widget * widget,Message * msg)=0;
				
				/*!
					Get current focused widget
					or nullptr if none
				*/
				virtual Widget * GetFocus()=0;
				
				/*!
					Cycle focus to the next Widget
					\param layer Cycle focus at given layer
				*/
				virtual void NextFocus(Layer * layer=nullptr)=0;
				
				
			};
		}
	}
}




#endif
