#pragma once

#include <dwarf.h>
#include <libdwarf.h>

#include <cstdio>
#include <string>

class dwarf_info {
    std::string dwarf_file_path;
    static constexpr size_t dw_true_path_buff_len = 512;
    char dw_true_path_buff[dw_true_path_buff_len];

    Dwarf_Debug dw_dbg;
    Dwarf_Error dw_error;

    static void err_handler(Dwarf_Error dw_error, Dwarf_Ptr dw_errarg) {
    }

public:
    dwarf_info() : dw_dbg(nullptr) {
    }
    ~dwarf_info() {
        close();
    }

    bool open(char const *dwarf_file_path_cstr) {
        if (dw_dbg != nullptr) {
            printf("dwarf file is already open : %s\n", dwarf_file_path.c_str());
            return false;
        }

        dwarf_file_path = (dwarf_file_path_cstr);

        unsigned int dw_groupnumber;
        Dwarf_Ptr dw_errarg;

        auto result = dwarf_init_path(dwarf_file_path.c_str(), dw_true_path_buff, dw_true_path_buff_len, dw_groupnumber, err_handler, dw_errarg,
                                      &dw_dbg, &dw_error);
        printf("dwarf_init_path : result : %d\n", result);
        if (result != DW_DLV_OK) {
            printf("dwarf_init_path : failed.\n");
            return false;
        }

        return true;
    }

    void analyze() {
        Dwarf_Bool dw_is_info = true;
        Dwarf_Die dw_cu_die;
        Dwarf_Unsigned dw_cu_header_length;
        Dwarf_Half dw_version_stamp;
        Dwarf_Off dw_abbrev_offset;
        Dwarf_Half dw_address_size;
        Dwarf_Half dw_length_size;
        Dwarf_Half dw_extension_size;
        Dwarf_Sig8 dw_type_signature = {0};
        Dwarf_Unsigned dw_typeoffset;
        Dwarf_Unsigned dw_next_cu_header_offset;
        Dwarf_Half dw_header_cu_type;
        Dwarf_Error dw_error;
        int result;
        bool finish = false;

        // CU取得
        while (!finish) {
            result = dwarf_next_cu_header_e(dw_dbg, dw_is_info, &dw_cu_die, &dw_cu_header_length, &dw_version_stamp, &dw_abbrev_offset,
                                            &dw_address_size, &dw_length_size, &dw_extension_size, &dw_type_signature, &dw_typeoffset,
                                            &dw_next_cu_header_offset, &dw_header_cu_type, &dw_error);

            // エラー
            if (result == DW_DLV_ERROR) {
                return;
            }
            //
            if (result == DW_DLV_NO_ENTRY) {
                if (dw_is_info == true) {
                    /*  Done with .debug_info, now check for
                        .debug_types. */
                    dw_is_info = false;
                    continue;
                }
                /*  No more CUs to read! Never found
                    what we were looking for in either
                    .debug_info or .debug_types. */
                return;
            }

            Dwarf_Half tag;
            // Dwarf_Die child;
            // dwarf_child(dw_cu_die, &child, &dw_error);

            dwarf_tag(dw_cu_die, &tag, &dw_error);
            if (tag == DW_TAG_compile_unit) {
                printf("get DW_TAG_compile_unit");
            }

            dwarf_dealloc_die(dw_cu_die);
        }
    }

    bool close() {
        if (dw_dbg == nullptr) {
            return true;
        }

        auto result = dwarf_finish(dw_dbg);
        printf("dwarf_finish : result : %d\n", result);
        return (result == DW_DLV_OK);
    }
};
