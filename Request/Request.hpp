#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "../ParsingUtility.hpp"
#include "Structure.hpp"

#define OK              200
#define WRONG_PARSING   400
#define WRONG_BODY      400
#define OVER_LENGTH     500

enum Request_state
{
    READ_START_LINE,
    READ_HEADER,
    READ_BODY,
    READ_BODY_CHUNKED,
    REQUEST_ERROR,
    REQUEST_FINISH
};

class Request
{
    public:
        Request();
        ~Request(){}

        void show_save();
        void clean();
        void appendBuf(std::string &);
        void parseBuf();
        bool checkValid();
        bool checkBodySize(struct server_config_struct);
        std::string    changeRedirectUrl(std::string url);

        /* setter */
        void setBuf(std::string buf);
        void setState(int);
        void setErrorCode(int);
        void setIsCgi(bool isCgi);
        void setServerUrl(std::string &);
        void setRedirectUrl(std::string);
        void setBody(void) { this->_m_body = "Sample Body";}

        /* getter */
        int getErrorCode();
        int getState();
        bool getIsCgi();

        std::string getBody();
        std::string getUrl();
        std::string getMethod();
        std::string getBuf();
        std::string getQueryString();
        std::string getServerUrl();
        std::string getRedirectUrl();
        std::string getSessionId();
        std::map< std::string, std::vector<std::string> >& getHeader();

    private:
        /* initialize need */
        std::string _m_redirectUrl;
        size_t  _m_bodyMaxSize;
        size_t  _m_chunkedRemain;
        bool    _m_isChunkedProcess;
        int     _m_state;
        int     _m_errorCode;
        bool    _m_isCgi;

        /* initialize with empty */
        std::map< std::string, std::vector<std::string> > _m_header;
        std::map< std::string, std::string> _m_cookies;
        std::string _m_sessionId;
        std::string _m_startLine;
        std::string _m_body;
        std::string _m_method;
        std::string _m_url;
        std::string _m_serverUrl;
        std::string _m_httpVersion;
        std::string _m_queryString;
        std::string _m_buf; // 잔여 버프들을 저장해놓는다.

        void _M_appendBody(std::string &);

        bool _M_checkBodyIsComing();
        
        void _M_parseBody();
        void _M_parseRequestheader();
        void _M_parseStartLine(size_t n);
        void _M_parseBodyChunked(size_t n);
        void _M_parseStringQuery(std::string &_line);
        void _M_parseKeyValue(std::string const &_line);
        void _M_parseCookies(std::vector<std::string> cookies);
        void _M_parseValueWithComma(std::string const &_line, std::string key);
        void _M_parseValueWithSlash(std::string const &_line, std::string key);
};