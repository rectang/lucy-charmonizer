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

#define CHAZ_USE_SHORT_NAMES

#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"
#include "Charmonizer/Core/Compiler.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ConfElemType {
    CONFELEM_DEF,
    CONFELEM_TYPEDEF,
    CONFELEM_SYS_INCLUDE,
    CONFELEM_LOCAL_INCLUDE
} ConfElemType;

typedef struct ConfElem {
    char *str1;
    char *str2;
    ConfElemType type;
} ConfElem;

/* Static vars. */
static FILE *charmony_fh  = NULL;
static ConfElem *defs      = NULL;
static size_t    def_cap   = 0;
static size_t    def_count = 0;

/* Push a new elem onto the def list. */
static void
S_push_def_list_item(const char *str1, const char *str2, ConfElemType type);

/* Free the def list. */
static void
S_clear_def_list(void);

void
ConfWriter_init(void) {
    return;
}

void
ConfWriter_open_charmony_h(const char *charmony_start) {
    /* Open the filehandle. */
    charmony_fh = fopen("charmony.h", "w+");
    if (charmony_fh == NULL) {
        Util_die("Can't open 'charmony.h': %s", strerror(errno));
    }

    /* Print supplied text (if any) along with warning, open include guard. */
    if (charmony_start != NULL) {
        fwrite(charmony_start, sizeof(char), strlen(charmony_start),
               charmony_fh);
    }
    fprintf(charmony_fh,
            "/* Header file auto-generated by Charmonizer. \n"
            " * DO NOT EDIT THIS FILE!!\n"
            " */\n\n"
            "#ifndef H_CHARMONY\n"
            "#define H_CHARMONY 1\n\n"
           );
}

FILE*
ConfWriter_get_charmony_fh(void) {
    return charmony_fh;
}

void
ConfWriter_clean_up(void) {
    /* Write the last bit of charmony.h and close. */
    fprintf(charmony_fh, "#endif /* H_CHARMONY */\n\n");
    if (fclose(charmony_fh)) {
        Util_die("Couldn't close 'charmony.h': %s", strerror(errno));
    }
}

void
ConfWriter_append_conf(const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(charmony_fh, fmt, args);
    va_end(args);
}

static int
S_sym_is_uppercase(const char *sym) {
    unsigned i;
    for (i = 0; sym[i] != 0; i++) {
        if (!isupper(sym[i])) {
            if (islower(sym[i])) {
                return false;
            }
            else if (sym[i] != '_') {
                return true;
            }
        }
    }
    return true;
}

void
ConfWriter_add_def(const char *sym, const char *value) {
    S_push_def_list_item(sym, value, CONFELEM_DEF);
}

static void
S_append_def_to_conf(const char *sym, const char *value) {
    if (value) {
        if (S_sym_is_uppercase(sym)) {
            fprintf(charmony_fh, "#define CHY_%s %s\n", sym, value);
        }
        else {
            fprintf(charmony_fh, "#define chy_%s %s\n", sym, value);
        }
    }
    else {
        if (S_sym_is_uppercase(sym)) {
            fprintf(charmony_fh, "#define CHY_%s\n", sym);
        }
        else {
            fprintf(charmony_fh, "#define chy_%s\n", sym);
        }
    }
}

void
ConfWriter_add_typedef(const char *type, const char *alias) {
    S_push_def_list_item(type, alias, CONFELEM_TYPEDEF);
}

static void
S_append_typedef_to_conf(const char *type, const char *alias) {
    if (S_sym_is_uppercase(alias)) {
        fprintf(charmony_fh, "typedef %s CHY_%s;\n", type, alias);
    }
    else {
        fprintf(charmony_fh, "typedef %s chy_%s;\n", type, alias);
    }
}

void
ConfWriter_add_sys_include(const char *header) {
    S_push_def_list_item(header, NULL, CONFELEM_SYS_INCLUDE);
}

static void
S_append_sys_include_to_conf(const char *header) {
    fprintf(charmony_fh, "#include <%s>\n", header);
}

void
ConfWriter_add_local_include(const char *header) {
    S_push_def_list_item(header, NULL, CONFELEM_LOCAL_INCLUDE);
}

