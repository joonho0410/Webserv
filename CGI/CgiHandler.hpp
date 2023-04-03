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

        std::string executeCgi();
    private:
        Request                                             &_m_request;    
        std::map<std::string, std::string>                  _m_env;

        void    _M_initEnv(Request &request);
        char    **_M_get_envArr() const;
};