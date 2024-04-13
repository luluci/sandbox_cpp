#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "dwarf_analyze_info.hpp"
#include "dwarf_attribute.hpp"
#include "dwarf_info.hpp"
#include "elf.hpp"

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

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

    // 変数情報
    using var_info = dwarf_info::var_info;

    // 型情報
    using type_tag   = dwarf_info::type_tag;
    using type_info  = dwarf_info::type_info;
    using type_child = dwarf_info::type_info::child_node_t;

private:
    std::string dwarf_file_path;
    static constexpr size_t dw_true_path_buff_len = 512;
    char dw_true_path_buff[dw_true_path_buff_len];

    static void err_handler(Dwarf_Error error, Dwarf_Ptr dw_errarg) {
        char *errmsg = dwarf_errmsg(error);
        printf("%s\n", errmsg);
        throw std::runtime_error("libdwarf API error!");
        exit(1);  // 一応書いておく
    }

    // 解析制御情報
    bool analyze_func_info_;
    // 解析ワーク情報
    Dwarf_Debug dw_dbg;
    Dwarf_Error dw_error;
    dwarf_analyze_info analyze_info_;

    // 解析情報

public:
    dwarf_analyzer() : dw_dbg(nullptr) {
    }
    ~dwarf_analyzer() {
        close();
    }

    void set_analyze_func_info(bool value) {
        analyze_func_info_ = value;
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
        if (result == DW_DLV_NO_ENTRY) {
            // 情報なし？
            return false;
        }
        if (result == DW_DLV_ERROR) {
            // return errv;
            utility::error_happen(&dw_error);
            return false;
        }

        return true;
    }

    void analyze(dwarf_info &info) {
        Dwarf_Bool dw_is_info = true;
        Dwarf_Die dw_cu_die;

        int result;
        bool finish = false;

        // 解析情報初期化
        analyze_info_          = dwarf_analyze_info();
        analyze_info_.dw_dbg   = dw_dbg;
        analyze_info_.dw_error = dw_error;

        // アーキテクチャ情報取得
        analyze_machine_architecture(info);

        while (!finish) {
            // compile_unit取得
            // ヘッダ情報が一緒に返される
            // ★cu_infoは使い捨てている。必要に応じてinfo.cu_infoに保持する
            analyze_info_.cu_info = dwarf_info::compile_unit_info();
            auto &cu_info         = analyze_info_.cu_info;
            result = dwarf_next_cu_header_e(dw_dbg, dw_is_info, &dw_cu_die, &cu_info.cu_header_length, &cu_info.version_stamp, &cu_info.abbrev_offset,
                                            &cu_info.address_size, &cu_info.length_size, &cu_info.extension_size, &cu_info.type_signature,
                                            &cu_info.typeoffset, &cu_info.next_cu_header_offset, &cu_info.header_cu_type, &dw_error);

            // エラー
            if (result == DW_DLV_ERROR) {
                utility::error_happen(&dw_error);
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
                utility::error_happen(&dw_error);
                return;
            }

            // DW_TAG_compile_unitを取得できている
            // 一応チェック
            Dwarf_Half tag;
            result = dwarf_tag(dw_cu_die, &tag, &dw_error);
            if (result != DW_DLV_OK) {
                utility::error_happen(&dw_error);
            }
            if (tag == DW_TAG_compile_unit) {
                analyze_cu(dw_cu_die, info);
            }

            dwarf_dealloc_die(dw_cu_die);
        }

        return;
    }

    bool close() {
        if (dw_dbg == nullptr) {
            return true;
        }

        auto result = dwarf_finish(dw_dbg);
        printf("dwarf_finish : result : %d\n", result);
        dw_dbg = nullptr;
        return (result == DW_DLV_OK);
    }

