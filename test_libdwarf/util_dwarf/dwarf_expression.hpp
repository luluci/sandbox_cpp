#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <variant>
#include <vector>

#include "LEB128.hpp"
#include "utility.hpp"

namespace util_dwarf {

class dw_op_value {
public:
    using value_type = std::variant<Dwarf_Unsigned, Dwarf_Signed>;
    value_type value;
    std::vector<uint8_t> expr;
    bool is_immediate;

    dw_op_value() : value(static_cast<Dwarf_Unsigned>(0)), is_immediate(true) {
    }
    dw_op_value(Dwarf_Unsigned val) : value(val), is_immediate(true) {
    }
    dw_op_value(Dwarf_Signed val) : value(val), is_immediate(true) {
    }
    dw_op_value(uint8_t const *buff, size_t buff_size) : expr(&buff[0], &buff[buff_size]) {
    }
};

// Dwarf expression 計算機
class dwarf_expression {
    using value_t = std::variant<Dwarf_Unsigned, Dwarf_Signed>;
    using stack_t = std::vector<value_t>;
    stack_t stack_;
    static constexpr size_t default_stack_size = 10;
    size_t pointer_size_;

public:
    dwarf_expression() : stack_(), pointer_size_(4) {
        stack_.reserve(default_stack_size);
    }

    void pointer_size(size_t size) {
        pointer_size_ = size;
    }

