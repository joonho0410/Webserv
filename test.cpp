#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include <string>

#include "./ServerConfig/ServerConfig.hpp"
#include "./ServerEngine/ServerEngine.hpp"
#include "./CGI/CgiHandler.hpp"

int main(int argc, char** argv)
{
    Request request = Request();
    request.setBody();
    CgiHandler handler = CgiHandler(request);
    std::string rv = handler.executeCgi();
    std::cout << "{ " << rv << " }" << std::endl;
    return (0);
}