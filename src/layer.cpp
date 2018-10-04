

#include "layer.hpp"

using namespace std;
using namespace net::lliurex::lgi;

/*!
 * Default constructor
 */ 
Layer::Layer(string name,float x,float y,int depth)
{
	this->name=name;
	this->x=x;
	this->y=y;
	this->depth=depth;
	
	visible=true;
	
	focus = NULL;
}

/*!
 * Simplified constructor
 */ 
Layer::Layer(string name)
{
	this->name=name;
	this->x=0.0f;
	this->y=0.0f;
	this->depth=0;
	
	visible=true;
	
	focus=NULL;
}


Layer::~Layer()
{
	for(int n=0;n<widgets.size();n++)
	{
		delete widgets[n];
	}
	
	widgets.clear();
}

void Layer::SetDepth(int depth)
{
	/* gods of saturated arithmethics, I request your mighty powers */
	if(depth<LGI_LAYER_DEPTH_TOP)depth=LGI_LAYER_DEPTH_TOP;
	
	if(depth>LGI_LAYER_DEPTH_BOTTOM)depth=LGI_LAYER_DEPTH_BOTTOM;
	
	this->depth=depth;
}


void Layer::Add(Widget * widget)
{
	widgets.push_back(widget);
}


void Layer::Draw(cairo_t * cairo)
{
	for(int n=0;n<widgets.size();n++)
	{	
		cairo_save(cairo);
		cairo_translate(cairo,x,y);	
			widgets[n]->Draw(cairo);
		cairo_restore(cairo);	
		
	}
}


/* Event hooks */

void Layer::OnExpose(ExposeEvent * event)
{
}

void Layer::OnDestroy(DestroyEvent * event)
{
}

void Layer::OnDrag(Widget * widget,DragEvent * event)
{
}

void Layer::OnDrop(Widget * widget,DropEvent * event)
{
}

void Layer::OnKeyPress(KeyPressEvent * event)
{
}

void Layer::OnKeyUp(KeyUpEvent * event)
{
}

void Layer::OnKeyDown(KeyDownEvent * event)
{
}

void Layer::OnCharacter(CharacterEvent * event)
{
}

void Layer::OnMouseClick(Widget * widget,MouseClickEvent * event)
{
}

void Layer::OnMouseDoubleClick(Widget * widget,MouseDoubleClickEvent * event)
{
}

void Layer::OnMouseDown(Widget * widget,MouseDownEvent * event)
{
}

void Layer::OnMouseEnter(Widget * widget,MouseEnterEvent * event)
{
}

void Layer::OnMouseExit(Widget * widget,MouseExitEvent * event)
{
}

void Layer::OnMouseMove(Widget * widget,MouseMoveEvent * event)
{
}

void Layer::OnMouseUp(Widget * widget,MouseUpEvent * event)
{
}

void Layer::OnResize(ResizeEvent * event)
{
}

void Layer::OnMessage(Widget * widget,MessageEvent * event)
{
}

void Layer::OnAdd(LayerAddEvent * event)
{
}

void Layer::OnRemove(LayerRemoveEvent * event)
{
}

void Layer::OnShow(LayerShowEvent * event)
{
}

void Layer::OnHide(LayerHideEvent * event)
{
}

void Layer::OnDndEnter(DndEnterEvent * event)
{
}

void Layer::OnDndLeave(DndLeaveEvent * event)
{
}

void Layer::OnDndDrop(DndDropEvent * event)
{
}

void Layer::OnDndMove(DndMoveEvent * event)
{
}

void Layer::OnGotFocus(Widget * widget,GotFocusEvent * event)
{
}

void Layer::OnLostFocus(Widget * widget,LostFocusEvent * event)
{
}
