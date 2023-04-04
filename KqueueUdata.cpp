#include "KqueueUdata.hpp"

KqueueUdata::KqueueUdata()
{
    _m_state = WAIT_CONNECT;
    _m_requested_fd = -1;
}

void KqueueUdata::clean(){
    _m_state = READ_REQUEST;
    _m_requested_fd = -1;
    _m_req.clean();
    _m_res.clean(); 
}
/* getter & setter */
int KqueueUdata::getState(){
    return _m_state;
}
Response& KqueueUdata::getResponse(){
    return _m_res;
}
Request& KqueueUdata::getRequest(){
    return _m_req;
}
FILE* KqueueUdata::getinFile(){
    return _m_inFile;
}
FILE* KqueueUdata::getoutFile(){
    return _m_outFile;
}
int KqueueUdata::getRequestedFd(){
    return _m_requested_fd;
}

void KqueueUdata::setState(int state){
    _m_state = state;
}
void KqueueUdata::setResponse(Response res){
    _m_res = res;
}
void KqueueUdata::setRequest(Request req){
    _m_req = req;
}
void KqueueUdata::setRequestedFd(int fd) {
    _m_requested_fd = fd;
}
void KqueueUdata::setoutFile(FILE *f){
    _m_outFile = f;
}
void KqueueUdata::setinFile(FILE *f){
    _m_inFile = f;
}