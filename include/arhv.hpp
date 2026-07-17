#pragma once
#include "../include/utils.hpp"
#include <string>
#include <stdint.h>

bool decompress(std::string input, std::string output);
bool compress(std::string input, std::string output, uint8_t mode = 0);