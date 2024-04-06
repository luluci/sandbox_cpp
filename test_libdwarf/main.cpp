
#include <libdwarf.h>

#include <iostream>
#include <cstdio>
#include <string>

void err_handler(Dwarf_Error dw_error, Dwarf_Ptr dw_errarg)
{
}

int main(int argc, char *argv[])
{

    if (argc <= 1)
    {
        std::cout << argv[0] << std::endl;
        printf("Usage: *.exe <dwarf file>\n");
        return 0;
    }

    std::string dwarf_file_path(argv[1]);
    constexpr size_t dw_true_path_buff_len = 512;
    char dw_true_path_buff[dw_true_path_buff_len];
    unsigned int dw_groupnumber;
    Dwarf_Ptr dw_errarg;
    Dwarf_Debug dw_dbg;
    Dwarf_Error dw_error;

    auto result = dwarf_init_path(
        dwarf_file_path.c_str(),
        dw_true_path_buff, dw_true_path_buff_len,
        dw_groupnumber, err_handler, dw_errarg, &dw_dbg, &dw_error
    );
    printf("dwarf_init_path : result : %d\n", result);

    result = dwarf_finish(dw_dbg);
    printf("dwarf_finish : result : %d\n", result);

    return 0;
}