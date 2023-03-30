#pragma once
#include <iostream>
class Response
{
    private :
        int _m_errorCode;
        std::string _m_response;

    public :
        void setErrorCode(int);

        int& getErrorCode();
};