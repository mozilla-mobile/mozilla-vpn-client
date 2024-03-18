# Clang-Tidy

## Clang-Tidy Setup

The Mozilla VPN Client project utilizes Clang-Tidy to enforce code quality standards. Clang-Tidy is provided with our [conda-environment](../Building/index.md).

## Prerequisites
In general, the default conda-env should be working out of the box, in case its not: 

To run clang-tidy you need to generate a [compile-commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html), 

1. Set the Generator to Ninja or Makefiles
2. Use Clang as the compiler
3. Enable compilation-databases (`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`)


## Running Clang-Tidy
You can build a clang-tidy report by calling:
```Bash
    # For all files
    cmake --build <build_dir> --target clang_tidy_report
    # Just a subtarget
    cmake --build <build_dir> --target ${target}_clang_tidy_report
```

This will report errors and suggestions to the console, as well as build a yaml report for each target in `<build_dir>/clang-tidy/<target>-report.yaml`.


## Fixing Issues: 
Some checks can be fixed automatically. You can use `clang-apply-replacements` to auto apply any suggestion. 
```Bash
    # Generate a report in <build_dir>/clang-tidy/
    cmake --build <build_dir> --target clang_tidy_report
    # Apply all suggestions: 
    clang-apply-replacements <build_dir>/clang-tidy/
```
