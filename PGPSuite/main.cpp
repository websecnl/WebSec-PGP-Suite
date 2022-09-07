#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

std::string read_file(const std::string& filename)
{
    std::ifstream file(filename);
    std::stringstream buffer_stream;
    buffer_stream << file.rdbuf();
    return buffer_stream.str();
}

int main()
{
    std::string filename;
    std::getline(std::cin, filename);

    std::cout << read_file(filename);
    
    return 0;
}
