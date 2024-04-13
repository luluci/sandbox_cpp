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
    printf("%s\n", errmsg);
    throw std::runtime_error("libdwarf API error!");
    exit(1);  // 一応書いておく
}

Dwarf_Unsigned concat_le(uint8_t const *buff, size_t begin, size_t end) {
    Dwarf_Unsigned result = 0;
    size_t shift          = 0;
    for (size_t i = begin; i < end; i++) {
        result |= (buff[i] << shift);
        shift += 8;
    }
    return result;
}
}  // namespace utility

}  // namespace util_dwarf