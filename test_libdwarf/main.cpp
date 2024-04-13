
#include <cstdio>
#include <iostream>
#include <string>

#include "util_dwarf/debug_info.hpp"
#include "util_dwarf/dwarf_analyzer.hpp"
#include "util_dwarf/dwarf_info.hpp"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << argv[0] << std::endl;
        printf("Usage: *.exe <dwarf file>\n");
        return 0;
    }

    util_dwarf::dwarf_analyzer di;
    di.set_analyze_func_info(false);
    auto result = di.open(argv[1]);
    if (result) {
        util_dwarf::dwarf_info dw_info;

        clock_t s, t;
        s = clock();
        // dwarf解析
        di.analyze(dw_info);
        t = clock();
        printf("%f\n", (double)(t - s) / CLOCKS_PER_SEC);

        //
        auto debug_info = util_dwarf::debug_info(dw_info);
        debug_info.build();

        di.close();
    }

    return 0;
}
