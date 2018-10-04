
#include "events.hpp"
#include <iostream>

using namespace net::lliurex::lgi;
using namespace std;

ExposeEvent::ExposeEvent()
{
	type=LGI_EVENT_EXPOSE;
}

ResizeEvent::ResizeEvent()
{
	type=LGI_EVENT_RESIZE;
}

DestroyEvent::DestroyEvent()
{
	type=LGI_EVENT_DESTROY;
}

KeyPressEvent::KeyPressEvent()
{
	type=LGI_EVENT_KEY_PRESS;
}

KeyUpEvent::KeyUpEvent()
{
	type=LGI_EVENT_KEY_UP;
}

KeyDownEvent::KeyDownEvent()
{
	type=LGI_EVENT_KEY_DOWN;
}

CharacterEvent::CharacterEvent()
{
	type=LGI_EVENT_CHARACTER;
}

MouseDownEvent::MouseDownEvent()
{
	type=LGI_EVENT_MOUSE_DOWN;
}

MouseUpEvent::MouseUpEvent()
{
	type=LGI_EVENT_MOUSE_UP;
}

MouseMoveEvent::MouseMoveEvent()
{
	type=LGI_EVENT_MOUSE_MOVE;
}

MouseClickEvent::MouseClickEvent()
{
	type=LGI_EVENT_MOUSE_CLICK;
}

MouseDoubleClickEvent::MouseDoubleClickEvent()
{
	type=LGI_EVENT_MOUSE_DOUBLE_CLICK;
}

MouseEnterEvent::MouseEnterEvent()
{
	type=LGI_EVENT_MOUSE_ENTER;
}

MouseExitEvent::MouseExitEvent()
{
	type=LGI_EVENT_MOUSE_EXIT;
}

DragEvent::DragEvent()
{
	type=LGI_EVENT_DRAG;
}

DropEvent::DropEvent()
{
	type=LGI_EVENT_DROP;
}

MessageEvent::MessageEvent()
{
	type=LGI_EVENT_MESSAGE;
}

MessageEvent::~MessageEvent()
{
	delete msg;
}

LayerAddEvent::LayerAddEvent()
{
	type=LGI_EVENT_LAYER_ADD;
}

LayerRemoveEvent::LayerRemoveEvent()
{
	type=LGI_EVENT_LAYER_REMOVE;
}

LayerShowEvent::LayerShowEvent()
{
	type=LGI_EVENT_LAYER_SHOW;
}

LayerHideEvent::LayerHideEvent()
{
	type=LGI_EVENT_LAYER_HIDE;
}

DndEnterEvent::DndEnterEvent()
{
	type=LGI_EVENT_DND_ENTER;
}

DndLeaveEvent::DndLeaveEvent()
{
	type=LGI_EVENT_DND_LEAVE;
}

DndDropEvent::DndDropEvent()
{
	type=LGI_EVENT_DND_DROP;
}

DndDropEvent::~DndDropEvent()
{
	delete data;
}

DndMoveEvent::DndMoveEvent()
{
	type=LGI_EVENT_DND_MOVE;
}

GotFocusEvent::GotFocusEvent()
{
	type=LGI_EVENT_GOT_FOCUS;
}

LostFocusEvent::LostFocusEvent()
{
	type=LGI_EVENT_LOST_FOCUS;
}
