#include "KqueueUdata.hpp"

KqueueUdata::KqueueUdata()
{
    _state = WAIT_CONNECT;
    _pipe[0] = -1;
    _pipe[1] = -1;
    _requested_fd = -1;
}

/* getter & setter */
int KqueueUdata::getState(){
    return _state;
}
Response& KqueueUdata::getResponse(){
    return _res;
}
Request& KqueueUdata::getRequest(){
    return _req;
}
int KqueueUdata::getReadPipe(){
    return _pipe[0];
}
int KqueueUdata::getWritePipe(){
    return _pipe[1];
}
int KqueueUdata::getRequestedFd(){
    return _requested_fd;
}

void KqueueUdata::setState(int state){
    _state = state;
}
void KqueueUdata::setResponse(Response res){
    _res = res;
}
void KqueueUdata::setRequest(Request req){
    req = req;
}