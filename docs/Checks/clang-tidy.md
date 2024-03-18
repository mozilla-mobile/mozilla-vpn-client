# Clang-Tidy

## Clang-Tidy Setup

The Mozilla VPN Client project utilizes Clang-Tidy to enforce code quality standards. Clang-Tidy is provided with our [conda-enviroment](../Building/index.md).

## Prequisites

To run clang-tidy you need to generate a [compile-commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html), 

1. Set the Generator to Ninja or Makefile
2. Use Clang as the compiler
3. Enable compilation-databases (`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`)


## Running Clang-Tidy

Full Analysis: To run Clang-Tidy on all source files of a target:
```Bash
    cmake --build <build_dir> --target clang_tidy_all
    cmake --build <build_dir> --target <target_name>_clang_tidy
```

Fixing Issues: To automatically fix certain issues:
```Bash
    cmake --build <build_dir> --target clang_tidy_fix_all
    cmake --build <build_dir> --target <target_name>_clang_tidy_fix
```
