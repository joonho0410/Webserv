#include "ServerConfig.hpp"

/* function for assist */
void show_location_block(struct server_config_struct& config)
{
    std::map <std::string, std::vector<std::string> >::iterator key_begin = config.key_and_value.begin();
    std::map <std::string, std::vector<std::string> >::iterator key_end = config.key_and_value.end();
    for (; key_begin != key_end; ++key_begin)
    {
        std::cout << "value : " << key_begin->first << std::endl;
        for (int i = 0; i < key_begin->second.size(); ++i)
        {
            std::cout << (key_begin->second)[i] << std::endl;
        }
    }

    if (!config.location_block.empty())
    {
        std::map <std::string, struct server_config_struct>::iterator begin = config.location_block.begin();
        std::map <std::string, struct server_config_struct>::iterator end = config.location_block.end();
        for (; begin != end; ++ begin)
        {
            std::cout << "location key : " << begin->first << std::endl;
            show_location_block(begin->second);
        }
    }
}

/* public function */
void ServerConfig::set_path(std::string path)
{
    _path = path;
}

std::vector<struct server_config_struct> ServerConfig::get_config() const
{
    return _config_struct;
}

void ServerConfig::show_config()
{
    for(auto it = _config_struct.begin(); it != _config_struct.end(); ++it)
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
            show_location_block(it2->second);
        }
    }
}

void ServerConfig::make_config()
{
    _M_parse_file();
    _M_check_config();
}

