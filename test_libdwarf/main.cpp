
#include <cstdio>
#include <format>
#include <iostream>
#include <string>

#include "util_dwarf/debug_info.hpp"
#include "util_dwarf/dwarf_analyzer.hpp"
#include "util_dwarf/dwarf_info.hpp"

void dump_memmap(util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, size_t array_idx);
void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off address);

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cout << argv[0] << std::endl;
        printf("Usage: *.exe <dwarf file>\n");
        return 0;
    }

    util_dwarf::dwarf_analyzer di;
    di.set_analyze_func_info(false);
    auto result = di.open(argv[1]);
    if (result) {
        util_dwarf::dwarf_info dw_info;

        clock_t s, t;
        s = clock();
        // dwarf解析
        di.analyze(dw_info);
        t = clock();
        printf("%f\n", (double)(t - s) / CLOCKS_PER_SEC);

        //
        auto debug_info = util_dwarf::debug_info(dw_info);
        debug_info.build();
        //
        debug_info.memmap([](util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type) -> void {
            std::string prefix("");
            dump_memmap(var, type, prefix, 0, 0);
            return;
        });

        di.close();
    }

    return 0;
}

std::string dump_make_tag(util_dwarf::debug_info::type_info &type) {
    // type name
    std::string tag;
    if (type.name != nullptr) {
        if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
            tag = std::format("struct/{}", *type.name);
        } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
            tag = std::format("union/{}", *type.name);
        } else {
            tag = std::format("{}", *type.name);
        }
    }
    if (type.pointer_depth > 0) {
        tag = std::format("{}*", tag);
    }
    //
    return std::move(tag);
}

void dump_memmap(util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, size_t array_idx) {
    // type name
    std::string tag = dump_make_tag(type);
    size_t offset   = 0;
    Dwarf_Off addr;
    //
    std::string name;

    // data
    if ((type.tag & util_dwarf::debug_info::type_tag::array) != 0) {
        // 配列, ポインタは含まない
        for (size_t i = 0; i < type.count; i++) {
            if (prefix.size() == 0) {
                name = std::format("{}[{}]", var.name, i);
            } else {
                name = std::format("{}.{}[{}]", prefix, var.name, i);
            }
            offset = i * type.byte_size;
            addr   = *var.location + offset;
            printf("0x%08X\t%20s\t%lld\t%*c%s\n", addr, tag.c_str(), type.byte_size, depth, '\t', name.c_str());
            // member
            if (type.child_list != nullptr) {
                dump_memmap_member(type, name, depth, addr);
            }
        }

    } else {
        if (prefix.size() == 0) {
            name = var.name;
        } else {
            name = prefix + "." + var.name;
        }
        // 配列以外
        printf("0x%08X\t%20s\t%lld\t%*c%s\n", *var.location, tag.c_str(), type.byte_size, depth, '\t', name.c_str());
    }
}
void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off base_address) {
    Dwarf_Off bit_offset = 0;
    std::string name;

    for (auto &mem : *type.child_list) {
        auto &member = *mem;
        std::string tag;
        if (member.sub_info != nullptr) {
            tag = dump_make_tag(*member.sub_info);
        } else {
            tag = dump_make_tag(member);
        }
        //
        Dwarf_Off address = 0;
        address += base_address;
        address += member.data_member_location;

        if (member.bit_size == 0) {
            // bit_sizeがゼロならビットフィールドでない
            if ((member.tag & util_dwarf::debug_info::type_tag::array) != 0) {
                for (size_t i = 0; i < type.count; i++) {
                    //
                    address += type.byte_size;
                    name = std::format("{}.{}[{}]", prefix, *(member.name), i);
                    printf("0x%08X\t%20s\t%lld\t%*c%s\n", address, tag.c_str(), member.byte_size, depth, '\t', name.c_str());
                }
            } else {
                //
                name = std::format("{}.{}", prefix, *(member.name));
                printf("0x%08X\t%20s\t%lld\t%*c%s\n", address, tag.c_str(), member.byte_size, depth, '\t', name.c_str());
            }

        } else {
            // bit_sizeがゼロ以外ならビットフィールド
            address += (member.bit_offset / 8);
            bit_offset = member.bit_offset % 8;
            //
            name = std::format("{}.{}", prefix, *(member.name));
            //
            printf("0x%08X\t%20s\t%lld bit\t%*c%s\n", address, tag.c_str(), member.bit_size, depth, '\t', name.c_str());
        }
    }
}