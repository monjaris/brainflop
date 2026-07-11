#pragma once
#include "core.hpp"



class Compiler
{
    static constexpr char INDENT[] = "    ";

    static constexpr strview Codegen_Fndecl_Out = {
        "void out(char);"
    };
    static constexpr strview Codegen_Fndef_Out = {
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

    static constexpr strview Codegen_Fndecl_in = {
        "void in(void*);"
    };
    static constexpr strview Codegen_Fndef_in = {
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

    const std::string Codegen_Normal_Begin = std::format(
        "// Forward declare IO functions\n"
        "{0}\n"  // func in();
        "{1}\n"  // func out();
        "\n"
        "#define MAP_SIZE ({2})\n"
        "static unsigned char map[MAP_SIZE] = {{0}};\n"
        "\n"
        "int main() {{\n"
        "{3}unsigned char* ptr = map;\n"  // indent it
        "\n",
        Codegen_Fndecl_in, Codegen_Fndecl_Out, 25000, INDENT
    );

    const std::string Codegen_Macro_Begin = std::format(
        "// Forward declare IO functions\n"
        "{0}\n"
        "{1}\n"
        "\n"
        "#define FORWARD(n)  ptr += (n);\n"
        "#define BACKWARD(n) ptr -= (n);\n"
        "#define ADD(n)      (*ptr) += (n);\n"
        "#define SUB(n)      (*ptr) -= (n);\n"
        "#define READ        in(ptr);\n"
        "#define WRITE       out(*ptr);\n"
        "#define LOOP_BEGIN  while (*ptr) {{\n"
        "#define LOOP_END    }}\n"
        "\n"
        "#define MAP_SIZE ({2})\n"
        "static unsigned char map[MAP_SIZE] = {{0}};\n"
        "\n"
        "int main() {{\n"
        "{3}unsigned char* ptr = map;\n"
        "\n",
        Codegen_Fndecl_in, Codegen_Fndecl_Out, 25000, INDENT
    );

    const std::string Codegen_Dense_Begin = std::format(
        "{0}{1}"
        "#define MAP_SIZE ({2})\n"
        "unsigned char map[MAP_SIZE]={{0}};"
        "int main(){{unsigned char*ptr=map;",
        Codegen_Fndecl_in, Codegen_Fndecl_Out, 25000
    );


public:
    enum class CodeGenMode {
        NORMAL_C,
        MACRO_C,
        DENSE_C
    };

    void setMode(CodeGenMode codegen_mode) { m_mode = codegen_mode; }
    CodeGenMode getMode() { return m_mode; }

    // possibily throws std::invalid_argument()
    static CodeGenMode StringToGenMode(strview str) {
        if (str == "default") {
            return CodeGenMode::NORMAL_C;
        } else if (str == "macros") {
            return CodeGenMode::MACRO_C;
        } else if (str == "dense" || str == "minimal") {
            return CodeGenMode::DENSE_C;
        }
        else throw std::invalid_argument(std::format("Unexpected value: \"{}\"", str));
    }

    private: CodeGenMode m_mode = CodeGenMode::NORMAL_C;


private:
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
    Compiler (const strview src);

    void parseInRawMode();
    void parseInMacroMode();
    void parseInDenseMode();

    int32 preprocess();
    int32 transpile(strview outc);
};

