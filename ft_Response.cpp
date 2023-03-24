#include "ft_Response.hpp"

Response::Response(){}
Response::~Response(){} 
void Response::append_response(std::string _str)
{
    _m_response += _str;
}

void Response::clear_response()
{
    _m_response.clear();
}