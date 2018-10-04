
#include <iostream>
#include "exceptions.hpp"

using namespace std;
using namespace net::lliurex::lgi;


Exception::Exception(std::string msg)
{
	this->msg=msg;
}

Exception::~Exception() throw()
{
}

const char * Exception::what() const throw()
{
	return msg.c_str();
}