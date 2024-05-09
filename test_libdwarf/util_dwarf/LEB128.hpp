#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace util_dwarf {

template <typename T>
struct LEB128
{
    T value;

    LEB128(uint8_t *buff, size_t len) : value(0) {
        decode(buff, len);
    }

    void decode(uint8_t *buff, size_t len) {
        size_t shift = 0;
        value        = 0;

        for (size_t i = 0; i < len; i++) {
            auto &data = buff[i];

            // 下位7bitを連結してデータを構築する
            value |= (data & 0x7F) << shift;

            // 最上位bitが0なら終了
            if ((data & 0x80) == 0) {
                break;
            }

            //
            shift += 7;
        }

        if constexpr (std::is_unsigned_v<T>) {
            // nothing
        } else if constexpr (std::is_signed_v<T>) {
            // signed用処理
            // 最上位bit位置を作成
            shift += 7;
            // 最上位bitより上位のbitのマスク
            T mask = std::numeric_limits<T>::max();
            mask <<= shift - 1;
            if ((value & mask) != 0) {
                value |= mask;
            }
        } else {
            // 整数以外はNG
            static_assert(false);
        }
    }
};

using ULEB128 = LEB128<uint32_t>;
using SLEB128 = LEB128<int32_t>;

}  // namespace util_dwarf