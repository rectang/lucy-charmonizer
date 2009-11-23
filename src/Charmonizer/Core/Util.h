/* Chaz/Core/Util.h -- miscellaneous utilities.
 */

#ifndef H_CHAZ_UTIL
#define H_CHAZ_UTIL 1

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

extern int chaz_Util_verbosity;

/* Open a file (truncating if necessary) and write [content] to it.  die() if
 * an error occurs.
 */
void
chaz_Util_write_file(const char *filename, const char *content);

/* Read an entire file into memory.
 */
char* 
chaz_Util_slurp_file(char *file_path, size_t *len_ptr);

/* Get the length of a file (may overshoot on text files under DOS).
 */
long  
chaz_Util_flength(FILE *f);

/* Print an error message to stderr and exit.
 */
void  
chaz_Util_die(char *format, ...);

/* Print an error message to stderr.
 */
void
chaz_Util_warn(char *format, ...);

/* Attept to delete a file.  Don't error if the file wasn't there to begin
 * with.  Return 1 if it seems like the file is gone because an attempt to
 * open it for reading fails (this doesn't guarantee that the file is gone,
 * but it works well enough for our purposes).  Return 0 if we can still 
 * read the file.
 */
int
chaz_Util_remove_and_verify(char *file_path);

/* Attempt to open a file for reading, then close it immediately.
 */
int
chaz_Util_can_open_file(char *file_path);

#ifdef CHAZ_USE_SHORT_NAMES
  #define verbosity              chaz_Util_verbosity 
  #define write_file             chaz_Util_write_file 
  #define slurp_file             chaz_Util_slurp_file 
  #define flength                chaz_Util_flength 
  #define die                    chaz_Util_die 
  #define warn                   chaz_Util_warn 
  #define remove_and_verify      chaz_Util_remove_and_verify 
  #define can_open_file          chaz_Util_can_open_file 
#endif

#endif /* H_CHAZ_UTIL */

/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

