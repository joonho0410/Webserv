#pragma once
#include "./Request/Request.hpp"
#include "./Response/Response.hpp"
#include "Structure.hpp"

class KqueueUdata
{
    private :
        int         _requested_fd;  //using at cgi
        int         _pipe[2];       //using at cgi
        int         _state;
        Request     _req;
        Response    _res;

    public :
        KqueueUdata();
        ~KqueueUdata(){}

    public :
        /* getter  & setter */
        int getState();
        int getReadPipe();
        int getWritePipe();
        int getRequestedFd();
        Response& getResponse();
        Request& getRequest();

        void setState(int state);
        void setResponse(Response res);
        void setRequest(Request req);
};