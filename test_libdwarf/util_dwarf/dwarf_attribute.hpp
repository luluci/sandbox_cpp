#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include "dwarf_expression.hpp"
#include "dwarf_form.hpp"
#include "dwarf_info.hpp"
#include "utility.hpp"

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

// DW_AT_* 参考
// 7.5.4 Attribute Encodings

// DW_AT_location
// exprloc, loclistptr
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_location(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.location = *result;
    }
}
// DW_AT_data_member_location
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_data_member_location(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.data_member_location = *result;
    } else {
        info.data_member_location = 0;
    }
}

// DW_AT_low_pc
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_low_pc(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.low_pc = *result;
    } else {
        info.low_pc = 0;
    }
}
// DW_AT_high_pc
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_high_pc(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.high_pc = *result;
    } else {
        info.high_pc = 0;
    }
}

// DW_AT_language
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_language(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.language = *result;
    } else {
        info.language = 0;
    }
}

// DW_AT_comp_dir
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_comp_dir(dwarf_analyze_info &dw_info, T &info) {
    char *str = nullptr;
    int result;
    result = dwarf_formstring(dw_info.dw_attr, &str, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.comp_dir = str;
}

// DW_AT_const_value
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_const_value(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.const_value = *result;
    } else {
        info.const_value = 0;
    }
}

// DW_AT_name
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_name(dwarf_analyze_info &dw_info, T &info) {
    char *str = nullptr;
    int result;
    result = dwarf_formstring(dw_info.dw_attr, &str, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.name = str;
}
// template <Dwarf_Half DW_TAG>
// void get_DW_AT_name(Dwarf_Attribute dw_attr, var_info_t &info) {
//     char *str         = nullptr;
//     Dwarf_Error error = nullptr;
//     int result;
//     result = dwarf_formstring(dw_info.dw_attr, &str, &error);
//     if (result == DW_DLV_OK) {
//         info.name = str;
//     }
// }
// DW_AT_byte_size
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_byte_size(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.byte_size = *result;
    } else {
        info.byte_size = 0;
    }
}
// DW_AT_bit_offset
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_bit_offset(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.bit_offset = *result;
    } else {
        info.bit_offset = 0;
    }
}
// DW_AT_bit_size
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_bit_size(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.bit_size = *result;
    } else {
        info.bit_size = 0;
    }
}
// DW_AT_data_bit_offset
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_data_bit_offset(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.data_bit_offset = *result;
    } else {
        info.data_bit_offset = 0;
    }
}

// DW_AT_stmt_list
// .debug_line内のline number情報までのオフセット
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_stmt_list(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Off>(dw_info);
    if (result) {
        info.stmt_list = *result;
    } else {
        info.stmt_list = 0;
    }
}

// DW_AT_linkage_name
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_linkage_name(dwarf_analyze_info &dw_info, T &info) {
    char *str = nullptr;
    int result;
    result = dwarf_formstring(dw_info.dw_attr, &str, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.linkage_name = str;
}

// DW_AT_signature
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_signature(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.signature = *result;
    } else {
        info.signature = 0;
    }
}

// DW_AT_accessibility
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_accessibility(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    // DW_ACCESS_public
    // DW_ACCESS_private
    // DW_ACCESS_protected
    if (result) {
        info.accessibility = *result;
    } else {
        info.accessibility = 0;
    }
}

// DW_AT_upper_bound
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_upper_bound(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.upper_bound = *result;
    }
}
// DW_AT_lower_bound
// 省略されることがある。省略時のデフォルト値は DW_AT_languageによって決まる。
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_lower_bound(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.lower_bound = *result;
    }
}

// DW_AT_producer
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_producer(dwarf_analyze_info &dw_info, T &info) {
    char *str = nullptr;
    int result;
    result = dwarf_formstring(dw_info.dw_attr, &str, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.producer = str;
}

// DW_AT_prototyped
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_prototyped(dwarf_analyze_info &dw_info, T &info) {
    Dwarf_Bool returned_bool = 0;
    int result;
    result = dwarf_formflag(dw_info.dw_attr, &returned_bool, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.prototyped = (returned_bool == 1);
}

// DW_AT_count
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_count(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.count = *result;
    }
}

// DW_AT_address_class
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_address_class(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.address_class = *result;
    }
}

