
#ifndef _LGI_LAYER_
#define _LGI_LAYER_

#include "widget.hpp"
#include "events.hpp"

#include <string>
#include <vector>

#include <cairo-xlib.h>

/* layer constants */
#define LGI_LAYER_DEPTH_TOP		0x00
#define LGI_LAYER_DEPTH_BOTTOM		0xFF

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class Layer
			{
				
				public:
				std::vector<Widget *> widgets;
				std::string name;
				float x;
				float y;
				int depth;
				bool visible;
				Widget * focus;
				
				/*!
					Layer constructor
					\param name layer name
					\param x screen x offset
					\param y screen y offset
					\param depth determines z sorting in the layer stack
				*/
				Layer(std::string name,float x,float y,int depth);
				
				
				Layer(std::string name);
				
				virtual ~Layer();
				
				virtual void SetDepth(int depth);
				virtual void Add(Widget * widget);
				
				virtual void Draw(cairo_t * cairo);
				
				
				/*! Event hooks */
				
				virtual void OnExpose(ExposeEvent * event);
				virtual void OnResize(ResizeEvent * event);
				virtual void OnKeyPress(KeyPressEvent * event);
				virtual void OnKeyUp(KeyUpEvent * event);
				virtual void OnKeyDown(KeyDownEvent * event);
				virtual void OnCharacter(CharacterEvent * event);
				virtual void OnMouseDown(Widget * widget,MouseDownEvent * event);
				virtual void OnMouseUp(Widget * widget,MouseUpEvent * event);
				virtual void OnMouseMove(Widget * widget,MouseMoveEvent * event);
				virtual void OnMouseEnter(Widget * widget,MouseEnterEvent * event);
				virtual void OnMouseExit(Widget * widget,MouseExitEvent * event);
				virtual void OnMouseClick(Widget * widget,MouseClickEvent * event);
				virtual void OnMouseDoubleClick(Widget * widget,MouseDoubleClickEvent * event);
				virtual void OnDrag(Widget * widget,DragEvent * event);
				virtual void OnDrop(Widget * widget,DropEvent * event);
				virtual void OnDestroy(DestroyEvent * event);
				virtual void OnMessage(Widget * widget,MessageEvent * event);
				virtual void OnAdd(LayerAddEvent * event);
				virtual void OnRemove(LayerRemoveEvent * event);
				virtual void OnShow(LayerShowEvent * event);
				virtual void OnHide(LayerHideEvent * event);
				virtual void OnDndEnter(DndEnterEvent * event);
				virtual void OnDndLeave(DndLeaveEvent * event);
				virtual void OnDndDrop(DndDropEvent * event);
				virtual void OnDndMove(DndMoveEvent * event);
				virtual void OnGotFocus(Widget * widget,GotFocusEvent * event);
				virtual void OnLostFocus(Widget * widget,LostFocusEvent * event);
				
				
			};
			
		}
	}
}

#endif
