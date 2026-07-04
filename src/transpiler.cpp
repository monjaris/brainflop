#include "bfc/transpiler.hpp"


Compiler::Compiler (const strv src) {
    using std::ios;

    std::ifstream ifile(src.data(), ios::in | ios::binary | ios::ate);
    if (!ifile) this->m_panic("Could not open file {}", src);

    std::streamsize fsize = ifile.tellg();
    m_code.resize(fsize);
    ifile.seekg(0, ios::beg);

    ifile.read(m_code.data(), fsize);
}



int32 Compiler::preprocess() {
    for (uint32 i=0;  i < m_code.size();  ++i) {
        char& c = m_code[i];

        if (!charset.contains(c)) {
            m_code.erase(i, 1);
            i--;
            continue;
        }

        if (c == '[') {
            bracket_pairs.push(i);
        } else if (c == ']') {
            if (bracket_pairs.empty()) {
                m_panic("Closing bracket with no matching opening bracket");
            }
            bracket_pairs.pop();
        }
    }

    if (!bracket_pairs.empty()) {
        m_panic("No match for bracket detected");
    }

    bracket_pairs = {};
    return 0;
}



int32 Compiler::transpile(strv outc) {
    static const char INDENT[] = "    ";
    std::ofstream ofile;

    auto get_indentation = [&ofile, this]() {
        std::string indentation = INDENT;
        for (uint32 i = 0; i < this->bracket_pairs.size(); ++i) {
            indentation += INDENT;
        }
        return indentation;
    };

    auto count_instructions = [this](char c) ->uint32 {
        uint32 n = 0;
        while (lengthCheck() && peek() == c) {
            step();
            ++n;
        };
        return n;
    };

    ofile.open(outc.data(), std::ios::out | std::ios::trunc);
    if (!ofile) this->m_panic("Could not open file: {}", outc);

    ofile
        << "// declare function specifications instead of an include\n"
        << "int printf(char*, ...);\n"
        << "int getchar();\n"
        << "\n"
        << "int main() {\n"
        << INDENT << "unsigned char map[25000] = {0};\n"
        << INDENT << "unsigned char* ptr = map;\n"
        << "\n"
    ;

    while (lengthCheck())
    {
        switch (peek())
        {
            case '>': {
                uint32 n = count_instructions(peek());
                ofile << get_indentation() << "ptr += "<< n <<";\n";
                continue;
            }
            case '<': {
                uint32 n = count_instructions(peek());
                ofile << get_indentation() << "ptr -= "<< n <<";\n";
                continue;
            }
            // inc/dec pointer
            case '+': {
                uint32 n = count_instructions(peek());
                ofile << get_indentation() << "(*ptr) += "<< n <<";\n";
                continue;
            }
            case '-': {
                uint32 n = count_instructions(peek());
                ofile << get_indentation() << "(*ptr) -= "<< n <<";\n";
                continue;
            }
            // out/in pointer
            case '.': {
                ofile << "\n";
                ofile << get_indentation() << "printf(\"%c\", *ptr);\n";
                break;
            }
            case ',': {
                ofile << "\n";
                ofile
                    << get_indentation()
                    << "char ch = getchar();" << ' '
                    << "*ptr = (ch==EOF)?  0 : ch;\n";
                break;
            }
            // loop
            case '[': {
                ofile << "\n";
                ofile << get_indentation() << "while (*ptr) {\n";
                bracket_pairs.push(m_cur);
                break;
            }
            case ']': {
                ofile << "\n";
                bracket_pairs.pop();
                ofile << get_indentation() << "}\n";
                break;
            }
        }

        step();
    }

    ofile << "\n" << INDENT << "return 0;\n}\n";
    return 0;
}
