#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <optional>

#include "dwarf_info.hpp"
#include "utility.hpp"

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_block1(Dwarf_Attribute dw_attr, dwarf_info &di) {
    Dwarf_Block *tempb = 0;
    Dwarf_Error error  = nullptr;
    int result;
    result = dwarf_formblock(dw_attr, &tempb, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    // [ length data1 data2 ... ] or [ DWARF expr ]
    Dwarf_Unsigned len   = 1 + ((uint8_t *)tempb->bl_data)[0];
    Dwarf_Unsigned value = 0;
    if (tempb->bl_len == len) {
        // length byte と valueの要素数が一致するとき、block1として解釈
        // dataをlittle endianで結合
        value = utility::concat_le((uint8_t *)tempb->bl_data, 1, tempb->bl_len);
    } else {
        // 一致しないとき、DWARF expression として解釈
        di.dwarf_expr.eval((uint8_t *)tempb->bl_data, tempb->bl_len);
        auto eval = di.dwarf_expr.pop();
        if (!eval) {
            // ありえない
            printf("error: get_DW_FORM_block1 : DWARF expr logic error.");
        }
        value = *eval;
    }

    // https://www.prevanders.net/libdwarfdoc/group__examplediscrlist.html
    dwarf_dealloc(*di.dw_dbg, tempb, DW_DLA_BLOCK);
    return ReturnT(value);
}
//
template <size_t N, typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_block_N(Dwarf_Attribute dw_attr, dwarf_info &di) {
    Dwarf_Block *tempb = 0;
    Dwarf_Error error  = nullptr;
    int result;
    result = dwarf_formblock(dw_attr, &tempb, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    // N = 1: [ length data1 data2 ... ] or [ DWARF expr ]
    // N = 2: [ length1 length2 data1 data2 ... ] or [ DWARF expr ]
    // N = 4: [ length1 length2 length3 length4 data1 data2 ... ] or [ DWARF expr ]
    auto buff_ptr        = (uint8_t *)tempb->bl_data;
    auto buff_len        = tempb->bl_len;
    Dwarf_Unsigned len   = N + utility::concat_le(buff_ptr, 0, N);
    Dwarf_Unsigned value = 0;
    if (buff_len == len) {
        // length byte と valueの要素数が一致するとき、block<N>として解釈
        // dataをlittle endianで結合
        value = utility::concat_le(buff_ptr, N, buff_len);
    } else {
        // 一致しないとき、DWARF expression として解釈
        di.dwarf_expr.eval(buff_ptr, buff_len);
        auto eval = di.dwarf_expr.pop();
        if (!eval) {
            // ありえない
            printf("error: get_DW_FORM_block1 : DWARF expr logic error.");
        }
        value = *eval;
    }

    // https://www.prevanders.net/libdwarfdoc/group__examplediscrlist.html
    dwarf_dealloc(*di.dw_dbg, tempb, DW_DLA_BLOCK);
    return ReturnT(value);
}
//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_udata(Dwarf_Attribute dw_attr) {
    Dwarf_Unsigned data;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_formudata(dw_attr, &data, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    return ReturnT(data);
}
//
struct DW_FORM_ref_result_t
{
    Dwarf_Off return_offset;
    Dwarf_Bool is_info;
};
//
std::optional<DW_FORM_ref_result_t> get_DW_FORM_ref_addr(Dwarf_Attribute dw_attr) {
    DW_FORM_ref_result_t ref_value;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_global_formref_b(dw_attr, &ref_value.return_offset, &ref_value.is_info, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
//
std::optional<DW_FORM_ref_result_t> get_DW_FORM_ref(Dwarf_Attribute dw_attr) {
    DW_FORM_ref_result_t ref_value;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_formref(dw_attr, &ref_value.return_offset, &ref_value.is_info, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
// DW_FORM_sec_offset
std::optional<DW_FORM_ref_result_t> get_DW_FORM_sec_offset(Dwarf_Attribute dw_attr) {
    DW_FORM_ref_result_t ref_value;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_global_formref_b(dw_attr, &ref_value.return_offset, &ref_value.is_info, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
// DW_FORM_exprloc
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_exprloc(Dwarf_Attribute dw_attr, dwarf_info &di) {
    Dwarf_Unsigned return_exprlen = 0;
    Dwarf_Ptr block_ptr           = nullptr;
    Dwarf_Error error             = nullptr;
    int result;
    result = dwarf_formexprloc(dw_attr, &return_exprlen, &block_ptr, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    di.dwarf_expr.eval((uint8_t *)block_ptr, return_exprlen);
    auto eval = di.dwarf_expr.pop();
    if (eval) {
        return ReturnT(*eval);
    }
    return std::nullopt;
}

template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM(Dwarf_Attribute dw_attr, dwarf_info &di) {
    Dwarf_Half form;
    Dwarf_Error error = nullptr;
    int result;
    // form形式を取得
    result = dwarf_whatform(dw_attr, &form, &error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&error);
        return std::nullopt;
    }
    // form
    switch (form) {
        case DW_FORM_ref_addr: {
            auto ret = get_DW_FORM_ref_addr(dw_attr);
            if (ret) {
                // DW_FORM_ref_addrは相対値データとして保持する
                return ReturnT(ret->return_offset);
            }
        } break;
        case DW_FORM_ref1:
        case DW_FORM_ref2:
        case DW_FORM_ref4:
        case DW_FORM_ref8:
        case DW_FORM_ref_udata: {
            auto ret = get_DW_FORM_ref(dw_attr);
            if (ret) {
                T addr = ret->return_offset;
                if (ret->is_info == true) {
                    // .debug_info の先頭からのoffsetを加算する
                    addr += di.cu_info.cu_offset;
                } else {
                    // .debug_types の先頭からのoffsetを加算する
                    // 暫定：debug_typesが出現することがあるか？
                    addr += di.cu_info.cu_offset;
                }
                return ReturnT(addr);
            }
        } break;

        case DW_FORM_indirect:
            break;

        case DW_FORM_sec_offset: {
            auto ret = get_DW_FORM_sec_offset(dw_attr);
            if (ret) {
                T addr = ret->return_offset;
                if (ret->is_info == true) {
                    // .debug_info の先頭からのoffsetを加算する
                    addr += di.cu_info.cu_offset;
                } else {
                    // .debug_types の先頭からのoffsetを加算する
                    // 暫定：debug_typesが出現することがあるか？
                    addr += di.cu_info.cu_offset;
                }
                return ReturnT(addr);
            }
        } break;

        case DW_FORM_block2:
            return get_DW_FORM_block_N<2, T>(dw_attr, di);
        case DW_FORM_block4:
            return get_DW_FORM_block_N<4, T>(dw_attr, di);
        case DW_FORM_block:
            printf("no implemented! : DW_FORM_block\n");
            break;
        case DW_FORM_block1:
            return get_DW_FORM_block_N<1, T>(dw_attr, di);

        case DW_FORM_udata:
        case DW_FORM_data2:
        case DW_FORM_data4:
        case DW_FORM_data8:
        case DW_FORM_data1:
            return get_DW_FORM_udata<T>(dw_attr);

        case DW_FORM_exprloc:
            return get_DW_FORM_exprloc<T>(dw_attr, di);
    }

    return std::nullopt;
}

}  // namespace util_dwarf