    bool eval_DW_OP_unimpl(uint8_t ope, uint8_t *, size_t buff_size, size_t &) {
        //
        fprintf(stderr, "no implemented! : DW_OP(0x%02X), ope_size=%lld\n", ope, buff_size);
        return false;
    }
    //
    bool eval_DW_OP_addr(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Unsigned value = 0;

        if (buff_pos + pointer_size_ > buff_size) {
            fprintf(stderr, "error: eval_DW_OP_addr : invalid buff_size(%lld) require %lld bytes\n", buff_size, buff_pos + pointer_size_);
            return false;
        }
        // N byteのデータを取得する
        value = utility::concat_le<Dwarf_Unsigned>(buff, buff_pos, buff_pos + pointer_size_);
        stack_.push_back(value);
        buff_pos += pointer_size_;
        //
        return true;
    }
    //
    bool eval_DW_OP_plus_uconst(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Unsigned value = 0;

        // stackに値が積まれていたら取得して加算する
        auto pop_value = pop<Dwarf_Unsigned>();
        if (pop_value) {
            value = *pop_value;
        }

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        ULEB128 leb(&buff[buff_pos], buff_size - buff_pos);
        value += leb.value;
        buff_pos += leb.used_bytes;
        //
        stack_.push_back(value);
        //
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_lit_N() {
        stack_.push_back(N);
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_breg_N(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Signed value = 0;

        // no impl!

        // register_N から値を取り出す
        // レジスタが64bit以上の場合はオーバーフロー注意
        // value = register;
        // SLEB128のoffsetを適用する
        SLEB128 leb(&buff[buff_pos], buff_size - buff_pos);
        value += leb.value;

        stack_.push_back(static_cast<Dwarf_Unsigned>(value));
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_bregx(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Signed value = 0;
        size_t reg_no;

        // no impl!

        // registerを指定するULEB128を取得
        ULEB128 uleb(&buff[buff_pos], buff_size - buff_pos);
        reg_no = uleb.value;
        // ULEB128の後ろにSLEB128
        buff_pos += uleb.used_bytes;

        // register_N から値を取り出す
        // レジスタが64bit以上の場合はオーバーフロー注意
        // value = register;
        // SLEB128のoffsetを適用する
        SLEB128 leb(&buff[buff_pos], buff_size - buff_pos);
        value += leb.value;
        buff_pos += uleb.used_bytes;

        stack_.push_back(static_cast<Dwarf_Unsigned>(value));
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_reg_N() {
        Dwarf_Unsigned value = 0;

        // no impl!

        // register_N から値を取り出す
        // value = register;

        stack_.push_back(value);
        return true;
    }
    //
    template <typename T, size_t N>
    bool eval_DW_OP_const_N(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        T value = 0;

        if (buff_pos + N > buff_size) {
            fprintf(stderr, "error: DW_OP_const_N : invalid buff_size(%lld) require %lld bytes\n", buff_size, 1 + N);
            return false;
        }
        // N byteのデータを取得する
        value = utility::concat_le<T>(buff, buff_pos, buff_pos + N);
        stack_.push_back(value);
        buff_pos += N;
        //
        return true;
    }
    //
    bool eval_DW_OP_constu(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Unsigned value = 0;

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        ULEB128 leb(&buff[buff_pos], buff_size - buff_pos);
        value += leb.value;
        //
        stack_.push_back(value);
        buff_pos += leb.used_bytes;
        //
        return true;
    }
    //
    bool eval_DW_OP_consts(uint8_t *buff, size_t buff_size, size_t &buff_pos) {
        Dwarf_Signed value = 0;

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        SLEB128 leb(&buff[buff_pos], buff_size - buff_pos);
        value += leb.value;
        //
        stack_.push_back(value);
        buff_pos += leb.used_bytes;
        //
        return true;
    }

    template <typename T, typename Result = std::optional<T>>
    Result pop() {
        if (stack_.size() > 0) {
            // スタックから値をpop
            Result result;
            value_t &value = stack_.back();
            // if (std::holds_alternative<T>(value)) {
            //     result = std::get<T>(value);
            // }
            // variantから値を取り出す
            // signedからunsignedのどちらかになるが、暫定でTにキャストしてしまう
            result = std::visit([](const auto &x) -> T { return static_cast<T>(x); }, value);
            stack_.pop_back();
            return result;
        }
        return std::nullopt;
    }

    std::optional<dw_op_value> eval(uint8_t *buff, size_t buff_size) {
        // バッファなしはエラーとする
        if (buff_size == 0) {
            return std::nullopt;
        }
        bool is_ok        = true;
        bool is_immediate = true;
        size_t buff_pos   = 0;
        while (is_ok && is_immediate && buff_pos < buff_size) {
            // opeコード取り出し
            // 1バイト目にあるものとする
            uint8_t ope = buff[buff_pos];
            buff_pos++;

            // opeコード操作
            // すべてstackに対するアクションになるはず
            switch (ope) {
                case DW_OP_addr:
                    is_ok = eval_DW_OP_addr(buff, buff_size, buff_pos);
                    break;
                case DW_OP_lit0:
                    is_ok = eval_DW_OP_lit_N<0>();
                    break;
                case DW_OP_lit1:
                    is_ok = eval_DW_OP_lit_N<1>();
                    break;
                case DW_OP_lit2:
                    is_ok = eval_DW_OP_lit_N<2>();
                    break;
                case DW_OP_lit3:
                    is_ok = eval_DW_OP_lit_N<3>();
                    break;
                case DW_OP_lit4:
                    is_ok = eval_DW_OP_lit_N<4>();
                    break;
                case DW_OP_lit5:
                    is_ok = eval_DW_OP_lit_N<5>();
                    break;
                case DW_OP_lit6:
                    is_ok = eval_DW_OP_lit_N<6>();
                    break;
                case DW_OP_lit7:
                    is_ok = eval_DW_OP_lit_N<7>();
                    break;
                case DW_OP_lit8:
                    is_ok = eval_DW_OP_lit_N<8>();
                    break;
                case DW_OP_lit9:
                    is_ok = eval_DW_OP_lit_N<9>();
                    break;
                case DW_OP_lit10:
                    is_ok = eval_DW_OP_lit_N<10>();
                    break;
                case DW_OP_lit11:
                    is_ok = eval_DW_OP_lit_N<11>();
                    break;
                case DW_OP_lit12:
                    is_ok = eval_DW_OP_lit_N<12>();
                    break;
                case DW_OP_lit13:
                    is_ok = eval_DW_OP_lit_N<13>();
                    break;
                case DW_OP_lit14:
                    is_ok = eval_DW_OP_lit_N<14>();
                    break;
                case DW_OP_lit15:
                    is_ok = eval_DW_OP_lit_N<15>();
                    break;
                case DW_OP_lit16:
                    is_ok = eval_DW_OP_lit_N<16>();
                    break;
                case DW_OP_lit17:
                    is_ok = eval_DW_OP_lit_N<17>();
                    break;
                case DW_OP_lit18:
                    is_ok = eval_DW_OP_lit_N<18>();
                    break;
                case DW_OP_lit19:
                    is_ok = eval_DW_OP_lit_N<19>();
                    break;
                case DW_OP_lit20:
                    is_ok = eval_DW_OP_lit_N<20>();
                    break;
                case DW_OP_lit21:
                    is_ok = eval_DW_OP_lit_N<21>();
                    break;
                case DW_OP_lit22:
                    is_ok = eval_DW_OP_lit_N<22>();
                    break;
                case DW_OP_lit23:
                    is_ok = eval_DW_OP_lit_N<23>();
                    break;
                case DW_OP_lit24:
                    is_ok = eval_DW_OP_lit_N<24>();
                    break;
                case DW_OP_lit25:
                    is_ok = eval_DW_OP_lit_N<25>();
                    break;
                case DW_OP_lit26:
                    is_ok = eval_DW_OP_lit_N<26>();
                    break;
                case DW_OP_lit27:
                    is_ok = eval_DW_OP_lit_N<27>();
                    break;
                case DW_OP_lit28:
                    is_ok = eval_DW_OP_lit_N<28>();
                    break;
                case DW_OP_lit29:
                    is_ok = eval_DW_OP_lit_N<29>();
                    break;
                case DW_OP_lit30:
                    is_ok = eval_DW_OP_lit_N<30>();
                    break;
                case DW_OP_lit31:
                    is_ok = eval_DW_OP_lit_N<31>();
                    break;
                case DW_OP_const1u:
                    is_ok = eval_DW_OP_const_N<Dwarf_Unsigned, 1>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const1s:
                    is_ok = eval_DW_OP_const_N<Dwarf_Signed, 1>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const2u:
                    is_ok = eval_DW_OP_const_N<Dwarf_Unsigned, 2>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const2s:
                    is_ok = eval_DW_OP_const_N<Dwarf_Signed, 2>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const4u:
                    is_ok = eval_DW_OP_const_N<Dwarf_Unsigned, 4>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const4s:
                    is_ok = eval_DW_OP_const_N<Dwarf_Signed, 4>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const8u:
                    is_ok = eval_DW_OP_const_N<Dwarf_Unsigned, 8>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_const8s:
                    is_ok = eval_DW_OP_const_N<Dwarf_Signed, 8>(buff, buff_size, buff_pos);
                    break;
                case DW_OP_constu:
                    is_ok = eval_DW_OP_constu(buff, buff_size, buff_pos);
                    break;
                case DW_OP_consts:
                    is_ok = eval_DW_OP_consts(buff, buff_size, buff_pos);
                    break;
                case DW_OP_fbreg:
                case DW_OP_breg0:
                case DW_OP_breg1:
                case DW_OP_breg2:
                case DW_OP_breg3:
                case DW_OP_breg4:
                case DW_OP_breg5:
                case DW_OP_breg6:
                case DW_OP_breg7:
                case DW_OP_breg8:
                case DW_OP_breg9:
                case DW_OP_breg10:
                case DW_OP_breg11:
                case DW_OP_breg12:
                case DW_OP_breg13:
                case DW_OP_breg14:
                case DW_OP_breg15:
                case DW_OP_breg16:
                case DW_OP_breg17:
                case DW_OP_breg18:
                case DW_OP_breg19:
                case DW_OP_breg20:
                case DW_OP_breg21:
                case DW_OP_breg22:
                case DW_OP_breg23:
                case DW_OP_breg24:
                case DW_OP_breg25:
                case DW_OP_breg26:
                case DW_OP_breg27:
                case DW_OP_breg28:
                case DW_OP_breg29:
                case DW_OP_breg30:
                case DW_OP_breg31:
                case DW_OP_bregx:
                    // 要実行時計算
                    is_immediate = false;
                    break;
                case DW_OP_dup:
                case DW_OP_drop:
                case DW_OP_pick:
                case DW_OP_over:
                case DW_OP_swap:
                case DW_OP_rot:
                case DW_OP_deref:
                case DW_OP_deref_size:
                case DW_OP_xderef:
                case DW_OP_xderef_size:
                case DW_OP_push_object_address:
                case DW_OP_form_tls_address:
                case DW_OP_call_frame_cfa:
                case DW_OP_abs:
                case DW_OP_and:
                case DW_OP_div:
                case DW_OP_minus:
                case DW_OP_mod:
                case DW_OP_mul:
                case DW_OP_neg:
                case DW_OP_not:
                case DW_OP_or:
                case DW_OP_plus:
                    is_ok = eval_DW_OP_unimpl(ope, buff, buff_size, buff_pos);
                    break;
                case DW_OP_plus_uconst:
                    is_ok = eval_DW_OP_plus_uconst(buff, buff_size, buff_pos);
                    break;
                case DW_OP_shl:
                case DW_OP_shr:
                case DW_OP_shra:
                case DW_OP_xor:
                case DW_OP_eq:
                case DW_OP_ge:
                case DW_OP_gt:
                case DW_OP_le:
                case DW_OP_lt:
                case DW_OP_ne:
                case DW_OP_skip:
                case DW_OP_bra:
                case DW_OP_call2:
                case DW_OP_call4:
                case DW_OP_call_ref:
                case DW_OP_nop:
                    is_ok = eval_DW_OP_unimpl(ope, buff, buff_size, buff_pos);
                    break;
                case DW_OP_reg0:
                case DW_OP_reg1:
                case DW_OP_reg2:
                case DW_OP_reg3:
                case DW_OP_reg4:
                case DW_OP_reg5:
                case DW_OP_reg6:
                case DW_OP_reg7:
                case DW_OP_reg8:
                case DW_OP_reg9:
                case DW_OP_reg10:
                case DW_OP_reg11:
                case DW_OP_reg12:
                case DW_OP_reg13:
                case DW_OP_reg14:
                case DW_OP_reg15:
                case DW_OP_reg16:
                case DW_OP_reg17:
                case DW_OP_reg18:
                case DW_OP_reg19:
                case DW_OP_reg20:
                case DW_OP_reg21:
                case DW_OP_reg22:
                case DW_OP_reg23:
                case DW_OP_reg24:
                case DW_OP_reg25:
                case DW_OP_reg26:
                case DW_OP_reg27:
                case DW_OP_reg28:
                case DW_OP_reg29:
                case DW_OP_reg30:
                case DW_OP_reg31:
                    // 要実行時計算
                    is_immediate = false;
                    break;

                default:
                    is_ok = eval_DW_OP_unimpl(ope, buff, buff_size, buff_pos);
                    break;
            }
        }
        //
        if (is_ok && is_immediate) {
            return std::make_optional(dw_op_value());
        } else if (is_ok && !is_immediate) {
            return std::make_optional(dw_op_value(buff, buff_size));
        } else {
            return std::nullopt;
        }
    }
};

}  // namespace util_dwarf