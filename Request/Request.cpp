#include "Request.hpp"

Request::Request()
{
    _m_bodyMaxSize = 0;
    _m_buf.clear();
    _m_state = READ_START_LINE;
    _m_errorCode = OK;
}

void Request::clean()
{
    _m_bodyMaxSize = 0;
    _m_state = READ_START_LINE;
    _m_errorCode = OK;
    _m_header.clear();
    _m_startLine.clear();
    _m_body.clear();
    _m_method.clear();
    _m_url.clear();
    _m_httpVersion.clear();
    _m_queryString.clear();
    _m_serverUrl.clear();
    _m_buf.clear();
}

bool Request::checkBodySize(struct server_config_struct config)
{
    if (config.key_and_value.find("client_max_body_size") != config.key_and_value.end()){
        size_t client_body_size;
        std::string temp;

        temp = *(config.key_and_value.find("client_max_body_size")->second.begin());
        client_body_size = static_cast<size_t> (std::atol(temp.c_str()));
        if (client_body_size < _m_body.size())
            return false;
    }
    return true;
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

void Request::_M_appendBody(std::string &buf)
{
    try {
        _m_body.append(buf);
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

    while (1)
    {
        if (_m_state == REQUEST_FINISH || _m_state == REQUEST_ERROR)
            break;
        findCRLF = _m_buf.find("\r\n");
        if (findCRLF == std::string::npos)
            break;
        switch (_m_state)
        {
            case READ_START_LINE:
                std::cout << "READ_START_LINE OCCURED" << std::endl;
                _M_parseStartLine(findCRLF);
                break;
            case READ_HEADER:
                std::cout << "READ_HEADER OCCURED" << std::endl;
                _M_parseRequestheader();
                break;
            case READ_BODY:
                std::cout << "READ_BODY OCCURED" << std::endl;
                _M_parseBody();
                break;
            case READ_BODY_CHUNKED:
                _M_parseBodyChunked(findCRLF);
                break;
            case REQUEST_ERROR:
                std::cout << "REQUEST_ERROR OCCURED" << std::endl;
                break;     
        }
    }
}

void Request::_M_parseStartLine(size_t n)
{
    size_t  keyStart = 0;
    size_t  keyEnd = 0;
    size_t  pos;
    size_t  questionMarkCount = 0;

    std::string key;
    std::string temp;
    std::string line;

    line = _m_buf.substr(0, n);
    keyEnd = line.find_first_of(" ", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    if (!((key.compare("GET") == 0) || (key.compare("POST") == 0) || (key.compare("DELETE") == 0 )))
    {
        std::cout << "invalid http method " << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    _m_method = key;
    keyStart = keyEnd + 1;

    keyEnd = line.find_first_of(" ", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    while ((pos = key.find("?", pos)) != std::string::npos){
        ++questionMarkCount;
        if (questionMarkCount > 1)
            break ;
        ++pos;
    }
    if (questionMarkCount > 1){
         _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    else if (questionMarkCount == 1)  
        _M_parseStringQuery(key);
    else
        _m_url = key;
    
    keyStart = keyEnd + 1;
    key = line.substr(keyStart);
    keyStart = key.find_first_of("/", 0);
    if (keyStart == std::string::npos)
    {
        std::cout << "is not http" << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    temp = key.substr(0, keyStart);
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
    _m_httpVersion = temp;

    _m_state = READ_HEADER;
    _m_buf = _m_buf.substr(n + 2);
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
    for(; begin != end; ++begin)
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
        if (valueEnd == std::string::npos)
            break;
        valueStart = valueEnd + 1;
    }
}

void Request::_M_parseStringQuery(std::string &_line)
{
    size_t pos;

    pos = _line.find("?", 0);
    _m_url = _line.substr(0, pos);
    _m_queryString = _line.substr(pos + 1);
}

void Request::_M_parseBody()
{
    try {
        std::string contentLength;
        std::string body;
        size_t      bodyLength;
        int         num;

        contentLength = *(_m_header["CONTENT-LENGTH"].begin());
        if (!ft_is_digit(contentLength)){
            _m_state = REQUEST_ERROR;
            _m_errorCode = WRONG_PARSING;
            return ;
        }

        num = atoi(contentLength.c_str());
        bodyLength = static_cast<size_t> (num);
        body = _m_buf.substr(0, bodyLength);
        _m_body.append(body);
        _m_buf.clear();

        _m_state = REQUEST_FINISH;
    }
    catch (std::length_error &e)
    {
        _m_state = REQUEST_ERROR;   
        _m_errorCode = OVER_LENGTH;
    }
}

void Request::_M_parseBodyChunked(size_t CRLF)
{
    try {        
        size_t      bodyLen;
        size_t      lineStart = 0;
        size_t      lineEnd = 0;
        std::string length;
        std::string chunked_body;
        char *endPtr;
        long len;
        
        if (_m_state == REQUEST_ERROR)
            return ;
        length = _m_buf.substr(0, CRLF);
        if (!ft_ishexdigit(length)){
            _m_state = REQUEST_ERROR;
            _m_errorCode = WRONG_BODY;
            return ;
        }
        long decimalNum = strtol(length.c_str(), &endPtr, 16); // 16진수 문자열을 10진수로 변환
        if (*endPtr != '\0') {
            _m_state = REQUEST_ERROR;
            _m_errorCode = WRONG_BODY;
        }
        bodyLen = static_cast<size_t>(decimalNum);
        lineStart = CRLF + 2;
        if (_m_buf.find("\r\n", lineStart) == std::string::npos || _m_buf.size() < length.size() + bodyLen + 4)
            return ;
        chunked_body = _m_buf.substr(lineStart, bodyLen);
        _M_appendBody(chunked_body);
        _m_buf = _m_buf.substr(0, lineStart + bodyLen);
        if (bodyLen == 0 && chunked_body.empty()){
            _m_state = REQUEST_FINISH;
            return ;
        }
    }
    catch (std::length_error &e)
    {
        _m_state = REQUEST_ERROR;
        _m_errorCode = OVER_LENGTH;
    }
}

void Request::_M_parseKeyValue(std::string const &line)
{
    size_t      keyStart = 0;
    size_t      keyEnd = 0;
    std::string key;
    std::string value;

    if (line.empty()){//empty line 즉 header의 끝이라는걸 알 수 있다.
        if (!_M_checkBodyIsComing())
            _m_state = REQUEST_FINISH;
        return ;
    }
    keyEnd = line.find_first_of(":", keyStart);
    if (keyEnd == std::string::npos){
        _m_state = REQUEST_ERROR;
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    key = line.substr(0, keyEnd);
    if (key.find(" \t") != std::string::npos){
        _m_state = REQUEST_ERROR;
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    ft_toupper(key);
    _M_parseValueWithComma(line.substr(keyEnd + 1), key);
}

bool Request::_M_checkBodyIsComing()
{
    if (_m_header.find("CONTENT-TYPE") != _m_header.end() && 
    _m_header.find("CONTENT-LENGTH") != _m_header.end()){
        _m_state = READ_BODY;
        return true;
    }
    else if (_m_header.find("TRANSEFER-ENCODING") != _m_header.end()){
        _m_state = READ_BODY_CHUNKED;
        return true;
    }
    return false;
}

void Request::_M_parseRequestheader()
{
    size_t      lineStart = 0;
    size_t      lineEnd = 0;
    size_t      keyStart = 0;
    size_t      keyEnd = 0;
    size_t      headerEnd = 0;

    std::string line;
    std::string key;
    std::string value;
    bool        is_emptyline = false;
    
    while (1) {
        if (_m_state != READ_HEADER)
            break;
        lineEnd = _m_buf.find("\r\n", lineStart);
        if (lineEnd == std::string::npos)
            break;
        line = _m_buf.substr(lineStart, lineEnd - lineStart);
        _M_parseKeyValue(line);
        lineStart = lineEnd + 2;
    }
    _m_buf = _m_buf.substr(lineStart);
}

/* getter & setter */
/* setter */
void Request::setErrorCode( int errorCode ){ _m_errorCode = errorCode; }
void Request::setState(int state){ _m_state = state; }
void Request::setBuf(std::string buf){ _m_buf = buf; }
void Request::setServerUrl(std::string &buf){ _m_serverUrl = buf; }

/* getter */
int Request::getState(){ return _m_state; }
int Request::getErrorCode(){ return _m_errorCode; }
std::string Request::getBuf(){ return _m_buf; }
std::string Request::getBody(){ return _m_body; }
std::string Request::getUrl(){ return _m_url; }
std::string Request::getMethod(){ return _m_method;}
std::string Request::getQueryString(){ return _m_queryString; }
std::string Request::getServerUrl(){ return _m_serverUrl; }
std::map< std::string, std::vector<std::string> > &Request::getHeader(){ return _m_header; }