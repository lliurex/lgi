
#ifndef _LGI_MESSAGE_
#define _LGI_MESSAGE_

#include <iostream>
#include <string>
#include <map>
#include <vector>

#define LGI_MESSAGE_TYPE_NONE		0x00
#define LGI_MESSAGE_TYPE_INT		0x01
#define LGI_MESSAGE_TYPE_FLOAT		0x02
#define LGI_MESSAGE_TYPE_DOUBLE		0x03
#define LGI_MESSAGE_TYPE_STRING		0x04
#define LGI_MESSAGE_TYPE_POINTER	0x05
#define LGI_MESSAGE_TYPE_VECTOR		0x06

namespace net
{
	namespace lliurex
	{
		namespace lgi
		{
			class MessageData
			{
				public:
					
					int type;
					MessageData();
					virtual ~MessageData();
					
			};
			
			class MessageDataInt : public MessageData
			{
				public:
					int value;
					MessageDataInt(int value);
				
			};
			
			class MessageDataFloat : public MessageData
			{
				public:
					float value;
					MessageDataFloat(float value);
				
			};
			
			class MessageDataDouble : public MessageData
			{
				public:
					double value;
					MessageDataDouble(double value);
				
			};
			
			class MessageDataString : public MessageData
			{
				public:
					std::string value;
					MessageDataString(std::string value);
				
			};
			
			class MessageDataPointer : public MessageData
			{
				public:
					void * value;
					MessageDataPointer(void * value);
				
			};
			
			class MessageDataVector : public MessageData
			{
				public:
					std::vector<MessageData *>value;
					
					MessageDataVector(std::vector<MessageData *> value);
					
			};
			
			class Message
			{
				public:
					int id;
					std::map<std::string,MessageData *> data;
				
					Message(int id);
					~Message();
					
					void Set(std::string name,MessageData * data);
					void Set(std::string name,int value);
					void Set(std::string name,float value);
					void Set(std::string name,double value);
					void Set(std::string name,std::string value);
					
				
			};
		}
	}
}
	

#endif
