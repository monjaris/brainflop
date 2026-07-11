#include "transpiler.hpp"


Compiler::Compiler (const strview src) {
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


int32 Compiler::transpile(strview outc) {
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

    switch (getMode())
    {
        case CodeGenMode::NORMAL_C: { ofile << Codegen_Normal_Begin;     break; }
        case CodeGenMode::MACRO_C:  { ofile << Codegen_Macro_Begin; break; }
        case CodeGenMode::DENSE_C:  { ofile << Codegen_Dense_Begin;     break; }
    }

    while (lengthCheck())
    {
        switch (peek())
        {
            case '>': {
                uint32 n = count_instructions(peek());
                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "ptr += "<< n <<";\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "FORWARD("<< n <<")\n";  break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "ptr+="<< n <<";";       break; }
                }
                continue;
            }
            case '<': {
                uint32 n = count_instructions(peek());
                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "ptr -= "<< n <<";\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "BACKWARD("<< n <<")\n"; break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "ptr-="<< n <<";";       break; }
                }
                continue;
            }
            case '+': {
                uint32 n = count_instructions(peek());
                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "(*ptr) += "<< n <<";\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "ADD("<< n <<")\n";        break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "(*ptr)+="<< n <<";";      break; }
                }
                continue;
            }
            case '-': {
                uint32 n = count_instructions(peek());
                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "(*ptr) -= "<< n <<";\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "SUB("<< n <<")\n";        break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "(*ptr)-="<< n <<";";      break; }
                }
                continue;
            }
            case ',': {
                if(m_mode != CodeGenMode::DENSE_C)
                    if(get_indentation()==strview(INDENT)) ofile << "\n";

                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "in(ptr);\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "READ\n";      break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "in(ptr);";    break; }
                }
                break;
            }
            case '.': {
                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "out(*ptr);\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "WRITE\n";       break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "out(*ptr);";    break; }
                }

                if(m_mode != CodeGenMode::DENSE_C)
                    if(get_indentation()==strview(INDENT)) ofile << "\n";
                break;
            }
            case '[': {
                if (step() && peek()=='-') {
                    if (step() && peek()==']') {
                        step();
                        ofile << get_indentation() << "*ptr = 0;\n";
                        break;
                    } else step(-1);
                } else step(-1);

                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << "\n" << get_indentation() << "while (*ptr) {\n"; break; }
                    case CodeGenMode::MACRO_C:  { ofile << "\n" << get_indentation() << "LOOP_BEGIN\n";      break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "while(*ptr){";              break; }
                }

                bracket_pairs.push(m_cur);
                break;
            }
            case ']': {
                bracket_pairs.pop();

                switch (m_mode)
                {
                    case CodeGenMode::NORMAL_C: { ofile << get_indentation() << "}\n";      break; }
                    case CodeGenMode::MACRO_C:  { ofile << get_indentation() << "LOOP_END\n"; break; }
                    case CodeGenMode::DENSE_C:  { ofile << get_indentation() << "}";          break; }
                }
                break;
            }
        }

        step();
    }


    if (m_mode == CodeGenMode::DENSE_C) {
        ofile << "}" << Codegen_Fndef_in << Codegen_Fndef_Out;
    } else {
        ofile
            << "}\n"
            << "\n\n\n/* Definitions of in() and out() */\n\n"
            << Codegen_Fndef_in
            << "\n\n"
            << Codegen_Fndef_Out
        ;
    }

    return 0;
}
