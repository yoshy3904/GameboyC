#include "Translator.hpp"

// if, if-else, while statements DONE
// arithmetic and boolean expressions DONE
// binary, hex, dec lesen DONE
// interrupt functions DONE
// function mit return und parameter DONE
// input
// display tiles and sprites
// kleine operatoren (++, --, +=, -=, *=, /=)
// Kommentare
// 0x0b wird als Binärzahl erkannt

int main(int argc, char** argv)
{
    std::string file_path;
    // Get file path.
    if(argc == 2)
    {
        file_path = std::string(argv[1]);
    }
    else
    {
        std::cout << "One argument containing file path required!" << std::endl;
        return 0;
    }

    try
    {
        Translator translator(file_path);
        translator.translate();
    }
    catch(const std::exception& e)
    {
        std::cout << "Caught exception: " << e.what() << '\n';
    }
}