#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "architecture.hpp"
#include "dwarf_expression.hpp"

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

// compile_unitから取得する情報
struct dwarf_info
{
    // 変数情報
    struct var_info
    {
        std::string name;
        bool external;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        bool decl_file_is_external;
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        std::optional<Dwarf_Off> type;  // reference
        std::optional<Dwarf_Off> location;
        bool declaration;  // 不完全型のときtrue
        Dwarf_Unsigned const_value;
        Dwarf_Unsigned sibling;
        Dwarf_Unsigned endianity;  // DW_END_*
    };

    // 変数情報リスト
    class var_info_container {
    public:
        // type def
        using var_list_node_t = std::unique_ptr<var_info>;
        using var_list_t      = std::vector<var_list_node_t>;

        var_list_t var_list;

    public:
        var_info_container() {
        }

        // var_list操作関数
        var_list_node_t make_new_var_info() {
            return std::make_unique<var_info>();
        }
        void add(var_list_node_t &&node) {
            var_list.push_back(std::move(node));
        }
    };

    // 型タグ
    enum class type_tag : uint16_t
    {
        none      = 0x0000,
        base      = 0x0001,
        array     = 0x0002,
        struct_   = 0x0004,
        union_    = 0x0008,
        func      = 0x0010,
        parameter = 0x0020,
        typedef_  = 0x0040,
        const_    = 0x0080,
        volatile_ = 0x0100,
        pointer   = 0x0200,
        restrict_ = 0x0400,
        enum_     = 0x0800,
        reference = 0x1000,
        member    = 0x2000,  // struct,union,classのメンバ

        func_ptr = func | pointer,
    };

    struct type_info
    {
        uint16_t tag;
        std::string name;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        bool decl_file_is_external;
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        Dwarf_Unsigned sibling;
        bool declaration;               // 不完全型のときtrue
        std::optional<Dwarf_Off> type;  // reference

        Dwarf_Unsigned byte_size;
        Dwarf_Unsigned bit_offset;
        Dwarf_Unsigned bit_size;
        Dwarf_Unsigned data_bit_offset;
        Dwarf_Off data_member_location;
        Dwarf_Unsigned binary_scale;
        Dwarf_Unsigned signature;
        Dwarf_Unsigned accessibility;
        std::optional<Dwarf_Unsigned> count;
        std::optional<Dwarf_Unsigned> upper_bound;
        std::optional<Dwarf_Unsigned> lower_bound;
        std::optional<Dwarf_Unsigned> address_class;
        Dwarf_Unsigned encoding;   // DW_ATE_*
        Dwarf_Unsigned endianity;  // DW_END_*

        // memberも単体でtype_mapに登録するのでchild_listは参照用のポインタでいい
        using child_node_t = type_info *;
        using child_list_t = std::list<child_node_t>;
        child_list_t child_list;
    };

    // 型情報リスト
    class type_info_container {
    public:
        // 型情報
        using type_map_t = std::map<Dwarf_Off, type_info>;

        type_map_t type_map;

    public:
        type_info_container() {
        }

        // type_map操作関数
        type_info &make_new_type_info(Dwarf_Off offset, type_tag tag) {
            auto result = type_map.try_emplace(offset, type_info());
            result.first->second.tag |= (uint16_t)tag;
            return result.first->second;
        }

        // DIEから収集したデータは木構造で情報が分散している
        // ルートオブジェクトに情報を集約して型情報を単一にする
        void build() {
            for (auto &elem : type_map) {
                build_node(elem.second);
            }
        }

        void build_node(type_info &info) {
            // debug
            if (info.tag == (uint16_t)type_tag::typedef_) {
                info;
            }

            // type(offset)を辿って型情報を更新していく
            // child要素が無ければ終了
            type_info *curr_info = &info;
            while (curr_info->type) {
                // child typeの存在チェック
                // 無ければ終了
                auto it = type_map.find(*(curr_info->type));
                if (it == type_map.end()) {
                    break;
                }
                //
                auto &child = it->second;
                switch (child.tag) {
                    case (uint16_t)type_tag::base:
                        adapt_info_base(info, child);
                        break;

                    default:
                        // 実装忘れ
                        printf("no impl : build_node : 0x%02X\n", child.tag);
                        break;
                }

                // 次ノード更新
                curr_info = &child;
            }
        }

    private:
        void adapt_info_base(type_info &dst, type_info &src) {
            //
            adapt_value(dst.name, src.name);
            adapt_value(dst.encoding, src.encoding);
            adapt_value(dst.byte_size, src.byte_size);
            //
            dst.tag |= src.tag;
        }

        void adapt_value(std::string &dst, std::string &src) {
            if (dst.size() == 0) {
                dst = src;
            }
        }
        void adapt_value(Dwarf_Unsigned &dst, Dwarf_Unsigned &src) {
            if (dst == 0) {
                dst = src;
            }
        }
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

    // elf machine_architectureデータ
    elf::machine_architecture machine_arch;
    arch::arch_info *arch_info;

    // DIE解析情報
    var_info_container global_var_tbl;
    type_info_container type_tbl;

    // 必要ならバッファするように変更
    // compile_unit_info cu_info;

    dwarf_info() : machine_arch(), arch_info(nullptr) {
    }
};

}  // namespace util_dwarf
