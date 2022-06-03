# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

android {
    message(Breakpad enabled)

    BREAKPAD_DIR = $$PWD/../../3rdparty/breakpad

    INCLUDEPATH += $$BREAKPAD_DIR
    INCLUDEPATH += $$BREAKPAD_DIR/src

    HEADERS +=  $$BREAKPAD_DIR/src/client/linux/crash_generation/crash_generation_client.h \
                $$BREAKPAD_DIR/src/client/linux/dump_writer_common/thread_info.h \
                $$BREAKPAD_DIR/src/client/linux/dump_writer_common/ucontext_reader.h \
                $$BREAKPAD_DIR/src/client/linux/handler/exception_handler.h \
                $$BREAKPAD_DIR/src/client/linux/handler/minidump_descriptor.h \
                $$BREAKPAD_DIR/src/client/linux/log/log.h \
                $$BREAKPAD_DIR/src/client/linux/microdump_writer/microdump_writer.h \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/linux_dumper.h \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/linux_ptrace_dumper.h \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/minidump_writer.h \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/pe_file.h \
                $$BREAKPAD_DIR/src/client/minidump_file_writer.h \
                $$BREAKPAD_DIR/src/common/convert_UTF.h \
                $$BREAKPAD_DIR/src/common/md5.h \
                $$BREAKPAD_DIR/src/common/string_conversion.h \
                $$BREAKPAD_DIR/src/common/linux/breakpad_getcontext.S \
                $$BREAKPAD_DIR/src/common/linux/elfutils.h \
                $$BREAKPAD_DIR/src/common/linux/file_id.h \
                $$BREAKPAD_DIR/src/common/linux/guid_creator.h \
                $$BREAKPAD_DIR/src/common/linux/linux_libc_support.h \
                $$BREAKPAD_DIR/src/common/linux/memory_mapped_file.h \
                $$BREAKPAD_DIR/src/common/linux/safe_readlink.h


    # LOCAL_C_INCLUDES 
    # (LOCAL_PATH)/src/common/android/include 
    HEADERS +=  $$BREAKPAD_DIR/src/common/android/include/asm-mips/asm.h \
                $$BREAKPAD_DIR/src/common/android/include/asm-mips/fpregdef.h \
                $$BREAKPAD_DIR/src/common/android/include/asm-mips/regdef.h \
                $$BREAKPAD_DIR/src/common/android/include/elf.h \
                $$BREAKPAD_DIR/src/common/android/include/link.h \
                $$BREAKPAD_DIR/src/common/android/include/stab.h \
                $$BREAKPAD_DIR/src/common/android/include/sys/procfs.h \
                $$BREAKPAD_DIR/src/common/android/include/sys/user.h 


    # List of client source files, directly taken from Android.mk
    SOURCES +=  $$BREAKPAD_DIR/src/client/linux/crash_generation/crash_generation_client.cc \
                $$BREAKPAD_DIR/src/client/linux/dump_writer_common/thread_info.cc \
                $$BREAKPAD_DIR/src/client/linux/dump_writer_common/ucontext_reader.cc \
                $$BREAKPAD_DIR/src/client/linux/handler/exception_handler.cc \
                $$BREAKPAD_DIR/src/client/linux/handler/minidump_descriptor.cc \
                $$BREAKPAD_DIR/src/client/linux/log/log.cc \
                $$BREAKPAD_DIR/src/client/linux/microdump_writer/microdump_writer.cc \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/linux_dumper.cc \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/linux_ptrace_dumper.cc \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/minidump_writer.cc \
                $$BREAKPAD_DIR/src/client/linux/minidump_writer/pe_file.cc \
                $$BREAKPAD_DIR/src/client/minidump_file_writer.cc \
                $$BREAKPAD_DIR/src/common/convert_UTF.cc \
                $$BREAKPAD_DIR/src/common/md5.cc \
                $$BREAKPAD_DIR/src/common/string_conversion.cc \
                $$BREAKPAD_DIR/src/common/linux/breakpad_getcontext.S \
                $$BREAKPAD_DIR/src/common/linux/elfutils.cc \
                $$BREAKPAD_DIR/src/common/linux/file_id.cc \
                $$BREAKPAD_DIR/src/common/linux/guid_creator.cc \
                $$BREAKPAD_DIR/src/common/linux/linux_libc_support.cc \
                $$BREAKPAD_DIR/src/common/linux/memory_mapped_file.cc \
                $$BREAKPAD_DIR/src/common/linux/safe_readlink.cc
    # Our own adaptor
    SOURCES +=   $$PWD/../crashreporter/breakpad/androidbreakpad.cpp
    HEADERS +=   $$PWD/../crashreporter/breakpad/androidbreakpad.cpp
     
}


