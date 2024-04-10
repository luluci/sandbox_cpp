
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

    util_dwarf::dwarf_analyzer di;
    auto result = di.open(argv[1]);
    if (result) {
        clock_t s, t;
        s = clock();
        di.analyze();
        t = clock();
        printf("%f\n", (double)(t - s) / CLOCKS_PER_SEC);

        di.close();
    }

    return 0;
}