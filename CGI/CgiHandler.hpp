#pragma once

#include "../Structure.hpp"
#include "../Request/Request.hpp"
class CgiHandler
{
    public:
        CgiHandler();
        CgiHandler(Request &request);
        ~CgiHandler(){};
    private:
        Request                                         _m_request;    
        std::map< std::string, std::vector<std::string> _m_env;
};