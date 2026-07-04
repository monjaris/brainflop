#include "bfc/transpiler.hpp"

int main(int argc, char **argv) {
    strv file = "main.bf";
    strv outc = "brainfuck.c";
    strv outb = "brainfucked";

    Compiler bf_tp(file.data());
    int32 err_preproc = bf_tp.preprocess();
    if (err_preproc) return 1;

    int32 err_transp = bf_tp.transpile(outc);
    if (err_transp) return 1;

    cr::log("Compiling C..");
    return cr::shell_exec("cc ./{0} -o ./{1}  &&  ./{1}", outc, outb);
}
