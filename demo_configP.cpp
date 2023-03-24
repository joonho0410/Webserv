#include "demo_configP.hpp"

ConfigParser::ConfigParser()
{
    _m_path = "";
}

ConfigParser::~ConfigParser()
{
    
}

void ConfigParser::set_path(std::string _path)
{
    _m_path = _path;
}

void ConfigParser::_M_check_ip(std::string _str)
{
    size_t  keyStart = 0;
    size_t  keyEnd;
    std::string s;
    std::vector<int> v;
    int         num;

    keyEnd = _str.find_first_of(".", 0);
    while(keyEnd != std::string::npos)
    {
        s = _str.substr(keyStart, keyEnd - keyStart);
        num = atoi(s.c_str());
        if (s.compare("0") != 0  && num == 0)
        {
            std::cout << "config file 'listen ip error' a" << std::endl;
            exit(1);
        }
        v.push_back(num);
        keyStart += (keyEnd - keyStart + 1);
        keyEnd = _str.find_first_of(".", keyStart);
    }
    
    if(v.size() != 4)
    {
        std::cout << "config file 'listen ip error' " << std::endl;
        exit(1);
    }

    for (int i = 0 ; i < 4 ; ++i)
    {
        if (v[i] > 255 || v[i] < 0)
        {
            std::cout << "config file 'listen ip error' " << std::endl;
            exit(1);
        }
    }
}
// root 이 여러개 x;
// listen 이 여러개 x;
// listen 이 host:port 인지 port만 인지, host만 인지 default는 port는 80, host는 모든주소 0.0.0.0

void ConfigParser::_M_check_listen(std::vector<struct config_struct_server>::iterator _temp)
{
    std::map <std::string, std::vector<std::string> >::iterator search;
    search = _temp->key_and_value.find("listen");
    if (search == _temp->key_and_value.end())
    {
        _temp->key_and_value["listen"].push_back("0.0.0.0");
        _temp->key_and_value["listen"].push_back("80");
    }
    else if (search->second.size() == 1)
    {
        std::string str;
        str = search->second[0];
        if (str.find_first_of(":", 0) != std::string::npos)
        {
            std::string ip;
            std::string port;
            ip = str.substr(0, str.find_first_of(":",0));
            _M_check_ip(ip);
            if (str.size() - str.find_first_of(":", 0) == 1)
                port = "80";
            else
                port = str.substr(str.find_first_of(":",0) + 1, str.size() - str.find_first_of(":",0));
            _temp->key_and_value["listen"].push_back(ip);
            _temp->key_and_value["listen"].push_back(port);
        }
    }
    else
    {
        std::cout << "parsing 'listen' error \n";
        exit(1);
    }
}

void ConfigParser::_M_check_root(std::vector<struct config_struct_server>::iterator _temp)
{

}

void ConfigParser::_M_check_config()
{
    std::vector<struct config_struct_server>::iterator begin = _m_config.begin();
    std::vector<struct config_struct_server>::iterator end = _m_config.end();

    for (; begin != end; ++begin)
    {
        _M_check_listen(begin);
        _M_check_root(begin);
    }
}

void ConfigParser::show_config()
{
    for(auto it = _m_config.begin(); it != _m_config.end(); ++it)
    {
        std::cout << std::endl;
        std::cout << "<<<<<<<<<<<<< server block >>>>>>>>>>>" << std::endl;
        std::cout << std::endl;
        for (auto it2 = it->key_and_value.begin(); it2 != it->key_and_value.end(); ++it2)
        {
            std::cout << it2->first << " : ";
            for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
                std::cout << *it3 ;
            std::cout << "\n";
        }
        std::cout << std::endl;
        std::cout << " <<<<<<<< location block >>>>>>>>>>>  " << std::endl;
        std::cout << std::endl;
        for(auto it2 = it->location_block.begin(); it2 != it->location_block.end(); ++it2)
        {
            std::cout << "location key : " << it2->first << std::endl;
            for(auto it3 = it2->second.key_and_value.begin(); it3 != it2->second.key_and_value.end(); ++it3)
            {
                std::cout << it3->first << " : " ; 
                for (auto it4 = it3->second.begin(); it4 != it3->second.end(); ++it4)
                    std::cout << *it4 << " " ;
                std::cout << "\n";
            }
        }
    }
}

void ConfigParser::_M_parse_line(struct config_struct_server &_config, std::string &_line)
{
    std::string key;
    std::string value;
    size_t      keyStart;
    size_t      keyEnd;
    size_t      lineEnd;
    std::vector<std::string> temp;
    
    keyStart = _line.find_first_not_of(" \t", 0);
    keyEnd = _line.find_first_of(" \t;", keyStart);
    key = _line.substr(keyStart, keyEnd - keyStart);
    while (_line.find_first_not_of(" \t", keyEnd) != std::string::npos)
    {
        keyStart = _line.find_first_not_of(" \t", keyEnd);
        //std::cout << "keyStart  : " <<  keyStart << std::endl;
        keyEnd = _line.find_first_of(" \t;", keyStart);
        //std::cout << "keyEnd  : " <<  keyEnd << std::endl;
        if (keyEnd == keyStart)
            break;
        value = _line.substr(keyStart, keyEnd - keyStart);
        //std::cout << value << std::endl;
        if (_config.key_and_value.find(key) != _config.key_and_value.end())
        {
            std::cout << "already exist key " << std::endl;
            exit(1);
        }
        temp.push_back(value);   
    }
    _config.key_and_value[key] = temp;
}

