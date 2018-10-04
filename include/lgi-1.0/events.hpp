#ifndef _LGI_EVENTS_
#define _LGI_EVENTS_

#include "message.hpp"

#include <string>

/*! Event type constants */
#define LGI_EVENT_NONE				0x00
#define LGI_EVENT_KEY_DOWN			0x01
#define LGI_EVENT_KEY_UP			0x02
#define LGI_EVENT_KEY_PRESS			0x03
#define LGI_EVENT_CHARACTER			0x04
#define LGI_EVENT_MOUSE_DOWN			0x05
#define LGI_EVENT_MOUSE_UP			0x06
#define LGI_EVENT_MOUSE_MOVE			0x07
#define LGI_EVENT_MOUSE_ENTER			0x08
#define LGI_EVENT_MOUSE_EXIT			0x09
#define LGI_EVENT_MOUSE_CLICK			0x0A
#define LGI_EVENT_MOUSE_DOUBLE_CLICK		0x0B
#define LGI_EVENT_DRAG				0x0C
#define LGI_EVENT_DROP				0x0D
#define LGI_EVENT_GOT_FOCUS			0x0E
#define LGI_EVENT_LOST_FOCUS			0x0F

#define LGI_EVENT_EXPOSE			0xA0
#define LGI_EVENT_RESIZE			0xA1
#define LGI_EVENT_DESTROY			0xAF

#define LGI_EVENT_MESSAGE			0xB0

#define LGI_EVENT_LAYER_ADD			0xC0
#define LGI_EVENT_LAYER_REMOVE			0xC1
#define LGI_EVENT_LAYER_SHOW			0xC3
#define LGI_EVENT_LAYER_HIDE			0xC4

#define LGI_EVENT_DND_ENTER			0xD0
#define LGI_EVENT_DND_LEAVE			0xD1
#define LGI_EVENT_DND_DROP			0xD2
#define LGI_EVENT_DND_MOVE			0xD3


namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			
			
			class BaseEvent
			{
				public:
					int type;
				
					virtual ~BaseEvent(){};
			};
			
			/*!
			* Expose Event
			**/
			class ExposeEvent: public BaseEvent
			{
				public:
					ExposeEvent();
					
			};
			
			/**
			 * Window has been resized
			 **/ 
			class ResizeEvent: public BaseEvent
			{
				public:
					ResizeEvent();
					int width;
					int height;
			};
			
			/*!
			 * Window request to be destroyed
			 **/ 
			class DestroyEvent : public BaseEvent
			{
				public:
					DestroyEvent();
				
			};
			
			/*!
			 * Key pressed down
			 **/ 
			class KeyPressEvent: public BaseEvent
			{
				public:
					KeyPressEvent();
					int key;
					
			};

			/*!
			 * Key Up
			 **/ 
			class KeyUpEvent: public BaseEvent
			{
				public:
					KeyUpEvent();
					int key;
			};
			
			/*!
			 * Key Down
			 **/ 
			class KeyDownEvent: public BaseEvent
			{
				public:
					KeyDownEvent();
					int key;
			};
			
			/*!
			 * Character Entered
			 **/ 
			class CharacterEvent: public BaseEvent
			{
				public:
					CharacterEvent();
					unsigned char str[4];
			};
			
			/*!
			 *  Mouse button down
			 **/ 
			class MouseDownEvent: public BaseEvent
			{
				public:
					MouseDownEvent();
					int button;
					int x;
					int y;
			};
			
			/*!
			 *  Mouse button up
			 **/ 
			class MouseUpEvent: public BaseEvent
			{
				public:
					MouseUpEvent();
					int button;
					int x;
					int y;
			};
			
			/*!
			* Mouse Motion 
			**/
			class MouseMoveEvent: public BaseEvent
			{
				public:
					MouseMoveEvent();
					int x;
					int y;
			};
			
			/*!
			* Mouse click
			**/
			class MouseClickEvent: public BaseEvent
			{
				public:
					MouseClickEvent();
					int button;
					int x;
					int y;
			};
			
			/*!
			* Mouse double click
			**/
			class MouseDoubleClickEvent: public BaseEvent
			{
				public:
					MouseDoubleClickEvent();
					int button;
					int x;
					int y;
			};
			
			/*!
			* Mouse enter
			**/
			class MouseEnterEvent: public BaseEvent
			{
				public:
					MouseEnterEvent();
					int x;
					int y;
			};
			
			/*!
			* Mouse exit
			**/
			class MouseExitEvent: public BaseEvent
			{
				public:
					MouseExitEvent();
					int x;
					int y;
			};
			
			/*!
			 * Drag
			 **/ 
			class DragEvent: public BaseEvent
			{
				public:
					DragEvent();
					void * data;
			};
			
			/*!
			 * Drop
			 **/ 
			class DropEvent: public BaseEvent
			{
				public:
					DropEvent();
					void * data;
					int x;
					int y;
					
			};
			
			/*!
			* Message
			**/
			class MessageEvent: public BaseEvent
			{
				public:
					MessageEvent();
					Message * msg;
					~MessageEvent();
			};
			
			/*!
			*	Layer Added to the window
			*/
			class LayerAddEvent: public BaseEvent
			{
				public:
					LayerAddEvent();
					
			};
			
			/*!
			*	Layer Removed to the window
			*/
			class LayerRemoveEvent: public BaseEvent
			{
				public:
					LayerRemoveEvent();
					
			};
			
			/*!
			*	Layer Shows
			*/
			class LayerShowEvent: public BaseEvent
			{
				public:
					LayerShowEvent();
					
			};
			
			/*!
			*	Layer Hides
			*/
			class LayerHideEvent: public BaseEvent
			{
				public:
					LayerHideEvent();
					
			};
			
			/*!
			 *  External drag and drop, enter event
			 */ 
			class DndEnterEvent: public BaseEvent
			{
				public:
					std::string target;
					
					DndEnterEvent();
				
			};
			
			/*!
			 *  External drag and drop, leave event
			 */ 
			class DndLeaveEvent: public BaseEvent
			{
				public:
					DndLeaveEvent();
				
			};
			
			/*!
			 *  External drag and drop, drop event
			 */ 
			class DndDropEvent: public BaseEvent
			{
				public:
					std::string target;
					int size;
					unsigned char * data;
					
					DndDropEvent();
					~DndDropEvent();
				
			};
			
			/*!
			 * External drag and drop move event
			 */
			 class DndMoveEvent: public BaseEvent
			 {
				 public:
					int x;
					int y;
					
					DndMoveEvent();
			 };
			 
			 
			 /*!
			 *	Widget Got the Focus
			 */
			 class GotFocusEvent: public BaseEvent
			 {
			 	public:
			 		GotFocusEvent();
			 		
			 };
			 
			 /*!
			 *	Widget Lost focus 
			 */
			 class LostFocusEvent: public BaseEvent
			 {
			 	public:
			 		LostFocusEvent();
			 };
		}
	}
}

#endif
