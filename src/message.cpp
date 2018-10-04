
#include "message.hpp"

#include <iostream> 
#include <map>
#include <string>

using namespace std;
using namespace net::lliurex::lgi;


MessageData::MessageData()
{
	type = LGI_MESSAGE_TYPE_NONE;
}

MessageData::~MessageData()
{
	
}

MessageDataInt::MessageDataInt(int value)
{
	type = LGI_MESSAGE_TYPE_INT;
	this->value = value;
}

MessageDataFloat::MessageDataFloat(float value)
{
	type = LGI_MESSAGE_TYPE_FLOAT;
	this->value = value;
}

MessageDataDouble::MessageDataDouble(double value)
{
	type = LGI_MESSAGE_TYPE_DOUBLE;
	this->value = value;
}

MessageDataString::MessageDataString(string value)
{
	type = LGI_MESSAGE_TYPE_STRING;
	this->value = value;
}

MessageDataPointer::MessageDataPointer(void * value)
{
	type = LGI_MESSAGE_TYPE_POINTER;
	this->value = value;
}

MessageDataVector::MessageDataVector(vector<MessageData *> value)
{
	type = LGI_MESSAGE_TYPE_VECTOR;
	this->value = value;
}

Message::Message(int id)
{
	this->id = id;
}

Message::~Message()
{
	map<string,MessageData *>::iterator it;
	
	for(it=data.begin();it!=data.end();it++)
	{
		delete it->second;
	}
	
	data.clear();
}

void Message::Set(string name,MessageData * data)
{
	if(this->data.find(name)!=this->data.end())
	{
		delete this->data[name];
	}
	
	this->data[name]=data;
}

void Message::Set(string name,int value)
{
	Set(name,new MessageDataInt(value));
}

void Message::Set(string name,float value)
{
	Set(name,new MessageDataFloat(value));
}

void Message::Set(string name,double value)
{
	Set(name,new MessageDataDouble(value));
}

void Message::Set(string name,string value)
{
	Set(name,new MessageDataString(value));
}
