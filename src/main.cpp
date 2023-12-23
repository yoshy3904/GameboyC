#include "Translator.hpp"

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
