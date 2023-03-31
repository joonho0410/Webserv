#include "Response.hpp"

Response::Response(){
    _m_errorCode = RESPONSE_OK;
}
Response::~Response(){
    
}
void Response::setErrorCode(int errorCode){ _m_errorCode = errorCode; }

int& Response::getErrorCode(){ return _m_errorCode; }

void Response::apeendResponse(std::string &str){
    _m_response += str;
}

std::string& Response::getResponse(){
    return _m_response;
}

void Response::clean(){
    _m_errorCode = RESPONSE_OK;
    _m_response.clear();
}