#pragma once
#include "core.hpp"

class Compiler
{
    static inline const std::set charset = {
        '>', '<', '+', '-', '.', '.', '[', ']'
    };
    std::string code;
    std::stack<uint32> bracket_pairs;

    template<const int32 exit_code = 1, typename... Args>
    [[noreturn]] void m_panic(std::format_string<Args...> fmtstr, Args... args) const {
        cr::log(
            "BrainFuck-Compiler: error:\n\t",
            std::format(fmtstr, std::forward<Args>(args)...)
        );
        ::exit(exit_code);
    }

    struct LoopStack {
        std::stack<uint32>* m_bracket_stack;
        uint32 m_gen = 1;

        LoopStack (std::stack<uint32>* bracket_stack)
            : m_bracket_stack(bracket_stack) {}
        ;
        void push(uint32 x) {
            ++m_gen;
            m_bracket_stack->push(x);
        }
        void pop() {
            m_bracket_stack->pop();
        }
        uint32 generation() {
            return m_gen;
        }
    };
    LoopStack loops = {&bracket_pairs};

    // mutable int32 _m_last_count;
    // int32 m_counter() const {
        // static int32 c = 0;
        // return _m_last_count = ++c;
    //     return bracket_pairs.size();
    // }
    // int32 m_last_count() const {
    //     // return _m_last_count-1;
    //     return bracket_pairs.size()+1;
    // }

public:

    Compiler (const strv src) {
        using std::ios;

        std::ifstream ifile(src.data(), ios::in | ios::binary | ios::ate);
        if (!ifile) this->m_panic("Could not open file {}", src);

        std::streamsize fsize = ifile.tellg();
        ifile.seekg(0, ios::beg);
        code.resize(fsize);

        ifile.read(code.data(), fsize);
    }

    int32 preprocess() {
        for (uint32 i=0;  i < code.size();  ++i) {
            char& c = code[i];

            if (!charset.contains(c)) {
                code.erase(i, 1);
                continue;
            }

            if (c == '[') {
                bracket_pairs.push(i);
            } else if (c == ']') {
                bracket_pairs.pop();
            }
        }

        if (!bracket_pairs.empty()) {
            m_panic("No match for bracket detected");
        }

        bracket_pairs = {};
        return 0;
    }

    int32 transpile(strv outc) {
        static const char* ind = "    ";
        bool looping = false;
        std::ofstream ofile;

        auto indent_stacked = [&ofile, this]() {
            uint32 k = this->bracket_pairs.size();
            while (1+k--) {
                ofile << ind;
            }
        };

        ofile.open(outc.data(), std::ios::out | std::ios::trunc);
        if (!ofile) this->m_panic("Could not open file: {}", outc);

        ofile <<
            "#include <stdio.h>\n\n" <<
            "int main() {\n" <<
            ind<< "char map[25000] = {0};\n" <<
            ind<< "char* ptr = map;\n" <<
            ind<< "char* restore = NULL;\n" <<
            "\n";

        for (uint32 i=0;  i < code.size();  ++i) {
            char& c = code[i];

            switch (c)
            {
                // move pointer
                case '>': {
                    indent_stacked();
                    ofile << "ptr++;\n";
                    break;
                }
                case '<': {
                    indent_stacked();
                    ofile << "ptr--;\n";
                    break;
                }
                // inc/dec pointer
                case '+': {
                    indent_stacked();
                    ofile << "++(*ptr);\n";
                    break;
                }
                case '-': {
                    indent_stacked();
                    ofile << "--(*ptr);\n";
                    break;
                }
                // out/in pointer
                case '.': {
                    ofile << "\n";
                    indent_stacked();
                    ofile << "printf(\"%c\", *ptr);\n";
                    break;
                }
                case ',': {
                    ofile << "\n";
                    indent_stacked();
                    ofile
                        << "char ch = getchar();" << ' '
                        << "*ptr = (ch==EOF)?  0 : ch;\n";
                    break;
                }
                // loop
                case '[': {
                    uint32 gen = bracket_pairs.size();
                    ofile << "\n";
                    indent_stacked(); ofile << "restore = ptr;\n";
                    indent_stacked(); ofile << "while (*ptr) {\n";
                    loops.push(i);
                    indent_stacked(); ofile << "char* save__"<<gen<< "= ptr;\n\n";
                    break;
                }
                case ']': {
                    uint32 gen = bracket_pairs.size();
                    ofile << "\n";
                    indent_stacked(); ofile << "restore = save__"<<--gen<<";\n";
                    loops.pop();
                    indent_stacked(); ofile << "}\n";
                    indent_stacked(); ofile << "ptr = restore;\n\n";
                    break;
                }
            };
        }

        ofile << "\n" << ind << "return 0;\n}\n";
        return 0;
    }
};
