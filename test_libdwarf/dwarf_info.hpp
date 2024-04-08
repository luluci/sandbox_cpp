#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

namespace {
// APIエラーが発生したときにどうするか？
// 暫定で例外を出して終了する
// 戻り値がOKじゃないけどエラーでもないケースある？
void error_happen(Dwarf_Error *error) {
    char *errmsg = dwarf_errmsg(*error);
    throw std::runtime_error("libdwarf API error!");
    exit(1);  // 一応書いておく
}
}  // namespace

// Dwarf expression 計算機
class dwarf_expression {
    using stack_t = std::vector<Dwarf_Unsigned>;
    stack_t stack_;
    static constexpr size_t default_stack_size = 10;

public:
    dwarf_expression() : stack_(default_stack_size) {
    }

    bool eval_DW_OP_addr(uint8_t *buff, size_t buff_size) {
        Dwarf_Unsigned value = 0;
        // little endianで結合
        // [buff_size]~[1]
        // [0]はopeコードなので除外
        for (size_t i = buff_size - 1; i > 0; i--) {
            value <<= 8;
            value |= buff[buff_size];
        }
        //
        stack_.push_back(value);
        //
        return true;
    }

    std::optional<Dwarf_Unsigned> pop() {
        if (stack_.size() > 0) {
            // スタックから値をpop
            Dwarf_Unsigned result = stack_.back();
            stack_.pop_back();
            return std::optional<Dwarf_Unsigned>(result);
        }
        return std::nullopt;
    }

