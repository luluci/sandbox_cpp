#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <string>

#include "dwarf_info.hpp"

namespace util_dwarf {

class debug_info {
public:
    using type_tag = dwarf_info::type_tag;

    // dwarf_info::type_infoを集約した型情報
    struct type_info
    {
        uint16_t tag;

        std::string *name;
        Dwarf_Unsigned byte_size;
        Dwarf_Unsigned bit_offset;
        Dwarf_Unsigned bit_size;
        Dwarf_Unsigned encoding;  // DW_ATE_*
        Dwarf_Unsigned count;
        Dwarf_Unsigned address_class;
        Dwarf_Unsigned pointer_depth;

        bool is_const;
        bool is_restrict;
        bool is_volatile;

        // member or parameter
        using child_list_t = dwarf_info::type_info::child_list_t;
        child_list_t *child_list;
        // array用
        type_info *sub_info;

        type_info()
            : tag(0),
              name(nullptr),
              byte_size(0),
              bit_offset(0),
              bit_size(0),
              encoding(0),
              count(0),
              address_class(0),
              pointer_depth(0),
              is_const(false),
              is_restrict(false),
              is_volatile(false),
              child_list(nullptr),
              sub_info(nullptr) {
        }
    };

    // 型情報
    using type_map_t = std::map<Dwarf_Unsigned, type_info>;
    type_map_t type_map;
    std::list<type_info> sub_type_list;

    // 固定情報
    std::string name_void;

private:
    dwarf_info &dw_info_;

public:
    debug_info(dwarf_info &dw_info) : dw_info_(dw_info), name_void("void") {
    }

    // DIEから収集したデータは木構造で情報が分散している
    // ルートオブジェクトに情報を集約して型情報を単一にする
    void build() {
        auto &dw_type_map = dw_info_.type_tbl.type_map;

        for (auto &elem : dw_type_map) {
            // 集約ノード取得
            // ダミー取得をして情報の作成を行う
            auto &dbg_info = get_type_info(elem.first);
        }
    }

    type_info &get_type_info(Dwarf_Unsigned offset) {
        // 指定したoffsetのtype_infoを取得する
        // 作成済みなら終了
        auto it = type_map.find(offset);
        if (it != type_map.end()) {
            return it->second;
        }
        // 未作成なら作成
        // ノード作成
        auto [it2, result] = type_map.try_emplace(offset, type_info());
        if (!result) {
            // 重複はありえない
        }
        // 情報作成
        build_type_info(it2->second, offset);
        return it2->second;
    }

private:
    void build_type_info(type_info &dbg_info, Dwarf_Unsigned offset) {
        auto &dw_type_map = dw_info_.type_tbl.type_map;
        // 開始ノード存在チェック
        auto it = dw_type_map.find(offset);
        if (it == dw_type_map.end()) {
            // 存在しなければ空のまま終了
            return;
        }
        // 開始ノードが存在したらデータ作成開始
        // child typeの存在をチェック
        auto &root_dw_info = it->second;
        if (root_dw_info.type) {
            // child typeが存在するとき、
            // child typeのtype_infoをまずコピーする
            // ここでchild typeが未作成でも関数コールにより作成される
            // 再帰呼び出しになるので注意
            auto &child_info = get_type_info(*(root_dw_info.type));
            dbg_info         = child_info;
        }
        // type_infoに今回対象となるoffsetの情報を適用する
        adapt_info(dbg_info, root_dw_info);
        adapt_info_fix(dbg_info);
    }

