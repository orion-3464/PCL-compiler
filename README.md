# PCL Compiler

A compiler for the PCL language, implemented in C++ using Flex, Bison and LLVM 19. It generates LLVM IR, assembly and executables for Linux x86_64.

## Requirements

The following tools must be available in your `PATH`:

- LLVM 19 and its development libraries (`llvm-config-19`).
- Clang / Clang++, with C23 support for the runtime library.
- Flex and Bison 3.8 or newer.
- GNU Make, GCC and Binutils (`ar`).

## Building

From the project root:

```bash
make
```

This builds the compiler `pclc` and the runtime library `build/libpcl.a`.
If your installed compilers use versioned names, run:

```bash
make CC=clang-19 CXX=clang++-19
```

## Usage

Run the compiler from the project root so it can locate the runtime library.

```bash
./pclc [-O] [-o executable] source.pcl
```

Compile and run a program:

```bash
./pclc -o program program.pcl
./program
```

The compiler also generates `program.imm` (LLVM IR) and `program.asm` (assembly) alongside the source file. Without `-o`, the executable is named `a.out`.

| Option | Description |
| --- | --- |
| `-o file` | Set the executable output path. |
| `-O` | Enable LLVM optimizations. |
| `-i` | Read source from stdin and write LLVM IR to stdout. |
| `-f` | Read source from stdin and write assembly to stdout. |

```bash
./pclc -O -o program program.pcl
./pclc -i < program.pcl
./pclc -f < program.pcl
```

The compiler returns `0` on successful compilation and a nonzero exit code on failure.

## Cleaning

- `make clean`: remove the `build` directory while keeping `pclc`.
- `make distclean`: also remove `pclc`.
