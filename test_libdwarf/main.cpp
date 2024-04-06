
#include <cstdio>
#include <iostream>
#include <string>

#include "dwarf_info.hpp"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << argv[0] << std::endl;
        printf("Usage: *.exe <dwarf file>\n");
        return 0;
    }

    dwarf_info di;
    auto result = di.open(argv[1]);
    if (result) {
        di.analyze();
        di.close();
    }

    return 0;
}