    void adapt_info(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        switch (dw_info.tag) {
            case type_tag::base:
                adapt_info_base(dbg_info, dw_info);
                break;

            case type_tag::func:
                adapt_info_func(dbg_info, dw_info);
                break;

            case type_tag::typedef_:
                adapt_info_typedef(dbg_info, dw_info);
                break;

            case type_tag::struct_:
            case type_tag::union_:
                adapt_info_struct_union(dbg_info, dw_info);
                break;

            case type_tag::array:
                adapt_info_array(dbg_info, dw_info);
                break;

            case type_tag::pointer:
                adapt_info_pointer(dbg_info, dw_info);
                break;

            case type_tag::const_:
                adapt_info_const(dbg_info, dw_info);
                break;

            case type_tag::restrict_:
                adapt_info_restrict(dbg_info, dw_info);
                break;

            case type_tag::volatile_:
                adapt_info_volatile(dbg_info, dw_info);
                break;

            case type_tag::enum_:
                adapt_info_enum(dbg_info, dw_info);
                break;

            default:
                // 実装忘れ
                printf("no impl : build_node : 0x%02X\n", dw_info.tag);
                break;
        }
    }

    void adapt_info_fix(type_info &dbg_info) {
        // 後処理
        // void型ケア
        // tagがnoneのときはvoid型？
        if (dbg_info.tag == 0) {
            dbg_info.tag = type_tag::none;
        }
        // unnamedのときはvoid型？
        if (dbg_info.name == nullptr) {
            dbg_info.name = &name_void;

            // pointer size
            if (dbg_info.byte_size == 0) {
                dbg_info.byte_size = dw_info_.machine_arch.obj_pointersize;
            }
        }
        // byte_sizeケア
        if (dbg_info.byte_size == 0) {
            // 関数ポインタのとき？
            if ((dbg_info.tag & type_tag::func_ptr) == type_tag::func_ptr) {
                dbg_info.byte_size = dw_info_.machine_arch.obj_pointersize;
            }
        }
        // arrayケア
        if ((dbg_info.tag & type_tag::array) == type_tag::array) {
            // arrayに関するデータをマスクしたデータが要素型のデータになる
            sub_type_list.push_back(dbg_info);
            auto &sub_type    = sub_type_list.back();
            dbg_info.sub_info = &sub_type;
            //
            sub_type.tag &= ~(type_tag::array);
            sub_type.count = 0;
        }
    }

private:
    void adapt_info_base(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.encoding, dw_info.encoding);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_enum(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_func(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.child_list, dw_info.child_list);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_typedef(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_struct_union(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.child_list, dw_info.child_list);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_array(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.count, dw_info.count);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_pointer(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // ポインタサイズが優先のため上書き
        if (dw_info.address_class) {
            // address_classを持っているとき
            dbg_info.address_class = *dw_info.address_class;
            auto addr_cls_tbl      = dw_info_.arch_info[dbg_info.address_class].address_class;
            if (addr_cls_tbl != nullptr) {
                //
                if (dbg_info.address_class < arch::addr_cls::size) {
                    dbg_info.byte_size = addr_cls_tbl[dbg_info.address_class];
                }
            }
        } else {
            // address_classを持っていないとき
            dbg_info.byte_size = dw_info.byte_size;
        }
        // DW_AT_*でポインタサイズを指定されていないときはアーキテクチャに従う
        if (dbg_info.byte_size == 0) {
            dbg_info.byte_size = dw_info_.machine_arch.obj_pointersize;
        }
        // ダブルポインタとかのケア
        dbg_info.pointer_depth++;

        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_const(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_const = true;
        //
        dbg_info.tag |= dw_info.tag;
    }
    void adapt_info_restrict(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_restrict = true;
        //
        dbg_info.tag |= dw_info.tag;
    }
    void adapt_info_volatile(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_volatile = true;
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_value(std::string *&dst, std::string &src) {
        if (dst == nullptr && src.size() > 0) {
            dst = &src;
        }
    }
    void adapt_value(Dwarf_Unsigned &dst, Dwarf_Unsigned &src) {
        if (dst == 0) {
            dst = src;
        }
    }
    void adapt_value(type_info::child_list_t *&dst, type_info::child_list_t &src) {
        if (dst == nullptr && src.size() > 0) {
            dst = &src;
        }
    }
    void adapt_value(Dwarf_Unsigned &dst, std::optional<Dwarf_Unsigned> &src) {
        if (dst == 0 && src) {
            dst = *src;
        }
    }
};

}  // namespace util_dwarf
