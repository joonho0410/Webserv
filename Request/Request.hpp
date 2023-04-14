#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "../ParsingUtility.hpp"
#include "Structure.hpp"

enum Request_state
{
    READ_START_LINE,
    READ_HEADER,
    READ_BODY,
    READ_BODY_CHUNKED,
    REQUEST_ERROR,
    REQUEST_FINISH
};

enum Requset_error
{
    OK,
    OVER_LENGTH,
    WRONG_PARSING, // 400 BAD REQUEST
    WRONG_BODY
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

        /* setter */
        void setBuf(std::string buf);
        void setState(int);
        void setErrorCode(int);
        void setServerUrl(std::string &);
        void setBody(void) { this->_m_body = "Sample Body";}

        /* getter */
        int getErrorCode();
        int getState();

        std::string getBody();
        std::string getUrl();
        std::string getMethod();
        std::string getBuf();
        std::string getQueryString();
        std::string getServerUrl();
        std::map< std::string, std::vector<std::string> >& getHeader();

    private:
        /* initialize need */
        size_t  _m_bodyMaxSize;
        size_t  _m_chunkedRemain;
        int     _m_state;
        int     _m_errorCode;

        /* initialize with empty */
        std::map< std::string, std::vector<std::string> > _m_header;
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
        void _M_parseValueWithComma(std::string const &_line, std::string key);
        void _M_parseValueWithSlash(std::string const &_line, std::string key);
};