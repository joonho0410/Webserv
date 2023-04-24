#include "ParsingUtility.hpp"

void ft_toupper(std::string &key)
{
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = std::toupper(key[i]);
}

bool ft_ishexdigit(std::string &str)
{
    if (str.find_first_not_of("0123456789abcde") != std::string::npos)
        return false;
    return true;
}

bool ft_is_digit(std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if(!isdigit(str[i]))
         return false;
    }
    return true;
}

void exit_with_perror(const std::string& msg)
{
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}