
#include "application.hpp"

#include <chrono>
#include <thread>

using namespace std;
using namespace net::lliurex::lgi;

Application * Application::instance=nullptr;


Application::Application(std::string name)
{
	this->name=name;
	quit_request=false;
}

Application::~Application()
{

	for(BaseWindow * w : windows)
	{
		delete w;
	}

}

void Application::Set(Application * app)
{
	Application::instance=app;
}


void Application::Quit()
{
	if(Application::instance!=nullptr)
		Application::instance->quit_request=true;
}

Application * Application::Get()
{
	return Application::instance;
}



void Application::AddWindow(BaseWindow * window)
{
	windows.push_back(window);
}

void Application::Run(int mode)
{

	while(Application::quit_request==false)
	{
		
		
		bool idle;
		int dp = 0; //dispatched event count
		int total_dp=0;
		
		for(BaseWindow * w : windows)
		{
			RawEvent * raw_event;
			
			dp=0;
			
			do
			{
				idle=false;
				
				w->GetEvent();
				raw_event=w->PopEvent();
			
				if(raw_event!=nullptr)
				{
				
					//catch Message events
					if(raw_event->event->type==LGI_EVENT_MESSAGE)
					{
						OnMessage(w,raw_event->layer,raw_event->widget,static_cast<MessageEvent *>(raw_event->event));
					}
				
					dp++;
					idle=true;
					w->ProcessEvent(raw_event);
				
					delete raw_event->event;
					delete raw_event;
				}
				
			}while(idle);
			
			if(mode==LGI_DISPATCH_EVENTS_FULL || dp>0)
			{
				w->Flip();
			}
			
			total_dp+=dp;
		}
		
		
		
		total_dp+=CustomEventsDispatch();
		
		
		if(mode==LGI_DISPATCH_EVENTS_WAIT && total_dp==0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(15));
		}
		
		
	}
}


void Application::OnMessage(BaseWindow * window,Layer * layer, Widget * widget,
MessageEvent * event)
{
}

int Application::CustomEventsDispatch()
{
	return 0;
}
