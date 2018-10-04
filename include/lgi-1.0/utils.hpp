#ifndef _LGI_UTILS_
#define _LGI_UTILS_

#include <cairo.h>
#include <cstdint>

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			namespace color
			{
				void RGB(cairo_t * cairo,uint8_t r,uint8_t g, uint8_t b);
				void RGB(cairo_t * cairo,uint32_t p);
			
				void RGBA(cairo_t * cairo,uint8_t r,uint8_t g, uint8_t b,uint8_t a);
				void RGBA(cairo_t * cairo,uint32_t p);
			}
			
			namespace draw
			{
				void RoundSquare(cairo_t * cairo,double x,double y,double width,double height,double corner);
			}
		}
	}
}


#endif
