#pragma once

#define LIBDWARF_STATIC 1
#include <dwarf.h>
#include <libdwarf.h>

#include <cstdint>
#include <cstdio>

// API examples
// https://www.prevanders.net/libdwarfdoc/modules.html

namespace util_dwarf {

namespace utility {

// APIエラーが発生したときにどうするか？
// 暫定で例外を出して終了する
// 戻り値がOKじゃないけどエラーでもないケースある？
void error_happen(Dwarf_Error *error) {
    char *errmsg = dwarf_errmsg(*error);
    fprintf(stderr, "%s\n", errmsg);
    throw std::runtime_error("libdwarf API error!");
    exit(1);  // 一応書いておく
}

template <typename T>
T concat_le(uint8_t const *buff, size_t begin, size_t end) {
    Dwarf_Unsigned result = 0;
    size_t index          = end - 1;
    for (size_t i = begin; i < end; i++, index--) {
        result = buff[index] | (result << 8);
    }
    return static_cast<T>(result);
}
}  // namespace utility

}  // namespace util_dwarf