// DW_AT_decl_column
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_column(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.decl_column = *result;
    } else {
        info.decl_column = 0;
    }
}

// DW_AT_decl_file 実装
// .debug_line内ファイルテーブルのindexを格納している
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_file(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        // index==1がデフォルトでソースファイルになるはずだが、そうでないこともある
        // CU情報内ファイル名が対応するindexとdecl_fileが同じなら自CU内で定義されたとみなす
        info.decl_file = *result;
    } else {
        info.decl_file = 0;
    }
}

// DW_AT_decl_line
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_decl_line(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.decl_line = *result;
    } else {
        info.decl_line = 0;
    }
}

// DW_AT_encoding
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_encoding(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.encoding = *result;
    } else {
        info.encoding = 0;
    }
}

// DW_AT_sibling
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_sibling(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.sibling = *result;
    } else {
        info.sibling = 0;
    }
}
// DW_AT_type
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_type(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.type = *result;
    }
}

// DW_AT_endianity
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_endianity(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.type = *result;
    }
}

// DW_AT_ranges
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_ranges(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.ranges = *result;
    } else {
        info.ranges = 0;
    }
}

// DW_AT_binary_scale
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_binary_scale(dwarf_analyze_info &dw_info, T &info) {
    auto result = get_DW_FORM<Dwarf_Unsigned>(dw_info);
    if (result) {
        info.binary_scale = *result;
    } else {
        info.binary_scale = 0;
    }
}

