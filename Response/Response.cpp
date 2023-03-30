#include "Response.hpp"

void Response::setErrorCode(int errorCode){ _m_errorCode = errorCode; }

int& Response::getErrorCode(){ return _m_errorCode; }