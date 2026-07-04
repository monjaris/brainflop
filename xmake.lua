--- UNIVERSAL
set_arch("x86_64")
set_plat("linux")

--- RULES/POLICIES
add_rules("mode.debug", "mode.release"); set_defaultmode("mode.debug")
add_rules("plugin.compile_commands.autoupdate")

--- TOOLCHAIN
toolchain(".gnu")
    set_kind("standalone"); set_toolset("cxx", "g++")
    set_toolset("as",    "as"); set_toolset("ar",    "ar")
    set_toolset("ld",    "g++"); set_toolset("sh",    "g++")
    set_toolset("ex",    "g++"); set_toolset("strip", "strip")
toolchain_end()

set_toolchains(".gnu")

--- SCRIPT-BEGIN
    if is_mode("debug") then
        set_optimize("none")
        -- set_symbols("debug")
        set_symbols("none")
    elseif is_mode("release") then
        add_defines("DEBUG_ON=0")
        set_optimize("fastest")
        set_symbols("none")
        set_strip("all")
    end
--- SCRIPT-END


--- GLOBAL
set_languages("c++23")
add_includedirs("include/")
set_pcxxheader("include/common.hpp")

--- TARGETS
target("bfc")
    set_kind("binary")
    add_files("src/*.cpp")
