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
            prior_typedef = 1 << 0,
            expand_array  = 1 << 1,
        };

        bool is_prior_typedef;  // typedefの名前を優先する
        bool is_expand_array;

        option(type flags = none) : is_prior_typedef(false), is_expand_array(false) {
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
            if (check_flag(flags, prior_typedef)) {
                is_prior_typedef = value;
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
    // 型情報構築状態
    enum class build_type_state
    {
        None,        // 初期化状態
        Building,    // 構築処理中
        Incomplete,  // 不完全(型)
        Complete,    // 構築完了
    };

    using type_tag = dwarf_info::type_tag;

    struct var_info
    {
        std::string *name;
        bool external;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        std::string *decl_file_path;
        std::string *decl_file_path_rel;
        bool declaration;  // 不完全型のときtrue
        Dwarf_Unsigned const_value;
        Dwarf_Unsigned sibling;
        Dwarf_Unsigned endianity;  // DW_END_*
        std::optional<dw_op_value *> location;
        std::optional<Dwarf_Off> type;  // reference
        dwarf_info::compile_unit_info *cu_info;

        var_info()
            : name(nullptr),
              external(false),
              decl_file(0),
              decl_line(0),
              decl_column(0),
              decl_file_path(nullptr),
              decl_file_path_rel(nullptr),
              declaration(false),
              const_value(0),
              sibling(0),
              endianity(0),
              location(),
              type(),
              cu_info(nullptr) {
        }
        ~var_info() {
        }

        void copy(dwarf_info::var_info &info) {
            // dwarf_infoから必要な情報をコピーする
            name               = &(info.name);
            external           = info.external;
            decl_file          = info.decl_file;
            decl_line          = info.decl_line;
            decl_column        = info.decl_column;
            decl_file_path     = &(info.decl_file_path);
            decl_file_path_rel = &(info.decl_file_path_rel);
            declaration        = info.declaration;
            const_value        = info.const_value;
            sibling            = info.sibling;
            endianity          = info.endianity;
            if (info.type)
                type = *info.type;
            if (info.location)
                location = &(*info.location);
            if (info.cu_info != nullptr)
                cu_info = info.cu_info;
        }
    };

    // dwarf_info::type_infoを集約した型情報
    struct type_info
    {
        uint16_t tag;

        std::string *name;  // unnamedのときはnullptrになる
        Dwarf_Unsigned byte_size;
        Dwarf_Unsigned bit_offset;
        Dwarf_Unsigned bit_size;
        Dwarf_Unsigned data_bit_offset;
        Dwarf_Off data_member_location;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        std::string *decl_file_path;
        std::string *decl_file_path_rel;
        Dwarf_Unsigned encoding;  // DW_ATE_*
        Dwarf_Unsigned count;
        Dwarf_Unsigned address_class;
        Dwarf_Unsigned pointer_depth;
        std::optional<Dwarf_Off> type;  // reference
        dwarf_info::compile_unit_info *cu_info;

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

        // 解析フラグ
        build_type_state build_state;

        type_info()
            : tag(0),
              name(nullptr),
              byte_size(0),
              bit_offset(0),
              bit_size(0),
              data_bit_offset(0),
              data_member_location(0),
              decl_file(0),
              decl_line(0),
              decl_column(0),
              decl_file_path(nullptr),
              decl_file_path_rel(nullptr),
              encoding(0),
              count(0),
              address_class(0),
              pointer_depth(0),
              type(),
              cu_info(nullptr),
              is_const(false),
              is_restrict(false),
              is_volatile(false),
              member_list(nullptr),
              param_list(nullptr),
              array_range_list(nullptr),
              sub_info(nullptr),
              has_bitfield(false),
              build_state(build_type_state::None) {
        }
        ~type_info() {
        }
    };

    // 変数情報
    // using var_info = dwarf_info::var_info;
    using var_map_node_t = std::unique_ptr<var_info>;
    using var_map_t      = std::map<Dwarf_Off, var_map_node_t>;
    var_map_t var_tbl;
    // 型情報
    using type_map_t = std::map<Dwarf_Unsigned, type_info>;
    type_map_t type_map;
    std::list<type_info> sub_type_list;
    std::list<type_info::child_list_t> child_list_list;

    // 固定情報
    std::string name_void;
    std::string name_unnamed;

    // その他解析情報
    std::size_t max_typename_len;  // 最大型名文字列長
    std::size_t max_varname_len;   // 最大変数名文字列長

private:
    dwarf_info &dw_info_;

    // option
    option opt_;

public:
    debug_info(dwarf_info &dw_info, option opt)
        : name_void("void"), name_unnamed("<unnamed>"), max_typename_len(0), max_varname_len(0), dw_info_(dw_info), opt_(opt) {
    }
    ~debug_info() {
    }

    void build() {
        build_var_info();
        build_type_info();
    }
    void build_var_info() {
        // 付加情報初期化
        // 最大変数名文字列長
        max_varname_len = 0;

        // ソート用に変数リストへのポインタをリストアップする
        auto &dw_var_tbl = dw_info_.var_tbl.container;
        for (auto &[offset, elem] : dw_var_tbl) {
            // location(address)を即値で持っている変数を対象とする
            // exprで持っている場合はローカル変数等の配置が動的に変わる変数
            if (elem.location && elem.location->is_immediate) {
                // 変数除外判定
                // parameterは除外する
                if (!elem.is_parameter && !elem.is_local_var) {
                    auto addr = std::get<Dwarf_Off>(elem.location->value);

                    // DWARF上で同じ変数が複数のCU上に出現することがある
                    // 重複になるので除外する
                    if (!var_tbl.contains(addr)) {
                        // dwarf_infoからデータコピー
                        auto info = std::make_unique<var_info>();
                        info->copy(elem);

                        // 付加情報作成
                        if (info->name != nullptr) {
                            if (max_varname_len < info->name->size()) {
                                max_varname_len = info->name->size();
                            }
                        } else {
                            // ありえない？
                        }

                        // map追加
                        var_tbl.insert(std::make_pair(addr, std::move(info)));
                    }
                }
            }
        }
    }
    void build_type_info() {
        // DIEから収集したデータは木構造で情報が分散している
        // ルートオブジェクトに情報を集約して型情報を単一にする
        auto &dw_type_map = dw_info_.type_tbl.container;

        // 付加情報初期化
        // 最大型名文字列長
        max_typename_len = 0;

        for (auto &elem : dw_type_map) {
            // ダミー取得をして情報の作成を行う
            auto info = get_type_info(elem.first);
            // 付加情報作成
            // 最大型名文字列長
            if (info->name != nullptr) {
                if (max_typename_len < info->name->size()) {
                    max_typename_len = info->name->size();
                }
            } else {
                // この時点でunnamed==nullptrがありうる
                // 後でname_unnamed表示用に使う文字列から最長のもので判定
                if (max_typename_len < name_unnamed.size()) {
                    max_typename_len = name_unnamed.size();
                }
            }
        }
    }

    void memmap(std::function<void(var_info &, type_info &)> &&func) {
        for (auto &[addr, var] : var_tbl) {
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
        // 作成済みデータがあれば終了
        auto it = type_map.find(offset);
        if (it != type_map.end()) {
            switch (it->second.build_state) {
                case build_type_state::Building:
                case build_type_state::Complete:
                    // Building: Dwarf定義内で循環参照している。arm_gccコンパイラで遭遇した。
                    // Complete: 情報構築済みなのでそのまま使用可能
                    return &(it->second);

                case build_type_state::None:
                case build_type_state::Incomplete:
                default:
                    // None: 未作成なのでありえない
                    // Incomplete: Dwarf定義内循環参照等により途中で情報構築を打ち切っている。再構築する
                    break;
            }
        }
        bool result;
        // itを上書きしているので注意
        if (it == type_map.end()) {
            // 未作成なら作成
            std::tie(it, result) = type_map.try_emplace(offset, type_info());
            if (!result) {
                // 重複はありえない
            }
        } else if (it->second.build_state == build_type_state::Incomplete) {
            // 未完成なら再構築
            using obj_type = decltype(it->second);
            it->second     = obj_type();
        }
        // 情報作成
        build_type_info(it->second, offset);
        return &(it->second);
    }

    void build_type_info(type_info &dbg_info, Dwarf_Unsigned offset) {
        // 情報構築開始
        dbg_info.build_state = build_type_state::Building;
        //
        auto &dw_type_map = dw_info_.type_tbl.container;
        // 開始ノード存在チェック
        auto it = dw_type_map.find(offset);
        if (it == dw_type_map.end()) {
            // 存在しなければ空のまま終了
            // return;
            throw std::runtime_error("logic error");
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
        // データ構築完了したか、循環参照で中断したかを返す
        auto result = adapt_info(dbg_info, root_dw_info);
        adapt_info_fix(dbg_info);
        //
        if (result) {
            dbg_info.build_state = build_type_state::Complete;
        } else {
            dbg_info.build_state = build_type_state::Incomplete;
        }
    }

    bool adapt_info(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // CumpileUnit情報
        adapt_value(dbg_info.cu_info, dw_info.cu_info);
        // type情報
        adapt_value(dbg_info.type, dw_info.type);
        // decl_*情報
        adapt_decl_info(dbg_info, dw_info);

        switch (dw_info.tag) {
            case type_tag::base:
                return adapt_info_base(dbg_info, dw_info);

            case type_tag::func:
                return adapt_info_func(dbg_info, dw_info);

            case type_tag::typedef_:
                return adapt_info_typedef(dbg_info, dw_info);

            case type_tag::struct_:
            case type_tag::union_:
                return adapt_info_struct_union(dbg_info, dw_info);

            case type_tag::array:
                return adapt_info_array(dbg_info, dw_info);

            case type_tag::pointer:
                return adapt_info_pointer(dbg_info, dw_info);

            case type_tag::const_:
                return adapt_info_const(dbg_info, dw_info);

            case type_tag::restrict_:
                return adapt_info_restrict(dbg_info, dw_info);

            case type_tag::volatile_:
                return adapt_info_volatile(dbg_info, dw_info);

            case type_tag::enum_:
                return adapt_info_enum(dbg_info, dw_info);

            case type_tag::member:
                return adapt_info_member(dbg_info, dw_info);

            case type_tag::reference:
                return adapt_info_reference(dbg_info, dw_info);

            case type_tag::parameter:
                return adapt_info_parameter(dbg_info, dw_info);

            case type_tag::subrange:
                return adapt_info_subrange(dbg_info, dw_info);

            default:
                // 実装忘れ
                fprintf(stderr, "no impl : build_node : 0x%02X\n", dw_info.tag);
                break;
        }

        return false;
    }

    void adapt_info_fix(type_info &dbg_info) {
        // 後処理
        // void型ケア
        // tagがnoneのときはvoid型？
        if (dbg_info.tag == 0) {
            dbg_info.tag = type_tag::none;
        }
        // unnamedのとき
        // 下記のようなケースがある。
        //   typedef struct {} type;
        //   enum {};
        // typedef出現時にnullptrであれば上書き判定が簡単になるので、nullptrのままにしておく
        // nullptr==unnamedとして後で処理
        // if (dbg_info.name == nullptr) {
        //     dbg_info.name = &name_void;

        //     // pointer size
        //     if (dbg_info.byte_size == 0) {
        //         dbg_info.byte_size = dw_info_.machine_arch.obj_pointersize;
        //     }
        // }
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

    void adapt_decl_info(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // decl_*情報
        adapt_value(dbg_info.decl_file, dw_info.decl_file);
        adapt_value(dbg_info.decl_line, dw_info.decl_line);
        adapt_value(dbg_info.decl_column, dw_info.decl_column);
        // 文字列情報
        if (dbg_info.decl_file_path == nullptr && dw_info.decl_file_path.size() > 0) {
            dbg_info.decl_file_path = &(dw_info.decl_file_path);
        }
        if (dbg_info.decl_file_path_rel == nullptr && dw_info.decl_file_path_rel.size() > 0) {
            dbg_info.decl_file_path_rel = &(dw_info.decl_file_path_rel);
        }
    }
    void adapt_decl_info_force(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // decl_*情報
        adapt_value_force(dbg_info.decl_file, dw_info.decl_file);
        adapt_value_force(dbg_info.decl_line, dw_info.decl_line);
        adapt_value_force(dbg_info.decl_column, dw_info.decl_column);
        // 文字列情報
        if (dw_info.decl_file_path.size() > 0) {
            dbg_info.decl_file_path = &(dw_info.decl_file_path);
        }
        if (dw_info.decl_file_path_rel.size() > 0) {
            dbg_info.decl_file_path_rel = &(dw_info.decl_file_path_rel);
        }
    }

    bool adapt_info_base(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.encoding, dw_info.encoding);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }

    bool adapt_info_enum(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }

    bool adapt_info_member(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        bool is_comple = true;
        adapt_decl_info_force(dbg_info, dw_info);
        // 対象データが空ならdw_infoを反映する
        adapt_value_force(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.bit_offset, dw_info.bit_offset);
        adapt_value(dbg_info.bit_size, dw_info.bit_size);
        adapt_value(dbg_info.data_bit_offset, dw_info.data_bit_offset);
        adapt_value(dbg_info.data_member_location, dw_info.data_member_location);
        // 型情報があれば取得
        // 不完全型かどうかを戻り値で返す
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
            is_comple         = dbg_info.sub_info->build_state == build_type_state::Complete;
        }
        //
        dbg_info.tag |= dw_info.tag;
        //
        return is_comple;
    }

    bool adapt_info_reference(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        bool is_comple = true;
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        // 型情報があれば取得
        // 不完全型かどうかを戻り値で返す
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
            is_comple         = dbg_info.sub_info->build_state == build_type_state::Complete;
        }
        //
        dbg_info.tag |= dw_info.tag;
        //
        return is_comple;
    }

    bool adapt_info_func(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        bool is_comple = true;
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
            auto it = dw_info.param_list.begin();
            if (it != dw_info.param_list.end()) {
                auto param_offset = *it;
                auto var_info_it  = dw_info_.var_tbl.container.find(param_offset);
                if (var_info_it != dw_info_.var_tbl.container.end()) {
                    auto dbg_param = get_type_info(*var_info_it->second.type);
                    switch (dbg_param->build_state) {
                        case build_type_state::Complete:
                            // 構築済みデータが取得出来ればOK
                            std::format_to(inserter, "{}", *dbg_param->name);
                            break;

                        case build_type_state::None:
                        case build_type_state::Building:
                        case build_type_state::Incomplete:
                        default:
                            // それ以外は終了
                            is_comple = false;
                            break;
                    }
                }
                it++;
                for (; is_comple && it != dw_info.param_list.end(); it++) {
                    param_offset = *it;
                    var_info_it  = dw_info_.var_tbl.container.find(param_offset);
                    if (var_info_it != dw_info_.var_tbl.container.end()) {
                        auto dbg_param = get_type_info(*var_info_it->second.type);
                        switch (dbg_param->build_state) {
                            case build_type_state::Complete:
                                // 構築済みデータが取得出来ればOK
                                std::format_to(inserter, ", {}", *dbg_param->name);
                                break;

                            case build_type_state::None:
                            case build_type_state::Building:
                            case build_type_state::Incomplete:
                            default:
                                // それ以外は終了
                                is_comple = false;
                                break;
                        }
                    }
                }
            }
            std::format_to(inserter, ")");
        }
        dbg_info.name = &dw_info.name;
        //
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.param_list, dw_info.child_list);
        //
        dbg_info.tag |= dw_info.tag;
        //
        if (!is_comple) {
            dw_info.name = "<funcptr>";
        }
        return is_comple;
    }

    bool adapt_info_parameter(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        bool is_comple = true;
        //
        adapt_value(dbg_info.name, dw_info.name);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
            is_comple         = dbg_info.sub_info->build_state == build_type_state::Complete;
        }
        //
        dbg_info.tag |= dw_info.tag;
        //
        return is_comple;
    }

    bool adapt_info_typedef(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        if (opt_.is_prior_typedef) {
            // typedef名称を優先する場合
            // dw_infoがtypedef名称になるので優先する
            adapt_value_force(dbg_info.name, dw_info.name);
        } else {
            // typedefより定義元の名前を優先する場合
            adapt_value(dbg_info.name, dw_info.name);
        }
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }

    bool adapt_info_struct_union(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        // 対象データが空ならdw_infoを反映する
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.byte_size, dw_info.byte_size);
        adapt_value(dbg_info.member_list, dw_info.child_list);
        adapt_value(dbg_info.has_bitfield, dw_info.has_bitfield);
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }

    bool adapt_info_array(type_info &dbg_info, dwarf_info::type_info &dw_info) {
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
        //
        return true;
    }

    bool adapt_info_subrange(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        bool is_comple = true;
        //
        adapt_value(dbg_info.name, dw_info.name);
        adapt_value(dbg_info.count, dw_info.count);
        //
        if (dw_info.type) {
            dbg_info.sub_info = get_type_info(*dw_info.type);
            is_comple         = dbg_info.sub_info->build_state == build_type_state::Complete;
        }
        //
        dbg_info.tag |= dw_info.tag;
        //
        return is_comple;
    }

    bool adapt_info_pointer(type_info &dbg_info, dwarf_info::type_info &dw_info) {
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
        //
        return true;
    }

    bool adapt_info_const(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_const = true;
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }
    bool adapt_info_restrict(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_restrict = true;
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }
    bool adapt_info_volatile(type_info &dbg_info, dwarf_info::type_info &dw_info) {
        //
        dbg_info.is_volatile = true;
        //
        dbg_info.tag |= dw_info.tag;
        //
        return true;
    }

    void adapt_value(dwarf_info::compile_unit_info *&dst, dwarf_info::compile_unit_info *&src) {
        if (dst == nullptr && src != nullptr) {
            dst = src;
        }
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
    void adapt_value_force(Dwarf_Unsigned &dst, Dwarf_Unsigned &src) {
        if (src != 0) {
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

        // 定義位置情報
        Dwarf_Unsigned var_decl_file;
        Dwarf_Unsigned var_decl_line;
        Dwarf_Unsigned var_decl_column;
        std::string *var_decl_file_path;
        std::string *var_decl_file_path_rel;
        Dwarf_Unsigned type_decl_file;
        Dwarf_Unsigned type_decl_line;
        Dwarf_Unsigned type_decl_column;
        std::string *type_decl_file_path;
        std::string *type_decl_file_path_rel;
        //
        dwarf_info::compile_unit_info *cu_info;

        size_t pointer_depth;
        bool is_struct;
        bool is_union;
        bool is_enum;
        bool is_struct_member;
        bool is_union_member;
        bool is_array;
        bool is_bitfield;
        bool is_const;

        //
        bool is_unnamed;

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
              var_decl_file(0),
              var_decl_line(0),
              var_decl_column(0),
              var_decl_file_path(nullptr),
              var_decl_file_path_rel(nullptr),
              type_decl_file(0),
              type_decl_line(0),
              type_decl_column(0),
              type_decl_file_path(nullptr),
              type_decl_file_path_rel(nullptr),
              cu_info(nullptr),
              pointer_depth(0),
              is_struct(false),
              is_union(false),
              is_enum(false),
              is_struct_member(false),
              is_union_member(false),
              is_array(false),
              is_bitfield(false),
              is_const(false),
              is_unnamed(false) {
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
        // 本館数内のdump処理では、このvar_nameのインスタンスを使いまわす
        // string内のバッファを維持してnew/deleteを繰り返さない
        std::string var_name = "";
        bool result;

        // global_varをすべてチェック
        for (auto &[addr, var] : var_tbl) {
            // 対応するtypeを取得
            if (var->type) {
                auto it = type_map.find(*(var->type));
                if (it != type_map.end()) {
                    // typeが存在するとき、変数情報のdump実行
                    var_name.clear();
                    result = lookup_var(*var, it->second, var_name, 0, func);
                    if (!result) {
                        break;
                    }
                }
            }
        }
    }

    struct func_info_view
    {
        std::string *tag_type;
        std::string *tag_name;
        std::string *tag_decl_file_path;
        std::string *tag_decl_file_path_rel;
        dwarf_info::compile_unit_info *cu_info;

        Dwarf_Unsigned low_pc;
        Dwarf_Unsigned high_pc;
        bool is_declaration;
        bool has_definition;

        func_info_view()
            : tag_type(nullptr),
              tag_name(nullptr),
              tag_decl_file_path(nullptr),
              tag_decl_file_path_rel(nullptr),
              cu_info(nullptr),
              low_pc(0),
              high_pc(0),
              is_declaration(false),
              has_definition(false) {
        }
    };

    void get_func_info(std::function<bool(func_info_view &)> &&func) {
        bool result;

        // funcをすべてチェック
        auto &dw_func_tbl = dw_info_.func_tbl.container;
        for (auto &[addr, func_info] : dw_func_tbl) {
            func_info_view view;
            view.tag_name               = &func_info.name;
            view.tag_decl_file_path     = &func_info.decl_file_path;
            view.tag_decl_file_path_rel = &func_info.decl_file_path_rel;
            view.cu_info                = func_info.cu_info;
            view.has_definition         = func_info.has_definition;
            view.is_declaration         = func_info.declaration;
            //
            result = func(view);
            if (!result) {
                break;
            }
        }
    }

private:
    void make_type_tag(var_info_view &view, type_info &type) {
        // pointer
        view.pointer_depth = type.pointer_depth;
        // type情報タグを作成
        // 型がnamedならそのまま名前として設定
        if (type.name != nullptr) {
            view.tag_type = type.name;
        }
        // typeに応じた付加情報をセット
        if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
            view.is_struct = true;
            // unnamedの場合
            if (type.name == nullptr) {
                view.tag_type   = &name_unnamed;
                view.is_unnamed = true;
            }
        } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
            view.is_union = true;
            // unnamedの場合
            if (type.name == nullptr) {
                view.tag_type   = &name_unnamed;
                view.is_unnamed = true;
            }
        } else if ((type.tag & util_dwarf::debug_info::type_tag::enum_) != 0) {
            view.is_enum = true;
            // unnamedの場合
            if (type.name == nullptr) {
                view.tag_type   = &name_unnamed;
                view.is_unnamed = true;
            }
        } else {
            // ありえない？
            if (type.name == nullptr) {
                view.tag_type   = &name_void;
                view.is_unnamed = true;
            }
        }

        //
        view.encoding = type.encoding;
    }

    template <typename Func>
    bool lookup_var(var_info &var, type_info &type, std::string &var_name, size_t depth, Func &&func) {
        bool result;
        var_info_view view;
        Dwarf_Off address;
        // 型タグ作成
        make_type_tag(view, type);
        // 表示名作成
        std::format_to(std::back_inserter(var_name), "{}", *var.name);
        // アドレス計算
        if (var.location && (*var.location)->is_immediate) {
            address = std::get<Dwarf_Off>((*var.location)->value);
        } else {
            address = 0;
        }
        // view作成
        view.tag_name = &var_name;
        view.is_const = type.is_const;
        // decl_*
        view.var_decl_file           = var.decl_file;
        view.var_decl_line           = var.decl_line;
        view.var_decl_column         = var.decl_column;
        view.var_decl_file_path      = var.decl_file_path;
        view.var_decl_file_path_rel  = var.decl_file_path_rel;
        view.type_decl_file          = type.decl_file;
        view.type_decl_line          = type.decl_line;
        view.type_decl_column        = type.decl_column;
        view.type_decl_file_path     = type.decl_file_path;
        view.type_decl_file_path_rel = type.decl_file_path_rel;
        //
        view.cu_info = var.cu_info;

        if ((type.tag & util_dwarf::debug_info::type_tag::array) != 0) {
            // 配列のとき
            result = lookup_var_impl_array(view, type, address, var_name, depth, func);
        } else {
            // 配列以外のとき
            result = lookup_var_impl_default(view, type, address, var_name, depth, func);
        }

        //
        var_name.clear();

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

        // struct or union判定
        bool is_struct_union = false;
        if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
            is_struct_union = false;
        } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
            is_struct_union = true;
        } else {
            // ありえない?
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
            result = lookup_var_member_each(member, *member_type, base_address, prefix, depth, func, is_struct_union);
            if (!result) {
                break;
            }
        }

        return result;
    }
    template <typename Func>
    bool lookup_var_member_each(type_info &member, type_info &type, Dwarf_Off base_address, std::string &var_name, size_t depth, Func &func,
                                bool is_struct_union) {
        bool result;
        Dwarf_Off address;
        var_info_view view;
        // 型タグ作成
        make_type_tag(view, type);
        // アドレス計算
        address = base_address + member.data_member_location;
        // view作成
        view.tag_name         = &var_name;
        view.is_struct_member = !is_struct_union;
        view.is_union_member  = is_struct_union;
        view.is_const         = type.is_const;
        // decl_*
        view.var_decl_file           = member.decl_file;
        view.var_decl_line           = member.decl_line;
        view.var_decl_column         = member.decl_column;
        view.var_decl_file_path      = member.decl_file_path;
        view.var_decl_file_path_rel  = member.decl_file_path_rel;
        view.type_decl_file          = type.decl_file;
        view.type_decl_line          = type.decl_line;
        view.type_decl_column        = type.decl_column;
        view.type_decl_file_path     = type.decl_file_path;
        view.type_decl_file_path_rel = type.decl_file_path_rel;
        //
        view.cu_info = member.cu_info;

        // prefix部分の末尾を記憶しておく
        auto org_end = var_name.size();
        // 表示名作成
        std::format_to(std::back_inserter(var_name), ".{}", *member.name);

        if (member.bit_size == 0) {
            // bit_sizeがゼロならビットフィールドでない
            if ((member.tag & util_dwarf::debug_info::type_tag::array) != 0) {
                result = lookup_var_impl_array(view, member, address, var_name, depth, func);
            } else {
                result = lookup_var_impl_default(view, member, address, var_name, depth, func);
            }

        } else {
            // bit_sizeがゼロ以外ならビットフィールド
            result = lookup_var_impl_bitfield(view, member, address, var_name, depth, func);
        }

        // 本関数で追加した文字列を削除して終了
        var_name.erase(org_end);

        return result;
    }

    template <typename Func>
    bool lookup_var_impl_default(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &var_name, size_t depth, Func &func) {
        bool result;

        // view作成
        view.address    = base_address;
        view.byte_size  = type.byte_size;
        view.bit_offset = 0;
        view.bit_size   = 0;
        view.is_const   = type.is_const;

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
    bool lookup_var_impl_bitfield(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string & /*var_name*/, size_t /*depth*/,
                                  Func &func) {
        bool result;
        Dwarf_Off address;
        Dwarf_Unsigned bit_offset;

        // アドレス計算
        address    = base_address + (type.bit_offset / 8);
        bit_offset = type.bit_offset % 8;

        // view作成
        view.address     = address;
        view.is_bitfield = true;
        view.byte_size   = 0;
        view.bit_offset  = bit_offset;
        view.bit_size    = type.bit_size;
        view.is_const    = type.is_const;

        // コールバック
        result = func(view);
        return result;
    }

    template <typename Func>
    bool lookup_var_impl_array(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &var_name, size_t depth, Func &func) {
        bool result = true;
        // 多次元配列ケアのために再帰的コールして変数名を作成する
        // 末尾まで到達したら変数の内容をコールバックする
        // arrayなら必ずsubrangeを持つはずだが一応チェック
        if (type.array_range_list != nullptr && type.array_range_list->size() > 0) {
            // インデックス作成
            auto it = type.array_range_list->begin();
            result  = lookup_var_impl_array_idx(view, type, base_address, var_name, depth, it, func);
        }

        return result;
    }

    template <typename Func>
    bool lookup_var_impl_array_idx(var_info_view &view, type_info &type, Dwarf_Off base_address, std::string &var_name, size_t depth,
                                   type_info::child_list_t::iterator array_d_it, Func &func) {
        Dwarf_Off address;
        bool result = true;

        // 本コールでの配列次元情報
        auto curr_d = *array_d_it;
        // 次の配列次元情報へのiteratorを作成
        array_d_it++;
        // array_d次の要素数から名称作成
        for (size_t i = 0; i < curr_d->count; i++) {
            address = base_address + (i * curr_d->byte_size);

            // prefix部分の末尾を記憶しておく
            auto org_end = var_name.size();
            // 表示名作成
            if (opt_.is_expand_array) {
                // array展開あり
                std::format_to(std::back_inserter(var_name), "[{}]", i);
            } else {
                // array展開なし
                std::format_to(std::back_inserter(var_name), "[{}]", curr_d->count);
            }

            if (array_d_it == type.array_range_list->end()) {
                // 最終次なら変数内容を表示
                result = lookup_var_impl_array_data(view, type, address, var_name, depth, func);
            } else {
                // 最終次でないなら名称作成を継続
                result = lookup_var_impl_array_idx(view, type, address, var_name, depth, array_d_it, func);
            }

            // 今回追加した文字列を削除
            var_name.erase(org_end);

            //
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

        // アドレス計算
        view.address = base_address;
        //
        view.is_array   = true;
        view.byte_size  = type.byte_size;
        view.bit_offset = 0;
        view.bit_size   = 0;
        view.is_const   = type.is_const;

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
