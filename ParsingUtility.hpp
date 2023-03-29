#pragma once

#include <iostream>
#include <string>
#include <cctype>

void ft_toupper(std::string &key)
{
    for (int i = 0; i < key.size(); ++i)
        key[i] = std::toupper(key[i]);
}