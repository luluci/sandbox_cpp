#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include "dwarf_info.hpp"

namespace util_dwarf {

class debug_info {
public:
    struct option
    {
        using type = uint32_t;

        enum mode : type
        {
            none,
            through_typedef = 1 << 0,
            expand_array    = 1 << 1,
        };

        bool is_through_typedef;
        bool is_expand_array;

        option(type flags = none) : is_through_typedef(true), is_expand_array(false) {
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
            if (check_flag(flags, through_typedef)) {
                is_through_typedef = value;
            }
            if (check_flag(flags, expand_array)) {
                is_expand_array = value;
            }
        }

        bool check_flag(type flags, mode flag) {
            return ((flags & flag) == flag);
        }
    };

public:
    using type_tag = dwarf_info::type_tag;

    struct var_info
    {
        std::string *name;
        bool external;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        bool decl_file_is_external;
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        bool declaration;  // 不完全型のときtrue
        Dwarf_Unsigned const_value;
        Dwarf_Unsigned sibling;
        Dwarf_Unsigned endianity;  // DW_END_*
        std::optional<Dwarf_Off> location;
        std::optional<Dwarf_Off> type;  // reference

        var_info()
            : name(nullptr),
              external(false),
              decl_file(0),
              decl_file_is_external(false),
              decl_line(0),
              decl_column(0),
              declaration(false),
              const_value(0),
              sibling(0),
              endianity(0),
              location(),
              type() {
        }

        void copy(dwarf_info::var_info &info) {
            // dwarf_infoから必要な情報をコピーする
            name                  = &(info.name);
            external              = info.external;
            decl_file             = info.decl_file;
            decl_file_is_external = info.decl_file_is_external;
            decl_line             = info.decl_line;
            decl_column           = info.decl_column;
            declaration           = info.declaration;
            const_value           = info.const_value;
            sibling               = info.sibling;
            endianity             = info.endianity;
            if (info.type)
                type = *info.type;
            if (info.location)
                location = *info.location;
        }
    };

    // dwarf_info::type_infoを集約した型情報
    struct type_info
    {
        uint16_t tag;

        std::string *name;
        Dwarf_Unsigned byte_size;
        Dwarf_Unsigned bit_offset;
        Dwarf_Unsigned bit_size;
        Dwarf_Unsigned data_bit_offset;
        Dwarf_Off data_member_location;
        Dwarf_Unsigned encoding;  // DW_ATE_*
        Dwarf_Unsigned count;
        Dwarf_Unsigned address_class;
        Dwarf_Unsigned pointer_depth;
        std::optional<Dwarf_Off> type;  // reference

        bool is_const;
        bool is_restrict;
        bool is_volatile;

        using child_node_t = type_info *;
        using child_list_t = std::list<child_node_t>;
        // member
        child_list_t *member_list;
        // parameter
        child_list_t *param_list;
        // subrange[]
        child_list_t *array_range_list;
        // array/member用
        type_info *sub_info;

        // 付加情報
        bool has_bitfield;

        type_info()
            : tag(0),
              name(nullptr),
              byte_size(0),
              bit_offset(0),
              bit_size(0),
              data_bit_offset(0),
              data_member_location(0),
              encoding(0),
              count(0),
              address_class(0),
              pointer_depth(0),
              type(),
              is_const(false),
              is_restrict(false),
              is_volatile(false),
              member_list(nullptr),
              param_list(nullptr),
              array_range_list(nullptr),
              sub_info(nullptr),
              has_bitfield(false) {
        }
    };

    // 変数情報
    // using var_info = dwarf_info::var_info;
    using var_list_node_t = std::unique_ptr<var_info>;
    using var_list_t      = std::vector<var_list_node_t>;
    var_list_t global_var_tbl;
    // 型情報
    using type_map_t = std::map<Dwarf_Unsigned, type_info>;
    type_map_t type_map;
    std::list<type_info> sub_type_list;
    std::list<type_info::child_list_t> child_list_list;

    // 固定情報
    std::string name_void;

private:
    dwarf_info &dw_info_;

    // option
    option opt_;

public:
    debug_info(dwarf_info &dw_info, option opt) : name_void("void"), dw_info_(dw_info), opt_(opt) {
        global_var_tbl.reserve(dw_info.global_var_tbl.var_list.size());
    }

