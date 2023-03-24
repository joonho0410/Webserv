#pragma once

#include <iostream>

class Response
{
    private :
        std::string     _m_response;
    public :
        void append_response(std::string str);
        void clear_response();
        Response();
        ~Response();    
};