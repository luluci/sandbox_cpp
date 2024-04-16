#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include "dwarf_expression.hpp"
#include "dwarf_info.hpp"

namespace util_dwarf {

struct dwarf_analyze_option
{
    using type = uint32_t;

    enum mode : type
    {
        none,
        func_info_analyze = 1 << 0,
        no_impl_warning   = 1 << 1,
    };

    bool is_func_info_analyze;
    bool is_no_impl_warning;

    dwarf_analyze_option(type flags = none) : is_func_info_analyze(false), is_no_impl_warning(false) {
        set(flags);
    }

    void set(type flags) {
        set_impl(flags, true);
    }
    void unset(type flags) {
        set_impl(flags, false);
    }

private:
    void set_impl(type flags, bool value) {
        if (check_flag(flags, func_info_analyze)) {
            is_func_info_analyze = value;
        }
        if (check_flag(flags, no_impl_warning)) {
            is_no_impl_warning = value;
        }
    }

    bool check_flag(type flags, mode flag) {
        return ((flags & flag) == flag);
    }
};

// dwarf解析用情報
// libdwarf APIデータ
// その他
struct dwarf_analyze_info
{
    Dwarf_Debug dw_dbg;
    Dwarf_Error dw_error;
    Dwarf_Attribute dw_attr;
    dwarf_expression dw_expr;
    dwarf_info::compile_unit_info cu_info;
    dwarf_analyze_option option;

    dwarf_analyze_info() : dw_dbg(nullptr), dw_error(nullptr), dw_attr(nullptr), dw_expr(), cu_info(), option() {
    }
};

}  // namespace util_dwarf
