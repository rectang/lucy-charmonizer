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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Charmonizer/CLI.h"
#include "Charmonizer/Core/Util.h"

typedef struct chaz_CLIOption {
    const char *name;
    const char *desc;
    char *value;
    int flags;
} chaz_CLIOption;

struct chaz_CLI {
    char *desc;
    char *usage;
    char *error;
    chaz_CLIOption *opts;
    int num_opts;
    int die_on_failure;
}

chaz_CLI*
chaz_CLI_new(const char *description) {
    chaz_CLI *self = calloc(sizeof(chaz_CLI));
    self->desc = chaz_Util_strdup(description ? description : "");
    self->error = chaz_Util_strdup("");
    self->num_opts = 0;
    self->opts = calloc(self->num_opts * sizeof(chaz_CLIOption));
    return self;
}

void
chaz_CLI_destroy(chaz_CLI *self) {
    int i;
    for (i = 0; i < self->num_opts; i++) {
        chaz_CLIOption *opt = &self->opts[i];
        free(opt->name);
        free(opt->desc);
        free(opt->value);
    }
    free(self->opts);
    free(self->usage);
    free(self->desc);
    free(self->error);
}

int
chaz_CLI_register(chaz_CLI *self, const char *name, const char *description,
                  int flags) {
    int rank;
    if (self->usage != NULL) {
        free(self->usage);
        self->usage = NULL;
    }
    for (rank = 0; rank < self->num_opts; rank++) {
        int result = strcmp(name, self->opts[rank].name);
        if (result == 0) {
            S_report_error(self);
            return 0;
        }
        else if result

    }
}
