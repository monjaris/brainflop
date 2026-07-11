#pragma once
#include "transpiler.hpp"
#include <getopt.h>


NAMESPACE_START(cli)
namespace detail {
    inline int _argc;
    inline char** _argv;

    inline std::string _out;
    inline int32 _opt_lvl = 1;

    inline std::string _input_file;
    inline Compiler::CodeGenMode _gen_mode = Compiler::CodeGenMode::NORMAL_C;
    inline bool _do_compile = false;
    inline bool _do_build = false;
    inline bool _do_run = false;

    inline std::string* out() {
        return &_out;
    }

    inline const char* this_arg() {
        return optarg;
    }
}
using namespace detail;

namespace o_short {
    enum : unsigned char {
        OUT = 'o', OPT = 'O', MODE = 'm',
        COMPILE = 'c', BUILD = 'b', RUN = 'r',
    };
    inline const char* get_all() { return "c:b:r:o:m:O:"; }
};

namespace o_long {
    static const option base[] = {
        // data flags`
        option {"out",     required_argument, nullptr, o_short::OUT},
        option {"optimize",     required_argument, nullptr, o_short::OPT},
        option {"mode",     required_argument, nullptr, o_short::MODE},
        // action flags
        option {"compile", required_argument, nullptr, o_short::COMPILE},
        option {"build",   required_argument, nullptr, o_short::BUILD},
        option {"run",     required_argument, nullptr, o_short::RUN},
        {nullptr, 0, nullptr, 0}
    };
};

namespace cmd {
    inline std::string transpile(const strview file) {
        Compiler bf_tp(file);
        bf_tp.setMode(detail::_gen_mode);
        bf_tp.preprocess();
        bf_tp.transpile(*out());
        return *out();
    }

    inline std::string build(
        const strview file
    ) {
        if (out()->empty()) *out() = file.substr(0, file.size()-3).data();

        auto out_bin = *out();  // save _out
        *out() = "/tmp/brainflop.c";
        std::string out_c = transpile(file);
        *out() = out_bin;

        ::system(std::format(
            "cc -O{} -o {} {}",
            _opt_lvl, *out(), out_c
        ).c_str());

        return *out();
    }


    inline void run(const strview file) {
        *out() = "/tmp/brainflop";
        build(file);
        ::system(std::format(
            "{}", *out()
        ).c_str());
    }
};


inline void init(int argc, char** argv) {
    _argc = argc;
    _argv = argv;
}


inline void parse_collect()
{
    int32 c;
    while ((c = ::getopt_long(_argc, _argv, o_short::get_all(), o_long::base, nullptr)) != -1)
    {
        switch (c)
        {
            case o_short::COMPILE: _do_compile = true; _input_file = this_arg(); break;
            case o_short::BUILD:   _do_build = true; _input_file = this_arg(); break;
            case o_short::RUN:     _do_run = true; _input_file = this_arg(); break;
            case o_short::OUT:     *out() = this_arg(); break;
            case o_short::MODE: {
                try { _gen_mode = Compiler::StringToGenMode(this_arg()); break; }
                catch(std::exception& e) { cr::log(e.what()); };
            }
            case o_short::OPT: {
                if (strview(this_arg()).empty()) {
                    cr::log("Optimization level is empty value! Defaulting to -O1");
                    _opt_lvl = 1;
                } else {
                    try {
                        _opt_lvl = std::stoi(this_arg());
                    } catch (const std::exception& e) {
                        cr::log("Optimization level should be a number! Defaulting to -O1");
                        _opt_lvl = 1;
                    }
                }
                if (_opt_lvl > 3) {
                    cr::print("Optimization level cant be more than 3, defaulting to -O3 instead");
                    _opt_lvl = 3;
                } else if  (_opt_lvl < 0) {
                    cr::print("Optimization level cant be less than 0, defaulting to -O0 instead");
                    _opt_lvl = 0;
                }
                break;
            }
            default: cr::print("Unknown option"); break;
        }
    }
}

inline void execute()
{
    // no getopt_long here at all — just act on what was collected
    if (_do_compile) {
        std::string out_c = !out()->empty() ? *out()
            : std::string(strview(_input_file).substr(0, _input_file.size()-3)) + ".c";
        cmd::transpile(_input_file);
        cr::print("Compiled \033[35m", _input_file, "\033[0m to \033[32m", out_c, "\033[0m");
    }
    if (_do_build) {
        auto out_b = cmd::build(_input_file);
        cr::print("Built \033[35m", _input_file, "\033[0m outputted \033[32m", out_b, "\033[0m");
    }
    if (_do_run) {
        cr::log("Running \033[33m", _input_file, "\033[0m");
        cmd::run(_input_file);
    }
}

NAMESPACE_END(cli)
