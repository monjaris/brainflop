#pragma once
#include "transpiler.hpp"
#include <getopt.h>


NAMESPACE_START(cli)

namespace detail {
    inline int _argc;
    inline char**  _argv;
}
using namespace detail;


namespace o_short {
    enum : unsigned char {
        BUILD = 'b',
        RUN = 'r',
    };

    inline const char* get_all() {
        static char all[] = {
            BUILD, ':',
            RUN, ':',
        };
        return all;
    }
};


namespace o_long {
    constexpr char ARG_NONE = no_argument;
    constexpr char ARG_REQUIRE = required_argument;
    constexpr char ARG_OPTIONAL = optional_argument;

    constexpr auto BUILD = "build";
    constexpr auto RUN = "run";

    static const option base[] = {
        option {BUILD, ARG_REQUIRE, nullptr, o_short::BUILD},
        option {RUN, ARG_REQUIRE, nullptr, o_short::RUN}
    };
};


namespace cmd {
    inline std::string build(const strv file, const strv out_c, strv out_bin="") {
        std::string output = out_bin.empty()?
            (std::string)file.substr(0, file.size()-3) : (std::string)out_bin
        ;

        Compiler bf_tp(file);
        bf_tp.preprocess();
        bf_tp.transpile(out_c);

        ::system(std::format(
            "cc -o {} {}",
            output, out_c
        ).c_str());

        return output;
    }


    inline void run(const strv file) {
        std::string out = build(file, "/tmp/brainfuck.c");
        ::system(std::format(
            "./{0} && rm {0}", out
        ).c_str());
    }
};


inline void init(int argc, char** argv) {
    _argc = argc;
    _argv = argv;
}

inline void parse()
{
    int32 c;
    while ((c = ::getopt_long(_argc, _argv, o_short::get_all(), o_long::base, nullptr)) != -1)
    {
        switch (c)
        {
            case o_short::BUILD: {
                cmd::build(optarg, "/tmp/brainfuck.c");
                break;
            }
            case o_short::RUN: {
                cmd::run(optarg);
                break;
            }
            default : {
                ;
            }
        }
    }
}

NAMESPACE_END(cli)