    void build() {
        build_var_info();
        build_type_info();
    }
    void build_var_info() {
        // ソート用に変数リストへのポインタをリストアップする
        auto &dw_var_list = dw_info_.global_var_tbl.var_list;
        for (auto &elem : dw_var_list) {
            // dwarf_infoからデータコピー
            auto info = std::make_unique<var_info>();
            info->copy(*elem);
            // list追加
            global_var_tbl.push_back(std::move(info));
        }
        // ソートする
        std::sort(global_var_tbl.begin(), global_var_tbl.end(), [](auto &a, auto &b) -> bool {
            if (!a->location) {
                return true;
            } else if (!b->location) {
                return false;
            } else {
                return a->location < b->location;
            }
        });
        // checkする
        bool is_del;
        auto it = global_var_tbl.begin();
        while (it != global_var_tbl.end()) {
            is_del = false;
            // 削除対象をチェック
            if ((*it)->decl_file_is_external) {
                is_del = true;
            }
            // 削除
            if (is_del) {
                it = global_var_tbl.erase(it);
            } else {
                it++;
            }
        }
    }
    void build_type_info() {
        // DIEから収集したデータは木構造で情報が分散している
        // ルートオブジェクトに情報を集約して型情報を単一にする
        auto &dw_type_map = dw_info_.type_tbl.type_map;

        for (auto &elem : dw_type_map) {
            // 集約ノード取得
            // ダミー取得をして情報の作成を行う
            get_type_info(elem.first);
        }
    }

