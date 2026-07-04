#pragma once
#include "common.hpp"

#ifndef LOG_ON
    #define LOG_ON 1
#endif

#ifndef PRINT_ON
    #define PRINT_ON 1
#endif


// namespace for CoRe utilities that will be used for the main project
NAMESPACE_START(cr)

template <bool nl=true, bool flush=false, typename... Args>
void print(Args&&... args) {
#if !PRINT_ON
    return;
#endif
    std::ios_base::sync_with_stdio(false);
    (std::cout << ... << std::forward<Args>(args));
    if constexpr (nl) std::cout << '\n';
    if constexpr (flush) std::flush(std::cout);
}

template <typename... Args>
void log(Args&&... args) {
#if !LOG_ON
    return;
#endif
    (std::clog << ... << std::forward<Args>(args)) << '\n' << std::flush;
}

template <typename... Args> [[noreturn]]
void terminate(Args&&... args) {
    (std::cerr << ... << std::forward<Args>(args)) << std::endl;
    std::exit(1);
}


template <typename... Args>
int32 shell_exec(std::format_string<Args...> fmtstr, Args&&... args) {
    return ::system(
        std::format(fmtstr, std::forward<Args>(args)...).c_str()
    );
}


NAMESPACE_END(cr)
