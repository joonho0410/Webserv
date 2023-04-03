#include "Request.hpp"

Request::Request()
{
    _m_buf.clear();
    _m_state = READ_START_LINE;
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

void Request::appendBuf(std::string &buf)
{
    try {
        _m_buf.append(buf);
    }
    catch (std::length_error &e)
    {
        _m_state = REQUEST_ERROR;   
        _m_errorCode = OVER_LENGTH;
    }
}

void Request::parseBuf()
{
    size_t  findCRLF;

    /* parsing request line */
    std::cout << "====== buf =========" << std::endl;
    std::cout << _m_buf << std::endl;
    while (1)
    {
        if (_m_state == REQUEST_FINISH)
            break;
        findCRLF = _m_buf.find("\r\n");
        if (findCRLF == std::string::npos && _m_state != READ_BODY)
            break;
        switch (_m_state)
        {
            case READ_START_LINE:
                _M_parseStartLine();
                break;
            case READ_KEY:
                break;
            case READ_VALUE:

                break;
            case READ_BODY:
                break;
            case REQUEST_ERROR:
                break;     
        }
    }
}

void Request::_M_parseStartLine()
{
    size_t  keyStart = 0;
    size_t  keyEnd = 0;

    std::string key;
    std::string temp;

    keyStart = _m_buf.find_first_not_of(" \t", keyEnd);
    keyEnd = _m_buf.find_first_of(" \t", keyStart);
    key = _m_buf.substr(keyStart, keyEnd - keyStart);
    if (!((key.compare("GET") == 0) || (key.compare("POST") == 0) || (key.compare("DELETE") == 0 )))
    {
        std::cout << "invalid http method " << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    _m_method = key;
    std::cout << "key : " << key << std::endl;
    keyStart = _m_buf.find_first_not_of(" \t", keyEnd);
    keyEnd = _m_buf.find_first_of(" \t", keyStart);
    key = _m_buf.substr(keyStart, keyEnd - keyStart);

    _m_url = key;
    std::cout << "url : " << key << std::endl;
    keyStart = _m_buf.find_first_not_of(" \t", keyEnd);
    keyEnd = _m_buf.find_first_of(" \t", keyStart);
    key = _m_buf.substr(keyStart, keyEnd - keyStart);
    
    keyStart = key.find_first_of("/", 0);
    temp = key.substr(0, keyStart);
    std::cout << "http : " << temp << std::endl;
    if (temp.compare("HTTP") != 0)
    {
        std::cout << "is not http" << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    temp = key.substr(keyStart + 1);
    std::cout << "version : " << temp << std::endl;
    if (!(temp.compare("1.1") == 0 || temp.compare("1.0") == 0))
    {
        std::cout << "http version error " << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    _m_state = READ_KEY;
    return ;
}

void Request::show_save()
{
    std::cout << "========= show save ===========" <<std::endl;
    std::map< std::string, std::vector<std::string> >::iterator begin = _m_header.begin();
    std::map< std::string, std::vector<std::string> >::iterator end = _m_header.end();
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

void Request::_M_parseValueWithComma(std::string const &_line, std::string _key)
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
        _m_header[_key].push_back(value);
        // std::cout <<  value;
        if (valueEnd == std::string::npos)
            break;
        valueStart = valueEnd + 1;
    }
    // std::cout << std::endl;
}

void Request::_M_parseValueWithSlash(std::string const &_line, std::string _key)
{

}

void Request::_M_parseBody(std::string const &_line)
{
    std::cout << "body line : " << _line << std::endl;
    _m_body = _line;
}

void Request::_M_parseKeyValue(std::string const &_line)
{
    //std::cout << "key value : " << _line << std::endl;
    size_t      keyStart = 0;
    size_t      keyEnd = 0;
    std::string key;
    std::string value;

    keyStart = _line.find_first_not_of(" \t", keyStart);
    keyEnd = _line.find_first_of(":", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);
    _M_parseValueWithComma(_line.substr(keyEnd + 1), key);
}

void Request::_M_parseRequestheader(std::string const &_line)
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
                _M_parseBody(_line.substr(lineStart));
                break ;
            }
            _M_parseKeyValue(_line.substr(lineStart, lineEnd - lineStart));
            lineStart = lineEnd + 1;
        }
    }
}

/* getter & setter */
void Request::setState(int state){ _m_state = state; }
void Request::setBuf(std::string buf){ _m_buf = buf; }
int Request::getState(){ return _m_state; }
std::string Request::getBuf(){ return _m_buf; }
std::string Request::getBody(){ return _m_body; }
std::string Request::getUrl(){ return _m_url; }
std::string Request::getMethod(){ return _m_method;}
std::string Request::getQueryString(){ return _m_queryString};
std::map< std::string, std::vector<std::string> > Request::get_header(){ return _m_header; }