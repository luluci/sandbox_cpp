
#include <time.h>

#include <cstdio>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

#include "util_dwarf/debug_info.hpp"
#include "util_dwarf/dwarf_analyzer.hpp"
#include "util_dwarf/dwarf_info.hpp"

// void dump_memmap(util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, size_t array_idx);
// void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off address);

static constexpr size_t make_type_tag_buff_size = 100;
char make_type_tag_buff[make_type_tag_buff_size];
std::string dump_make_tag(util_dwarf::debug_info::type_info &type) {
    // type name
    std::string tag;

    // pointer情報を作成
    size_t i;
    for (i = 0; i < make_type_tag_buff_size - 1 && i < type.pointer_depth; i++) {
        make_type_tag_buff[i] = '*';
    }
    make_type_tag_buff[i] = '\0';
    // // type情報タグを作成
    // if (type.name != nullptr) {
    //     view.tag_type = type.name;
    //     if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
    //         view.is_struct = true;
    //         std::format_to(std::back_inserter(view.tag_type), "struct/{}{}", *type.name, make_type_tag_buff);
    //     } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
    //         view.is_union = true;
    //         std::format_to(std::back_inserter(view.tag_type), "union/{}{}", *type.name, make_type_tag_buff);
    //     } else {
    //         std::format_to(std::back_inserter(view.tag_type), "{}{}", *type.name, make_type_tag_buff);
    //     }
    // }
    if (type.name != nullptr) {
        if ((type.tag & util_dwarf::debug_info::type_tag::struct_) != 0) {
            tag = std::format("struct/{}", *type.name);
        } else if ((type.tag & util_dwarf::debug_info::type_tag::union_) != 0) {
            tag = std::format("union/{}", *type.name);
        } else {
            tag = std::format("{}", *type.name);
        }
    }

    //
    return tag;
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

    func(var, type);

    // data
    if ((type.tag & util_dwarf::debug_info::type_tag::array) != 0) {
        // 配列, ポインタは含まない
        for (size_t i = 0; i < type.count; i++) {
            if (prefix.size() == 0) {
                name = std::format("{}[{}]", *var.name, i);
            } else {
                name = std::format("{}.{}[{}]", prefix, *var.name, i);
            }
            offset = i * type.byte_size;
            addr   = *var.location + offset;
            printf("0x%08X\t%20s\t%lld\t%*c%s\n", addr, tag.c_str(), type.byte_size, depth, '\t', name.c_str());
            //  member
            //  pointerは展開しない
            //  function: 引数としてchildを持つ -> 展開しない
            if ((type.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
                if (type.member_list != nullptr) {
                    dump_memmap_member(type, name, depth, addr, std::forward<Func>(func));
                }
            }

            // dbg
            break;
        }

    } else {
        if (prefix.size() == 0) {
            name = *var.name;
        } else {
            name = prefix + "." + *var.name;
        }
        addr = *var.location;
        // 配列以外
        printf("0x%08X\t%20s\t%lld\t%*c%s\n", addr, tag.c_str(), type.byte_size, depth, '\t', name.c_str());
        // member
        // pointerは展開しない
        // function: 引数としてchildを持つ -> 展開しない
        if ((type.tag & util_dwarf::debug_info::type_tag::func_ptr) == 0) {
            if (type.member_list != nullptr) {
                dump_memmap_member(type, name, depth, addr, std::forward<Func>(func));
            }
        }
    }
}

template <typename Func>
void dump_memmap_member(util_dwarf::debug_info::type_info &type, std::string &prefix, int depth, Dwarf_Off base_address, Func &&func) {
    Dwarf_Off bit_offset = 0;
    std::string name;

    for (auto &mem : *type.member_list) {
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
                        if (member.member_list != nullptr) {
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
                    if (member.member_list != nullptr) {
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

    // コマンドライン引数チェック
    char const *file_path = nullptr;
    bool is_cmdline_ok    = false;
    bool is_prior_typedef = false;
    if (argc > 1) {
        int arg_idx = 1;
        // 末尾以外をチェック
        while (arg_idx < argc - 1) {
            std::string_view arg(argv[arg_idx]);
            if (arg.find("--prior-typedef") == 0) {
                is_prior_typedef = true;
            }
            arg_idx++;
        }
        // 末尾はファイル名
        file_path     = argv[argc - 1];
        is_cmdline_ok = true;
    }
    if (!is_cmdline_ok) {
        printf("Usage: %s [options] <dwarf file>\n", argv[0]);
        printf("\n");
        printf("options:\n");
        printf("  --prior-typedef : prior typedef name\n");
        return -1;
    }

    util_dwarf::dwarf_analyzer di;
    auto result = di.open(file_path);
    if (result) {
        util_dwarf::dwarf_info dw_info;

        clock_t s, t;
        s = clock();
        // dwarf解析
        using da_opt = util_dwarf::dwarf_analyze_option;
        da_opt daopt;
        // daopt.unset(da_opt::func_info_analyze | da_opt::no_impl_warning);
        daopt.unset(da_opt::no_impl_warning);
        daopt.set(da_opt::func_info_analyze);
        di.analyze(dw_info, daopt);
        t = clock();
        printf("%f\n", static_cast<double>(t - s) / CLOCKS_PER_SEC);

        // 全optionは初期値でfalse
        using diopt = util_dwarf::debug_info::option;
        diopt opt;
        if (is_prior_typedef) {
            opt.set(diopt::prior_typedef);
        }
        // opt.set(diopt::expand_array);
        // opt.set(diopt::through_typedef | diopt::expand_array);
        // opt.unset(diopt::through_typedef);
        // opt.unset(diopt::expand_array);
        auto debug_info = util_dwarf::debug_info(dw_info, opt);
        debug_info.build();
        //
        // debug_info.memmap([](util_dwarf::debug_info::var_info &var, util_dwarf::debug_info::type_info &type) -> void {
        //     std::string prefix("");
        //     dump_memmap(var, type, prefix, 0, 0, [](auto &var1, auto &type1) -> void {
        //         // if constexpr (std::is_pointer_v<decltype(var1.name)>) {
        //         //     if (var1.name == nullptr) {
        //         //         return;
        //         //     }
        //         //     printf("var:%s, type:%s\n", var1.name->c_str(), type1.name->c_str());
        //         // } else {
        //         //     printf("var:%s, type:%s\n", var1.name.c_str(), type1.name->c_str());
        //         // }
        //     });
        //     return;
        // });
        if constexpr (false) {
            debug_info.get_func_info([](util_dwarf::debug_info::func_info_view &view) -> bool {
                printf("%s\n", view.tag_name->c_str());
                return true;
            });
        }
        if constexpr (true) {
            int typelen = static_cast<int>(debug_info.max_typename_len);

            debug_info.get_var_info([typelen](util_dwarf::debug_info::var_info_view &view) -> bool {
                // 相対パスを作成
                std::string decl_file_path_rel;
                if (view.var_decl_file_path != nullptr && view.cu_info != nullptr) {
                    if (view.var_decl_file_path->find(view.cu_info->comp_dir) == 0) {
                        // comp_dirの末尾に区切り文字がおそらく付かないため、相対パスの先頭に区切り文字が残る
                        // コンパイラにより挙動が変わる可能性があるので、余計な加工をしないようにした
                        decl_file_path_rel = view.var_decl_file_path->substr(view.cu_info->comp_dir.size());
                    }
                }
                //
                printf("0x%08llX %*s\t%lld\t%s\t[", view.address, typelen, view.tag_type->c_str(), view.byte_size, view.tag_name->c_str());
                if (view.is_array) {
                    printf("array, ");
                }
                if (view.is_struct) {
                    printf("struct, ");
                }
                if (view.is_union) {
                    printf("union, ");
                }
                if (view.is_enum) {
                    printf("enum, ");
                }
                if (view.is_const) {
                    printf("const, ");
                }
                if (view.is_struct_member) {
                    printf("struct_member, ");
                }
                if (view.is_union_member) {
                    printf("union_member, ");
                }
                if (view.is_unnamed) {
                    printf("is_unnamed, ");
                }
                printf("]\n");
                return true;
            });
        }

        di.close();
    }

    return 0;
}
