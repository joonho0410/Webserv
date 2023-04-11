#pragma once
#include <iostream>

#define BAD_REQUEST 413;

enum Response_erorr{
    RESPONSE_OK
};


class Response
{
    private :
        int _m_errorCode;
        std::string _m_response;

    public :
        Response();
        ~Response();
        void clean();
        void apeendResponse(std::string &);
    
        void setErrorCode(int);
        int& getErrorCode();
        std::string& getResponse();
};