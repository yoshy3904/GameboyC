#pragma once
#include <string>
#include <array>
#include <vector>

std::string imageTo2bpp(const std::string& p_file_path, const std::array<unsigned int, 4>& p_color_palette);
std::string imageToMap(const std::string& p_file_path, const std::vector<unsigned int>& p_color_palette);