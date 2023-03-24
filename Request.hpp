#pragma once
#include <iostream>

#include <vector>
#include <map>

class Request
{
    public:
        Request(){}
        Request(std::string buf);
        ~Request();
        void set_buf(std::string buf);
        void parse_and_check_valid();
        void show_save();
        std::string get_body();
        std::string get_url();
        std::string get_method();
        std::string get_buf();
        void check_body_size(size_t _size);
        std::map< std::string, std::vector<std::string> > get_map();

    private:
        std::map< std::string, std::vector<std::string> > _m_key_and_value;
        std::string _m_buf;
        std::string _m_body;
        std::string _m_method;
        std::string _m_url;

        void _M_parse_buf();
        void _M_parse_requestline(std::string const &_line);
        void _M_parse_requestheader(std::string const &_line);
        void _M_parse_body(std::string const &_line);
        void _M_parse_key_value_line(std::string const &_line);

        void _M_parse_value_with_comma(std::string const &_line, std::string key);
        void _M_parse_value_with_slash(std::string const &_line, std::string key);
};