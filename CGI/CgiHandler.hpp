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
        CgiHandler(Request &request, int infile, int outfile);
        ~CgiHandler(){};

        std::string executeCgi();
    private:
        Request                                             &_m_request;    
        std::map<std::string, std::string>                  _m_env;
        int                                                 _m_inFilefd;
        int                                                 _m_outFilefd;

        void    _M_initEnv(Request &request);
        void    _M_findAndInit(std::string, std::string, std::map< std::string, std::vector< std::string > > &);
        char    **_M_get_envArr() const;
};