    bool eval(uint8_t *buff, size_t buff_size) {
        // バッファなしはエラーとする
        if (buff_size == 0) {
            return false;
        }
        // opeコード取り出し
        // 1バイト目にあるものとする
        uint8_t ope = buff[0];
        // opeコード操作
        // すべてstackに対するアクションになるはず
        switch (ope) {
            case DW_OP_addr:
                return eval_DW_OP_addr(buff, buff_size);
            case DW_OP_lit0:
                break;
            case DW_OP_lit1:
                break;
            case DW_OP_lit2:
                break;
            case DW_OP_lit3:
                break;
            case DW_OP_lit4:
                break;
            case DW_OP_lit5:
                break;
            case DW_OP_lit6:
                break;
            case DW_OP_lit7:
                break;
            case DW_OP_lit8:
                break;
            case DW_OP_lit9:
                break;
            case DW_OP_lit10:
                break;
            case DW_OP_lit11:
                break;
            case DW_OP_lit12:
                break;
            case DW_OP_lit13:
                break;
            case DW_OP_lit14:
                break;
            case DW_OP_lit15:
                break;
            case DW_OP_lit16:
                break;
            case DW_OP_lit17:
                break;
            case DW_OP_lit18:
                break;
            case DW_OP_lit19:
                break;
            case DW_OP_lit20:
                break;
            case DW_OP_lit21:
                break;
            case DW_OP_lit22:
                break;
            case DW_OP_lit23:
                break;
            case DW_OP_lit24:
                break;
            case DW_OP_lit25:
                break;
            case DW_OP_lit26:
                break;
            case DW_OP_lit27:
                break;
            case DW_OP_lit28:
                break;
            case DW_OP_lit29:
                break;
            case DW_OP_lit30:
                break;
            case DW_OP_lit31:
                break;
            case DW_OP_const1u:
                break;
            case DW_OP_const1s:
                break;
            case DW_OP_const2u:
                break;
            case DW_OP_const2s:
                break;
            case DW_OP_const4u:
                break;
            case DW_OP_const4s:
                break;
            case DW_OP_const8u:
                break;
            case DW_OP_const8s:
                break;
            case DW_OP_constu:
                break;
            case DW_OP_consts:
                break;
            case DW_OP_fbreg:
                break;
            case DW_OP_breg0:
                break;
            case DW_OP_breg1:
                break;
            case DW_OP_breg2:
                break;
            case DW_OP_breg3:
                break;
            case DW_OP_breg4:
                break;
            case DW_OP_breg5:
                break;
            case DW_OP_breg6:
                break;
            case DW_OP_breg7:
                break;
            case DW_OP_breg8:
                break;
            case DW_OP_breg9:
                break;
            case DW_OP_breg10:
                break;
            case DW_OP_breg11:
                break;
            case DW_OP_breg12:
                break;
            case DW_OP_breg13:
                break;
            case DW_OP_breg14:
                break;
            case DW_OP_breg15:
                break;
            case DW_OP_breg16:
                break;
            case DW_OP_breg17:
                break;
            case DW_OP_breg18:
                break;
            case DW_OP_breg19:
                break;
            case DW_OP_breg20:
                break;
            case DW_OP_breg21:
                break;
            case DW_OP_breg22:
                break;
            case DW_OP_breg23:
                break;
            case DW_OP_breg24:
                break;
            case DW_OP_breg25:
                break;
            case DW_OP_breg26:
                break;
            case DW_OP_breg27:
                break;
            case DW_OP_breg28:
                break;
            case DW_OP_breg29:
                break;
            case DW_OP_breg30:
                break;
            case DW_OP_breg31:
                break;
            case DW_OP_bregx:
                break;
            case DW_OP_dup:
                break;
            case DW_OP_drop:
                break;
            case DW_OP_pick:
                break;
            case DW_OP_over:
                break;
            case DW_OP_swap:
                break;
            case DW_OP_rot:
                break;
            case DW_OP_deref:
                break;
            case DW_OP_deref_size:
                break;
            case DW_OP_xderef:
                break;
            case DW_OP_xderef_size:
                break;
            case DW_OP_push_object_address:
                break;
            case DW_OP_form_tls_address:
                break;
            case DW_OP_call_frame_cfa:
                break;
            case DW_OP_abs:
                break;
            case DW_OP_and:
                break;
            case DW_OP_div:
                break;
            case DW_OP_minus:
                break;
            case DW_OP_mod:
                break;
            case DW_OP_mul:
                break;
            case DW_OP_neg:
                break;
            case DW_OP_not:
                break;
            case DW_OP_or:
                break;
            case DW_OP_plus:
                break;
            case DW_OP_plus_uconst:
                break;
            case DW_OP_shl:
                break;
            case DW_OP_shr:
                break;
            case DW_OP_shra:
                break;
            case DW_OP_xor:
                break;
            case DW_OP_eq:
                break;
            case DW_OP_ge:
                break;
            case DW_OP_gt:
                break;
            case DW_OP_le:
                break;
            case DW_OP_lt:
                break;
            case DW_OP_ne:
                break;
            case DW_OP_skip:
                break;
            case DW_OP_bra:
                break;
            case DW_OP_call2:
                break;
            case DW_OP_call4:
                break;
            case DW_OP_call_ref:
                break;
            case DW_OP_nop:
                break;
            case DW_OP_reg0:
                break;
            case DW_OP_reg1:
                break;
            case DW_OP_reg2:
                break;
            case DW_OP_reg3:
                break;
            case DW_OP_reg4:
                break;
            case DW_OP_reg5:
                break;
            case DW_OP_reg6:
                break;
            case DW_OP_reg7:
                break;
            case DW_OP_reg8:
                break;
            case DW_OP_reg9:
                break;
            case DW_OP_reg10:
                break;
            case DW_OP_reg11:
                break;
            case DW_OP_reg12:
                break;
            case DW_OP_reg13:
                break;
            case DW_OP_reg14:
                break;
            case DW_OP_reg15:
                break;
            case DW_OP_reg16:
                break;
            case DW_OP_reg17:
                break;
            case DW_OP_reg18:
                break;
            case DW_OP_reg19:
                break;
            case DW_OP_reg20:
                break;
            case DW_OP_reg21:
                break;
            case DW_OP_reg22:
                break;
            case DW_OP_reg23:
                break;
            case DW_OP_reg24:
                break;
            case DW_OP_reg25:
                break;
            case DW_OP_reg26:
                break;
            case DW_OP_reg27:
                break;
            case DW_OP_reg28:
                break;
            case DW_OP_reg29:
                break;
            case DW_OP_reg30:
                break;
            case DW_OP_reg31:
                break;
        }

        return false;
    }
};

