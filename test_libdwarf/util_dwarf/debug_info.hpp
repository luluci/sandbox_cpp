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

        type_info() : tag(0), name(nullptr), byte_size(0), bit_offset(0), bit_size(0), encoding(0) {
        }
    };

    // 型情報
    using type_map_t = std::map<Dwarf_Unsigned, type_info>;
    type_map_t type_map;

private:
    dwarf_info &dw_info_;

public:
    debug_info(dwarf_info &dw_info) : dw_info_(dw_info) {
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
    }

    void adapt_info(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        switch (dw_info.tag) {
            case (uint16_t)type_tag::base:
                adapt_info_base(dbg_info, dw_info);
                break;

            case (uint16_t)type_tag::func:
                adapt_info_func(dbg_info, dw_info);
                break;

            case (uint16_t)type_tag::typedef_:
                break;

            default:
                // 実装忘れ
                printf("no impl : build_node : 0x%02X\n", dw_info.tag);
                break;
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

    void adapt_info_func(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.encoding, dw_info.encoding);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_value(std::string *&dst, std::string &src) {
        if (dst == nullptr) {
            dst = &src;
        }
    }
    void adapt_value(Dwarf_Unsigned &dst, Dwarf_Unsigned &src) {
        if (dst == 0) {
            dst = src;
        }
    }
};

}  // namespace util_dwarf
