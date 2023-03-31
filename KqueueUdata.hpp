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

    public :
        KqueueUdata();
        ~KqueueUdata(){}

    public :
        void clean();

        /* getter  & setter */
        int getRequestedFd();
        int getState();
        int getReadPipe();
        int getWritePipe();
        Response& getResponse();
        Request& getRequest();

        void setState(int state);
        void setResponse(Response res);
        void setRequest(Request req);
        void setRequestedFd(int);
};