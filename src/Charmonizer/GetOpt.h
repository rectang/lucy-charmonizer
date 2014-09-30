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

#ifndef H_CHAZ_CLI
#define H_CHAZ_CLI 1

#ifdef __cplusplus
extern "C" {
#endif

#define CHAZ_CLI_ARG_REQUIRED (1 << 0)
#define CHAZ_CLI_ARG_OPTIONAL (1 << 1)
#define CHAZ_CLI_STRING       (1 << 16)
#define CHAZ_CLI_INT          (1 << 17)

typedef struct chaz_CLI chaz_CLI;

/* Constructor.
 */
chaz_CLI*
chaz_CLI_new(const char *description);

/* Set error handling behavior to die on error if true.
 */
void
chaz_CLI_set_die_on_error(chaz_CLI *self, int die_on_error);

/* Return a string with the names and descriptions of all options registered.
 */
const char*
chaz_CLI_usage(chaz_CLI *self);

/* Register an option.  Returns true on success, or sets error and returns
 * false if the option was already registered.
 */
int
chaz_CLI_register(chaz_CLI *self, const char *name, const char *description,
                  int flags);

/* Set an option.  The specified option must have been registered previously.
 * The supplied `value` is optional and will be copied.
 *
 * Returns true on success.  Sets `error` and returns false on failure.
 */
int
chaz_CLI_set(chaz_CLI *self, const char *name, void *value);

/* Search for an option.  Returns true if the option has been set.
 * 
 * If `value_ptr` is not NULL, it will be populated, either with a value or
 * with NULL.
 */
int
chaz_CLI_get(const char *name, void **value_ptr);

/* Parse `argc` and `argv`, setting options as appropriate.  Returns true on
 * success.  Sets `error` and returns false if either an unexpected option was
 * encountered or an option which requires an argument was supplied without
 * one.
 */
void
chaz_CLI_parse(chaz_CLI *self, int argc, const char *argv[]);

/* Return a pointer to a string describing the last error that occurred.
 */
const char**
chaz_CLI_error(chaz_CLI *self);

/* Destructor.
 */
void
chaz_CLI_destroy(chaz_CLI *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_CLI */


