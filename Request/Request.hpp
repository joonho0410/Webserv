#pragma once
#include <iostream>
#include <vector>
#include <map>

#include "../Structure.hpp"

class Request
{
    public:
        Request();
        ~Request(){}

        void show_save();
        void check_body_size(size_t _size);
        
        void appendBuf(std::string &);
        void parseBuf();

        /* setter */
        void setBuf(std::string buf);
        void setCheckHeader(bool);
        void setCheckBody(bool);
        void setState(int);

        /* getter */
        int getState();
        std::string getBody();
        std::string getUrl();
        std::string getMethod();
        std::string getBuf();
        std::string getQueryString();
        std::map< std::string, std::vector<std::string> > getHeader();

    private:
        /* initialize with false */
        int     _m_state;
        int     _m_errorCode;

        /* initialize with empty */
        std::map< std::string, std::vector<std::string> > _m_header;
        std::string _m_startLine;
        std::string _m_body;

        std::string _m_method;
        std::string _m_url;
        std::string _m_httpVersion;
        std::string _m_queryString;

        std::string _m_buf; // 잔여 버프들을 저장해놓는다.
        
        void _M_parseStartLine();
        void _M_parseRequestheader(std::string const &_line);
        void _M_parseBody(std::string const &_line);
        void _M_parseKeyValue(std::string const &_line);

        void _M_parseValueWithComma(std::string const &_line, std::string key);
        void _M_parseValueWithSlash(std::string const &_line, std::string key);
};