/* _M_parse function */
void ServerConfig::_M_parse_file()
{                                         
    std::ifstream                                       readfile;
    std::string                                         line;
    std::string                                         key;
    std::string                                         currentBlock_name;
    std::string                                         save_location_name;
    size_t                                              keyStart;
    size_t                                              keyEnd;
    std::map <std::string, std::vector<std::string> >   temp_key_and_value;
    struct server_config_struct                         serverBlock = { true };
    struct server_config_struct                         locationBlock = { true };
    std::vector <std::pair<std::string, struct server_config_struct> >           vector_loca;
    //open default file when _m_path is not defined;
    
    readfile.open(_path);

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
            //server block 에서 location block이 나왔을 때
            else if (currentBlock_name.compare("server") == 0 && line.find("{") != std::string::npos)
            {
                keyStart = line.find_first_not_of(" \t", 0);
                //std::cout << " KeyStart : " << keyStart << std::endl;
                keyEnd = line.find_first_of(" \t", keyStart);
                //std::cout << " keyEnd : " << keyEnd << std::endl;
                key = line.substr(keyStart, keyEnd - keyStart);
                //std::cout << "key : " << key << std::endl << "current : " << currentBlock_name << std::endl;
                if (key.compare("location") == 0)
                {
                    //location block parsing
                    currentBlock_name = "location";
                    keyStart = line.find_first_not_of(" \t", keyEnd);
                    keyEnd = line.find_first_of(" \t{", keyStart);
                    key = line.substr(keyStart, keyEnd - keyStart);
                    save_location_name = key;
                    locationBlock.block_name = key;
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
            //location block 에서 다른 location block이 나왔을 때
            else if (currentBlock_name.compare("location") == 0 && line.find("{") != std::string::npos)
            {
                vector_loca.push_back(std::make_pair(save_location_name, locationBlock));
                locationBlock.key_and_value.clear();
                locationBlock.location_block.clear();

                keyStart = line.find_first_not_of(" \t", 0);
                //std::cout << " KeyStart : " << keyStart << std::endl;
                keyEnd = line.find_first_of(" \t", keyStart);
                //std::cout << " keyEnd : " << keyEnd << std::endl;
                key = line.substr(keyStart, keyEnd - keyStart);
                //std::cout << "key : " << key << std::endl << "current : " << currentBlock_name << std::endl;
                if (key.compare("location") == 0)
                {
                    //location block parsing
                    keyStart = line.find_first_not_of(" \t", keyEnd);
                    keyEnd = line.find_first_of(" \t{", keyStart);
                    key = line.substr(keyStart, keyEnd - keyStart);
                    save_location_name = key;
                    locationBlock.block_name = key;
                    if (locationBlock.key_and_value.find(key) != locationBlock.key_and_value.end())
                        exit(1);
                    // else // need add on location structure..
                    //     vector_loca.back().second.location_block[key] = locationBlock;
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
                    if(vector_loca.size() != 0)
                    {
                        std::cout << "<<<<<<<<<<<<< double location <<<<<<<<<<<<<" <<std::endl;
                        vector_loca.back().second.location_block[save_location_name] = locationBlock;
                        save_location_name = vector_loca.back().first;
                        locationBlock = vector_loca.back().second;
                        vector_loca.pop_back();
                    }
                    else
                    {
                        serverBlock.location_block[save_location_name] = locationBlock;
                        locationBlock.block_name.clear();
                        locationBlock.key_and_value.clear();
                        locationBlock.location_block.clear();
                        currentBlock_name = "server";
                    }
                    //지금까지 파싱한걸 넣어줘야함.
                }
                else
                {
                    _config_struct.push_back(serverBlock);
                    serverBlock.key_and_value.clear();
                    serverBlock.location_block.clear();
                    locationBlock.key_and_value.clear();
                    locationBlock.location_block.clear();
                    locationBlock.block_name.clear();
                    currentBlock_name.clear(); // 현재 블록 이름 초기화
                }
            }
            // 블록 내용 파싱 및 값 저장.
            else if (!currentBlock_name.empty()) {
                std::cout << "LINE :: " << line << std::endl;
                if (line[line.size() - 1] != ';')
                {
                    std::cout << line.length() - 1 << std::endl;
                    std::cout << line[line.length() - 1] << std::endl;
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
        std::cout << "file is closed " << std::endl;
    }
    else // if file is not available return throw;
    {
        std::cout << "config file error" << std::endl;
        exit(1);
    }
}

void ServerConfig::_M_parse_line(struct server_config_struct &config, std::string &line)
{
    std::string key;
    std::string value;
    size_t      keyStart;
    size_t      keyEnd;
    size_t      lineEnd;
    std::vector<std::string> temp;
    
    keyStart = line.find_first_not_of(" \t", 0);
    keyEnd = line.find_first_of(" \t;", keyStart);
    key = line.substr(keyStart, keyEnd - keyStart);
    while (line.find_first_not_of(" \t", keyEnd) != std::string::npos)
    {
        keyStart = line.find_first_not_of(" \t", keyEnd);
        //std::cout << "keyStart  : " <<  keyStart << std::endl;
        keyEnd = line.find_first_of(" \t;", keyStart);
        //std::cout << "keyEnd  : " <<  keyEnd << std::endl;
        if (keyEnd == keyStart)
            break;
        value = line.substr(keyStart, keyEnd - keyStart);
        //std::cout << value << std::endl;
        if (config.key_and_value.find(key) != config.key_and_value.end())
        {
            std::cout << "already exist key " << std::endl;
            exit(1);
        }
        temp.push_back(value);   
    }
    config.key_and_value[key] = temp;
}

void ServerConfig::_M_parse_KeyValuePairs(std::string const &_input)
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

////////////////////////
/* _M_check_ fucntion */
////////////////////////

// void ServerConfig::_M_check_ip(std::string str)
// {
//     size_t keyStart = 0;
//     size_t keyEnd;
//     std::string s;
//     std::vector<int> v;
//     int     num;

//     /* split with . */
//     keyEnd = str.find_first_of(".", 0);
//     while(keyEnd != std::string::npos)
//     {
//         s = str.substr(keyStart, keyEnd - keyStart);
//         if (!ft_is_digit(s))
//         {
//             std::cout << "ip address is not number" << std::endl;
//             exit(1);
//         }
//         num = atoi(s.c_str());
//         if (s.compare("0") != 0  && num == 0)
//         {
//             std::cout << "config file 'listen ip error' a" << std::endl;
//             exit(1);
//         }
//         v.push_back(num);
//         keyStart += (keyEnd - keyStart + 1);
//         keyEnd = str.find_first_of(".", keyStart);
//     }
    
//     /* ip form is xxx.xxx.xxx.xxx */

//     if(v.size() != 4)
//     {
//         std::cout << "config file 'listen ip error' " << std::endl;
//         exit(1);
//     }

//     /* 0 =< ip.address =< 255 */

//     for (int i = 0 ; i < 4 ; ++i)
//     {
//         if (v[i] > 255 || v[i] < 0)
//         {
//             std::cout << "config file 'listen ip error' " << std::endl;
//             exit(1);
//         }
//     }
// }

void ServerConfig::_M_check_serverName(std::vector<struct server_config_struct >::iterator _temp)
{
    std::cout << "_M_check_serverName" << std::endl;
    std::map <std::string, std::vector<std::string> >::iterator search;
    search = _temp->key_and_value.find("server_name");
    
    /* if listen is not exist set default */ 
    if (search == _temp->key_and_value.end()) {
        std::cout << "server name is not defined " << std::endl;
        exit(1);
    } else if (search->second.size() == 0) {
        std::cout << "server name is empty " << std::endl;
        exit(1);
    }
}

void ServerConfig::_M_check_listen(std::vector<struct server_config_struct >::iterator _temp)
{
     std::cout << "_M_check_listen" << std::endl;
    std::map <std::string, std::vector<std::string> >::iterator search;
    search = _temp->key_and_value.find("listen");
    
    /* if listen is not exist set default */ 
    if (search == _temp->key_and_value.end())
        _temp->key_and_value["listen"].push_back("80");
    else if (search->second.size() == 1) {
        std::string ports;
        ports = search->second[0];
        if(!ft_is_digit(ports)){
            std::cout << "parsing 'listen' error";
        }
        _temp->key_and_value["listen"].push_back(ports);
    }
    else if (search->second.size() == 2 && search->second[1] == "default_server") {
        if (_temp->key_and_value.find("server_name") == _temp->key_and_value.end()){
            std::cout << "server_name error " << std::endl;
            exit(1);
        }
        else if (_temp->key_and_value["server_name"].size() != 1 && _temp->key_and_value["server_name"].front() != "_"){
            std::cout << "listen default server error" << std::endl;
            exit(1);
        }
        /* need set this block as default server at temporary ports */
    }
    else {
        std::cout << "parsing 'listen' error" << std::endl;
        exit(1);
    }
}

void ServerConfig::_M_check_config()
{
    std::vector<struct server_config_struct>::iterator begin = _config_struct.begin();
    std::vector<struct server_config_struct>::iterator end = _config_struct.end();

    for (; begin != end; ++begin)
    {
        std::cout << "_M_check_config" << std::endl;
        _M_check_serverName(begin);
        _M_check_listen(begin);
    }
}