static void
S_append_local_include_to_conf(const char *header) {
    fprintf(charmony_fh, "#include \"%s\"\n", header);
}

void
ConfWriter_start_short_names(void) {
    ConfWriter_append_conf(
        "\n#if defined(CHY_USE_SHORT_NAMES) "
        "|| defined(CHAZ_USE_SHORT_NAMES)\n"
    );
}

void
ConfWriter_end_short_names(void) {
    ConfWriter_append_conf("#endif /* USE_SHORT_NAMES */\n");
}

void
ConfWriter_start_module(const char *module_name) {
    if (chaz_Util_verbosity > 0) {
        printf("Running %s module...\n", module_name);
    }
    ConfWriter_append_conf("\n/* %s */\n", module_name);
}

void
ConfWriter_end_module(void) {
    size_t i;
    for (i = 0; i < def_count; i++) {
        switch (defs[i].type) {
            case CONFELEM_DEF:
                S_append_def_to_conf(defs[i].str1, defs[i].str2);
                break;
            case CONFELEM_TYPEDEF:
                S_append_typedef_to_conf(defs[i].str1, defs[i].str2);
                break;
            case CONFELEM_SYS_INCLUDE:
                S_append_sys_include_to_conf(defs[i].str1);
                break;
            case CONFELEM_LOCAL_INCLUDE:
                S_append_local_include_to_conf(defs[i].str1);
                break;
            default:
                Util_die("Internal error: bad element type %d",
                         (int)defs[i].type);
        }
    }

    /* Write out short names. */
    ConfWriter_append_conf(
        "\n#if defined(CHY_USE_SHORT_NAMES) "
        "|| defined(CHAZ_USE_SHORT_NAMES)\n"
    );
    for (i = 0; i < def_count; i++) {
        switch (defs[i].type) {
            case CONFELEM_DEF: {
                    const char *sym = defs[i].str1;
                    const char *value = defs[i].str2;
                    if (!value || strcmp(sym, value) != 0) {
                        const char *prefix = S_sym_is_uppercase(sym)
                                             ? "CHY_" : "chy_";
                        ConfWriter_append_conf("  #define %s %s%s\n", sym,
                                               prefix, sym);
                    }
                }
                break;
            case CONFELEM_TYPEDEF: {
                    const char *sym = defs[i].str2;
                    const char *value = defs[i].str1;
                    if (strcmp(sym, value) != 0) {
                        const char *prefix = S_sym_is_uppercase(sym)
                                             ? "CHY_" : "chy_";
                        ConfWriter_append_conf("  #define %s %s%s\n", sym,
                                               prefix, sym);
                    }
                }
                break;
            case CONFELEM_SYS_INCLUDE:
            case CONFELEM_LOCAL_INCLUDE:
                /* no-op */
                break;
            default:
                Util_die("Internal error: bad element type %d",
                         (int)defs[i].type);
        }
    }

    ConfWriter_append_conf("#endif /* USE_SHORT_NAMES */\n");
    ConfWriter_append_conf("\n");

    S_clear_def_list();
}

void
ConfWriter_shorten_macro(const char *sym) {
    ConfWriter_append_conf("  #define %s CHY_%s\n", sym, sym);
}

void
ConfWriter_shorten_typedef(const char *sym) {
    ConfWriter_append_conf("  #define %s chy_%s\n", sym, sym);
}

void
ConfWriter_shorten_function(const char *sym) {
    ConfWriter_append_conf("  #define %s chy_%s\n", sym, sym);
}

static void
S_push_def_list_item(const char *str1, const char *str2, ConfElemType type) {
    if (def_count >= def_cap) {
        def_cap += 10;
        defs = (ConfElem*)realloc(defs, def_cap * sizeof(ConfElem));
    }
    defs[def_count].str1 = str1 ? Util_strdup(str1) : NULL;
    defs[def_count].str2 = str2 ? Util_strdup(str2) : NULL;
    defs[def_count].type = type;
    def_count++;
}

static void
S_clear_def_list(void) {
    size_t i;
    for (i = 0; i < def_count; i++) {
        free(defs[i].str1);
        free(defs[i].str2);
    }
    free(defs);
    defs      = NULL;
    def_cap   = 0;
    def_count = 0;
}

