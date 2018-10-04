

#include <lgi-1.0/lgi.hpp>
#include <lgi-1.0/x11window.hpp>

#include <iostream>

using namespace std;
using namespace net::lliurex::lgi;




class Button: public Widget
{
	public:
	
	string text;
	
	Button(string text,float x,float y)
	{
		this->text=text;
		this->x=x;
		this->y=y;
	}
	
	void Draw(cairo_t * cairo)
	{
		cairo_text_extents_t te;
		cairo_font_extents_t fe;
	
		cairo_select_font_face (cairo, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (cairo, 16.0);
		cairo_text_extents (cairo, text.c_str(), &te);
		cairo_font_extents (cairo, &fe);
	
		width=te.width;
		height=(fe.height);
		width+=20.0f; // 10+10 margins 
		height+=(height*0.8f);
	
		
		if(focus)
		{
			cairo_set_source_rgb(cairo, 0.35, 0.73, 1.0);
			cairo_set_line_width (cairo, 2.0);
			draw::RoundSquare(cairo,x-2,y-2,width+4,height+4,10.0);
			
			cairo_stroke(cairo);
		}

		if(mouse_press)
			color::RGB(cairo,0xf7e26b);
		else
		{
			if(mouse_over)
				color::RGB(cairo,0x5db6f4);
			else
				color::RGB(cairo,0x31a2f2);
			
		}
		
		
	
		
		draw::RoundSquare(cairo,x,y,width,height,10.0);
		cairo_fill(cairo);
	
		cairo_set_source_rgb(cairo, 0.2, 0.2, 0.2);
		cairo_set_line_width (cairo, 2.0);
		
		draw::RoundSquare(cairo,x,y,width,height,10.0);
		cairo_stroke(cairo);
	
		cairo_pattern_t *pat;

		pat = cairo_pattern_create_linear (x, y,x , y+height);
		if(mouse_press)
		{
			cairo_pattern_add_color_stop_rgb(pat, 0,0.0f,0.0f,0.0f);
			cairo_pattern_add_color_stop_rgb(pat, 1,0.0f,0.0f,0.0f);
			//cairo_pattern_add_color_stop_rgb(pat, 1,1.0,1.0,1.0f);
		}
		else
		{
			cairo_pattern_add_color_stop_rgb(pat, 0,1.0,1.0,1.0f);
			cairo_pattern_add_color_stop_rgb(pat, 1,0.0f,0.0f,0.0f);
		}
	
		cairo_set_source (cairo, pat);
		cairo_set_line_width (cairo, 1.0);
		
		draw::RoundSquare(cairo,x+1,y+1,width-2,height-2,10.0);
		cairo_stroke(cairo);
		cairo_pattern_destroy (pat);
			
		
		color::RGB(cairo,128,128,128);
	
		if(mouse_press)
			cairo_move_to (cairo, x+((width-te.width)/2.0)+1,y+(height)-((height-fe.height)/2.0)-fe.descent+2);
		else
			cairo_move_to (cairo, x+((width-te.width)/2.0)+1,y+(height)-((height-fe.height)/2.0)-fe.descent+1);
	
		cairo_show_text (cairo, text.c_str());
	
		color::RGB(cairo,250,250,250);
	
		if(mouse_press)
			cairo_move_to (cairo, x+((width-te.width)/2.0),y+(height)-((height-fe.height)/2.0)-fe.descent+1);
		else
			cairo_move_to (cairo, x+((width-te.width)/2.0),y+(height)-((height-fe.height)/2.0)-fe.descent);
	
		cairo_show_text (cairo, text.c_str());
	}
};


class DemoLayer : public Layer
{
	public:
	
	Button * btn1;
	
	DemoLayer() : Layer("demo1")
	{
		btn1= new Button("Accept",50,50);
		Add(btn1);
	}
	
	void OnDestroy(DestroyEvent * event)
	{
		cout<<"Destroy request"<<endl;
		
		Application::Quit();
	}
	
	void OnMouseClick(Widget * w,MouseClickEvent * event)
	{
		if(w==btn1)
		{
			/*
			Message * msg = new Message(32);
			msg->Set("text","Hello message!");
			*/
		}
		
	}
	
	void Draw(cairo_t * cairo)
	{
		color::RGB(cairo,0xDDDDDD);
		cairo_paint(cairo);
		
		Layer::Draw(cairo);
	}
};

int main(int argc,char * argv[])
{

	cout<<"Demo 1"<<endl;
	
	Application * app = new Application("net.lliurex.demo1");

	Application::Set(app);
	
	X11Window * window = new X11Window(800,600,0);
	DemoLayer * demoLayer = new DemoLayer();

	window->AddLayer(demoLayer);
	
	app->AddWindow(window);
	
	app->Run(LGI_DISPATCH_EVENTS_WAIT);
	
	delete app;
	
	return 0;
}