    void memmap(std::function<void(var_info &, type_info &)> &&func) {
        for (auto &var : global_var_tbl) {
            if (var->type) {
                auto it = type_map.find(*(var->type));
                if (it != type_map.end()) {
                    func(*var, it->second);
                }
            }
        }
    }

private:
    type_info *get_type_info(Dwarf_Unsigned offset) {
        // 指定したoffsetのtype_infoを取得する
        // 作成済みなら終了
        auto it = type_map.find(offset);
        if (it != type_map.end()) {
            return &(it->second);
        }
        // 未作成なら作成
        // ノード作成
        auto [it2, result] = type_map.try_emplace(offset, type_info());
        if (!result) {
            // 重複はありえない
        }
        // 情報作成
        build_type_info(it2->second, offset);
        return &(it2->second);
    }

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
            auto child_info = get_type_info(*(root_dw_info.type));
            dbg_info        = *child_info;
            if (dbg_info.sub_info == nullptr) {
                dbg_info.sub_info = child_info;
            }
        }
        // type_infoに今回対象となるoffsetの情報を適用する
        adapt_info(dbg_info, root_dw_info);
        adapt_info_fix(dbg_info);
    }

    void adapt_info(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        adapt_value(dbg_info.type, dw_info.type);

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

            case type_tag::member:
                adapt_info_member(dbg_info, dw_info);
                break;

            case type_tag::reference:
                adapt_info_reference(dbg_info, dw_info);
                break;

            case type_tag::parameter:
                adapt_info_parameter(dbg_info, dw_info);
                break;

            case type_tag::subrange:
                adapt_info_subrange(dbg_info, dw_info);
                break;

            default:
                // 実装忘れ
                fprintf(stderr, "no impl : build_node : 0x%02X\n", dw_info.tag);
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
        if (dbg_info.sub_info == nullptr) {
            if ((dbg_info.tag & type_tag::array) == type_tag::array) {
                // arrayに関するデータをマスクしたデータが要素型のデータになる
                sub_type_list.push_back(dbg_info);
                auto &sub_type    = sub_type_list.back();
                dbg_info.sub_info = &sub_type;
                //
                sub_type.byte_size = sub_type.byte_size / sub_type.count;
                sub_type.count     = 0;
                sub_type.tag &= ~(type_tag::array);
            }
        }
    }

    void adapt_info_base(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.encoding, dw_info.encoding);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_enum(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // dwarfデータのnameが空の場合は無名定義
        if (dw_info.name.size() == 0) {
            dw_info.name = "<unnamed>";
        }
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_member(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // dwarfデータのnameが空の場合は無名定義
        if (dw_info.name.size() == 0) {
            dw_info.name = "<unnamed>";
        }
        // 対象データが空ならdw_infoを反映する
        adapt_value_force(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.bit_offset, dw_info.bit_offset);
        adapt_value(dbg_info.bit_size, dw_info.bit_size);
        adapt_value(dbg_info.data_bit_offset, dw_info.data_bit_offset);
        adapt_value(dbg_info.data_member_location, dw_info.data_member_location);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
        }
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_reference(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
        }
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_func(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 関数ポインタ型名前作成
        if (dw_info.name.size() > 0) {
            // 関数ポインタ型名称を優先
            // adapt_value(dbg_info.name, dw_info.name);
        } else {
            auto inserter = std::back_inserter(dw_info.name);
            // 返り値型適用
            if (dbg_info.name == nullptr) {
                std::format_to(inserter, "void");
            } else {
                std::format_to(inserter, "{}", *dbg_info.name);
            }
            // 関数ポインタ表示
            std::format_to(inserter, "(*)(");
            // 引数型
            auto it = dw_info.child_list.begin();
            if (it != dw_info.child_list.end()) {
                auto &param    = *it;
                auto dbg_param = get_type_info(param->offset);
                std::format_to(inserter, "{}", *dbg_param->name);
                it++;
            }
            for (; it != dw_info.child_list.end(); it++) {
                auto &param    = *it;
                auto dbg_param = get_type_info(param->offset);
                std::format_to(inserter, ", {}", *dbg_param->name);
            }
            std::format_to(inserter, ")");
        }
        dbg_info.name = &dw_info.name;
        //
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.param_list, dw_info.child_list);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_parameter(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        adapt_value(dbg_info.name, dw_info.name);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
        }
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_typedef(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        if (opt_.is_through_typedef) {
            adapt_value(dbg_info.name, dw_info.name);
        } else {
            adapt_value_force(dbg_info.name, dw_info.name);
        }
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_struct_union(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // dwarfデータのnameが空の場合は無名定義
        if (dw_info.name.size() == 0) {
            dw_info.name = "<unnamed>";
        }
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.member_list, dw_info.child_list);
        adapt_value(dbg_info.has_bitfield, dw_info.has_bitfield);
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_array(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        // arrayはsub_infoに型情報を保持している
        // dwarf_infoではchild_listにsubrangeを保持している
        // debug_infoではarray_range_listに参照を持たせる
        adapt_value_force(dbg_info.array_range_list, dw_info.child_list);
        // array_range_list から配列の各次元のサイズ数を計算する
        // 最終次からループして、各次元の要素1つあたりのサイズを計算する
        Dwarf_Unsigned child_size;
        auto it = dbg_info.array_range_list->rbegin();
        // 最終次はarrayの型サイズになる
        child_size = dbg_info.byte_size;
        for (; it != dbg_info.array_range_list->rend(); it++) {
            (*it)->byte_size = child_size;
            // 上位次のサイズを計算
            // 現在次の型サイズ * 要素数 になる
            child_size = child_size * (*it)->count;
        }
        //
        dbg_info.tag |= dw_info.tag;
    }

    void adapt_info_subrange(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.count, dw_info.count);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
        }
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
        // name作成
        if (dw_info.name.size() > 0) {
            dbg_info.name = &dw_info.name;
        } else {
            if ((dbg_info.tag & type_tag::func) == 0) {
                auto it = std::back_inserter(dw_info.name);
                if (dbg_info.name == nullptr) {
                    std::format_to(it, "void");
                } else {
                    std::format_to(it, "{}", *dbg_info.name);
                }
                std::format_to(it, "*");
                dbg_info.name = &dw_info.name;
            }
        }

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
    void adapt_value_force(std::string *&dst, std::string &src) {
        if (src.size() > 0) {
            dst = &src;
        }
    }

    void adapt_value(std::optional<Dwarf_Off> &dst, std::optional<Dwarf_Off> &src) {
        if (!dst && src) {
            dst = *src;
        }
    }
    void adapt_value(Dwarf_Unsigned &dst, Dwarf_Unsigned &src) {
        if (dst == 0) {
            dst = src;
        }
    }
    void adapt_value(debug_info::type_info::child_list_t *&dst, dwarf_info::type_info::child_list_t &src) {
        if (dst == nullptr && src.size() > 0) {
            type_info::child_list_t list;

            for (auto &child : src) {
                auto dbg_child = get_type_info(child->offset);
                list.push_back(dbg_child);
            }

            child_list_list.push_back(std::move(list));
            auto &new_list = child_list_list.back();
            dst            = &new_list;
        }
    }
    void adapt_value_force(debug_info::type_info::child_list_t *&dst, dwarf_info::type_info::child_list_t &src) {
        dst = nullptr;
        adapt_value(dst, src);
    }
    void adapt_value(Dwarf_Unsigned &dst, std::optional<Dwarf_Unsigned> &src) {
        if (dst == 0 && src) {
            dst = *src;
        }
    }
    void adapt_value(bool &dst, bool &src) {
        if (src) {
            dst = src;
        }
    }

public:
    struct var_info_view
    {
        std::string *tag_type;
        std::string *tag_name;

        Dwarf_Off address;
        Dwarf_Unsigned byte_size;
        Dwarf_Unsigned bit_offset;
        Dwarf_Unsigned bit_size;
        Dwarf_Unsigned data_bit_offset;
        Dwarf_Unsigned encoding;  // DW_ATE_*
        Dwarf_Off data_member_location;

        size_t pointer_depth;
        bool is_struct;
        bool is_union;
        bool is_array;
        bool is_bitfield;

        var_info_view()
            : tag_type(nullptr),
              tag_name(nullptr),
              address(0),
              byte_size(0),
              bit_offset(0),
              bit_size(0),
              data_bit_offset(0),
              encoding(0),
              data_member_location(0),
              pointer_depth(0),
              is_struct(false),
              is_union(false),
              is_array(false),
              is_bitfield(false) {
        }
    };

    struct lookup_mode
    {
        using type = uint32_t;

        enum mode
        {
            not_expand_array,
        };
    };

    void get_var_info(std::function<bool(var_info_view &)> &&func) {
        std::string prefix = "";
        bool result;

        // global_varをすべてチェック
        for (auto &var : global_var_tbl) {
            // 対応するtypeを取得
            if (var->type) {
                auto it = type_map.find(*(var->type));
                if (it != type_map.end()) {
                    result = lookup_var(*var, it->second, prefix, 0, func);
                    if (!result) {
                        break;
                    }
                }
            }
        }
    }

private:
    void make_type_tag(var_info_view &view, type_info &type) {
        // pointer
        view.pointer_depth = type.pointer_depth;
        // type情報タグを作成
        if (type.name != nullptr) {
            view.tag_type = type.name;
            if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
                view.is_struct = true;
            } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
                view.is_union = true;
            } else {
                //
            }
        } else {
            // ありえない
            view.tag_type = &name_void;
        }
        //
        view.encoding = type.encoding;
    }

    template <typename Func>
    bool lookup_var(var_info &var, type_info &type, std::string &prefix, size_t depth, Func &&func) {
        bool result;
        var_info_view view;
        Dwarf_Off address;
        // 型タグ作成
        make_type_tag(view, type);
        // 変数情報
        view.tag_name = var.name;
        if (var.location) {
            address = *var.location;
        } else {
            address = 0;
        }

        if ((type.tag & util_dwarf::debug_info::type_tag::array) != 0) {
            // 配列のとき
            result = lookup_var_impl_array(view, type, address, prefix, depth, func);
        } else {
            // 配列以外のとき
            result = lookup_var_impl_default(view, type, address, prefix, depth, func);
        }

        return result;
    }

    template <typename Func>
    bool lookup_var_member(type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth, Func &func) {
        bool result;

        //  pointerは展開しない
        //  function: 引数としてchildを持つ -> 展開しない
        if ((type.tag & util_dwarf::debug_info::type_tag::func_ptr) != 0) {
            return true;
        }
        if (type.member_list == nullptr) {
            return true;
        }

        for (auto &mem : *type.member_list) {
            // memberが変数名になる
            auto &member = *mem;
            // memberのchild要素がmemberの型情報を示しているはず
            type_info *member_type = &(*mem);
            if (member.sub_info != nullptr) {
                member_type = member.sub_info;
            }
            // member毎処理
            result = lookup_var_member_each(member, *member_type, base_address, prefix, depth, func);
            if (!result) {
                break;
            }
        }

        return result;
    }
    template <typename Func>
    bool lookup_var_member_each(type_info &member, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth, Func &func) {
        bool result;
        Dwarf_Off address;
        var_info_view view;
        // 型タグ作成
        make_type_tag(view, type);
        // 変数情報
        view.tag_name = member.name;
        address       = base_address + member.data_member_location;

        if (member.bit_size == 0) {
            // bit_sizeがゼロならビットフィールドでない
            if ((member.tag & util_dwarf::debug_info::type_tag::array) != 0) {
                result = lookup_var_impl_array(view, member, address, prefix, depth, func);
            } else {
                result = lookup_var_impl_default(view, member, address, prefix, depth, func);
            }

        } else {
            // bit_sizeがゼロ以外ならビットフィールド
            result = lookup_var_impl_bitfield(view, member, address, prefix, depth, func);
        }

        return result;
    }

    template <typename Func>
    bool lookup_var_impl_default(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth, Func &func) {
        auto tag_name_org = view.tag_name;
        std::string var_name;
        bool result;

        // 表示名作成
        if (prefix.size() == 0) {
            std::format_to(std::back_inserter(var_name), "{}", *tag_name_org);
        } else {
            std::format_to(std::back_inserter(var_name), "{}.{}", prefix, *tag_name_org);
        }
        view.tag_name = &var_name;
        // アドレス計算
        view.address = base_address;
        //
        view.is_array   = true;
        view.byte_size  = type.byte_size;
        view.bit_offset = 0;
        view.bit_size   = 0;

        // コールバック
        result = func(view);
        if (!result) {
            return result;
        }

        //  member check
        result = lookup_var_member(type, view.address, var_name, depth + 1, func);
        return result;
    }

    template <typename Func>
    bool lookup_var_impl_bitfield(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t /*depth*/, Func &func) {
        auto tag_name_org = view.tag_name;
        std::string var_name;
        bool result;
        Dwarf_Off address;
        Dwarf_Unsigned bit_offset;

        address    = base_address + (type.bit_offset / 8);
        bit_offset = type.bit_offset % 8;

        // 表示名作成
        if (prefix.size() == 0) {
            std::format_to(std::back_inserter(var_name), "{}", *tag_name_org);
        } else {
            std::format_to(std::back_inserter(var_name), "{}.{}", prefix, *tag_name_org);
        }
        view.tag_name = &var_name;
        // アドレス計算
        view.address = address;

        view.is_bitfield = true;
        view.byte_size   = 0;
        view.bit_offset  = bit_offset;
        view.bit_size    = type.bit_size;

        // コールバック
        result = func(view);
        return result;
    }

    template <typename Func>
    bool lookup_var_impl_array(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth, Func &func) {
        bool result = true;
        // 多次元配列ケアのために再帰的コールして変数名を作成する
        // 末尾まで到達したら変数の内容をコールバックする
        // arrayなら必ずsubrangeを持つはずだが一応チェック
        if (type.array_range_list != nullptr && type.array_range_list->size() > 0) {
            // インデックス手前までのprefixを作成
            auto tag_name_org = view.tag_name;
            std::string var_name;
            if (prefix.size() == 0) {
                std::format_to(std::back_inserter(var_name), "{}", *tag_name_org);
            } else {
                std::format_to(std::back_inserter(var_name), "{}.{}", prefix, *tag_name_org);
            }
            // インデックス作成
            auto it = type.array_range_list->begin();
            result  = lookup_var_impl_array_idx(view, type, base_address, var_name, depth, it, func);

            // 念のため
            view.tag_name = tag_name_org;
        }

        return result;
    }

    template <typename Func>
    bool lookup_var_impl_array_idx(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth,
                                   type_info::child_list_t::iterator &array_d_it, Func &func) {
        std::string var_name;
        Dwarf_Off address;
        bool result = true;

        // array_d次の要素数から名称作成
        for (size_t i = 0; i < (*array_d_it)->count; i++) {
            address = base_address + (i * (*array_d_it)->byte_size);
            // 表示名作成
            var_name.clear();
            if (opt_.is_expand_array) {
                // array展開あり
                std::format_to(std::back_inserter(var_name), "{}[{}]", prefix, i);
            } else {
                // array展開なし
                std::format_to(std::back_inserter(var_name), "{}[{}]", prefix, (*array_d_it)->count);
            }

            array_d_it++;
            if (array_d_it == type.array_range_list->end()) {
                // 最終次なら変数内容を表示
                result = lookup_var_impl_array_data(view, type, address, var_name, depth, func);
            } else {
                // 最終次でないなら名称作成を継続
                result = lookup_var_impl_array_idx(view, type, address, var_name, depth, array_d_it, func);
            }
            if (!result)
                break;

            // array展開無しなら終了
            if (!opt_.is_expand_array)
                break;
        }

        return result;
    }

    template <typename Func>
    bool lookup_var_impl_array_data(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &prefix, size_t depth, Func &func) {
        bool result;

        view.tag_name = &prefix;
        // アドレス計算
        view.address = base_address;
        //
        view.is_array   = true;
        view.byte_size  = type.byte_size;
        view.bit_offset = 0;
        view.bit_size   = 0;

        // コールバック
        result = func(view);
        if (!result) {
            return result;
        }

        //  member check
        result = lookup_var_member(type, base_address, prefix, depth + 1, func);
        return result;
    }
};

}  // namespace util_dwarf
