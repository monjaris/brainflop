#pragma once
#include "transpiler.hpp"
#include <getopt.h>


NAMESPACE_START(cli)
namespace detail {
    inline int _argc;
    inline char** _argv;

    inline std::string _out;
    inline uint32 _opt_lvl = 1;

    inline std::string _pending_input;
    inline bool _do_compile = false;
    inline bool _do_build = false;
    inline bool _do_run = false;

    inline std::string* out() {
        return &_out;
    }
}
using namespace detail;

namespace o_short {
    enum : unsigned char {
        COMPILE = 'c', BUILD = 'b', RUN = 'r', OUT = 'o'
    };
    inline const char* get_all() { return "c:b:r:o:"; }
};

namespace o_long {
    static const option base[] = {
        option {"compile", required_argument, nullptr, o_short::COMPILE},
        option {"build",   required_argument, nullptr, o_short::BUILD},
        option {"run",     required_argument, nullptr, o_short::RUN},
        option {"out",     required_argument, nullptr, o_short::OUT},
        {nullptr, 0, nullptr, 0}
    };
};

namespace cmd {
    inline std::string transpile(const strv file) {
        Compiler bf_tp(file);
        bf_tp.preprocess();
        bf_tp.transpile(*out());
        return *out();
    }

    inline std::string build(
        const strv file
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


    inline void run(const strv file) {
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
            case o_short::OUT:     *out() = optarg; break;
            case o_short::COMPILE: _do_compile = true; _pending_input = optarg; break;
            case o_short::BUILD:   _do_build = true; _pending_input = optarg; break;
            case o_short::RUN:     _do_run = true; _pending_input = optarg; break;
            default: cr::print("Unknown option"); break;
        }
    }
}

inline void execute()
{
    // no getopt_long here at all — just act on what was collected
    if (_do_compile) {
        std::string out_c = !out()->empty() ? *out()
            : std::string(strv(_pending_input).substr(0, _pending_input.size()-3)) + ".c";
        cmd::transpile(_pending_input);
        cr::print("Compiled '", _pending_input, "' to '", out_c, "'");
    }
    if (_do_build) {
        auto out_b = cmd::build(_pending_input);
        cr::print("Built '", _pending_input, "' outputted '", out_b, "'");
    }
    if (_do_run) {
        cr::log("Running '", _pending_input, "'");
        cmd::run(_pending_input);
    }
}

NAMESPACE_END(cli)
