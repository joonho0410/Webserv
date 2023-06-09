#pragma once
#include <iostream>
#include <sstream>
#include <map>
#include <unistd.h>
#include <vector>
#include <string>
#include "ParsingUtility.hpp"
#include "Structure.hpp"

enum Response_erorr{
    RESPONSE_OK
};

class Response
{
    private :
        int _m_errorPageCode;
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
        std::vector<std::string> _m_cookies;

        void _M_initStatusCodeMap(void);
        void _M_initStatusCodeBodyMap(void);
        void ErrorCodeBody(int errorCode);
        void _M_parseAndSetHeader(std::string header);
    public :
        Response();
        ~Response();
        /* Setter */
        void setErrorPageCode(int);
        void setErrorCode(int errorCode);
        void setStatusLine(int errorCode);
        void setHeader(std::map<std::string, std::string> header);
        void setAddHead(bool b);
        void setRedirectUrl(std::string);
        void setServer(struct server_config_struct&);
        
        
        /* Getter */
        int getErrorPageCode();
        struct server_config_struct&    getServer();
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
        void addCookie(std::string cookieStr);
        void addBasicHeader(); //Server, Date, Content-Length
        void setResponseByCgiResult(std::string cgiResult);
        void setResponseByErrorCode();
        std::string    changeRedirectUrl(std::string url);
};