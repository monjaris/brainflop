#pragma once
#include "core.hpp"



class Compiler
{
    static constexpr strv codegen_fn_decl_out = {
        "void out(char);"
    };
    static constexpr strv codegen_fn_def_out = {
        "void out(char ch) {\n"
        "    __asm__ volatile (\n"
        "        \"mov $1, %%rax\\n\"\n"
        "        \"mov $1, %%rdi\\n\"\n"
        "        \"lea %0, %%rsi\\n\"\n"
        "        \"mov $1, %%rdx\\n\"\n"
        "        \"syscall\\n\"\n"
        "        :\n"
        "        : \"m\"(ch)\n"
        "        : \"rax\", \"rdi\", \"rsi\", \"rdx\", \"rcx\", \"r11\", \"memory\"\n"
        "    );\n"
        "}\n"
    };

    static constexpr strv codegen_fn_decl_in = {
        "void in(void*);"
    };
    static constexpr strv codegen_fn_def_in = {
        "void in(void* r) {\n"
        "    __asm__ volatile (\n"
        "        \"mov $0, %%rax\\n\"\n"
        "        \"mov $0, %%rdi\\n\"\n"
        "        \"mov %0, %%rsi\\n\"\n"
        "        \"mov $1, %%rdx\\n\"\n"
        "        \"syscall\\n\"\n"
        "        :\n"
        "        : \"m\"(r)\n"
        "        : \"rax\", \"rdi\", \"rsi\", \"rdx\", \"rcx\", \"r11\", \"memory\"\n"
        "    );\n"
        "\n"
        "    char junk;\n"
        "    long n;\n"
        "    do {\n"
        "        __asm__ volatile (\n"
        "            \"mov $0, %%rax\\n\"\n"
        "            \"mov $0, %%rdi\\n\"\n"
        "            \"lea %1, %%rsi\\n\"\n"
        "            \"mov $1, %%rdx\\n\"\n"
        "            \"syscall\\n\"\n"
        "            \"mov %%rax, %0\\n\"\n"
        "            : \"=r\" (n)\n"
        "            : \"m\" (junk)\n"
        "            : \"rax\", \"rdi\", \"rsi\", \"rdx\", \"rcx\", \"r11\", \"memory\"\n"
        "        );\n"
        "    } while (n == 1 && junk != '\\n');\n"
        "}\n"
    };

    enum class GenMode {
        RAW_C,
        MACRO_C,
        DENSE_C
    };
    GenMode m_mode = GenMode::RAW_C;

    static inline const std::set charset = {
        '>', '<', '+', '-', '.', ',', '[', ']'
    };
    std::string m_code;
    usize m_cur = 0;
    std::stack<uint32> bracket_pairs;

    template<const int32 exit_code = 1, typename... Args>
    [[noreturn]] void m_panic(std::format_string<Args...> fmtstr, Args... args) const {
        cr::log(
            "BrainFuck-Compiler: error:\n\t",
            std::format(fmtstr, std::forward<Args>(args)...)
        );
        ::exit(exit_code);
    }

    inline bool lengthCheck(usize custom_cur=SIZE_MAX) {
        if (custom_cur == SIZE_MAX) {
            return m_cur < m_code.size();
        } else {
            return custom_cur < m_code.size();
        }
    }

    inline const char& peek() {
        static constexpr char bad_ch = '\0';
        return lengthCheck()?
            m_code[m_cur] : bad_ch
        ;
    }

    inline bool step(int32 n = 1) {
        m_cur += n;
        return lengthCheck();
    }

public:
    Compiler (const strv src);
    int32 preprocess();
    int32 transpile(strv outc);
};
