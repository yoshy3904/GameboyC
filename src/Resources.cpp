#include "Resources.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <iomanip>

std::string imageTo2bpp(const std::string& p_file_path, const std::array<unsigned int, 4>& p_color_palette)
{
    sf::Image image;
    if(!image.loadFromFile(p_file_path))
    {
        throw std::runtime_error("Couldn't open image " + p_file_path);
    }

    std::stringstream stream(p_file_path);
    stream << "; Tiles loaded from \"" << p_file_path << "\"\n\n";

    int rows = image.getSize().y/8;
    int columns = image.getSize().x/8;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            stream << "; Tile_" << i << "_" << j << "\n";

            // Read tile.
            for (int y = 0; y < 8; y++)
            {
                stream << "\tdw `"; 
                for (int x = 0; x < 8; x++)
                {
                    sf::Color pixel = image.getPixel(j * 8 + x, i * 8 + y);
                    if(pixel == (sf::Color)p_color_palette[0])
                    {
                        stream << 0;
                    }
                    else if(pixel == (sf::Color)p_color_palette[1])
                    {
                        stream << 1;
                    }
                    else if(pixel == (sf::Color)p_color_palette[2])
                    {
                        stream << 2;
                    }
                    else if(pixel == (sf::Color)p_color_palette[3])
                    {
                        stream << 3;
                    }
                    else stream << 'f';
                }
                stream << "\n";
            }                
        }
    }
    return stream.str();
}

std::string imageToMap(const std::string& p_file_path, const std::vector<unsigned int>& p_color_palette)
{
    sf::Image image;
    if(!image.loadFromFile(p_file_path))
    {
        throw std::runtime_error("Couldn't open image " + p_file_path);
    }

    std::stringstream stream(p_file_path);
    stream << "; Map loaded from \"" << p_file_path << "\"\n\n";

    for (int y = 0; y < image.getSize().y; y++)
    {
        stream << "db ";   
        for (int x = 0; x < image.getSize().x; x++)
        {
            sf::Color pixel = image.getPixel(x, y);
            bool color_found = false;
            for (int i = 0; i < p_color_palette.size(); i++)
            {
                if((sf::Color)p_color_palette[i] == pixel)
                {
                    stream << "$" << std::hex << i << ", ";
                    color_found = true;
                }
            }
            if(!color_found) stream << "unrecognized, ";
        }
        stream << "\n";
    }
    stream << "; Acceptable colors in the original image are: ";
    for (int i = 0; i < p_color_palette.size(); i++)
    {
        stream << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << p_color_palette[i];
        if(i != p_color_palette.size() - 1) stream << ", ";
        else stream << ".\n";
    }

    return stream.str();
}