// DW_AT_external 実装
template <Dwarf_Half DW_TAG, typename T>
void get_DW_AT_external(dwarf_analyze_info &dw_info, T &info) {
    Dwarf_Bool returned_bool = 0;
    int result;
    result = dwarf_formflag(dw_info.dw_attr, &returned_bool, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
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
void get_DW_AT_declaration(dwarf_analyze_info &dw_info, T &info) {
    Dwarf_Bool returned_bool = 0;
    int result;
    result = dwarf_formflag(dw_info.dw_attr, &returned_bool, &dw_info.dw_error);
    if (result != DW_DLV_OK) {
        utility::error_happen(&dw_info.dw_error);
    }
    info.declaration = (returned_bool == 1);
}
// template <>
// void get_DW_AT_declaration<DW_TAG_variable>(Dwarf_Attribute dw_attr, var_info &info) {
//     printf("no implemented! : get_DW_AT_declaration\n");
// }

// DW_AT_name
template <Dwarf_Half DW_TAG, typename T>
void analyze_DW_AT_impl(Dwarf_Attribute dw_attr, Dwarf_Half attrnum, dwarf_analyze_info &dw_info, T &info) {
    dw_info.dw_attr = dw_attr;

    // Attrubute解析
    switch (attrnum) {
        case DW_AT_sibling:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_sibling<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_location:
            if constexpr (std::is_same_v<T, dwarf_info::var_info>) {
                get_DW_AT_location<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_name:
            get_DW_AT_name<DW_TAG>(dw_info, info);
            return;

        case DW_AT_ordering:
        case DW_AT_subscr_data:
            break;
        case DW_AT_byte_size:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_byte_size<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_bit_offset:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_bit_offset<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_bit_size:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_bit_size<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_element_list:
            break;
        case DW_AT_stmt_list:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_stmt_list<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_low_pc:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_low_pc<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_high_pc:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_high_pc<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_language:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_language<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_member:
        case DW_AT_discr:
        case DW_AT_discr_value:
        case DW_AT_visibility:
        case DW_AT_import:
        case DW_AT_string_length:
        case DW_AT_common_reference:
            break;

        case DW_AT_comp_dir:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_comp_dir<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_const_value:
            if constexpr (std::is_same_v<T, dwarf_info::var_info>) {
                get_DW_AT_const_value<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_containing_type:
        case DW_AT_default_value:
        case DW_AT_inline:
        case DW_AT_is_optional:
            break;

        case DW_AT_lower_bound:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_lower_bound<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_producer:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_producer<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_prototyped:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_prototyped<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_return_addr:
        case DW_AT_start_scope:
        case DW_AT_bit_stride:
            // case DW_AT_stride_size:
            break;

        case DW_AT_upper_bound:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_upper_bound<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_abstract_origin:
            break;

        case DW_AT_accessibility:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_accessibility<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_address_class:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_address_class<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_artificial:
        case DW_AT_base_types:
        case DW_AT_calling_convention:
            break;

        case DW_AT_count:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_count<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_data_member_location:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_data_member_location<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_decl_column:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_decl_column<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_decl_file:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_decl_file<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_decl_line:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_decl_line<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_declaration:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_declaration<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_discr_list:
            break;

        case DW_AT_encoding:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_encoding<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_external:
            if constexpr (std::is_same_v<T, dwarf_info::var_info>) {
                get_DW_AT_external<DW_TAG>(dw_info, info);
            }
            return;

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
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_type<DW_TAG>(dw_info, info);
            }
            return;

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
            break;

        case DW_AT_ranges:
            if constexpr (std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_ranges<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_trampoline:
        case DW_AT_call_column:
        case DW_AT_call_file:
        case DW_AT_call_line:
        case DW_AT_description:
            break;

        case DW_AT_binary_scale:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_binary_scale<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_decimal_scale:
        case DW_AT_small:
        case DW_AT_decimal_sign:
        case DW_AT_digit_count:
        case DW_AT_picture_string:
        case DW_AT_mutable:
        case DW_AT_threads_scaled:
        case DW_AT_explicit:
        case DW_AT_object_pointer:
            break;

        case DW_AT_endianity:
            if constexpr (!std::is_same_v<T, dwarf_info::cu_info>) {
                get_DW_AT_endianity<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_elemental:
        case DW_AT_pure:
        case DW_AT_recursive:
            break;

        case DW_AT_signature:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_signature<DW_TAG>(dw_info, info);
            }
            return;
        case DW_AT_main_subprogram:
            break;

        case DW_AT_data_bit_offset:
            if constexpr (std::is_same_v<T, dwarf_info::type_info>) {
                get_DW_AT_data_bit_offset<DW_TAG>(dw_info, info);
            }
            return;

        case DW_AT_const_expr:
        case DW_AT_enum_class:
            break;

        case DW_AT_linkage_name:
            if constexpr (std::is_same_v<T, dwarf_info::var_info>) {
                get_DW_AT_linkage_name<DW_TAG>(dw_info, info);
            }
            return;

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
        default:
            break;
    }

    // no impl
    const char *attrname = nullptr;
    dwarf_get_AT_name(attrnum, &attrname);
    fprintf(stderr, "no impl : %s (%u)\n", attrname, attrnum);
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
void analyze_DW_AT(Dwarf_Die die, dwarf_analyze_info &dw_info, T &info) {
    Dwarf_Signed atcount;
    Dwarf_Attribute *atlist;
    Dwarf_Signed i = 0;
    int errv;

    errv = dwarf_attrlist(die, &atlist, &atcount, &dw_info.dw_error);
    if (errv == DW_DLV_NO_ENTRY) {
        // DW_AT_* なし
        return;
    }
    if (errv == DW_DLV_ERROR) {
        // return errv;
        utility::error_happen(&dw_info.dw_error);
        return;
    }
    for (i = 0; i < atcount; ++i) {
        Dwarf_Half attrnum = 0;

        /*  use atlist[i], likely calling
            libdwarf functions and likely
            returning DW_DLV_ERROR if
            what you call gets DW_DLV_ERROR */
        errv = dwarf_whatattr(atlist[i], &attrnum, &dw_info.dw_error);
        if (errv != DW_DLV_OK) {
            /* Something really bad happened. */
            // return errv;
            utility::error_happen(&dw_info.dw_error);
            return;
        }

        // DW_AT_*を文字列化して取得
        // const char *attrname = 0;
        // dwarf_get_AT_name(attrnum, &attrname);
        // printf("Attribute[%ld], value %u name %s\n", (long int)i, attrnum, attrname);

        // DW_AT_*解析
        analyze_DW_AT_impl<DW_TAG>(atlist[i], attrnum, dw_info, info);

        dwarf_dealloc_attribute(atlist[i]);
        atlist[i] = 0;
    }
    dwarf_dealloc(dw_info.dw_dbg, atlist, DW_DLA_LIST);
}

}  // namespace util_dwarf
