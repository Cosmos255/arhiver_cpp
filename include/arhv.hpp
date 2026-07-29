#pragma once
#include "../include/utils.hpp"
#include <string>
#include <stdint.h>

bool inflate(std::string input, std::string output);
bool deflate(std::string input, std::string output, uint8_t mode = 0, int memLevel = 8);