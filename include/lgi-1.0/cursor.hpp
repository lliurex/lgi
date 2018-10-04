
#ifndef _LGI_CURSOR_
#define _LGI_CURSOR_

#include <cairo.h>

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class BaseCursor
			{
				public:
					virtual void SetCursor(int type)=0;
					virtual void Draw(cairo_t * cairo)=0;
				
			};
		}
	}
}

#endif