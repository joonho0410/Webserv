#include "Request.hpp"
#include <unistd.h>

Request::Request()
{
    _m_bodyMaxSize = 0;
    _m_buf.clear();
    _m_state = READ_START_LINE;
    _m_errorCode = OK;
    _m_chunkedRemain = 0;
    _m_isChunkedProcess = false;
}

void Request::clean()
{
    _m_bodyMaxSize = 0;
    _m_chunkedRemain = 0;
    _m_isChunkedProcess = false;
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
    _m_redirectUrl.clear();
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
        if (_m_state == REQUEST_FINISH || _m_state == REQUEST_ERROR || _m_buf.empty())
            break;
        findCRLF = _m_buf.find("\r\n");
        if ((findCRLF == std::string::npos && !(_m_state == READ_BODY)))
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

std::string    Request::changeRedirectUrl(std::string url)
{
    std::size_t pos;
    std::size_t next_pos;
    std::string var = "";
    std::string changedUrl = "";
    pos = url.find("$");
    if (pos == std::string::npos)
        return url;
    changedUrl = url.substr(0, pos);
    while (1)
    {
        next_pos = url.find("$", pos + 1);
        var = url.substr(pos, next_pos);
        if (var == "$request_uri")
            changedUrl += getUrl();
        if (next_pos == std::string::npos)
            break ;
        pos = next_pos;
    }
    return changedUrl;
}

void Request::_M_parseStartLine(size_t n)
{
    size_t  keyStart = 0;
    size_t  keyEnd = 0;
    size_t  pos = 0;
    size_t  questionMarkCount = 0;

    std::string key;
    std::string temp;
    std::string line;

    line = _m_buf.substr(0, n);
    keyEnd = line.find_first_of(" ", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    if (! ((key.compare("GET") == 0) || (key.compare("POST") == 0) || (key.compare("DELETE") == 0 ) || (key.compare("HEAD") == 0) || (key.compare("Put"))) )
    {
        for (int i = 0; i < key.size(); ++i)
        {
            std::cout << (int)key[i] << std::endl;
        }
        std::cout << key << " is " << key.compare("HEAD") << std::endl;
        std::cout << "invalid http method " << std::endl;
        _m_state = REQUEST_ERROR;   
        _m_errorCode = WRONG_PARSING;
        return ;
    }
    _m_method = key;
    keyStart = keyEnd + 1;

    keyEnd = line.find_first_of(" ", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    // std::cout << "KEY = " << key << std::endl;
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
    // std::cout << "method : " << _m_method << std::endl;
    // std::cout << "_m_url : " << _m_url << std::endl;
    // std::cout << "_m_body : " << _m_body << std::endl;
     std::cout << "_m_body.size() : " << _m_body.size() << std::endl;
    //std::cout << "_m_buf : " << _m_buf << std::endl;
    std::cout << "_m_buf.size() " << _m_buf.size() << std::endl; 
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
        valueEnd = _line.find_first_of(",", valueStart);
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
        size_t      myBodyLength = _m_body.length();
        int         num;

        contentLength = *(_m_header["CONTENT-LENGTH"].begin());
        if (!ft_is_digit(contentLength)){
            _m_state = REQUEST_ERROR;
            _m_errorCode = WRONG_PARSING;
            return ;
        }

        num = atoi(contentLength.c_str());
        bodyLength = static_cast<size_t> (num);
        body = _m_buf.substr(0, bodyLength - myBodyLength);
        _m_body.append(body);
        if (_m_body.size() < bodyLength)
            _m_buf = _m_buf.substr(body.size());
        else{
            _m_buf.clear();
            _m_state = REQUEST_FINISH;
        }
    }
    catch (std::length_error &e)
    {
        _m_state = REQUEST_ERROR;   
        _m_errorCode = OVER_LENGTH;
    }
}

void Request::_M_parseBodyChunked(size_t CRLF)
{
    std::cout << "_M_parseBodyChunked OCCURED " << std::endl;
    try {        
        size_t      bodyLen;
        std::string length;
        std::string chunked_body;
        char *endPtr;
        long len;
        // std::cout << "CRLF : " << CRLF << std::endl;
        // std::cout << "_m_buf : " << _m_buf << std::endl;
        // std::cout << "_m_chunked remain : " << _m_chunkedRemain << std::endl;
        // std::cout << "_m_isChunkedProcess :" << _m_isChunkedProcess << std::endl;
        if (_m_state == REQUEST_ERROR)
            return ;
        if (_m_chunkedRemain != 0 && _m_isChunkedProcess ){//remain some read chunked body
            if (CRLF + 2 >= _m_chunkedRemain){
                chunked_body = _m_buf.substr(0, CRLF);
                _m_buf = _m_buf.substr(CRLF + 2);
                _m_chunkedRemain = 0;
                _m_isChunkedProcess = false;
            } else { // CRLF < _m_chunkedRemain
                chunked_body = _m_buf;
                _m_chunkedRemain -= _m_buf.length();
                _m_buf = _m_buf.substr(0, CRLF + 2);
            }
            _M_appendBody(chunked_body);
            return ;
        } else if (_m_chunkedRemain == 0 && _m_isChunkedProcess) {// end 
            length = _m_buf.substr(0, CRLF);
            if (length.empty()) {
                std::cout << "================================ chunked end ========================" << std::endl;
                std::cout << "chunked size = " << _m_body.size() << std::endl;
                _m_buf = _m_buf.substr(CRLF + 2);
                _m_state = REQUEST_FINISH;
                _m_isChunkedProcess = false;
            } else {
                _m_state = REQUEST_ERROR;
                _m_isChunkedProcess = false;
            }
            return ;
        } else if (!(_m_isChunkedProcess)){ //accept new chunked length; 
            length = _m_buf.substr(0, CRLF);
            if (!ft_ishexdigit(length)){
                std::cout << length;
                std::cout << "is not hex digit " << std::endl;
                _m_state = REQUEST_ERROR;
                _m_errorCode = WRONG_BODY;
                return ;
            }
            std::cout << "length :: " << length << std::endl;
            long decimalNum = strtol(length.c_str(), &endPtr, 16); // 16진수 문자열을 10진수로 변환
            _m_chunkedRemain = static_cast<size_t>(decimalNum);
            std::cout << "_m_ chunked remain :: " << _m_chunkedRemain << std::endl;
            if (*endPtr != '\0') {//    strtol error
                std::cout << "16 to 10 error" << std::endl;
                _m_state = REQUEST_ERROR;
                _m_errorCode = WRONG_BODY;
                return ;
            }
            _m_isChunkedProcess = true;
            _m_buf = _m_buf.substr(CRLF + 2);
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
    std::cout << "_M_checkBodyIsComig" << std::endl;
    if (_m_header.find("CONTENT-TYPE") != _m_header.end() && 
    _m_header.find("CONTENT-LENGTH") != _m_header.end()){
        _m_state = READ_BODY;
        return true;
    } else if (_m_header.find("TRANSFER-ENCODING") != _m_header.end()){
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
    std::cout << "when REQUEST HEADER IS ENDED ::: " << _m_buf.size() << std::endl;
}

/* getter & setter */
/* setter */
void Request::setErrorCode( int errorCode ){ _m_errorCode = errorCode; }
void Request::setState(int state){ _m_state = state; }
void Request::setBuf(std::string buf){ _m_buf = buf; }
void Request::setServerUrl(std::string &buf){ _m_serverUrl = buf; }
void Request::setRedirectUrl(std::string buf){ _m_redirectUrl = buf; }

/* getter */
int Request::getState(){ return _m_state; }
int Request::getErrorCode(){ return _m_errorCode; }
std::string Request::getBuf(){ return _m_buf; }
std::string Request::getBody(){ return _m_body; }
std::string Request::getUrl(){ return _m_url; }
std::string Request::getMethod(){ return _m_method;}
std::string Request::getQueryString(){ return _m_queryString; }
std::string Request::getServerUrl(){ return _m_serverUrl; }
std::string Request::getRedirectUrl() { return _m_redirectUrl; }
std::map< std::string, std::vector<std::string> > &Request::getHeader(){ return _m_header; }