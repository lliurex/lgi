
#ifndef _LGI_WIDGET_
#define _LGI_WIDGET_

#include "events.hpp"

#include <string>
#include <cairo.h>



namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class Widget
			{
				public:
									
					float x;
					float y;
					float width;
					float height;
				
					bool mouse_over;
					bool mouse_press;
					bool focus;
				
					std::string name;
				
					unsigned int tag;
					
					
					Widget();
					
					virtual void Draw(cairo_t * cairo) = 0;
					
					virtual void OnMouseClick(MouseClickEvent * event);
					virtual void OnMouseDoubleClick(MouseDoubleClickEvent * event);
					virtual void OnMouseMove(MouseMoveEvent * event);
					virtual void OnMouseDown(MouseDownEvent * event);
					virtual void OnMouseUp(MouseUpEvent * event);
					virtual void OnMouseEnter(MouseEnterEvent * event);
					virtual void OnMouseExit(MouseExitEvent * event);
					virtual void OnDrag(DragEvent * event);
					virtual void OnDrop(DropEvent * event);
					virtual void OnMessage(MessageEvent * event);
					virtual void OnGotFocus(GotFocusEvent * event);
					virtual void OnLostFocus(LostFocusEvent * event);
				
			};
		}
	}
}


#endif
