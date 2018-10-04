
#ifndef _LGI_APPLICATION_
#define _LGI_APPLICATION_

#include "window.hpp"

#include <vector>

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class Application
			{
			
				
				
				
				protected:
				
				static Application * instance;
				std::string name;
				std::vector<BaseWindow *> windows;
				bool quit_request;
				
				
				/*!
					Message event hook
					\param window Target Window
					\param layer Target Layer, can be nullptr if the message is broadcasted to all layers
					\param widget Target Widget, can be nullptr if the message is either broadcasted to all layers or to all layer widgets
					\param event Message event
				*/
				virtual void OnMessage(BaseWindow * window,
				Layer * layer, Widget * widget,
				MessageEvent * event);
				
				/*!
					Dispatch custom events, ex: Gtk
					This method is call at each loop iteration in
					order to allow antoher ui toolkits to share the 
					same event loop.
					
					It should return the number of processed events.
				*/
				virtual int CustomEventsDispatch();
				
				public:
				
				/*!
					Constructor
					\param name Application name, ie: com.domain.application
				*/
				Application(std::string name);
				
				/*!
					Destructor
				*/
				virtual ~Application();
				
				
				/*!
					Sets the Application instance. This method should be call before calling Run
					\param app Pointer to an application instance
				*/
				static void Set(Application * app);
				
				/*!
					Exits from the event loop, causes Run
					to stop
				*/
				static void Quit();
				
				/*!
					Gets the application instance, if exists,
					else it returns nullptr
				*/
				static Application * Get();
				
								
				/*!
					Adds a window to the event loop
					\param window Adds a window to the event loop
				*/
				void AddWindow(BaseWindow * window);
				
				
				/*!
					Run the event loop. This is a blocking
					method
					\param mode Dispatch method, valid values are LGI_DISPATCH_EVENTS_FULL and LGI_DISPATCH_EVENTS_WAIT
				*/
				void Run(int mode=LGI_DISPATCH_EVENTS_FULL);
				
				
				
			};
		}
	}
}

#endif

