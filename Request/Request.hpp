#pragma once
#include <iostream>
#include <vector>
#include <map>

#include "../Structure.hpp"

class Request
{
    public:
        Request(){}
        Request(std::string buf);
        ~Request(){}

        void parse_and_check_valid();
        void show_save();
        void check_body_size(size_t _size);

        /* setter */
        void set_buf(std::string buf);
        void setCheckHeader(bool);
        void setCheckBody(bool);

        /* getter */
        bool getCheckHeader();
        bool getCheckBody();
        std::string get_body();
        std::string get_url();
        std::string get_method();
        std::string get_buf();
        std::map< std::string, std::vector<std::string> > get_header();

    private:
        /* initialize with false */
        bool    _check_header;
        bool    _check_body;
        int     _state;

        /* initialize with empty */
        std::map< std::string, std::vector<std::string> > _header;
        std::string _request_line;
        std::string _body;

        std::string _method;
        std::string _url;
        std::string _http_version;
        std::string _query_string;

        std::string _buf; // 잔여 버프들을 저장해놓는다.
        
        void _M_parse_buf();
        void _M_parse_requestline(std::string const &_line);
        void _M_parse_requestheader(std::string const &_line);
        void _M_parse_body(std::string const &_line);
        void _M_parse_key_value_line(std::string const &_line);

        void _M_parse_value_with_comma(std::string const &_line, std::string key);
        void _M_parse_value_with_slash(std::string const &_line, std::string key);
};