void ConfigParser::_M_parse_KeyValuePairs(std::string const &_input)
{
    std::vector<KeyValue> result;
    KeyValue keyvalue;

    size_t pos = 0;
    while ((pos < _input.size()) && (_input.size() != 1)) {
        //std::cout << _input << std::endl;
        // Find the next key-value pair
        size_t keyStart = _input.find_first_not_of(" \t\n\r", pos);
        //std::cout << "keyStart : " << _input[keyStart] << std::endl;
        size_t keyEnd = _input.find_first_of(" \t", keyStart);
        //std::cout << "keyEnd : " << keyEnd << std::endl;
        size_t valueStart = _input.find_first_not_of(" \t\n\r", keyEnd + 1);
        //std::cout << "valueStart : " << _input[valueStart] << std::endl;
        size_t valueEnd = _input.find_first_of(";", valueStart);
        //std::cout << "valueEnd : " << _input[valueEnd] << std::endl;

        if (keyEnd == std::string::npos || valueEnd == std::string::npos) {
            // No more key-value pairs found
            break;
        }

        // Extract the key and value
        std::string key = _input.substr(keyStart, keyEnd - keyStart);
        std::string value = _input.substr(valueStart, valueEnd - valueStart);

        keyvalue.key = key;
        keyvalue.value = value;
        result.push_back(keyvalue);
        std::cout << "key : " << key << ", value : " << value << std::endl;
        // Move to the next line
        pos = valueEnd + 1;
    }
    //return result;
}

std::vector<struct config_struct_server>
ConfigParser::get_config() const
{
    return _m_config;
}

// only server block -> location block 
void ConfigParser::_M_parse_file()
{                                         
    std::ifstream                                       readfile;
    std::string                                         line;
    std::string                                         key;
    std::string                                         currentBlock_name;
    std::string                                         save_location_name;
    size_t                                              keyStart;
    size_t                                              keyEnd;
    std::map <std::string, std::vector<std::string> >   temp_key_and_value;
    struct config_struct_server                         serverBlock;
    struct config_struct_server                         locationBlock;
    //open default file when _m_path is not defined;
    
    if (_m_path == "")
        readfile.open("default.txt");
    else
        readfile.open(_m_path);

    if (readfile.is_open() == true)
    {
        while (std::getline(readfile, line, '\n')) {
            if (line.find_first_not_of(" \t") == std::string::npos)
                continue;
            if (line[line.find_first_not_of(" \t")] == '#')
                continue; 
            // 현재 블록이 없으면서 중괄호가 나오면 블록 시작
            if (currentBlock_name.empty() && line.find("{") != std::string::npos) {
                keyStart = line.find_first_not_of(" \t", 0);
                keyEnd = line.find_first_of(" \t{", keyStart);
                key = line.substr(keyStart, keyEnd - keyStart);
                if (key.compare("server") == 0)
                {
                    currentBlock_name = "server";
                }
                else
                {
                    std::cout << "ConfigFile parsing error empty & {" << std::endl;
                    exit(1);
                }
            }
            else if (!currentBlock_name.empty() && line.find("{") != std::string::npos)
            {
                keyStart = line.find_first_not_of(" \t", 0);
                //std::cout << " KeyStart : " << keyStart << std::endl;
                keyEnd = line.find_first_of(" \t", keyStart);
                //std::cout << " keyEnd : " << keyEnd << std::endl;
                key = line.substr(keyStart, keyEnd - keyStart);
                //std::cout << "key : " << key << std::endl << "current : " << currentBlock_name << std::endl;
                if (key.compare("location") == 0 && currentBlock_name.compare("server") == 0)
                {
                    //location block parsing
                    currentBlock_name = "location";
                    keyStart = line.find_first_not_of(" \t", keyEnd);
                    keyEnd = line.find_first_of(" \t{", keyStart);
                    key = line.substr(keyStart, keyEnd - keyStart);
                    save_location_name = key;
                    if (locationBlock.key_and_value.find(key) != locationBlock.key_and_value.end())
                        exit(1);
                    else // need add on location structure..
                        serverBlock.location_block[key] = locationBlock;
                }
                else
                {
                    std::cout << "ConfigFile parsing error not empty & {" <<std::endl;
                    exit(1);
                }
            }
            // 현재 블록이 있으면서 블록 끝을 나타내는 문자열("}")이 나오면 블록 끝
            else if (!currentBlock_name.empty() && line.find("}") != std::string::npos) {
                if (currentBlock_name.compare("location") == 0)
                {
                    serverBlock.location_block[save_location_name] = locationBlock;
                    locationBlock.key_and_value.clear();
                    currentBlock_name = "server";
                    //지금까지 파싱한걸 넣어줘야함.
                }
                else
                {
                    _m_config.push_back(serverBlock);
                    serverBlock.key_and_value.clear();
                    serverBlock.location_block.clear();
                    currentBlock_name.clear(); // 현재 블록 이름 초기화
                } 
            }
            // 블록 내용 파싱 및 값 저장.
            else if (!currentBlock_name.empty()) {
                if (line[line.size() - 1] != ';')
                {
                    std::cout << "parsing error line does not closed with ;" << std::endl;
                    exit(1);
                }
                if (line.find_first_of(";", 0) != line.size() - 1)
                {
                    std::cout << "line have another ; except end of line" << std::endl;
                    exit(1);
                }
                if (currentBlock_name.compare("location") == 0)
                    _M_parse_line(locationBlock, line);
                else // currentBlock_name is servers
                    _M_parse_line(serverBlock, line);
            }
        }
        readfile.close(); // 파일 닫기
    }
    else // if file is not available return throw;
    {
        std::cout << "config file error" << std::endl;
        exit(1);
    }
}

void ConfigParser::make_config()
{
    _M_parse_file();
    _M_check_config();
}