// 型情報
struct type_info
{
    std::string name;
};
// 変数情報
struct var_info
{
    std::string name;
    bool external;
    Dwarf_Unsigned decl_file;  // filelistのインデックス
    bool decl_file_is_external;
    Dwarf_Unsigned decl_line;
    Dwarf_Unsigned decl_column;
    Dwarf_Off type;  // reference
    Dwarf_Off location;
};

// compile_unitから取得する情報
struct compile_unit_info
{
    // compile_unit info
    Dwarf_Unsigned cu_header_length;
    Dwarf_Half version_stamp;
    Dwarf_Off abbrev_offset;
    Dwarf_Half address_size;
    Dwarf_Half length_size;
    Dwarf_Half extension_size;
    Dwarf_Sig8 type_signature;
    Dwarf_Unsigned typeoffset;
    Dwarf_Unsigned next_cu_header_offset;
    Dwarf_Half header_cu_type;
    //
    Dwarf_Off cu_offset;

    compile_unit_info()
        : cu_header_length(0),
          version_stamp(0),
          abbrev_offset(0),
          address_size(0),
          length_size(0),
          extension_size(0),
          type_signature({0}),
          typeoffset(0),
          next_cu_header_offset(0),
          header_cu_type(0),
          cu_offset(0) {
    }
};

// compile_unitから取得する情報
struct dwarf_info
{
    // libdwarf APIデータ
    Dwarf_Debug *dw_dbg;

    // compile_unit info
    compile_unit_info cu_info;

    // DWARF expression 制御クラス
    dwarf_expression dwarf_expr;

    dwarf_info() : dw_dbg(nullptr), cu_info(), dwarf_expr() {
    }
};
//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_block1(Dwarf_Attribute dw_attr, dwarf_info &di) {
    Dwarf_Block *tempb = 0;
    Dwarf_Error error  = nullptr;
    int result;
    result = dwarf_formblock(dw_attr, &tempb, &error);
    if (result != DW_DLV_OK) {
        error_happen(&error);
        return std::nullopt;
    }
    // https://www.prevanders.net/libdwarfdoc/group__examplediscrlist.html
    dwarf_dealloc(*di.dw_dbg, tempb, DW_DLA_BLOCK);
    return ReturnT(0);
}
//
template <typename T, typename ReturnT = std::optional<T>>
ReturnT get_DW_FORM_udata(Dwarf_Attribute dw_attr) {
    Dwarf_Unsigned data;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_formudata(dw_attr, &data, &error);
    if (result != DW_DLV_OK) {
        error_happen(&error);
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
        error_happen(&error);
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
        error_happen(&error);
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
        error_happen(&error);
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
        error_happen(&error);
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
        error_happen(&error);
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
        case DW_FORM_block4:
        case DW_FORM_block:
            break;
        case DW_FORM_block1:
            return get_DW_FORM_block1<T>(dw_attr, di);

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

// DW_AT_* 参考
// 7.5.4 Attribute Encodings

// DW_AT_location
// exprloc, loclistptr
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_location(Dwarf_Attribute dw_attr, dwarf_info &di, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_attr, di);
    if (result) {
        info.location = *result;
    } else {
        info.location = 0;
    }
}

// DW_AT_name 実装
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_name(Dwarf_Attribute dw_attr, T &info) {
    char *str         = nullptr;
    Dwarf_Error error = nullptr;
    int result;
    result = dwarf_formstring(dw_attr, &str, &error);
    if (result != DW_DLV_OK) {
        error_happen(&error);
    }
    info.name = str;
}
// template <Dwarf_Half DW_TAG>
// void get_DW_AT_name(Dwarf_Attribute dw_attr, var_info_t &info) {
//     char *str         = nullptr;
//     Dwarf_Error error = nullptr;
//     int result;
//     result = dwarf_formstring(dw_attr, &str, &error);
//     if (result == DW_DLV_OK) {
//         info.name = str;
//     }
// }

// DW_AT_decl_column
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_column(Dwarf_Attribute dw_attr, dwarf_info &di, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_attr, di);
    if (result) {
        info.decl_column = *result;
    } else {
        info.decl_column = 0;
    }
}

