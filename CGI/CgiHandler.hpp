#pragma once

#include "../Structure.hpp"
#include "../Request/Request.hpp"
#include <unistd.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define BUF_SIZE 10
class CgiHandler
{
    public:
        CgiHandler();
        CgiHandler(Request &request);
        ~CgiHandler(){};
    private:
        Request                                         &_m_request;    
        std::map< std::string, std::vector<std::string> _m_env;
};