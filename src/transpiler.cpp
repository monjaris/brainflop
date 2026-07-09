#include "transpiler.hpp"


Compiler::Compiler (const strv src) {
    if (!src.ends_with(".bf")) {
        this->m_panic("File should have '.bf' as extension");
    }

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

    auto get_indentation = [&ofile, this]() ->std::string {
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
        << "// Forward declare IO functions\n"
        << codegen_fn_decl_in << "\n"
        << codegen_fn_decl_out << "\n"
        << "\n"
        << "#define MAP_SIZE (" << "25000" << ")\n"
        << "static unsigned char map[MAP_SIZE] = {0};\n"
        << "\n"
        << "int main() {\n"
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
            // in/out pointer
            case ',': {
                if(get_indentation()==strv(INDENT)) ofile << "\n"; // put nl only if not in loop
                ofile
                    << get_indentation()
                    << "in(ptr);\n";
                break;
            }
            case '.': {
                ofile << get_indentation() << "out(*ptr);\n";
                if(get_indentation()==strv(INDENT)) ofile << "\n"; // put nl only if not in loop
                break;
            }
            // loop
            case '[': {
                if (step() && peek()=='-') {
                    if (step() && peek()==']') {
                        step();
                        ofile << get_indentation() << "*ptr = 0;\n";
                        break;
                    } else step(-1);
                } else step(-1);

                ofile << "\n";
                ofile << get_indentation() << "while (*ptr) {\n";
                bracket_pairs.push(m_cur);
                break;
            }
            case ']': {
                bracket_pairs.pop();
                ofile << get_indentation() << "}\n";
                break;
            }
        }

        step();
    }

    ofile
        << "}\n"
        << "\n\n\n/* Definitions of in() and out() */\n\n"
        << codegen_fn_def_in
        << "\n\n"
        << codegen_fn_def_out
    ;
    return 0;
}
