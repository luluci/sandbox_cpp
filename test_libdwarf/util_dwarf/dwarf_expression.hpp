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

// Dwarf expression 計算機
class dwarf_expression {
    using value_t = std::variant<Dwarf_Unsigned, Dwarf_Signed>;
    using stack_t = std::vector<value_t>;
    stack_t stack_;
    static constexpr size_t default_stack_size = 10;

public:
    dwarf_expression() : stack_() {
        stack_.reserve(default_stack_size);
    }

    bool eval_DW_OP_unimpl(uint8_t *buff, size_t buff_size) {
        //
        fprintf(stderr, "no implemented! : DW_OP(0x%02X), ope_size=%lld\n", buff[0], buff_size);
        return false;
    }
    //
    bool eval_DW_OP_addr(uint8_t *buff, size_t buff_size) {
        Dwarf_Unsigned value = 0;
        // little endianで結合
        // [buff_size]~[1]
        // [0]はopeコードなので除外
        for (size_t i = buff_size - 1; i > 0; i--) {
            value <<= 8;
            value |= buff[i];
        }
        //
        stack_.push_back(value);
        //
        return true;
    }
    //
    bool eval_DW_OP_plus_uconst(uint8_t *buff, size_t buff_size) {
        Dwarf_Unsigned value = 0;

        // stackに値が積まれていたら取得して加算する
        auto pop_value = pop<Dwarf_Unsigned>();
        if (pop_value) {
            value = *pop_value;
        }

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        ULEB128 leb(&buff[1], buff_size - 1);
        value += leb.value;
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
    bool eval_DW_OP_breg_N(uint8_t *buff, size_t buff_size) {
        Dwarf_Signed value = 0;

        // no impl!

        // register_N から値を取り出す
        // レジスタが64bit以上の場合はオーバーフロー注意
        // value = register;
        // SLEB128のoffsetを適用する
        SLEB128 leb(&buff[1], buff_size - 1);
        value += leb.value;

        stack_.push_back(static_cast<Dwarf_Unsigned>(value));
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_bregx(uint8_t *buff, size_t buff_size) {
        Dwarf_Signed value = 0;
        size_t index       = 1;
        size_t reg_no;

        // no impl!

        // registerを指定するULEB128を取得
        ULEB128 uleb(&buff[index], buff_size - 1);
        reg_no = uleb.value;
        // ULEB128の後ろにSLEB128
        index += uleb.used_bytes;

        // register_N から値を取り出す
        // レジスタが64bit以上の場合はオーバーフロー注意
        // value = register;
        // SLEB128のoffsetを適用する
        SLEB128 leb(&buff[index], buff_size - 1);
        value += leb.value;

        stack_.push_back(static_cast<Dwarf_Unsigned>(value));
        return true;
    }
    //
    template <size_t N>
    bool eval_DW_OP_reg_N(uint8_t *buff, size_t buff_size) {
        Dwarf_Unsigned value = 0;

        // no impl!

        // register_N から値を取り出す
        // value = register;

        stack_.push_back(value);
        return true;
    }
    //
    template <typename T, size_t N>
    bool eval_DW_OP_const_N(uint8_t *buff, size_t buff_size) {
        T value = 0;

        if (1 + N >= buff_size) {
            fprintf(stderr, "error: DW_OP_const_N : invalid buff_size(%lld) require %lld bytes\n", buff_size, 1 + N);
            return false;
        }
        // N byteのデータを取得する
        value = utility::concat_le<T>(buff, 1, 1 + N);
        stack_.push_back(value);
        //
        return true;
    }
    //
    bool eval_DW_OP_constu(uint8_t *buff, size_t buff_size) {
        Dwarf_Unsigned value = 0;

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        ULEB128 leb(&buff[1], buff_size - 1);
        value += leb.value;
        //
        stack_.push_back(value);
        //
        return true;
    }
    //
    bool eval_DW_OP_consts(uint8_t *buff, size_t buff_size) {
        Dwarf_Signed value = 0;

        // LEB128形式でデコードする
        // [0]はopeコードなので除外
        SLEB128 leb(&buff[1], buff_size - 1);
        value += leb.value;
        //
        stack_.push_back(value);
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
                return eval_DW_OP_lit_N<0>();
            case DW_OP_lit1:
                return eval_DW_OP_lit_N<1>();
            case DW_OP_lit2:
                return eval_DW_OP_lit_N<2>();
            case DW_OP_lit3:
                return eval_DW_OP_lit_N<3>();
            case DW_OP_lit4:
                return eval_DW_OP_lit_N<4>();
            case DW_OP_lit5:
                return eval_DW_OP_lit_N<5>();
            case DW_OP_lit6:
                return eval_DW_OP_lit_N<6>();
            case DW_OP_lit7:
                return eval_DW_OP_lit_N<7>();
            case DW_OP_lit8:
                return eval_DW_OP_lit_N<8>();
            case DW_OP_lit9:
                return eval_DW_OP_lit_N<9>();
            case DW_OP_lit10:
                return eval_DW_OP_lit_N<10>();
            case DW_OP_lit11:
                return eval_DW_OP_lit_N<11>();
            case DW_OP_lit12:
                return eval_DW_OP_lit_N<12>();
            case DW_OP_lit13:
                return eval_DW_OP_lit_N<13>();
            case DW_OP_lit14:
                return eval_DW_OP_lit_N<14>();
            case DW_OP_lit15:
                return eval_DW_OP_lit_N<15>();
            case DW_OP_lit16:
                return eval_DW_OP_lit_N<16>();
            case DW_OP_lit17:
                return eval_DW_OP_lit_N<17>();
            case DW_OP_lit18:
                return eval_DW_OP_lit_N<18>();
            case DW_OP_lit19:
                return eval_DW_OP_lit_N<19>();
            case DW_OP_lit20:
                return eval_DW_OP_lit_N<20>();
            case DW_OP_lit21:
                return eval_DW_OP_lit_N<21>();
            case DW_OP_lit22:
                return eval_DW_OP_lit_N<22>();
            case DW_OP_lit23:
                return eval_DW_OP_lit_N<23>();
            case DW_OP_lit24:
                return eval_DW_OP_lit_N<24>();
            case DW_OP_lit25:
                return eval_DW_OP_lit_N<25>();
            case DW_OP_lit26:
                return eval_DW_OP_lit_N<26>();
            case DW_OP_lit27:
                return eval_DW_OP_lit_N<27>();
            case DW_OP_lit28:
                return eval_DW_OP_lit_N<28>();
            case DW_OP_lit29:
                return eval_DW_OP_lit_N<29>();
            case DW_OP_lit30:
                return eval_DW_OP_lit_N<30>();
            case DW_OP_lit31:
                return eval_DW_OP_lit_N<31>();
            case DW_OP_const1u:
                return eval_DW_OP_const_N<Dwarf_Unsigned, 1>(buff, buff_size);
            case DW_OP_const1s:
                return eval_DW_OP_const_N<Dwarf_Signed, 1>(buff, buff_size);
            case DW_OP_const2u:
                return eval_DW_OP_const_N<Dwarf_Unsigned, 2>(buff, buff_size);
            case DW_OP_const2s:
                return eval_DW_OP_const_N<Dwarf_Signed, 2>(buff, buff_size);
            case DW_OP_const4u:
                return eval_DW_OP_const_N<Dwarf_Unsigned, 4>(buff, buff_size);
            case DW_OP_const4s:
                return eval_DW_OP_const_N<Dwarf_Signed, 4>(buff, buff_size);
            case DW_OP_const8u:
                return eval_DW_OP_const_N<Dwarf_Unsigned, 8>(buff, buff_size);
            case DW_OP_const8s:
                return eval_DW_OP_const_N<Dwarf_Signed, 8>(buff, buff_size);
            case DW_OP_constu:
                return eval_DW_OP_constu(buff, buff_size);
            case DW_OP_consts:
                return eval_DW_OP_consts(buff, buff_size);
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
                return eval_DW_OP_plus_uconst(buff, buff_size);
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

            default:
                break;
        }

        return eval_DW_OP_unimpl(buff, buff_size);
    }
};

}  // namespace util_dwarf