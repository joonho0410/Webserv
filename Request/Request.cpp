#include "Request.hpp"

Request::Request()
{
    _m_bodyMaxSize = 0;
    _m_buf.clear();
    _m_state = READ_START_LINE;
    _m_errorCode = OK;
}

void Request::_M_checkBodySize(size_t _size)
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
                _m_buf = _m_buf.substr(findCRLF + 2);
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

    std::string key;
    std::string temp;
    std::string line;

    line = _m_buf.substr(0, n);
    keyStart = line.find_first_not_of(" \t", keyEnd);
    keyEnd = line.find_first_of(" \t", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    if (!((key.compare("GET") == 0) || (key.compare("POST") == 0) || (key.compare("DELETE") == 0 )))
    {
        std::cout << "invalid http method " << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    _m_method = key;
    std::cout << "key : " << key << std::endl;
    keyStart = line.find_first_not_of(" \t", keyEnd);
    keyEnd = line.find_first_of(" \t", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);

    _m_url = key;
    std::cout << "url : " << key << std::endl;
    keyStart = line.find_first_not_of(" \t", keyEnd);
    keyEnd = line.find_first_of(" \t", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    
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
    _m_state = READ_HEADER;
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

void Request::_M_parseValueWithSlash(std::string const &_line, std::string _key)
{

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

        if (_m_bodyMaxSize != 0 && _m_body.size() > _m_bodyMaxSize)
        {
            _m_state = REQUEST_ERROR;
            _m_errorCode = OVER_LENGTH;
            return ;
        }
        show_save();
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
        if (_m_buf.find("\r\n", lineStart) == std::string::npos)
            return ;
        if (_m_buf.size() < length.size() + 2 + bodyLen + 2)
            return ;
        chunked_body = _m_buf.substr(lineStart, bodyLen);
        _M_appendBody(chunked_body);
        _m_buf = _m_buf.substr(lineStart + bodyLen + 2);
        if (bodyLen == 0 && chunked_body.empty()){
            _m_state = REQUEST_FINISH;
            return ;
        }
        if (_m_bodyMaxSize != 0 && _m_body.size() > _m_bodyMaxSize){
            _m_state = REQUEST_ERROR;
            _m_errorCode = OVER_LENGTH;
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
    keyStart = line.find_first_not_of(" \t", keyStart);
    keyEnd = line.find_first_of(":", keyStart);
    if (keyEnd == std::string::npos){
        _m_state = REQUEST_ERROR;
        return ;
    }
    key = line.substr(keyStart, keyEnd - keyStart);
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

/* getter */
int Request::getState(){ return _m_state; }
int Request::getErrorCode(){ return _m_errorCode; }
std::string Request::getBuf(){ return _m_buf; }
std::string Request::getBody(){ return _m_body; }
std::string Request::getUrl(){ return _m_url; }
std::string Request::getMethod(){ return _m_method;}
std::map< std::string, std::vector<std::string> > Request::getHeader(){ return _m_header; }