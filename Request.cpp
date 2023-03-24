#include "Request.hpp"

Request::Request(std::string _buf)
{
    _m_buf = _buf;
}

Request::~Request()
{

}

std::string Request::get_buf()
{
    return _m_buf;
}

void Request::set_buf(std::string _buf)
{
    _m_buf = _buf;
}

void Request::parse_and_check_valid()
{
    _M_parse_buf();
}

void Request::check_body_size(size_t _size)
{
    std::cout << "body line " << _m_body << std::endl;
    std::cout << "body size : " << _m_body.size() << std::endl;
    if (_m_body.size() > _size)
    {
        std::cout << "body size error " << std::endl;
        exit(1);
    }
}

void Request::_M_parse_buf()
{
    size_t      lineStart = 0;
    size_t      lineEnd = 0;

    /* parsing request line */
    std::cout << "====== buf =========" << std::endl;
    std::cout << _m_buf << std::endl;
    while (1)
    {
        lineEnd = _m_buf.find_first_of("\n", lineEnd);
        std::cout << lineEnd << std::endl;
        if (lineEnd == std::string::npos)
            break;
        // if (lineEnd != 0)
        //     break;
        if (_m_buf[lineEnd - 1] == '\r')
            break;
        lineEnd += 1;
    }
    _M_parse_requestline(_m_buf.substr(lineStart, lineEnd - lineStart - 1));
    lineStart = lineEnd +1;
    /* parsing request headers */
    /* + parsing body */
    _M_parse_requestheader(_m_buf.substr(lineStart));
}

void Request::_M_parse_requestline(std::string const &_line)
{
    size_t  keyStart = 0;
    size_t  keyEnd = 0;

    std::string key;
    std::string temp;

    keyStart = _line.find_first_not_of(" \t", keyEnd);
    keyEnd = _line.find_first_of(" \t", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);
    if (!((key.compare("GET") == 0) || (key.compare("POST") == 0) || (key.compare("DELETE") == 0 )))
    {
        std::cout << "invalid http method " << std::endl;
        exit(1);
    }
    _m_method = key;
    std::cout << "key : " << key << std::endl;
    keyStart = _line.find_first_not_of(" \t", keyEnd);
    keyEnd = _line.find_first_of(" \t", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);

    _m_url = key;
    std::cout << "url : " << key << std::endl;
    keyStart = _line.find_first_not_of(" \t", keyEnd);
    keyEnd = _line.find_first_of(" \t", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);
    
    keyStart = key.find_first_of("/", 0);
    temp = key.substr(0, keyStart);
    std::cout << "http : " << temp << std::endl;
    if (temp.compare("HTTP") != 0)
    {
        std::cout << "is not http" << std::endl;
        exit(1);
    }
    temp = key.substr(keyStart + 1);
    std::cout << "version : " << temp << std::endl;
    if (!(temp.compare("1.1") == 0 || temp.compare("1.0") == 0))
    {
        std::cout << "http version error " << std::endl;
        exit(1);
    }
}

void Request::show_save()
{
    std::cout << "========= show save ===========" <<std::endl;
    std::map< std::string, std::vector<std::string> >::iterator begin = _m_key_and_value.begin();
    std::map< std::string, std::vector<std::string> >::iterator end = _m_key_and_value.end();
    std::cout << "method : " << _m_method << std::endl;
    std::cout << "_m_url : " << _m_url << std::endl;
    std::cout << "_m_body : " << _m_body << std::endl;
    for(; begin != end; ++ begin)
    {
        std::cout << "key : " << begin->first << std::endl;
        for (int i = 0; i < begin->second.size(); ++i)
        {
            std::cout << (begin->second)[i] ;
        }
        std::cout << std::endl;
    }
}

void Request::_M_parse_value_with_comma(std::string const &_line, std::string _key)
{
    size_t      valueStart = 0;
    size_t      valueEnd = 0;
    std::string value;
    
    // std::cout << "key : " << _key << std::endl;
    // std::cout << "value : " ;
    while (1)
    {
        valueStart = _line.find_first_not_of(" \t", valueStart);
        if (valueStart == std::string::npos)
            break ;
        valueEnd = _line.find_first_of(" ,", valueStart);
        if (valueEnd == std::string::npos)
            value = _line.substr(valueStart);
        else
            value = _line.substr(valueStart, valueEnd - valueStart);
        _m_key_and_value[_key].push_back(value);
        // std::cout <<  value;
        if (valueEnd == std::string::npos)
            break;
        valueStart = valueEnd + 1;
    }
    // std::cout << std::endl;
}

void Request::_M_parse_value_with_slash(std::string const &_line, std::string _key)
{

}

void Request::_M_parse_body(std::string const &_line)
{
    std::cout << "body line : " << _line << std::endl;
    _m_body = _line;
}

void Request::_M_parse_key_value_line(std::string const &_line)
{
    //std::cout << "key value : " << _line << std::endl;
    size_t      keyStart = 0;
    size_t      keyEnd = 0;
    std::string key;
    std::string value;

    keyStart = _line.find_first_not_of(" \t", keyStart);
    keyEnd = _line.find_first_of(":", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);
    _M_parse_value_with_comma(_line.substr(keyEnd + 1), key);
}

void Request::_M_parse_requestheader(std::string const &_line)
{
    std::cout << "========= header ========= \n" << _line << std::endl;
    size_t      lineStart = 0;
    size_t      lineEnd = 0;
    size_t      keyStart = 0;
    size_t      keyEnd = 0;
    std::string key;
    std::string value;
    bool        is_emptyline = false;

    while (lineEnd != std::string::npos && !is_emptyline)
    {
        lineEnd = _line.find_first_of("\n", lineStart);
        // std::cout << "temp line : " << _line.substr(lineStart, lineEnd - lineStart) << std::endl;
        // std::cout << "line Start : " << lineStart << std::endl;
        // std::cout << "line End : " << lineEnd << std::endl;

        if (_line[lineEnd - 1] == '\r')
        {
            if (lineEnd - lineStart == 1)
            {
                // std::cout << "empty line " << std::endl;
                lineStart = lineEnd + 1;
                _M_parse_body(_line.substr(lineStart));
                break ;
            }
            _M_parse_key_value_line(_line.substr(lineStart, lineEnd - lineStart));
            lineStart = lineEnd + 1;
        }
    }
}

std::string Request::get_body(){ return _m_body; }
std::string Request::get_url(){ return _m_url; }
std::string Request::get_method(){ return _m_method;}
std::map< std::string, std::vector<std::string> > Request::get_map(){ return _m_key_and_value; }