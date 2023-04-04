#pragma once

#include "./Request/Request.hpp"
#include "./Response/Response.hpp"
#include "Structure.hpp"

class KqueueUdata
{
    private :
        int         _m_requested_fd;  //using at cgi
        int         _m_pipe[2];       //using at cgi
        int         _m_state;
        Request     _m_req;
        Response    _m_res;
        FILE*       _m_inFile;
        FILE*       _m_outFile;

    public :
        KqueueUdata();
        ~KqueueUdata(){}

    public :
        void clean();

        /* getter  & setter */
        int getState();
        FILE* getinFile();
        FILE* getoutFile();
        int getRequestedFd();
        Request& getRequest();
        Response& getResponse();

        void setState(int state);
        void setRequestedFd(int);
        void setRequest(Request req);
        void setResponse(Response res);
        void setinFile(FILE *);
        void setoutFile(FILE *);
};