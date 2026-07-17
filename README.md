# Brainflop - Modern BrainF*ck compiler

Brainflop is an optimized Brainfuck compiler and runner. It can execute Brainfuck programs directly, or transpile them to C with a few different code-generation modes depending on what you're optimizing for.

This isn't a naive interpreter. The compiler applies real optimizations on top of the raw instruction stream before anything gets run or generated, so both execution and the generated C output are meant to be fast.

## Features

- Built with modern C++ (C++23)
- Runs Brainfuck programs directly
- Transpiles Brainfuck to C
- Additional C generation modes (including a dense/macro-based mode) so you can pick between readability and raw output size/speed depending on what you're doing with the generated code

## Installing

On Arch Linux and derivatives, install via AUR:

```sh
yay -S brainflop  # or paru
```

## Building from source

Brainflop uses [xmake](https://xmake.io) as its build system.

```sh
git clone https://github.com/Monjaris/brainflop.git
cd brainflop
xmake
```

## Usage

```sh
bfc --run main.bf              # -r, run directly, no C generated
bfc --build main.bf -o main    # -b, transpile to C and compile to a binary
bfc --compile main.bf -o main.c  # -c, transpile to C only, don't build
```

Optimization level for the `--build` step can be set the same way you would for a C compiler:

```sh
bfc -O3 --build main.bf -o main
```

## Examples

See the `examples/` directory for sample Brainfuck programs you can run or transpile.

## License

MIT (See [license file](LICENSE.txt))
