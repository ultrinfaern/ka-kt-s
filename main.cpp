#include <iostream>
#include <filesystem>
#include "ar.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << std::endl;
        std::cout << "  ka_kt_s.exe scatter file_name" << std::endl;
        std::cout << "    Unpack save file into folder" << std::endl;
        std::cout << "  ka_kt_s.exe gather folder_name" << std::endl;
        std::cout << "    Pack folder into save file" << std::endl;
    } else{
        std::string p1 = argv[1];
        if (p1 == "gather") {
            std::string p2 = argv[2];
            if (!std::filesystem::is_directory(p2)) {
                std::cout << "ERROR: " << p2 << " is not a folder" << std::endl;
            } else {
                gather(p2);
            }
        } else if (p1 == "scatter") {
            std::string p2 = argv[2];
            if (!std::filesystem::is_regular_file(p2)) {
                std::cout << "ERROR: " << p2 << " is not a file" << std::endl;
            } else {
                scatter(p2);
            }
        } else {
            std::cout << "ERROR: Unknown parameter " << p1 << std::endl;
        }
    }
    return 0;
}