private:
    void analyze_machine_architecture(dwarf_info &info) {
        auto result = dwarf_machine_architecture(dw_dbg, &info.machine_arch.ftype, &info.machine_arch.obj_pointersize,
                                                 &info.machine_arch.obj_is_big_endian, &info.machine_arch.obj_machine, &info.machine_arch.obj_flags,
                                                 &info.machine_arch.path_source, &info.machine_arch.ub_offset, &info.machine_arch.ub_count,
                                                 &info.machine_arch.ub_index, &info.machine_arch.comdat_groupnumber);
        if (result == DW_DLV_NO_ENTRY) {
            return;
        }
        // アーキテクチャ情報選択
        info.arch_info = &(arch::arch_info_tbl[info.machine_arch.obj_machine]);
    }

    void analyze_cu(Dwarf_Die dw_cu_die, dwarf_info &info) {
        // 先にcompile_unitの情報を取得
        analyze_die_TAG_compile_unit(dw_cu_die, info);

        // .debug_line解析
        // 必要になったら実装
        // analyze_debug_line(dw_cu_die);

        // https://www.prevanders.net/libdwarfdoc/group__examplecuhdre.html

        bool result = get_child_die(dw_cu_die, [this, &info](Dwarf_Die die) -> bool {
            analyze_die(die, info);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }

    template <typename Func>
    bool get_child_die(Dwarf_Die parent, Func &&callback) {
        int result;
        // parentから最初のchildを取得する
        // このchildのsiblingを全部取得することですべてのchildの取得になる
        Dwarf_Die cur_die;
        result = dwarf_child(parent, &cur_die, &dw_error);
        if (result == DW_DLV_ERROR) {
            // エラー発生
            // printf("Error in dwarf_child , depth %d \n", depth);
            //  exit(EXIT_FAILURE);
            // utility::error_happen(&dw_error);
            return false;
        }
        if (result == DW_DLV_NO_ENTRY) {
            // childが存在しない
            // 正常終了とする
            return true;
        }

        // childが存在する限りループしてcallbackを実行する
        bool func_result = true;
        bool callback_result;
        Dwarf_Die sib_die = nullptr;
        while (cur_die != nullptr) {
            // コールバック
            callback_result = callback(cur_die);
            // falseを返したら終了する
            if (!callback_result) {
                break;
            }

            // 次のDIEを検索
            result = dwarf_siblingof_c(cur_die, &sib_die, &dw_error);
            // エラー検出
            if (result == DW_DLV_ERROR) {
                // utility::error_happen(&dw_error);
                // printf("Error in dwarf_siblingof_c , depth %d \n", depth);
                // exit(EXIT_FAILURE);
                func_result = false;
                break;
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

        // childを解放して処理終了
        if (cur_die != nullptr) {
            dwarf_dealloc(dw_dbg, cur_die, DW_DLA_DIE);
            cur_die = nullptr;
        }

        return func_result;
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
        if (res == DW_DLV_ERROR) {
            /*  dwarf_finish() will dealloc srcfiles, not doing
                that here.  */
            // return res;
            utility::error_happen(&dw_error);
            return;
        }
        // DW_DLV_OK, DW_DLV_NO_ENTRYなら次へ

        res = dwarf_srcfiles(dw_cu_die, &srcfiles, &count, &dw_error);
        if (res == DW_DLV_ERROR) {
            // return res;
            utility::error_happen(&dw_error);
            dwarf_srclines_dealloc_b(line_context);
            return;
        }
        // DW_DLV_OK, DW_DLV_NO_ENTRYなら次へ

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

    void analyze_die(Dwarf_Die die, dwarf_info &info) {
        // DIEを解析して情報取得する
        // ★die_infoは使い捨てにしている。必要に応じて保持するように変更する
        // die_infoを構築したらTAGに応じて変数情報、型情報に変換して記憶する
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }

        auto die_info = die_info_t(tag);

        switch (die_info.tag) {
            case DW_TAG_compile_unit:
                // compile_unitはrootノードとして別扱いしている
                // 上流で解析済み
                return;

            // 変数タグ
            case DW_TAG_variable:
                analyze_DW_TAG_variable(die, info, die_info);
                return;
            case DW_TAG_constant:
                // no implement
                break;

            // 関数タグ
            case DW_TAG_subprogram:
                if (analyze_func_info_) {
                    // no implement
                    break;
                } else {
                    return;
                }
            case DW_TAG_subroutine_type:
                analyze_DW_TAG_subroutine_type(die, info, die_info);
                return;
            case DW_TAG_formal_parameter:
                // おそらくここには出現しない
                break;
            case DW_TAG_label:
                if (analyze_func_info_) {
                    // no implement
                    break;
                } else {
                    return;
                }

            // 型情報タグ
            case DW_TAG_base_type:
                analyze_DW_TAG_base_type(die, info, die_info);
                return;
            case DW_TAG_unspecified_type:
                // analyze_DW_TAG_unspecified_type(die, die_info);
                break;
            case DW_TAG_enumeration_type:
                analyze_DW_TAG_enumeration_type(die, info, die_info);
                return;
            case DW_TAG_enumerator:
            case DW_TAG_member:
            case DW_TAG_subrange_type:
                // おそらくここには出現しない
                break;

            case DW_TAG_reference_type:
                analyze_DW_TAG_reference_type(die, info, die_info);
                return;

            case DW_TAG_structure_type:
            case DW_TAG_class_type:
                analyze_DW_TAG_structure_type(die, info, die_info);
                return;
            case DW_TAG_union_type:
                analyze_DW_TAG_union_type(die, info, die_info);
                return;
            case DW_TAG_typedef:
                analyze_DW_TAG_typedef(die, info, die_info);
                return;
            case DW_TAG_array_type:
                analyze_DW_TAG_array_type(die, info, die_info);
                return;

            // type-qualifier
            case DW_TAG_const_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_const_type>(die, info, die_info, type_tag::const_);
                return;
            case DW_TAG_pointer_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_pointer_type>(die, info, die_info, type_tag::pointer);
                return;
            case DW_TAG_restrict_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_restrict_type>(die, info, die_info, type_tag::restrict_);
                return;
            case DW_TAG_volatile_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_volatile_type>(die, info, die_info, type_tag::volatile_);
                return;

            default:
                // case DW_TAG_inlined_subroutine:
                // case DW_TAG_entry_point:
                // case DW_TAG_imported_declaration:
                // case DW_TAG_lexical_block:
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

        const char *name = 0;
        dwarf_get_TAG_name(die_info.tag, &name);
        printf("no impl : %s (%u)\n", name, die_info.tag);
    }

    // // DW_TAG_*に0がアサインされていないことを前提として、TAG無指定のみ有効化している
    // template <Dwarf_Half Tag = 0>
    // auto analyze_DW_TAG(Dwarf_Die die, die_info_t &die_info) -> std::enable_if_t<Tag == 0> {
    // }

    void analyze_die_TAG_compile_unit(Dwarf_Die, dwarf_info &) {
    }

    void analyze_DW_TAG_variable(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // 変数情報作成
        auto info = dw_info.global_var_tbl.make_new_var_info();
        analyze_DW_AT<DW_TAG_variable>(die, analyze_info_, *info);
        // decl_fileチェック
        if (info->decl_file > 0) {
            // file_listからこの変数が定義されたファイル名を取得できる
        }
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_variable");
        //
        if (!info->location or !info->type or info->name.size() == 0) {
            // いずれかに当てはまるとローカル変数, 定数, etc
            // アドレスを持たない
            // typeを持たない
            // 名前を持たない
        } else {
            // アドレスを持っているとグローバル変数
            dw_info.global_var_tbl.add(std::move(info));
        }
    }

    void analyze_DW_TAG_base_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::base);
        analyze_DW_AT<DW_TAG_base_type>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_base_type");
    }

    // void analyze_DW_TAG_unspecified_type(Dwarf_Die die, die_info_t &die_info) {
    // }

    void analyze_DW_TAG_enumeration_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::enum_);
        analyze_DW_AT<DW_TAG_enumeration_type>(die, analyze_info_, info);
        // child dieチェック
        bool result = get_child_die(die, [this, &dw_info, &die_info, &info](Dwarf_Die child) -> bool {
            analyze_DW_TAG_enumeration_type_child(child, dw_info, die_info, info);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }
    void analyze_DW_TAG_enumeration_type_child(Dwarf_Die die, dwarf_info &dw_info, die_info_t &, type_info &parent_type) {
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }

        // DW_TAG_subroutine_typeのchildとして出現するDW_TAG_*を処理する
        auto die_info = die_info_t(tag);
        switch (die_info.tag) {
            case DW_TAG_enumerator: {
                // member情報を作成
                auto mem_info = analyze_DW_TAG_enumerator(die, dw_info, die_info);
                // parentのchildにparameterとして登録
                parent_type.child_list.push_back(std::move(mem_info));
                return;
            }

            default:
                break;
        }

        const char *name = 0;
        dwarf_get_TAG_name(die_info.tag, &name);
        printf("no impl : DW_TAG_enumeration_type child : %s (%u)\n", name, die_info.tag);
    }
    // DW_TAG_enumerator
    type_child analyze_DW_TAG_enumerator(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::enum_);
        analyze_DW_AT<DW_TAG_enumerator>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_enumerator");
        //
        return &info;
    }

    void analyze_DW_TAG_structure_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // structとunionがほぼ同じなので共通処理にする
        analyze_DW_TAG_struct_union<DW_TAG_structure_type>(die, dw_info, die_info, type_tag::struct_);
    }
    void analyze_DW_TAG_union_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // structとunionがほぼ同じなので共通処理にする
        analyze_DW_TAG_struct_union<DW_TAG_union_type>(die, dw_info, die_info, type_tag::struct_);
    }

    template <size_t DW_TAG>
    void analyze_DW_TAG_struct_union(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info, type_tag tag) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, tag);
        analyze_DW_AT<DW_TAG>(die, analyze_info_, info);
        // child dieチェック
        bool result = get_child_die(die, [this, &dw_info, &die_info, &info](Dwarf_Die child) -> bool {
            analyze_DW_TAG_struct_union_child<DW_TAG>(child, dw_info, die_info, info);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }
    template <Dwarf_Half DW_TAG>
    void analyze_DW_TAG_struct_union_child(Dwarf_Die die, dwarf_info &dw_info, die_info_t &, type_info &parent_type) {
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }

        // struct/unionのchildとして出現するDW_TAG_*を処理する
        auto die_info = die_info_t(tag);
        switch (die_info.tag) {
            case DW_TAG_member: {
                // member情報を作成
                auto mem_info = analyze_DW_TAG_member(die, dw_info, die_info);
                // parentのmemberに登録
                parent_type.child_list.push_back(std::move(mem_info));
                return;
            }

            // struct/union/class内で使う型情報の定義
            // member要素にはならない
            // 解析して型情報として登録する
            case DW_TAG_array_type:
                analyze_DW_TAG_array_type(die, dw_info, die_info);
                return;

            case DW_TAG_reference_type:
                analyze_DW_TAG_reference_type(die, dw_info, die_info);
                return;

            // type-qualifier
            case DW_TAG_const_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_const_type>(die, dw_info, die_info, type_tag::const_);
                return;
            case DW_TAG_pointer_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_pointer_type>(die, dw_info, die_info, type_tag::pointer);
                return;
            case DW_TAG_restrict_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_restrict_type>(die, dw_info, die_info, type_tag::restrict_);
                return;
            case DW_TAG_volatile_type:
                analyze_DW_TAG_type_qualifier<DW_TAG_volatile_type>(die, dw_info, die_info, type_tag::volatile_);
                return;

            // 関数タグ
            case DW_TAG_subprogram:
                // メンバ関数
                if (analyze_func_info_) {
                    // no implement
                    break;
                } else {
                    return;
                }

            default:
                break;
        }

        const char *name = 0;
        dwarf_get_TAG_name(die_info.tag, &name);
        printf("no impl : DW_TAG_struct/union child : %s (%u)\n", name, die_info.tag);
    }

    type_child analyze_DW_TAG_member(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::member);
        analyze_DW_AT<DW_TAG_member>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_member");
        //
        return &info;
    }

    void analyze_DW_TAG_array_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::array);
        analyze_DW_AT<DW_TAG_array_type>(die, analyze_info_, info);
        // omitチェック
        check_omitted_type_info(info);
        // child dieチェック
        bool result = get_child_die(die, [this, &dw_info, &die_info, &info](Dwarf_Die child) -> bool {
            analyze_DW_TAG_array_type_child(child, dw_info, die_info, info);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }

    void analyze_DW_TAG_array_type_child(Dwarf_Die die, dwarf_info &dw_info, die_info_t &parent_die_info, type_info &parent_type) {
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }

        // DW_TAG_array_typeのchildとして出現するDW_TAG_*を処理する
        auto die_info = die_info_t(tag);
        switch (die_info.tag) {
            case DW_TAG_subrange_type: {
                // subrange情報を作成
                auto child_info = analyze_DW_TAG_subrange_type(die, dw_info, die_info);
                // array要素数を示す
                if (child_info->count) {
                    parent_type.count = *child_info->count;
                } else if (child_info->upper_bound && child_info->lower_bound) {
                    parent_type.count = *child_info->upper_bound - *child_info->lower_bound + 1;
                } else {
                    // nullopt
                }
                return;
            }

            default:
                break;
        }

        const char *name = 0;
        dwarf_get_TAG_name(die_info.tag, &name);
        printf("no impl : DW_TAG_array_type child : %s (%u)\n", name, die_info.tag);
    }

    type_child analyze_DW_TAG_subrange_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::member);
        analyze_DW_AT<DW_TAG_subrange_type>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_subrange_type");
        //
        return &info;
    }

    // DW_TAG_subroutine_type
    void analyze_DW_TAG_subroutine_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &die_info) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::func);
        analyze_DW_AT<DW_TAG_subroutine_type>(die, analyze_info_, info);
        // child dieチェック
        bool result = get_child_die(die, [this, &dw_info, &die_info, &info](Dwarf_Die child) -> bool {
            analyze_DW_TAG_subroutine_type_child(child, dw_info, die_info, info);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }
    void analyze_DW_TAG_subroutine_type_child(Dwarf_Die die, dwarf_info &dw_info, die_info_t &, type_info &parent_type) {
        int result;
        Dwarf_Half tag;
        result = dwarf_tag(die, &tag, &dw_error);
        if (result != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }

        // DW_TAG_subroutine_typeのchildとして出現するDW_TAG_*を処理する
        auto die_info = die_info_t(tag);
        switch (die_info.tag) {
            case DW_TAG_formal_parameter: {
                // member情報を作成
                auto mem_info = analyze_DW_TAG_formal_parameter(die, dw_info, die_info);
                // parentのchildにparameterとして登録
                parent_type.child_list.push_back(std::move(mem_info));
                return;
            }

            default:
                break;
        }

        const char *name = 0;
        dwarf_get_TAG_name(die_info.tag, &name);
        printf("no impl : DW_TAG_subroutine_type child : %s (%u)\n", name, die_info.tag);
    }
    // DW_TAG_formal_parameter
    type_child analyze_DW_TAG_formal_parameter(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::parameter);
        analyze_DW_AT<DW_TAG_formal_parameter>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_formal_parameter");
        //
        return &info;
    }

    // DW_TAG_reference_type
    void analyze_DW_TAG_reference_type(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::reference);
        analyze_DW_AT<DW_TAG_reference_type>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_reference_type");
    }

    // DW_TAG_TAG_type_qualifier
    template <Dwarf_Half DW_TAG>
    void analyze_DW_TAG_type_qualifier(Dwarf_Die die, dwarf_info &dw_info, die_info_t &, type_tag tag) {
        int res;
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, tag);
        analyze_DW_AT<DW_TAG>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_const/pointer/restrict/volatile_type");
    }

    // DW_TAG_typedef
    void analyze_DW_TAG_typedef(Dwarf_Die die, dwarf_info &dw_info, die_info_t &) {
        // DIE offset取得
        Dwarf_Off offset = get_die_offset(die);
        // 型情報作成
        auto &info = dw_info.type_tbl.make_new_type_info(offset, type_tag::typedef_);
        analyze_DW_AT<DW_TAG_typedef>(die, analyze_info_, info);
        // child dieチェックしない
        // childが存在したら表示だけ出しておく
        debug_dump_no_impl_child(die, "DW_TAG_typedef");
    }

    Dwarf_Off get_die_offset(Dwarf_Die die) {
        int res;
        Dwarf_Off dw_global_offset = 0;
        Dwarf_Off dw_local_offset  = 0;
        // DIE offset取得
        res = dwarf_die_offsets(die, &dw_global_offset, &dw_local_offset, &dw_error);
        if (res != DW_DLV_OK) {
            utility::error_happen(&dw_error);
        }
        //
        return dw_global_offset;
    }

    void check_omitted_type_info(type_info &info) {
        // 省略されたDW_AT_*を手動で補完する
        // decl_fileが省略された場合、[1]のファイルが該当する
        if (info.decl_file == 0) {
            info.decl_file = 1;
        }
    }

    void debug_dump_no_impl_child(Dwarf_Die die, char const *parent_tag) {
        // child dieチェック
        bool result = get_child_die(die, [this, parent_tag](Dwarf_Die child) -> bool {
            // analyze_die(child);
            int res;
            Dwarf_Half tag;
            res = dwarf_tag(child, &tag, &dw_error);
            if (res != DW_DLV_OK) {
                utility::error_happen(&dw_error);
            }

            const char *name = 0;
            dwarf_get_TAG_name(tag, &name);
            printf("no impl : %s child : %s (%u)\n", parent_tag, name, tag);
            return true;
        });
        // 異常が発生していたらfalseが返される
        if (!result) {
            utility::error_happen(&dw_error);
        }
    }
};

}  // namespace util_dwarf
