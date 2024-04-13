#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include "dwarf_expression.hpp"
#include "dwarf_info.hpp"

namespace util_dwarf {

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

    dwarf_analyze_info() : dw_dbg(nullptr), dw_error(nullptr), dw_attr(nullptr), dw_expr(), cu_info() {
    }
};

}  // namespace util_dwarf
