/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/CFlags.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/SharedLibrary.h"

struct chaz_CFlags {
    int   style;
    char *string;
};

chaz_CFlags*
chaz_CFlags_new(int style) {
    chaz_CFlags *flags = (chaz_CFlags*)malloc(sizeof(chaz_CFlags));
    flags->style  = style;
    flags->string = chaz_Util_strdup("");
    return flags;
}

void
chaz_CFlags_destroy(chaz_CFlags *flags) {
    free(flags->string);
    free(flags);
}

const char*
chaz_CFlags_get_string(chaz_CFlags *flags) {
    return flags->string;
}

void
chaz_CFlags_append(chaz_CFlags *flags, const char *string) {
    char *new_string;
    if (flags->string[0] == '\0') {
        new_string = chaz_Util_strdup(string);
    }
    else {
        new_string = chaz_Util_join(" ", flags->string, string, NULL);
    }
    free(flags->string);
    flags->string = new_string;
}

void
chaz_CFlags_clear(chaz_CFlags *flags) {
    if (flags->string[0] != '\0') {
        free(flags->string);
        flags->string = chaz_Util_strdup("");
    }
}

void
chaz_CFlags_set_output_obj(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/c /Fo";
    }
    else {
        /* POSIX */
        output = "-c -o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_set_output_exe(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/Fe";
    }
    else {
        /* POSIX */
        output = "-o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_define(chaz_CFlags *flags, const char *name,
                       const char *value) {
    const char *define;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        define = "/D";
    }
    else {
        /* POSIX */
        define = "-D ";
    }
    if (value) {
        string = chaz_Util_join("", define, name, "=", value, NULL);
    }
    else {
        string = chaz_Util_join("", define, name, NULL);
    }
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_include_dir(chaz_CFlags *flags, const char *dir) {
    const char *include;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC)  {
        include = "/I ";
    }
    else {
        /* POSIX */
        include = "-I ";
    }
    string = chaz_Util_join("", include, dir, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_enable_optimization(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/O2";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        string = "-O2";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        string = "-xO4";
    }
    else {
        /* POSIX */
        string = "-O 1";
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_enable_debugging(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU
        || flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        chaz_CFlags_append(flags, "-g");
    }
}

void
chaz_CFlags_disable_strict_aliasing(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        return;
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        chaz_CFlags_append(flags, "-fno-strict-aliasing");
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        chaz_CFlags_append(flags, "-xalias_level=any");
    }
    else {
        chaz_Util_die("Don't know how to disable strict aliasing with '%s'",
                      chaz_CC_get_cc());
    }
}

void
chaz_CFlags_set_warnings_as_errors(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/WX";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        string = "-Werror";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        string = "-errwarn=%all";
    }
    else {
        chaz_Util_die("Don't know how to set warnings as errors with '%s'",
                      chaz_CC_get_cc());
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_compile_shared_library(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/MD";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        const char *shlib_ext = chaz_OS_shared_lib_ext();
        if (strcmp(shlib_ext, ".dylib") == 0) {
            string = "-fno-common";
        }
        else if (strcmp(shlib_ext, ".so") == 0) {
            string = "-fPIC";
        }
        else if (strcmp(shlib_ext, ".dll") == 0) {
            /* MinGW */
            return;
        }
        else {
            /* unknown */
            return;
        }
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        string = "-KPIC";
    }
    else {
        return;
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_hide_symbols(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        if (strcmp(chaz_OS_shared_lib_ext(), ".dll") != 0) {
            chaz_CFlags_append(flags, "-fvisibility=hidden");
        }
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        if (chaz_CC_sun_c_version_num() >= 0x550) {
            /* Sun Studio 8. */
            chaz_CFlags_append(flags, "-xldscope=hidden");
        }
    }
}

void
chaz_CFlags_link_shared_library(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/DLL";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        if (strcmp(chaz_OS_shared_lib_ext(), ".dylib") == 0) {
            string = "-dynamiclib";
        }
        else {
            string = "-shared";
        }
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        string = "-G";
    }
    else {
        chaz_Util_die("Don't know how to link a shared library with '%s'",
                      chaz_CC_get_cc());
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_set_shared_library_version(chaz_CFlags *flags,
                                       chaz_SharedLib *lib) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        const char *shlib_ext = chaz_OS_shared_lib_ext();

        if (strcmp(shlib_ext, ".dylib") == 0) {
            const char *version = chaz_SharedLib_get_version(lib);
            char *string
                = chaz_Util_join(" ", "-current_version", version, NULL);
            chaz_CFlags_append(flags, string);
            free(string);
        }
        else if (strcmp(shlib_ext, ".so") == 0) {
            char *soname = chaz_SharedLib_major_version_filename(lib);
            char *string = chaz_Util_join("", "-Wl,-soname,", soname, NULL);
            chaz_CFlags_append(flags, string);
            free(string);
            free(soname);
        }
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_SUN_C) {
        char *soname = chaz_SharedLib_major_version_filename(lib);
        char *string = chaz_Util_join(" ", "-h", soname, NULL);
        chaz_CFlags_append(flags, string);
        free(string);
        free(soname);
    }
}

void
chaz_CFlags_set_link_output(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/OUT:";
    }
    else {
        output = "-o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_library_path(chaz_CFlags *flags, const char *directory) {
    const char *lib_path;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        if (strcmp(directory, ".") == 0) {
            /* The MS linker searches the current directory by default. */
            return;
        }
        else {
            lib_path = "/LIBPATH:";
        }
    }
    else {
        lib_path = "-L ";
    }
    string = chaz_Util_join("", lib_path, directory, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_library(chaz_CFlags *flags, chaz_SharedLib *lib) {
    char *filename;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        filename = chaz_SharedLib_implib_filename(lib);
    }
    else {
        filename = chaz_SharedLib_filename(lib);
    }
    chaz_CFlags_append(flags, filename);
    free(filename);
}

void
chaz_CFlags_add_external_library(chaz_CFlags *flags, const char *library) {
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = chaz_Util_join("", library, ".lib", NULL);
    }
    else {
        string = chaz_Util_join(" ", "-l", library, NULL);
    }
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_enable_code_coverage(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        chaz_CFlags_append(flags, "--coverage");
    }
    else {
        chaz_Util_die("Don't know how to enable code coverage with '%s'",
                      chaz_CC_get_cc());
    }
}


