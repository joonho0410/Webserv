#pragma once
#include <iostream>
#include <string>
#include <cctype>

void ft_toupper(std::string &key);
bool ft_ishexdigit(std::string &str);
bool ft_is_digit(std::string &str);
void exit_with_perror(const std::string& msg);