// DW_AT_decl_file 実装
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_file(Dwarf_Attribute dw_attr, dwarf_info &di, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_attr, di);
    if (result) {
        info.decl_file             = *result;
        info.decl_file_is_external = (info.decl_file != 1);
    } else {
        info.decl_file = 0;
    }
}

// DW_AT_decl_line
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_line(Dwarf_Attribute dw_attr, dwarf_info &di, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_attr, di);
    if (result) {
        info.decl_line = *result;
    } else {
        info.decl_line = 0;
    }
}

// DW_AT_type
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_type(Dwarf_Attribute dw_attr, dwarf_info &di, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_attr, di);
    if (result) {
        info.type = *result;
    } else {
        info.type = 0;
    }
}

// DW_AT_external 実装
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_external(Dwarf_Attribute dw_attr, T &info) {
    Dwarf_Bool returned_bool = 0;
    Dwarf_Error error        = nullptr;
    int result;
    result = dwarf_formflag(dw_attr, &returned_bool, &error);
    if (result != DW_DLV_OK) {
        error_happen(&error);
    }
    info.external = (returned_bool == 1);
}
// template <Dwarf_Half DW_TAG>
// void get_DW_AT_external(Dwarf_Attribute dw_attr, var_info_t &info) {
//     Dwarf_Bool returned_bool = 0;
//     Dwarf_Error error        = nullptr;
//     int result;
//     result = dwarf_formflag(dw_attr, &returned_bool, &error);
//     if (result == DW_DLV_OK) {
//         info.external = (returned_bool == 1);
//     }
// }

//
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_declaration(Dwarf_Attribute dw_attr, T &info) {
    printf("no implemented!\n");
}
template <>
void get_DW_AT_declaration<DW_TAG_variable>(Dwarf_Attribute dw_attr, var_info &info) {
    printf("no implemented!\n");
}

