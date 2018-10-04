

#include "widget.hpp"

using namespace std;
using namespace net::lliurex::lgi;


Widget::Widget()
{
	mouse_over=false;
	mouse_press=false;
	focus=false;
	x=0;
	y=0;
	name=string("Widget");
}

void Widget::OnMouseClick(MouseClickEvent * event)
{	
}

void Widget::OnMouseDoubleClick(MouseDoubleClickEvent * event)
{	
}

void Widget::OnMouseMove(MouseMoveEvent * event)
{	
}

void Widget::OnMouseEnter(MouseEnterEvent * event)
{	
}

void Widget::OnMouseExit(MouseExitEvent * event)
{	
}

void Widget::OnMouseDown(MouseDownEvent * event)
{
}

void Widget::OnMouseUp(MouseUpEvent * event)
{
}

void Widget::OnDrag(DragEvent * event)
{
}

void Widget::OnDrop(DropEvent * event)
{
}

void Widget::OnMessage(MessageEvent * event)
{
}

void Widget::OnGotFocus(GotFocusEvent * event)
{
}

void Widget::OnLostFocus(LostFocusEvent * event)
{
}
