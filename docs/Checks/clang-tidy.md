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


## Help 


### Error: error: 'some.h' file not found [clang-diagnostic-error]
Make sure that either all targets (including tests) are built or try configuring with -DBUILD_TESTS=OFF. 

This might happen clang-tidy reads the compilation database for each file i.e `a.cpp`, if there are 2 objects build from `a.cpp` i.e:
```cmake
add_library(lib_a a.cpp)
add_library(lib_b b.cpp)
```
it will pick one, so either make sure only one lib is configured or even better break the double build if possible :) 
```
add_library(a a.cpp)
add_target_dependencies(lib_a a)
add_target_dependencies(lib_b a)
```