// DW_AT_name
template <Dwarf_Half DW_TAG, typename T>
void analyze_DW_AT_impl(Dwarf_Attribute dw_attr, Dwarf_Half attrnum, dwarf_info &di, T &info) {
    // Attrubute解析
    switch (attrnum) {
        case DW_AT_sibling:
            break;
        case DW_AT_location:
            get_DW_AT_location<DW_TAG>(dw_attr, di, info);
            break;

        case DW_AT_name:
            get_DW_AT_name<DW_TAG>(dw_attr, info);
            break;

        case DW_AT_ordering:
        case DW_AT_subscr_data:
        case DW_AT_byte_size:
        case DW_AT_bit_offset:
        case DW_AT_bit_size:
        case DW_AT_element_list:
        case DW_AT_stmt_list:
        case DW_AT_low_pc:
        case DW_AT_high_pc:
        case DW_AT_language:
        case DW_AT_member:
        case DW_AT_discr:
        case DW_AT_discr_value:
        case DW_AT_visibility:
        case DW_AT_import:
        case DW_AT_string_length:
        case DW_AT_common_reference:
        case DW_AT_comp_dir:
        case DW_AT_const_value:
        case DW_AT_containing_type:
        case DW_AT_default_value:
        case DW_AT_inline:
        case DW_AT_is_optional:
        case DW_AT_lower_bound:
        case DW_AT_producer:
        case DW_AT_prototyped:
        case DW_AT_return_addr:
        case DW_AT_start_scope:
        case DW_AT_bit_stride:
        // case DW_AT_stride_size:
        case DW_AT_upper_bound:
        case DW_AT_abstract_origin:
        case DW_AT_accessibility:
        case DW_AT_address_class:
        case DW_AT_artificial:
        case DW_AT_base_types:
        case DW_AT_calling_convention:
        case DW_AT_count:
        case DW_AT_data_member_location:
            break;

        case DW_AT_decl_column:
            get_DW_AT_decl_column<DW_TAG>(dw_attr, di, info);
            break;
        case DW_AT_decl_file:
            get_DW_AT_decl_file<DW_TAG>(dw_attr, di, info);
            break;
        case DW_AT_decl_line:
            get_DW_AT_decl_line<DW_TAG>(dw_attr, di, info);
            break;

        case DW_AT_declaration:
            get_DW_AT_declaration<DW_TAG>(dw_attr, info);
            break;

        case DW_AT_discr_list:
        case DW_AT_encoding:
            break;

        case DW_AT_external:
            get_DW_AT_external<DW_TAG>(dw_attr, info);
            break;

        case DW_AT_frame_base:
        case DW_AT_friend:
        case DW_AT_identifier_case:
        case DW_AT_macro_info:
        case DW_AT_namelist_item:
        case DW_AT_priority:
        case DW_AT_segment:
        case DW_AT_specification:
        case DW_AT_static_link:
            break;

        case DW_AT_type:
            get_DW_AT_type<DW_TAG>(dw_attr, di, info);
            break;

        case DW_AT_use_location:
        case DW_AT_variable_parameter:
        case DW_AT_virtuality:
        case DW_AT_vtable_elem_location:
        case DW_AT_allocated:
        case DW_AT_associated:
        case DW_AT_data_location:
        case DW_AT_byte_stride:
        case DW_AT_entry_pc:
        case DW_AT_use_UTF8:
        case DW_AT_extension:
        case DW_AT_ranges:
        case DW_AT_trampoline:
        case DW_AT_call_column:
        case DW_AT_call_file:
        case DW_AT_call_line:
        case DW_AT_description:
        case DW_AT_binary_scale:
        case DW_AT_decimal_scale:
        case DW_AT_small:
        case DW_AT_decimal_sign:
        case DW_AT_digit_count:
        case DW_AT_picture_string:
        case DW_AT_mutable:
        case DW_AT_threads_scaled:
        case DW_AT_explicit:
        case DW_AT_object_pointer:
        case DW_AT_endianity:
        case DW_AT_elemental:
        case DW_AT_pure:
        case DW_AT_recursive:
        case DW_AT_signature:
        case DW_AT_main_subprogram:
        case DW_AT_data_bit_offset:
        case DW_AT_const_expr:
        case DW_AT_enum_class:
        case DW_AT_linkage_name:
        case DW_AT_string_length_bit_size:
        case DW_AT_string_length_byte_size:
        case DW_AT_rank:
        case DW_AT_str_offsets_base:
        case DW_AT_addr_base:
        case DW_AT_rnglists_base:
        case DW_AT_dwo_id:
        case DW_AT_dwo_name:
        case DW_AT_reference:
        case DW_AT_rvalue_reference:
        case DW_AT_macros:
        case DW_AT_call_all_calls:
        case DW_AT_call_all_source_calls:
        case DW_AT_call_all_tail_calls:
        case DW_AT_call_return_pc:
        case DW_AT_call_value:
        case DW_AT_call_origin:
        case DW_AT_call_parameter:
        case DW_AT_call_pc:
        case DW_AT_call_tail_call:
        case DW_AT_call_target:
        case DW_AT_call_target_clobbered:
        case DW_AT_call_data_location:
        case DW_AT_call_data_value:
        case DW_AT_noreturn:
        case DW_AT_alignment:
        case DW_AT_export_symbols:
        case DW_AT_deleted:
        case DW_AT_defaulted:
        case DW_AT_loclists_base:
            break;
    }
}

/// @brief 対象DIEに紐づくattributeを解析して情報を取得する
/// @tparam T
/// @tparam DW_TAG
/// @param dbg
/// @param die
/// @param dw_error
/// @param di
/// @param info
template <Dwarf_Half DW_TAG, typename T>
void analyze_DW_AT(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Error *dw_error, dwarf_info &di, T &info) {
    Dwarf_Signed atcount;
    Dwarf_Attribute *atlist;
    Dwarf_Signed i = 0;
    int errv;

    errv = dwarf_attrlist(die, &atlist, &atcount, dw_error);
    if (errv != DW_DLV_OK) {
        // return errv;
        error_happen(dw_error);
        return;
    }
    for (i = 0; i < atcount; ++i) {
        Dwarf_Half attrnum = 0;

        /*  use atlist[i], likely calling
            libdwarf functions and likely
            returning DW_DLV_ERROR if
            what you call gets DW_DLV_ERROR */
        errv = dwarf_whatattr(atlist[i], &attrnum, dw_error);
        if (errv != DW_DLV_OK) {
            /* Something really bad happened. */
            // return errv;
            error_happen(dw_error);
            return;
        }

        // DW_AT_*を文字列化して取得
        // const char *attrname = 0;
        // dwarf_get_AT_name(attrnum, &attrname);
        // printf("Attribute[%ld], value %u name %s\n", (long int)i, attrnum, attrname);

        // DW_AT_*解析
        analyze_DW_AT_impl<DW_TAG>(atlist[i], attrnum, di, info);

        dwarf_dealloc_attribute(atlist[i]);
        atlist[i] = 0;
    }
    dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
}

