--- RULES/POLICIES
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")
add_rules("plugin.compile_commands.autoupdate")


--- SCRIPT-BEGIN
    if is_mode("debug") then
        set_optimize("none")
        set_symbols("debug")
        set_symbols("none")
    elseif is_mode("release") then
        add_defines("DEBUG_ON=0")
        set_optimize("fastest")
        set_symbols("none")
        set_strip("all")
    end


--- GLOBAL
set_languages("c++23")
add_includedirs("include/")
set_pcxxheader("include/common.hpp")

--- TARGETS
target("bfc")
    set_kind("binary")
    add_files("src/*.cpp")

    after_build(function (target)
        os.cp(target:targetfile(), "$(projectdir)/bfc")
    end)
