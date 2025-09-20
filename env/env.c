#define _CRT_SECURE_NO_WARNINGS
#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Internal key/value pair for parsed .env entries.
 *
 * Used inside env.c to hold variables before injection into the process
 * environment. Not exposed as part of the public API.
 */
typedef struct {
	char* key;   /**< Variable name */
	char* value; /**< Variable value */
} EnvEntry;

static EnvEntry* entries=NULL;
static size_t entry_count=0;

/**
 * @brief Duplicate a string with malloc().
 * @param s Input string (can be NULL).
 * @return Newly allocated copy of @p s, or NULL if @p s is NULL or malloc fails.
 */
static char* xstrdup(const char* s) {
	if(!s) return NULL;
	size_t n=strlen(s)+1;
	char* p=(char*)malloc(n);
	if(p) memcpy(p, s, n);
	return p;
}

/**
 * @brief Skip leading spaces and tabs in a string.
 * @param s Input string (modifiable).
 * @return Pointer to first non-space/tab character in @p s.
 */
static char* trim_left(char* s) {
	while(*s==' '||*s=='\t') s++;
	return s;
}

/**
 * @brief Remove trailing spaces, tabs, and line endings from a string in-place.
 * @param s Input string (modified in place).
 */
static void trim_right_inplace(char* s) {
	char* end=s+strlen(s);
	while(end>s&&(end[-1]==' '||end[-1]=='\t'||end[-1]=='\r'||end[-1]=='\n'))
		*--end=0;
}

/**
 * @brief Remove surrounding quotes and process escape sequences in-place.
 *
 * Surrounding single or double quotes are stripped if present. Recognizes:
 * - `\n` → newline
 * - `\r` → carriage return
 * - `\t` → tab
 * - `\\` → backslash
 * - `\"` → double quote
 * - `\'` → single quote
 *
 * @param s Input string (modified in place).
 */
static void unquote_and_unescape_inplace(char* s) {
	if(!s||!*s) return;
	size_t len=strlen(s);
	if((s[0]=='"'&&s[len-1]=='"')||(s[0]=='\''&&s[len-1]=='\'')) {
		s[len-1]=0;
		memmove(s, s+1, strlen(s+1)+1);
	}
	char* r=s;
	char* p=s;
	while(*p) {
		if(*p=='\\'&&p[1]) {
			p++;
			switch(*p) {
				case 'n': *r++='\n'; break;
				case 'r': *r++='\r'; break;
				case 't': *r++='\t'; break;
				case '\\': *r++='\\'; break;
				case '"': *r++='"'; break;
				case '\'': *r++='\''; break;
				default: *r++=*p; break;
			}
			p++;
		}
		else {
			*r++=*p++;
		}
	}
	*r=0;
}

/**
 * @brief Trim both left and right whitespace in-place.
 * @param s Input string.
 * @return Pointer to trimmed string.
 */
static char* trim(char* s) {
	s=trim_left(s);
	trim_right_inplace(s);
	return s;
}

/**
 * @brief Load environment variables from a .env file.
 *
 * Lines beginning with `#` are treated as comments and ignored.
 * Keys and values are separated by `=`.
 *
 * @param path Path to .env file.
 * @return 0 on success, -1 on error (file not found, allocation failure, etc.).
 */
int env_load(const char* path) {
	if(!path) return -1;
	FILE* f=fopen(path, "rb");
	if(!f) return -1;
	char buf[4096];
	while(fgets(buf, sizeof(buf), f)) {
		char* line=trim(buf);
		if(*line==0||*line=='#') continue;
		char* eq=strchr(line, '=');
		if(!eq) continue;
		*eq=0;
		char* k=trim(line);
		char* v=trim(eq+1);
		unquote_and_unescape_inplace(v);
		EnvEntry* tmp=(EnvEntry*)realloc(entries, (entry_count+1)*sizeof(EnvEntry));
		if(!tmp) {
			fclose(f); return -1;
		}
		entries=tmp;
		entries[entry_count].key=xstrdup(k);
		entries[entry_count].value=xstrdup(v);
		if(!entries[entry_count].key||!entries[entry_count].value) {
			fclose(f); return -1;
		}
		entry_count++;
	}
	fclose(f);
	return 0;
}

/**
 * @brief Get value of an environment variable loaded from .env.
 * @param key Environment variable key.
 * @return Value string or NULL if not found.
 */
const char* env_get(const char* key) {
	if(!key) return NULL;
	for(size_t i=0;i<entry_count;i++)
		if(strcmp(entries[i].key, key)==0)
			return entries[i].value;
	return NULL;
}

/**
 * @brief Inject a single key/value into the process environment.
 *
 * On Windows uses `_putenv_s()`. On POSIX uses `setenv()`.
 *
 * @param k Key.
 * @param v Value.
 * @param overwrite Nonzero to overwrite existing.
 * @return 0 on success, -1 on failure.
 */
static int inject_single(const char* k, const char* v, int overwrite) {
#ifdef _WIN32
	if(!overwrite) {
		if(getenv(k)) return 0;
	}
	return _putenv_s(k, v)==0 ? 0 : -1;
#else
	return setenv(k, v, overwrite)==0 ? 0 : -1;
#endif
}

/**
 * @brief Inject all loaded .env variables into the process environment.
 * @param overwrite Nonzero to overwrite existing system variables.
 * @return 0 on success, -1 on error.
 */
int env_inject_all(int overwrite) {
	for(size_t i=0;i<entry_count;i++) {
		if(inject_single(entries[i].key, entries[i].value, overwrite)!=0)
			return -1;
	}
	return 0;
}

/**
 * @brief Free all memory used by loaded .env variables.
 */
void env_free(void) {
	for(size_t i=0;i<entry_count;i++) {
		free(entries[i].key);
		free(entries[i].value);
	}
	free(entries);
	entries=NULL;
	entry_count=0;
}
