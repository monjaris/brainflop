#pragma once
#include "core.hpp"

class Compiler
{
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

    inline bool step(uint32 n = 1) {
        m_cur += n;
        return lengthCheck();
    }

public:
    Compiler (const strv src);
    int32 preprocess();
    int32 transpile(strv outc);
};
