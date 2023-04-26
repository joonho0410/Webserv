#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <unistd.h>
#include <vector>
#include <string>
#include "ParsingUtility.hpp"

enum Response_erorr{
    RESPONSE_OK
};

class Response
{
    private :
        int _m_errorCode;
        int _m_totalSendedBytes;
        bool _m_addhead;
        std::map<int, std::string> _m_statusCodeMap;
        std::map<int, std::string> _m_statusCodeMessageMap;
        std::string _m_statusLine; // ex) HTTP/1.1 200 OK
        std::map<std::string, std::string> _m_header;
        std::string _m_response;
        std::string _m_redirectUrl;
        struct server_config_struct _m_server;

        void _M_initStatusCodeMap(void);
        void _M_initStatusCodeBodyMap(void);
        void ErrorCodeBody(int errorCode);
        void _M_parseAndSetHeader(std::string header);
    public :
        Response();
        ~Response();
        /* Setter */
        void setErrorCode(int errorCode);
        void setStatusLine(int errorCode);
        void setHeader(std::map<std::string, std::string> header);
        void setAddHead(bool b);
        void setRedirectUrl(std::string);
        void setServer(struct server_config_struct&);
        
        
        /* Getter */
        int&    getTotalSendedBytes();
        std::string     getStatusLine();
        std::map<std::string, std::string> getHeader();
        std::string     getResponse();
        std::string     getRedirectUrl();
        int             getErrorCode();

        /* Functions */
        void clean();
        void appendResponse(std::string &);
        void addHeader(std::string headerName, std::string content);
        void addBasicHeader(); //Server, Date, Content-Length
        void setResponseByCgiResult(std::string cgiResult);
        void setResponseByErrorCode();
        std::string    changeRedirectUrl(std::string url);
};