class dwarf_analyzer {
public:
    // DIE情報
    // Debugging Information Entry
    struct die_info_t
    {
        // 共通情報
        Dwarf_Half tag;  // DIE tag(DW_TAG_*)

        die_info_t(Dwarf_Half tag_) : tag(tag_) {
        }
    };

    using var_info = ::util_dwarf::var_info;

private:
    std::string dwarf_file_path;
    static constexpr size_t dw_true_path_buff_len = 512;
    char dw_true_path_buff[dw_true_path_buff_len];

    Dwarf_Debug dw_dbg;
    Dwarf_Error dw_error;

    static void err_handler(Dwarf_Error dw_error, Dwarf_Ptr dw_errarg) {
    }

    // 解析ワーク情報
    // 解析中compile_unit情報
    dwarf_info dwarf_info_;

    // 解析情報

public:
    dwarf_analyzer() : dw_dbg(nullptr) {
    }
    ~dwarf_analyzer() {
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

        //
        dwarf_info_.dw_dbg = &dw_dbg;

        return true;
    }

    void analyze() {
        Dwarf_Bool dw_is_info = true;
        Dwarf_Die dw_cu_die;

        int result;
        bool finish = false;

        while (!finish) {
            // compile_unit取得
            // ヘッダ情報が一緒に返される
            // ★cu_infoは使い捨てている。必要に応じて保持する
            dwarf_info_.cu_info = compile_unit_info();
            auto &cu_info       = dwarf_info_.cu_info;
            result = dwarf_next_cu_header_e(dw_dbg, dw_is_info, &dw_cu_die, &cu_info.cu_header_length, &cu_info.version_stamp, &cu_info.abbrev_offset,
                                            &cu_info.address_size, &cu_info.length_size, &cu_info.extension_size, &cu_info.type_signature,
                                            &cu_info.typeoffset, &cu_info.next_cu_header_offset, &cu_info.header_cu_type, &dw_error);

            // エラー
            if (result == DW_DLV_ERROR) {
                error_happen(&dw_error);
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
            // オフセットを取得
            result = dwarf_CU_dieoffset_given_die(dw_cu_die, &cu_info.cu_offset, &dw_error);
            if (result != DW_DLV_OK) {
                /*  FAIL */
                // return result;
                error_happen(&dw_error);
                return;
            }

            // DW_TAG_compile_unitを取得できている
            // 一応チェック
            Dwarf_Half tag;
            result = dwarf_tag(dw_cu_die, &tag, &dw_error);
            if (result != DW_DLV_OK) {
                error_happen(&dw_error);
            }
            if (tag == DW_TAG_compile_unit) {
                analyze_cu(dw_cu_die);
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

private:
    void analyze_cu(Dwarf_Die dw_cu_die) {
        // 先にcompile_unitの情報を取得
        analyze_die_TAG_compile_unit(dw_cu_die);

        // .debug_line解析
        // 必要になったら実装
        // analyze_debug_line(dw_cu_die);

        // https://www.prevanders.net/libdwarfdoc/group__examplecuhdre.html

        int result;
        int depth = 0;

        // childがcompile_unit配下の最初のDIEのはず
        Dwarf_Die child;
        result = dwarf_child(dw_cu_die, &child, &dw_error);
        // childを取得できなかったら終了
        if (result == DW_DLV_ERROR) {
            // printf("Error in dwarf_child , depth %d \n", depth);
            //  exit(EXIT_FAILURE);
            error_happen(&dw_error);
            return;
        }
        if (result != DW_DLV_OK) {
            return;
        }

        Dwarf_Die cur_die = child;
        Dwarf_Die sib_die = nullptr;

        // compile_unit配下のDIEをすべてチェック
        // compile_unit直下のDIEを解析にかける
        // compile_unitのchildを起点にsiblingをすべて取得すればいいはず
        while (cur_die != nullptr) {
            // 解析
            analyze_die(cur_die);

            // 次のDIEを検索
            result = dwarf_siblingof_c(cur_die, &sib_die, &dw_error);
            // エラー検出
            if (result == DW_DLV_ERROR) {
                error_happen(&dw_error);
                printf("Error in dwarf_siblingof_c , depth %d \n", depth);
                break;
                // exit(EXIT_FAILURE);
            }
            // すべてのDIEをチェック完了
            if (result == DW_DLV_NO_ENTRY) {
                break;
            }
            // 次のDIEを取得出来たら
            // cur_dieはnullptrにしない設計だが一応チェック
            // 前DIE(cur_die)を解放してsib_dieを今回DIEとする
            if (cur_die != nullptr) {
                dwarf_dealloc(dw_dbg, cur_die, DW_DLA_DIE);
                cur_die = nullptr;
            }
            cur_die = sib_die;
            sib_die = nullptr;
        }

        if (cur_die != nullptr) {
            // 子DIE解放
            dwarf_dealloc(dw_dbg, cur_die, DW_DLA_DIE);
            cur_die = nullptr;
        }
    }

    void analyze_debug_line(Dwarf_Die dw_cu_die) {
        // Dwarf_Line_Context
        // dwarf_srcfiles
        // dwarf_srclines_table_offset
        // dwarf_srclines_version

        // .debug_line 解析のはず
        Dwarf_Signed count              = 0;
        Dwarf_Signed i                  = 0;
        char **srcfiles                 = nullptr;
        int res                         = 0;
        Dwarf_Line_Context line_context = nullptr;  // .debug_line
        Dwarf_Small table_count         = 0;
        Dwarf_Unsigned lineversion      = 0;

        // 情報取得
        res = dwarf_srclines_b(dw_cu_die, &lineversion, &table_count, &line_context, &dw_error);
        if (res != DW_DLV_OK) {
            /*  dwarf_finish() will dealloc srcfiles, not doing
                that here.  */
            // return res;
            error_happen(&dw_error);
            return;
        }
        res = dwarf_srcfiles(dw_cu_die, &srcfiles, &count, &dw_error);
        if (res != DW_DLV_OK) {
            error_happen(&dw_error);
            dwarf_srclines_dealloc_b(line_context);
            // return res;
            return;
        }

        // 暫定：値取り出し
        for (i = 0; i < count; ++i) {
            Dwarf_Signed propernumber = 0;

            /*  Use srcfiles[i] If you  wish to print 'i'
                mostusefully
                you should reflect the numbering that
                a DW_AT_decl_file attribute would report in
                this CU. */
            if (lineversion == 5) {
                propernumber = i;
            } else {
                propernumber = i + 1;
            }
            printf("File %4ld %s\n", (unsigned long)propernumber, srcfiles[i]);
            dwarf_dealloc(dw_dbg, srcfiles[i], DW_DLA_STRING);
            srcfiles[i] = 0;
        }
        /*  We could leave all dealloc to dwarf_finish() to
            handle, but this tidies up sooner. */
        dwarf_dealloc(dw_dbg, srcfiles, DW_DLA_LIST);
        dwarf_srclines_dealloc_b(line_context);
        // return DW_DLV_OK;
        return;
    }

    void analyze_die(Dwarf_Die dw_cu_die) {
        // DIEを解析して情報取得する
        // ★die_infoは使い捨てにしている。必要に応じて保持するように変更する
        // die_infoを構築したらTAGに応じて変数情報、型情報に変換して記憶する
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(dw_cu_die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            error_happen(&dw_error);
        }

        auto die_info = die_info_t(tag);

        switch (die_info.tag) {
            case DW_TAG_compile_unit:
                // compile_unitはrootノードとして別扱いしている
                // 上流で解析済み
                break;

            // 変数タグ
            case DW_TAG_variable:
                analyze_DW_TAG_variable(dw_cu_die, die_info);
                break;
            case DW_TAG_constant:
                // no implement
                break;

            // 関数タグ
            case DW_TAG_subprogram:
                // no implement
                break;

            // 型情報タグ
            case DW_TAG_base_type:
                break;
            case DW_TAG_unspecified_type:
                break;
            case DW_TAG_enumeration_type:
                break;
            case DW_TAG_enumerator:
            case DW_TAG_member:
            case DW_TAG_subrange_type:
                // おそらくここには出現しない
                break;
            case DW_TAG_structure_type:
            case DW_TAG_class_type:
                break;
            case DW_TAG_union_type:
                break;
            case DW_TAG_typedef:
                break;
            case DW_TAG_array_type:
                break;
            case DW_TAG_subroutine_type:
                break;
            case DW_TAG_formal_parameter:
                break;
            // type-qualifier
            case DW_TAG_const_type:
                break;
            case DW_TAG_pointer_type:
                break;
            case DW_TAG_restrict_type:
                break;
            case DW_TAG_volatile_type:
                break;

            default:
                // case DW_TAG_inlined_subroutine:
                // case DW_TAG_entry_point:
                // case DW_TAG_imported_declaration:
                // case DW_TAG_label:
                // case DW_TAG_lexical_block:
                // case DW_TAG_reference_type:
                // case DW_TAG_compile_unit:
                // case DW_TAG_string_type:
                // case DW_TAG_unspecified_parameters:
                // case DW_TAG_variant:
                // case DW_TAG_common_block:
                // case DW_TAG_common_inclusion:
                // case DW_TAG_inheritance:
                // case DW_TAG_module:
                // case DW_TAG_ptr_to_member_type:
                // case DW_TAG_set_type:
                // case DW_TAG_with_stmt:
                // case DW_TAG_access_declaration:
                // case DW_TAG_catch_block:
                // case DW_TAG_file_type:
                // case DW_TAG_friend:
                // case DW_TAG_namelist:
                // case DW_TAG_namelist_item:
                // case DW_TAG_packed_type:
                // case DW_TAG_template_type_parameter:
                // case DW_TAG_template_type_param:
                // case DW_TAG_template_value_parameter:
                // case DW_TAG_thrown_type:
                // case DW_TAG_try_block:
                // case DW_TAG_variant_part:
                // case DW_TAG_dwarf_procedure:
                // case DW_TAG_interface_type:
                // case DW_TAG_namespace:
                // case DW_TAG_imported_module:
                // case DW_TAG_partial_unit:
                // case DW_TAG_imported_unit:
                // case DW_TAG_condition:
                // case DW_TAG_shared_type:
                // case DW_TAG_type_unit:
                // case DW_TAG_rvalue_reference_type:
                // case DW_TAG_template_alias:
                // case DW_TAG_coarray_type:
                // case DW_TAG_generic_subrange:
                // case DW_TAG_dynamic_type:
                // case DW_TAG_atomic_type:
                // case DW_TAG_call_site:
                // case DW_TAG_call_site_parameter:
                // case DW_TAG_skeleton_unit:
                // case DW_TAG_immutable_type:
                // no implement
                break;
        }

        printf("tag: %d\n", tag);
    }

    // // DW_TAG_*に0がアサインされていないことを前提として、TAG無指定のみ有効化している
    // template <Dwarf_Half Tag = 0>
    // auto analyze_DW_TAG(Dwarf_Die die, die_info_t &die_info) -> std::enable_if_t<Tag == 0> {
    // }

    void analyze_die_TAG_compile_unit(Dwarf_Die dw_cu_die) {
    }

    void analyze_DW_TAG_variable(Dwarf_Die die, die_info_t &die_info) {
        // 変数情報作成
        var_info info;
        analyze_DW_AT<DW_TAG_variable>(dw_dbg, die, &dw_error, dwarf_info_, info);
    }
};

}  // namespace util_dwarf
