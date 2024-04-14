
#include <cstdio>
#include <format>
#include <iostream>
#include <string>
#include <type_traits>

#include "util_dwarf/debug_info.hpp"
#include "util_dwarf/dwarf_analyzer.hpp"
#include "util_dwarf/dwarf_info.hpp"

// void dump_memmap(util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, size_t array_idx);
// void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off address);

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

template <typename Func>
void dump_memmap(util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, size_t array_idx,
                 Func &&func) {
    // type name
    std::string tag = dump_make_tag(type);
    size_t offset   = 0;
    Dwarf_Off addr;
    //
    std::string name;
    //
    if (var.decl_file_is_external) {
        return;
    }

    func(var, type);

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
            //  member
            //  pointerは展開しない
            //  function: 引数としてchildを持つ -> 展開しない
            if ((type.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
                if (type.child_list != nullptr) {
                    dump_memmap_member(type, name, depth, addr, std::forward<Func>(func));
                }
            }

            // dbg
            break;
        }

    } else {
        if (prefix.size() == 0) {
            name = var.name;
        } else {
            name = prefix + "." + var.name;
        }
        addr = *var.location;
        // 配列以外
        printf("0x%08X\t%20s\t%lld\t%*c%s\n", addr, tag.c_str(), type.byte_size, depth, '\t', name.c_str());
        // member
        // pointerは展開しない
        // function: 引数としてchildを持つ -> 展開しない
        if ((type.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
            if (type.child_list != nullptr) {
                dump_memmap_member(type, name, depth, addr, std::forward<Func>(func));
            }
        }
    }
}

template <typename Func>
void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off base_address, Func &&func) {
    Dwarf_Off bit_offset = 0;
    std::string name;

    for (auto &mem : *type.child_list) {
        auto &member = *mem;
        std::string tag;
        if (member.sub_info != nullptr) {
            tag = dump_make_tag(*member.sub_info);
            func(member, *member.sub_info);

        } else {
            tag = dump_make_tag(member);
        }

        //
        Dwarf_Off address = 0;
        address           = base_address;
        address += member.data_member_location;

        if (member.bit_size == 0) {
            // bit_sizeがゼロならビットフィールドでない
            if ((member.tag & util_dwarf::debug_info::type_tag::array) != 0) {
                for (size_t i = 0; i < member.count; i++) {
                    //
                    name = std::format("{}.{}[{}]", prefix, *(member.name), i);
                    printf("0x%08X\t%20s\t%lld\t%*c%s\n", address, tag.c_str(), member.byte_size, depth, '\t', name.c_str());
                    //  member
                    //  pointerは展開しない
                    //  function: 引数としてchildを持つ -> 展開しない
                    if ((member.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
                        if (member.child_list != nullptr) {
                            dump_memmap_member(member, name, depth, address, std::forward<Func>(func));
                        }
                    }
                    address += member.byte_size;

                    // dbg
                    break;
                }
            } else {
                //
                name = std::format("{}.{}", prefix, *(member.name));
                printf("0x%08X\t%20s\t%lld\t%*c%s\n", address, tag.c_str(), member.byte_size, depth, '\t', name.c_str());
                //  member
                //  pointerは展開しない
                //  function: 引数としてchildを持つ -> 展開しない
                if ((member.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
                    if (member.child_list != nullptr) {
                        dump_memmap_member(member, name, depth, address, std::forward<Func>(func));
                    }
                }
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
            dump_memmap(var, type, prefix, 0, 0, [](auto &var1, auto &type1) -> void {
                // if constexpr (std::is_pointer_v<decltype(var1.name)>) {
                //     if (var1.name == nullptr) {
                //         return;
                //     }
                //     printf("var:%s, type:%s\n", var1.name->c_str(), type1.name->c_str());
                // } else {
                //     printf("var:%s, type:%s\n", var1.name.c_str(), type1.name->c_str());
                // }
            });
            return;
        });

        di.close();
    }

    return 0;
}
