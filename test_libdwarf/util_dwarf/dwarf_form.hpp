#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <optional>

#include "LEB128.hpp"
#include "dwarf_analyze_info.hpp"
#include "utility.hpp"

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_block(dwarf_analyze_info &info) {
    // blockデータ取得
    Dwarf_Block *tempb = 0;
    int result;
    result = dwarf_formblock(info.dw_attr, &tempb, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    // [ULEB128データ] + [ULEB128で示されたデータ数]
    auto buff_ptr = static_cast<uint8_t *>(tempb->bl_data);
    auto buff_len = tempb->bl_len;
    // ULEB128を取得
    ULEB128 uleb(buff_ptr, buff_len);
    // blockを取得
    Dwarf_Unsigned value = 0;
    if (buff_len == (uleb.used_bytes + uleb.value)) {
        // ULEB128が示すデータ長がblockサイズと同じならそのまま取り出す
        // dataをlittle endianで結合
        value = utility::concat_le<Dwarf_Unsigned>(buff_ptr, uleb.used_bytes, buff_len);
    } else {
        // 一致しないとき、DWARF expression として解釈
        info.dw_expr.eval(buff_ptr, buff_len);
        auto eval = info.dw_expr.pop<T>();
        if (!eval) {
            // ありえない
            fprintf(stderr, "error: get_DW_FORM_block : DWARF expr logic error.");
        }
        value = *eval;
    }

    // https://www.prevanders.net/libdwarfdoc/group__examplediscrlist.html
    dwarf_dealloc(info.dw_dbg, tempb, DW_DLA_BLOCK);
    return ReturnT(value);
}
//
template <size_t N, typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_block_N(dwarf_analyze_info &info) {
    Dwarf_Block *tempb = 0;
    int result;
    result = dwarf_formblock(info.dw_attr, &tempb, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    // N = 1: [ length data1 data2 ... ] or [ DWARF expr ]
    // N = 2: [ length1 length2 data1 data2 ... ] or [ DWARF expr ]
    // N = 4: [ length1 length2 length3 length4 data1 data2 ... ] or [ DWARF expr ]
    auto buff_ptr        = static_cast<uint8_t *>(tempb->bl_data);
    auto buff_len        = tempb->bl_len;
    Dwarf_Unsigned len   = N + utility::concat_le<Dwarf_Unsigned>(buff_ptr, 0, N);
    Dwarf_Unsigned value = 0;
    if (buff_len == len) {
        // length byte と valueの要素数が一致するとき、block<N>として解釈
        // dataをlittle endianで結合
        value = utility::concat_le<Dwarf_Unsigned>(buff_ptr, N, buff_len);
    } else {
        // 一致しないとき、DWARF expression として解釈
        info.dw_expr.eval(buff_ptr, buff_len);
        auto eval = info.dw_expr.pop<T>();
        if (!eval) {
            // ありえない
            fprintf(stderr, "error: get_DW_FORM_block_N : DWARF expr logic error.");
        }
        value = *eval;
    }

    // https://www.prevanders.net/libdwarfdoc/group__examplediscrlist.html
    dwarf_dealloc(info.dw_dbg, tempb, DW_DLA_BLOCK);
    return ReturnT(value);
}
//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_udata(dwarf_analyze_info &info) {
    Dwarf_Unsigned data;
    int result;
    result = dwarf_formudata(info.dw_attr, &data, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
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
std::optional<DW_FORM_ref_result_t> get_DW_FORM_ref_addr(dwarf_analyze_info &info) {
    DW_FORM_ref_result_t ref_value;
    int result;
    result = dwarf_global_formref_b(info.dw_attr, &ref_value.return_offset, &ref_value.is_info, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
//
std::optional<DW_FORM_ref_result_t> get_DW_FORM_ref(dwarf_analyze_info &info) {
    DW_FORM_ref_result_t ref_value;
    int result;
    result = dwarf_formref(info.dw_attr, &ref_value.return_offset, &ref_value.is_info, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
// DW_FORM_sec_offset
std::optional<DW_FORM_ref_result_t> get_DW_FORM_sec_offset(dwarf_analyze_info &info) {
    DW_FORM_ref_result_t ref_value;
    int result;
    result = dwarf_global_formref_b(info.dw_attr, &ref_value.return_offset, &ref_value.is_info, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    return std::optional<DW_FORM_ref_result_t>(ref_value);
}
// DW_FORM_exprloc
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_exprloc(dwarf_analyze_info &info) {
    Dwarf_Unsigned return_exprlen = 0;
    Dwarf_Ptr block_ptr           = nullptr;
    int result;
    result = dwarf_formexprloc(info.dw_attr, &return_exprlen, &block_ptr, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    info.dw_expr.eval(static_cast<uint8_t *>(block_ptr), return_exprlen);
    auto eval = info.dw_expr.pop<T>();
    if (eval) {
        return ReturnT(*eval);
    }
    return std::nullopt;
}

template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM(dwarf_analyze_info &info) {
    Dwarf_Half form;
    int result;
    // form形式を取得
    result = dwarf_whatform(info.dw_attr, &form, &info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&info.dw_error);
        return std::nullopt;
    }
    // form
    switch (form) {
        case DW_FORM_ref_addr: {
            auto ret = get_DW_FORM_ref_addr(info);
            if (ret) {
                // DW_FORM_ref_addrは他のCUの.debug_info のheader offset
                // つまりどういうこと？
                return ReturnT(ret->return_offset);
            }
        } break;
        case DW_FORM_ref1:
        case DW_FORM_ref2:
        case DW_FORM_ref4:
        case DW_FORM_ref8:
        case DW_FORM_ref_udata: {
            auto ret = get_DW_FORM_ref(info);
            if (ret) {
                T addr = ret->return_offset;
                if (ret->is_info == true) {
                    // .debug_info のheader offsetを加算する
                    addr += info.cu_info.cu_header_offset;
                } else {
                    // .debug_types の先頭からのoffsetを加算する
                    // 暫定：debug_typesが出現することがあるか？
                    addr += info.cu_info.cu_offset;
                }
                return ReturnT(addr);
            }
        } break;

        case DW_FORM_indirect:
            break;

        case DW_FORM_sec_offset: {
            auto ret = get_DW_FORM_sec_offset(info);
            if (ret) {
                // DW_AT_* に該当するセクションの先頭からのoffset
                // ここでは何も加算しない
                T addr = ret->return_offset;
                if (ret->is_info == true) {
                    // .debug_info の先頭からのoffsetを加算する
                    // addr += info.cu_info.cu_header_offset;
                } else {
                    // .debug_types の先頭からのoffsetを加算する
                    // 暫定：debug_typesが出現することがあるか？
                    // addr += info.cu_info.cu_offset;
                }
                return ReturnT(addr);
            }
        } break;

        case DW_FORM_block2:
            return get_DW_FORM_block_N<2, T>(info);
        case DW_FORM_block4:
            return get_DW_FORM_block_N<4, T>(info);
        case DW_FORM_block:
            return get_DW_FORM_block<T>(info);
        case DW_FORM_block1:
            return get_DW_FORM_block_N<1, T>(info);

        case DW_FORM_udata:
        case DW_FORM_data2:
        case DW_FORM_data4:
        case DW_FORM_data8:
        case DW_FORM_data1:
            return get_DW_FORM_udata<T>(info);

        case DW_FORM_exprloc:
            return get_DW_FORM_exprloc<T>(info);

        default:
            break;
    }

    return std::nullopt;
}

}  // namespace util_dwarf
