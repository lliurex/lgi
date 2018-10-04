
#ifndef _LGI_EXCEPTIONS_
#define _LGI_EXCEPTIONS_

#include <exception>

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class Exception: public std::exception
			{
				public:
					
				std::string msg;
				Exception(std::string msg);
				virtual ~Exception() throw();
				const char* what() const throw();
				
			};
		}
	}
}

#endif