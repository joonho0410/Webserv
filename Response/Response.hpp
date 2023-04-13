#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

enum Response_erorr{
    RESPONSE_OK
};

class Response
{
    private :
        int _m_errorCode;
        std::map<int, std::string> _m_statusCodeMap;
        std::map<int, std::string> _m_statusCodeMessageMap;
        std::string _m_statusLine; // ex) HTTP/1.1 200 OK
        std::map<std::string, std::string> _m_header;
        std::string _m_response;

        void _M_initStatusCodeMap(void);
        void _M_initStatusCodeBodyMap(void);
        void setResponseByErrorCode(int errorCode);
        void ErrorCodeBody(int errorCode);
        void _M_parseAndSetHeader(std::string header);
    public :
        Response();
        ~Response();
        /* Setter */
        void setErrorCode(int errorCode);
        void setStatusLine(int errorCode);
        void setHeader(std::map<std::string, std::string> header);
        /* Getter */
        std::string     getStatusLine();
        std::map<std::string, std::string> getHeader();
        std::string     getResponse();

        /* Functions */
        void clean();
        void appendResponse(std::string &);
        void addHeader(std::string headerName, std::string content);
        void addBasicHeader(); //Server, Date, Content-Length
        void setResponseByCgiResult(std::string cgiResult);
        void setResponseByErrorCode(int errorCode);
};