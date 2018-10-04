

#include "utils.hpp"


void net::lliurex::lgi::color::RGB(cairo_t * cairo,uint8_t r,uint8_t g, uint8_t b)
{
	const double f=1.0/255.0;
	cairo_set_source_rgb(cairo, r*f,g*f,b*f);
}

/*!
	expected format:
	RGB as in hex web notation
*/
void net::lliurex::lgi::color::RGB(cairo_t * cairo,uint32_t p)
{
	uint8_t r,g,b;
	const double f=1.0/255.0;
	
	b = 0x000000FF & p; 
	g = (0x0000FF00 & p)>>8; 
	r = (0x00FF0000 & p)>>16; 
	
	cairo_set_source_rgb(cairo, r*f,g*f,b*f);
}

void net::lliurex::lgi::color::RGBA(cairo_t * cairo,uint8_t r,uint8_t g, uint8_t b,uint8_t a)
{
	const double f=1.0/255.0;
	cairo_set_source_rgba(cairo, r*f,g*f,b*f,a*f);

}

void net::lliurex::lgi::color::RGBA(cairo_t * cairo,uint32_t p)
{
	uint8_t r,g,b,a;
	const double f=1.0/255.0;
	
	a = 0x000000FF & p; 
	b = (0x0000FF00 & p)>>8; 
	g = (0x00FF0000 & p)>>16; 
	r = (0xFF000000 & p)>>24; 
	
	cairo_set_source_rgba(cairo, r*f,g*f,b*f,a*f);
}

void net::lliurex::lgi::draw::RoundSquare(cairo_t * cairo,double x,double y,double width,double height,double corner)
{
	double rwidth = width - (corner*2.0);
	double rheight = height -(corner*2.0);
	
	//top-left corner
	cairo_move_to(cairo,x,y+corner);
	cairo_curve_to(cairo,x,y,x,y,x+corner,y);
	
	//top
	cairo_line_to(cairo,x+corner+rwidth,y);
	
	//top-right corner
	cairo_curve_to(cairo,x+width,y,x+width,y,x+width,y+corner);
	
	//right
	cairo_line_to(cairo,x+width,y+corner+rheight);
	
	//bottom-right corner
	cairo_curve_to(cairo,x+width,y+height,x+width,y+height,x+corner+rwidth,y+height);
	
	//bottom
	cairo_line_to(cairo,x+corner,y+height);
	
	//bottom-left corner
	cairo_curve_to(cairo,x,y+height,x,y+height,x,y+corner+rheight);
	
	//left
	cairo_line_to(cairo,x,y+corner);
}
