/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_util_h__
#define INCLUDE_util_h__

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define bitsizeof(x) (CHAR_BIT * sizeof(x))
#define MSB(x, bits) ((x) & (~0ULL << (bitsizeof(x) - (bits))))
#ifndef min
# define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
# define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/*
 * Custom memory allocation wrappers
 * that set error code and error message
 * on allocation failure
 */
GIT_INLINE(void *) git__malloc(size_t len)
{
	void *ptr = malloc(len);
	if (!ptr) giterr_set_oom();
	return ptr;
}

GIT_INLINE(void *) git__calloc(size_t nelem, size_t elsize)
{
	void *ptr = calloc(nelem, elsize);
	if (!ptr) giterr_set_oom();
	return ptr;
}

GIT_INLINE(char *) git__strdup(const char *str)
{
	char *ptr = strdup(str);
	if (!ptr) giterr_set_oom();
	return ptr;
}

GIT_INLINE(char *) git__strndup(const char *str, size_t n)
{
	size_t length = 0;
	char *ptr;

	while (length < n && str[length])
		++length;

	ptr = (char*)git__malloc(length + 1);

	if (length)
		memcpy(ptr, str, length);

	ptr[length] = '\0';

	return ptr;
}

/* NOTE: This doesn't do null or '\0' checking.  Watch those boundaries! */
GIT_INLINE(char *) git__substrdup(const char *start, size_t n)
{
	char *ptr = (char*)git__malloc(n+1);
	memcpy(ptr, start, n);
	ptr[n] = '\0';
	return ptr;
}

GIT_INLINE(void *) git__realloc(void *ptr, size_t size)
{
	void *new_ptr = realloc(ptr, size);
	if (!new_ptr) giterr_set_oom();
	return new_ptr;
}

#define git__free(ptr) free(ptr)

#define STRCMP_CASESELECT(IGNORE_CASE, STR1, STR2) \
	((IGNORE_CASE) ? strcasecmp((STR1), (STR2)) : strcmp((STR1), (STR2)))

#define CASESELECT(IGNORE_CASE, ICASE, CASE) \
	((IGNORE_CASE) ? (ICASE) : (CASE))

extern int git__prefixcmp(const char *str, const char *prefix);
extern int git__prefixcmp_icase(const char *str, const char *prefix);
extern int git__suffixcmp(const char *str, const char *suffix);

GIT_INLINE(int) git__signum(int val)
{
	return ((val > 0) - (val < 0));
}

extern int git__strtol32(int32_t *n, const char *buff, const char **end_buf, int base);
extern int git__strtol64(int64_t *n, const char *buff, const char **end_buf, int base);

extern void git__hexdump(const char *buffer, size_t n);
extern uint32_t git__hash(const void *key, int len, uint32_t seed);

/** @return true if p fits into the range of a size_t */
GIT_INLINE(int) git__is_sizet(git_off_t p)
{
	size_t r = (size_t)p;
	return p == (git_off_t)r;
}

/* 32-bit cross-platform rotl */
#ifdef _MSC_VER /* use built-in method in MSVC */
#	define git__rotl(v, s) (uint32_t)_rotl(v, s)
#else /* use bitops in GCC; with o2 this gets optimized to a rotl instruction */
#	define git__rotl(v, s) (uint32_t)(((uint32_t)(v) << (s)) | ((uint32_t)(v) >> (32 - (s))))
#endif

extern char *git__strtok(char **end, const char *sep);
extern char *git__strsep(char **end, const char *sep);

extern void git__strntolower(char *str, size_t len);
extern void git__strtolower(char *str);

GIT_INLINE(const char *) git__next_line(const char *s)
{
	while (*s && *s != '\n') s++;
	while (*s == '\n' || *s == '\r') s++;
	return s;
}

typedef int (*git__tsort_cmp)(const void *a, const void *b);

extern void git__tsort(void **dst, size_t size, git__tsort_cmp cmp);

typedef int (*git__tsort_r_cmp)(const void *a, const void *b, void *payload);

extern void git__tsort_r(
	void **dst, size_t size, git__tsort_r_cmp cmp, void *payload);


/**
 * @param position If non-NULL, this will be set to the position where the
 * 		element is or would be inserted if not found.
 * @return 0 if found; GIT_ENOTFOUND if not found
 */
extern int git__bsearch(
	void **array,
	size_t array_len,
	const void *key,
	int (*compare)(const void *key, const void *element),
	size_t *position);

extern int git__bsearch_r(
	void **array,
	size_t array_len,
	const void *key,
	int (*compare_r)(const void *key, const void *element, void *payload),
	void *payload,
	size_t *position);

extern int git__strcmp_cb(const void *a, const void *b);

extern int git__strcmp(const char *a, const char *b);
extern int git__strcasecmp(const char *a, const char *b);
extern int git__strncmp(const char *a, const char *b, size_t sz);
extern int git__strncasecmp(const char *a, const char *b, size_t sz);

typedef struct {
	short refcount;
	void *owner;
} git_refcount;

typedef void (*git_refcount_freeptr)(void *r);

#define GIT_REFCOUNT_INC(r) { \
	((git_refcount *)(r))->refcount++; \
}

#define GIT_REFCOUNT_DEC(_r, do_free) { \
	git_refcount *r = (git_refcount *)(_r); \
	r->refcount--; \
	if (r->refcount <= 0 && r->owner == NULL) { do_free(_r); } \
}

#define GIT_REFCOUNT_OWN(r, o) { \
	((git_refcount *)(r))->owner = o; \
}

#define GIT_REFCOUNT_OWNER(r) (((git_refcount *)(r))->owner)

static signed char from_hex[] = {
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 00 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 10 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 20 */
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, /* 30 */
-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 40 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 50 */
-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 60 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 70 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 80 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 90 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* a0 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* b0 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* c0 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* d0 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* e0 */
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* f0 */
};

GIT_INLINE(int) git__fromhex(char h)
{
	return from_hex[(unsigned char) h];
}

GIT_INLINE(int) git__ishex(const char *str)
{
	unsigned i;
	for (i=0; i<strlen(str); i++)
		if (git__fromhex(str[i]) < 0)
			return 0;
	return 1;
}

GIT_INLINE(size_t) git__size_t_bitmask(size_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	return v;
}

GIT_INLINE(size_t) git__size_t_powerof2(size_t v)
{
	return git__size_t_bitmask(v) + 1;
}

GIT_INLINE(bool) git__isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

GIT_INLINE(bool) git__isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

GIT_INLINE(bool) git__isdigit(int c)
{
    return (c >= '0' && c <= '9');
}

GIT_INLINE(bool) git__isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == '\v' || c == 0x85 /* Unicode CR+LF */);
}

GIT_INLINE(bool) git__iswildcard(int c)
{
	return (c == '*' || c == '?' || c == '[');
}

/*
 * Parse a string value as a boolean, just like Core Git
 * does.
 *
 * Valid values for true are: 'true', 'yes', 'on'
 * Valid values for false are: 'false', 'no', 'off'
 */
extern int git__parse_bool(int *out, const char *value);

/*
 * Parse a string into a value as a git_time_t.
 *
 * Sample valid input:
 * - "yesterday"
 * - "July 17, 2003"
 * - "2003-7-17 08:23"
 */
int git__date_parse(git_time_t *out, const char *date);

/*
 * Unescapes a string in-place.
 * 
 * Edge cases behavior:
 * - "jackie\" -> "jacky\"
 * - "chan\\" -> "chan\"
 */
extern size_t git__unescape(char *str);

#endif /* INCLUDE_util_h__ */
