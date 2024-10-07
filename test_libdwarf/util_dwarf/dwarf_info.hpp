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

namespace attr_class {
// attribute_class

using block = std::vector<uint8_t>;

}  // namespace attr_class

struct dwarf_info
{
    // 変数情報
    struct var_info
    {
        std::string name;
        std::string linkage_name;
        bool external;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        std::optional<Dwarf_Off> type;  // reference
        std::optional<dw_op_value> location;
        bool declaration;  // 不完全型のときtrue
        Dwarf_Unsigned const_value;
        Dwarf_Unsigned sibling;
        Dwarf_Unsigned endianity;                // DW_END_*
        std::optional<Dwarf_Off> specification;  // 分割定義offset, offsetが指すDIEに情報を付与する

        // 付加情報
        bool is_parameter;
        bool is_local_var;

        var_info()
            : name(),
              linkage_name(),
              external(false),
              decl_file(0),
              decl_line(0),
              decl_column(0),
              type(),
              location(),
              declaration(false),
              const_value(0),
              sibling(0),
              endianity(0),
              specification(),
              is_parameter(false),
              is_local_var(false) {
        }
        ~var_info() {
        }
    };

    // 型タグ
    struct type_tag
    {
        using type = uint16_t;

        static constexpr type none      = 0x0000;
        static constexpr type base      = 0x0001;
        static constexpr type array     = 0x0002;
        static constexpr type struct_   = 0x0004;
        static constexpr type union_    = 0x0008;
        static constexpr type func      = 0x0010;
        static constexpr type parameter = 0x0020;
        static constexpr type typedef_  = 0x0040;
        static constexpr type const_    = 0x0080;
        static constexpr type volatile_ = 0x0100;
        static constexpr type pointer   = 0x0200;
        static constexpr type restrict_ = 0x0400;
        static constexpr type enum_     = 0x0800;
        static constexpr type reference = 0x1000;
        static constexpr type member    = 0x2000;  // struct,union,classのメンバ
        static constexpr type subrange  = 0x4000;  // 配列要素数

        static constexpr type struct_union = struct_ | union_;
        static constexpr type func_ptr     = func | pointer;
    };

    struct type_info
    {
        uint16_t tag;
        Dwarf_Off offset;  // 自分自身のglobal offset

        std::string name;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        bool declaration;                  // 不完全型のときtrue
        std::optional<Dwarf_Off> type;     // reference
        std::optional<Dwarf_Off> sibling;  //

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
        bool prototyped;
        bool artificial;

        // memberも単体でtype_mapに登録するのでchild_listは参照用のポインタでいい
        using child_node_t = type_info *;
        using child_list_t = std::list<child_node_t>;
        child_list_t child_list;

        // subroutine_typeの場合にparameter変数情報が付与される
        // parameter変数も変数テーブルに登録して、Offsetをparameter情報として記憶しておく
        using param_node_t = Dwarf_Off;
        using param_list_t = std::list<param_node_t>;
        param_list_t param_list;

        // メンバ関数参照テーブル
        // func_tblに登録した関数へのオフセット参照
        using func_node_t = Dwarf_Off;
        using func_list_t = std::list<func_node_t>;
        func_list_t member_func_list;

        // 付加情報
        bool has_bitfield;

        type_info()
            : tag(0),
              name(),
              decl_file(0),
              decl_line(0),
              decl_column(0),
              declaration(false),
              type(),
              sibling(),
              byte_size(0),
              bit_offset(0),
              bit_size(0),
              data_bit_offset(0),
              data_member_location(0),
              binary_scale(0),
              signature(0),
              accessibility(0),
              encoding(0),
              endianity(0),
              prototyped(false),
              artificial(false),
              child_list(),
              param_list(),
              has_bitfield(false) {
        }
        ~type_info() {
        }
    };

    // 関数情報
    struct func_info
    {
        std::string name;
        std::string linkage_name;
        bool external;
        Dwarf_Unsigned decl_file;  // filelistのインデックス
        Dwarf_Unsigned decl_line;
        Dwarf_Unsigned decl_column;
        std::optional<Dwarf_Unsigned> low_pc;
        std::optional<Dwarf_Unsigned> high_pc;
        std::optional<dw_op_value> return_addr;
        std::optional<dw_op_value> frame_base;
        std::optional<Dwarf_Off> type;  // reference
        std::optional<Dwarf_Off> location;
        bool declaration;  // 関数宣言のみが存在するときtrue?
        Dwarf_Unsigned const_value;
        Dwarf_Unsigned sibling;
        Dwarf_Unsigned endianity;                // DW_END_*
        std::optional<Dwarf_Off> specification;  // 分割定義offset, offsetが指すDIEに情報を付与する

        // parameter/local変数も変数テーブルに登録して、Offsetをparameter情報として記憶しておく
        using var_node_t = Dwarf_Off;
        using var_list_t = std::list<var_node_t>;
        var_list_t param_list;
        var_list_t local_var_list;

        // 付加情報
        std::string decl_file_name;
        bool has_definition;  // 関数定義あり？

        func_info()
            : name(),
              linkage_name(),
              external(false),
              decl_file(0),
              decl_line(0),
              decl_column(0),
              low_pc(),
              high_pc(),
              return_addr(),
              frame_base(),
              type(),
              location(),
              declaration(false),
              const_value(0),
              sibling(0),
              endianity(0),
              specification(),
              param_list(),
              local_var_list(),
              decl_file_name(),
              has_definition(false) {
        }
        ~func_info() {
        }
    };

    // 情報コンテナ
    template <typename T>
    class info_container {
    public:
        // 型情報
        using container_t = std::map<Dwarf_Off, T>;
        container_t container;

    public:
        info_container() : container() {
        }
        ~info_container() {
        }

        // type_map操作関数
        T &make_new_info(Dwarf_Off offset) {
            auto result = container.try_emplace(offset, T());
            return result.first->second;
        }
    };
    // 変数情報リスト
    using var_info_container = info_container<var_info>;
    // 型情報リスト
    using type_info_container = info_container<type_info>;
    // 関数情報リスト
    using func_info_container = info_container<func_info>;

    // compile_unitから取得する情報
    struct cu_info
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
        Dwarf_Off cu_header_offset;
        Dwarf_Off cu_length;
        // DW_AT_* info
        std::string name;
        std::string producer;
        Dwarf_Unsigned language;
        Dwarf_Off stmt_list;
        std::string comp_dir;
        std::optional<Dwarf_Unsigned> low_pc;
        std::optional<Dwarf_Unsigned> high_pc;
        Dwarf_Unsigned ranges;  // .debug_rangesへの参照

        cu_info()
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
              cu_offset(0),
              cu_header_offset(0),
              cu_length(0),
              name(),
              producer(),
              language(),
              stmt_list(0),
              comp_dir(),
              low_pc(),
              high_pc() {
        }
        ~cu_info() {
        }
    };

    // elf machine_architectureデータ
    elf::machine_architecture machine_arch;
    arch::arch_info *arch_info;

    // DIE解析情報
    var_info_container var_tbl;
    type_info_container type_tbl;
    func_info_container func_tbl;

    // 必要ならバッファするように変更
    // cu_info_container cu_tbl;

    dwarf_info() : machine_arch(), arch_info(nullptr) {
    }
    ~dwarf_info() {
    }
};

}  // namespace util_dwarf
