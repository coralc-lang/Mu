#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

extern int memcmp(const void*, const void*, size_t);

#undef INT8_MIN
#undef INT8_MAX
#undef INT16_MIN
#undef INT16_MAX
#undef INT32_MIN
#undef INT32_MAX
#undef INT64_MIN
#undef INT64_MAX

typedef struct _coral_str { const uint8_t* ptr; size_t len; } _coral_str;
static bool _coral_streq(_coral_str a, _coral_str b) { return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0; }
static bool _coral_strtruth(_coral_str s) { return s.len > 0; }
typedef void (*FnP_void_int32_t)(int32_t);
typedef void (*FnP_void)();
typedef void* (*FnP_voidp_voidp)(void*);
typedef struct _coral_tuple_uint32_t_uint32_t {
    uint32_t _1;
    uint32_t _2;
} _coral_tuple_uint32_t_uint32_t;
typedef uint8_t crl__lib__core__cchar__cchar;

int64_t read(int32_t fd, char* buf, uint64_t count);
int64_t write(int32_t fd, char* const buf, uint64_t count);
int32_t getcchar(void);
int32_t putcchar(int32_t c);
int32_t fflush(void* stream);
uint64_t strlen(char* const s);
int32_t strcmp(char* const a, char* const b);
char* strcpy(char* dst, char* const src);
void* malloc(uint64_t size);
void free(void* ptr);
void* realloc(void* ptr, uint64_t newSize);
int32_t memcmp(const void* a, const void* b, uint64_t n);
int32_t snprintf(char* buf, uint64_t size, char* const fmt, ...);
int32_t printf(char* const fmt, ...);
int32_t puts(char* const s);
void* mmap(void* addr, uint64_t length, int32_t prot, int32_t flags, int32_t fd, int64_t offset);
int32_t open(char* const path, int32_t flags, int32_t mode);
int32_t close(int32_t fd);
int32_t stat(char* const path, void* buf);
int32_t mkdir(char* const path, int32_t mode);
int32_t chdir(char* const path);
int32_t rmdir(char* const path);
int32_t unlink(char* const path);
int32_t rename(char* const old, char* const newName);
int32_t access(char* const path, int32_t mode);
void* opendir(char* const path);
void* readdir(void* dir);
int32_t closedir(void* dir);
int32_t getchar(void);
double strtod(char* const nptr, char** endptr);
int32_t vsnprintf(char* buf, uint64_t size, char* const fmt, ...);
void* fopen(char* const path, char* const mode);
int32_t fclose(void* stream);
uint64_t fread(void* ptr, uint64_t size, uint64_t count, void* stream);
uint64_t fwrite(const void* ptr, uint64_t size, uint64_t count, void* stream);
int32_t feof(void* stream);
int32_t ferror(void* stream);
int64_t lseek(int32_t fd, int64_t offset, int32_t whence);
int32_t mkstemp(char* templateStr);
char* mkdtemp(char* templateStr);
int32_t regcomp(void* preg, char* const pattern, int32_t cflags);
int32_t regexec(const void* preg, char* const string, uint64_t nmatch, void* pmatch, int32_t eflags);
void regfree(void* preg);
uint64_t regerror(int32_t errcode, const void* preg, char* errbuf, uint64_t errbufSize);
int32_t fork(void);
int32_t execvp(char* const file, char** const argv);
int32_t waitpid(int32_t pid, int32_t* status, int32_t options);
int32_t poll(void* fds, uint64_t nfds, int32_t timeout);
void _exit(int32_t code);
int32_t pipe(int32_t* fds);
int32_t dup2(int32_t oldfd, int32_t newfd);
int32_t execvpe(char* const file, char** const argv, char** const envp);
FnP_void_int32_t signal(int32_t signum, FnP_void_int32_t handler);
int32_t kill(int32_t pid, int32_t sig);
int32_t raise(int32_t sig);
char* getenv(char* const name);
int32_t setenv(char* const name, char* const value, int32_t overwrite);
int32_t unsetenv(char* const name);
int64_t readlink(char* const path, char* buf, uint64_t bufsize);
int32_t gettimeofday(void* tv, void* tz);
uint64_t time(void* t);
int64_t clock_gettime(int32_t clockId, void* tp);
int32_t nanosleep(const void* req, void* rem);
int32_t getcontext(void* uc);
void makecontext(void* uc, FnP_void fn, int32_t argc);
int32_t swapcontext(void* ocp, void* ucp);
int32_t isatty(int32_t fd);
int32_t pthread_create(void** thread, const void* attr, FnP_voidp_voidp start, void* arg);
int32_t pthread_join(void* thread, void** retval);
void pthread_exit(void* retval);
void* pthread_self(void);
int32_t pthread_mutex_init(void* mutex, const void* attr);
int32_t pthread_mutex_destroy(void* mutex);
int32_t pthread_mutex_lock(void* mutex);
int32_t pthread_mutex_unlock(void* mutex);
int32_t pthread_mutex_trylock(void* mutex);
int32_t pthread_mutexattr_init(void* attr);
int32_t pthread_mutexattr_destroy(void* attr);
int32_t pthread_mutexattr_settype(void* attr, int32_t type);
int32_t pthread_cond_init(void* cond, const void* attr);
int32_t pthread_cond_destroy(void* cond);
int32_t pthread_cond_wait(void* cond, void* mutex);
int32_t pthread_cond_signal(void* cond);
int32_t pthread_cond_broadcast(void* cond);
int32_t pthread_cond_timedwait(void* cond, void* mutex, void* abstime);
int32_t pthread_rwlock_init(void* lock, const void* attr);
int32_t pthread_rwlock_destroy(void* lock);
int32_t pthread_rwlock_rdlock(void* lock);
int32_t pthread_rwlock_wrlock(void* lock);
int32_t pthread_rwlock_unlock(void* lock);
int32_t pthread_rwlock_tryrdlock(void* lock);
int32_t pthread_rwlock_trywrlock(void* lock);
int32_t sem_init(void* sem, int32_t pshared, uint32_t value);
int32_t sem_destroy(void* sem);
int32_t sem_wait(void* sem);
int32_t sem_trywait(void* sem);
int32_t sem_post(void* sem);
int32_t sem_getvalue(void* sem, int32_t* sval);
int32_t socket(int32_t domain, int32_t type, int32_t protocol);
int32_t connect(int32_t fd, const void* addr, int32_t addrlen);
int32_t bind(int32_t fd, const void* addr, int32_t addrlen);
int32_t listen(int32_t fd, int32_t backlog);
int32_t accept(int32_t fd, void* addr, int32_t* addrlen);
int32_t setsockopt(int32_t fd, int32_t level, int32_t optname, uint8_t* const optval, int32_t optlen);
int64_t send(int32_t fd, uint8_t* const buf, uint64_t len, int32_t flags);
int64_t recv(int32_t fd, uint8_t* buf, uint64_t len, int32_t flags);
int64_t sendto(int32_t fd, uint8_t* const buf, uint64_t len, int32_t flags, const void* dstAddr, int32_t addrLen);
int64_t recvfrom(int32_t fd, uint8_t* buf, uint64_t len, int32_t flags, void* srcAddr, int32_t* addrLen);
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
_Bool crl__lib__core__char_util__isDigit(uint8_t c);
_Bool crl__lib__core__char_util__isAlpha(uint8_t c);
_Bool crl__lib__core__char_util__isAlnum(uint8_t c);
_Bool crl__lib__core__char_util__isLower(uint8_t c);
_Bool crl__lib__core__char_util__isUpper(uint8_t c);
uint8_t crl__lib__core__char_util__toLower(uint8_t c);
uint8_t crl__lib__core__char_util__toUpper(uint8_t c);
_Bool crl__lib__core__char_util__isSpace(uint8_t c);
_Bool crl__lib__core__char_util__isPrint(uint8_t c);
_Bool crl__lib__core__char_util__isControl(uint8_t c);
_Bool crl__lib__core__char_util__isHexDigit(uint8_t c);
uint8_t crl__lib__core__char_util__digitValue(uint8_t c);
_Bool crl__lib__core__char_util__isAscii(uint8_t c);
_Bool crl__lib__core__char_util__isWhitespace(uint8_t c);
_Bool crl__lib__core__char_util__isNewline(uint8_t c);
_Bool crl__lib__core__char_util__isPunctuation(uint8_t c);
_Bool crl__lib__core__char_util__isSymbol(uint8_t c);
uint32_t crl__lib__core__char_util__utf8Width(uint32_t c);
uint32_t crl__lib__core__char_util__encodeUtf8(uint32_t c, uint8_t* buf);
_coral_tuple_uint32_t_uint32_t crl__lib__core__char_util__decodeUtf8(uint8_t* const buf);
uint32_t crl__lib__core__char_util__toUtf8(uint32_t c, uint8_t* buf);
uint32_t crl__lib__core__char_util__fromUtf8(uint8_t* const buf);
uint8_t crl__lib__core__char_util__toAscii(uint8_t c);
_Bool crl__lib__core__char_util__u8Eq(uint8_t a, uint8_t b);
_Bool crl__lib__core__char_util__u8EqLower(uint8_t a, uint8_t b);
int32_t crl__lib__core__char_util__u8Cmp(uint8_t a, uint8_t b);
uint32_t crl__lib__core__char_util__toStr(uint8_t c, uint8_t* buf);
_coral_str crl__lib__core__str___coral_str_fromCstr(char* const cstr);
_coral_str crl__lib__core__str___coral_str_fromParts(uint8_t* const ptr, size_t len);
_coral_str crl__lib__core__str___coral_str_empty(void);
char* const crl__lib__core__str___coral_str_asU8(_coral_str* self);
char crl__lib__core__str___coral_str_at(_coral_str* self, size_t idx);
_Bool crl__lib__core__str___coral_str_isEmpty(_coral_str* self);
_Bool crl__lib__core__str___coral_str_equals(_coral_str* self, _coral_str other);
int32_t crl__lib__core__str___coral_str_compare(_coral_str* self, _coral_str other);
_Bool crl__lib__core__str___coral_str_startsWith(_coral_str* self, _coral_str prefix);
_Bool crl__lib__core__str___coral_str_endsWith(_coral_str* self, _coral_str suffix);
_Bool crl__lib__core__str___coral_str_containsView(_coral_str* self, _coral_str needle);
_Bool crl__lib__core__str___coral_str_containsByte(_coral_str* self, uint8_t c);
size_t crl__lib__core__str___coral_str_findByte(_coral_str* self, uint8_t c);
size_t crl__lib__core__str___coral_str_rfindByte(_coral_str* self, uint8_t c);
size_t crl__lib__core__str___coral_str_findView(_coral_str* self, _coral_str needle);
_coral_str crl__lib__core__str___coral_str_slice(_coral_str* self, size_t start, size_t end);
_coral_str crl__lib__core__str___coral_str_trimStart(_coral_str* self);
_coral_str crl__lib__core__str___coral_str_trimEnd(_coral_str* self);
_coral_str crl__lib__core__str___coral_str_trim(_coral_str* self);
_Bool crl__lib__core__str___coral_str_isAsciiDigit(_coral_str* self);
_Bool crl__lib__core__str___coral_str_isAsciiAlpha(_coral_str* self);
_Bool crl__lib__core__str___coral_str_isAsciiAlnum(_coral_str* self);extern char** environ;
uint8_t crl__lib__core__char_util__newline = (uint32_t)10;
uint8_t crl__lib__core__char_util__tab = (uint32_t)9;
uint8_t crl__lib__core__char_util__space = ' ';
uint8_t crl__lib__core__char_util__nullu8 = (uint32_t)0;
uint8_t crl__lib__core__char_util__carriageReturn = (uint32_t)13;

_Bool crl__lib__core__char_util__isDigit(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v >= ((uint16_t)('0'))))) && (((v <= ((uint16_t)('9'))))));
}
_Bool crl__lib__core__char_util__isAlpha(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((((v >= ((uint16_t)('a'))))) && (((v <= ((uint16_t)('z'))))))) || (((((v >= ((uint16_t)('A'))))) && (((v <= ((uint16_t)('Z'))))))));
}
_Bool crl__lib__core__char_util__isAlnum(uint8_t c) {
    return ((crl__lib__core__char_util__isDigit(c)) || (crl__lib__core__char_util__isAlpha(c)));
}
_Bool crl__lib__core__char_util__isLower(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v >= ((uint16_t)('a'))))) && (((v <= ((uint16_t)('z'))))));
}
_Bool crl__lib__core__char_util__isUpper(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v >= ((uint16_t)('A'))))) && (((v <= ((uint16_t)('Z'))))));
}
uint8_t crl__lib__core__char_util__toLower(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    if (((((v >= ((uint16_t)('A'))))) && (((v <= ((uint16_t)('Z')))))))
    {
        return ((uint8_t)(((v + 32))));
    }
    return c;
}
uint8_t crl__lib__core__char_util__toUpper(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    if (((((v >= ((uint16_t)('a'))))) && (((v <= ((uint16_t)('z')))))))
    {
        return ((uint8_t)(((v - 32))));
    }
    return c;
}
_Bool crl__lib__core__char_util__isSpace(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((((((((((v == ((uint16_t)(' '))))) || (((v == ((uint16_t)((uint32_t)9))))))) || (((v == ((uint16_t)((uint32_t)10))))))) || (((v == ((uint16_t)((uint32_t)13))))))) || (((v == ((uint16_t)((uint32_t)12))))))) || (((v == ((uint16_t)((uint32_t)11))))));
}
_Bool crl__lib__core__char_util__isPrint(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v >= 32))) && (((v <= 126))));
}
_Bool crl__lib__core__char_util__isControl(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v < 32))) || (((v == 127))));
}
_Bool crl__lib__core__char_util__isHexDigit(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((((((v >= ((uint16_t)('0'))))) && (((v <= ((uint16_t)('9'))))))) || (((((v >= ((uint16_t)('a'))))) && (((v <= ((uint16_t)('f'))))))))) || (((((v >= ((uint16_t)('A'))))) && (((v <= ((uint16_t)('F'))))))));
}
uint8_t crl__lib__core__char_util__digitValue(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    if (((((v >= ((uint16_t)('0'))))) && (((v <= ((uint16_t)('9')))))))
    {
        return ((uint8_t)(((v - ((uint16_t)('0'))))));
    }
    if (((((v >= ((uint16_t)('a'))))) && (((v <= ((uint16_t)('f')))))))
    {
        return ((uint8_t)(((((v - ((uint16_t)('a')))) + 10))));
    }
    if (((((v >= ((uint16_t)('A'))))) && (((v <= ((uint16_t)('F')))))))
    {
        return ((uint8_t)(((((v - ((uint16_t)('A')))) + 10))));
    }
    return ((uint8_t)(0));
}
_Bool crl__lib__core__char_util__isAscii(uint8_t c) {
    return ((((uint16_t)(c)) <= 127));
}
_Bool crl__lib__core__char_util__isWhitespace(uint8_t c) {
    return crl__lib__core__char_util__isSpace(c);
}
_Bool crl__lib__core__char_util__isNewline(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((v == ((uint16_t)((uint32_t)10))))) || (((v == ((uint16_t)((uint32_t)13))))));
}
_Bool crl__lib__core__char_util__isPunctuation(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((((((((v >= 33))) && (((v <= 47))))) || (((((v >= 58))) && (((v <= 64))))))) || (((((v >= 91))) && (((v <= 96))))))) || (((((v >= 123))) && (((v <= 126))))));
}
_Bool crl__lib__core__char_util__isSymbol(uint8_t c) {
    uint16_t v = ((uint16_t)(c));
    return ((((((((((((((((((((((((((((((v == ((uint16_t)('+'))))) || (((v == ((uint16_t)('-'))))))) || (((v == ((uint16_t)('*'))))))) || (((v == ((uint16_t)('/'))))))) || (((v == ((uint16_t)('='))))))) || (((v == ((uint16_t)('<'))))))) || (((v == ((uint16_t)('>'))))))) || (((v == ((uint16_t)('!'))))))) || (((v == ((uint16_t)('&'))))))) || (((v == ((uint16_t)('|'))))))) || (((v == ((uint16_t)('^'))))))) || (((v == ((uint16_t)('~'))))))) || (((v == ((uint16_t)('%'))))))) || (((v == ((uint16_t)('?'))))))) || (((v == ((uint16_t)('@'))))));
}
uint32_t crl__lib__core__char_util__utf8Width(uint32_t c) {
    if (((c <= 127)))
    {
        return 1;
    }
    if (((c <= 2047)))
    {
        return 2;
    }
    if (((c <= 65535)))
    {
        return 3;
    }
    return 4;
}
uint32_t crl__lib__core__char_util__encodeUtf8(uint32_t c, uint8_t* buf) {
    if (((c <= 127)))
    {
        ((buf[0] = ((uint8_t)(c))));
        return 1;
    }
    if (((c <= 2047)))
    {
        ((buf[0] = ((uint8_t)(((192 | ((c >> 6))))))));
        ((buf[1] = ((uint8_t)(((128 | ((c & 63))))))));
        return 2;
    }
    if (((c <= 65535)))
    {
        ((buf[0] = ((uint8_t)(((224 | ((c >> 12))))))));
        ((buf[1] = ((uint8_t)(((128 | ((((c >> 6)) & 63))))))));
        ((buf[2] = ((uint8_t)(((128 | ((c & 63))))))));
        return 3;
    }
    ((buf[0] = ((uint8_t)(((240 | ((c >> 18))))))));
    ((buf[1] = ((uint8_t)(((128 | ((((c >> 12)) & 63))))))));
    ((buf[2] = ((uint8_t)(((128 | ((((c >> 6)) & 63))))))));
    ((buf[3] = ((uint8_t)(((128 | ((c & 63))))))));
    return 4;
}
_coral_tuple_uint32_t_uint32_t crl__lib__core__char_util__decodeUtf8(uint8_t* const buf) {
    uint8_t b0 = buf[0];
    if (((b0 <= 127)))
    {
        return ((_coral_tuple_uint32_t_uint32_t){((uint32_t)(b0)), 1});
    }
    if (((((b0 & 224)) == 192)))
    {
        return ((_coral_tuple_uint32_t_uint32_t){((((((uint32_t)(((b0 & 31)))) << 6)) | ((uint32_t)(((buf[1] & 63)))))), 2});
    }
    if (((((b0 & 240)) == 224)))
    {
        return ((_coral_tuple_uint32_t_uint32_t){((((((((uint32_t)(((b0 & 15)))) << 12)) | ((((uint32_t)(((buf[1] & 63)))) << 6)))) | ((uint32_t)(((buf[2] & 63)))))), 3});
    }
    return ((_coral_tuple_uint32_t_uint32_t){((((((((((uint32_t)(((b0 & 7)))) << 18)) | ((((uint32_t)(((buf[1] & 63)))) << 12)))) | ((((uint32_t)(((buf[2] & 63)))) << 6)))) | ((uint32_t)(((buf[3] & 63)))))), 4});
}
uint32_t crl__lib__core__char_util__toUtf8(uint32_t c, uint8_t* buf) {
    return crl__lib__core__char_util__encodeUtf8(c, buf);
}
uint32_t crl__lib__core__char_util__fromUtf8(uint8_t* const buf) {
    _coral_tuple_uint32_t_uint32_t r = crl__lib__core__char_util__decodeUtf8(buf);
    return r._1;
}
uint8_t crl__lib__core__char_util__toAscii(uint8_t c) {
    return ((c & 127));
}
_Bool crl__lib__core__char_util__u8Eq(uint8_t a, uint8_t b) {
    return ((a == b));
}
_Bool crl__lib__core__char_util__u8EqLower(uint8_t a, uint8_t b) {
    return ((crl__lib__core__char_util__toLower(a) == crl__lib__core__char_util__toLower(b)));
}
int32_t crl__lib__core__char_util__u8Cmp(uint8_t a, uint8_t b) {
    uint16_t av = ((uint16_t)(a));
    uint16_t bv = ((uint16_t)(b));
    if (((av < bv)))
    {
        return (-(1));
    }
    if (((av > bv)))
    {
        return 1;
    }
    return 0;
}
uint32_t crl__lib__core__char_util__toStr(uint8_t c, uint8_t* buf) {
    return crl__lib__core__char_util__encodeUtf8(((uint32_t)(c)), buf);
}
_coral_str crl__lib__core__str___coral_str_fromCstr(char* const cstr) {
    return ((_coral_str){.ptr = ((uint8_t* const)(cstr)), .len = ((size_t)(strlen(cstr)))});
}
_coral_str crl__lib__core__str___coral_str_fromParts(uint8_t* const ptr, size_t len) {
    return ((_coral_str){.ptr = ptr, .len = len});
}
_coral_str crl__lib__core__str___coral_str_empty(void) {
    return (_coral_str){ (const uint8_t*)"", 0 };
}
char* const crl__lib__core__str___coral_str_asU8(_coral_str* self) {
    return ((char* const)(self->ptr));
}
char crl__lib__core__str___coral_str_at(_coral_str* self, size_t idx) {
    assert(((idx < self->len)));
    0;
    return ((char)(self->ptr[idx]));
}
_Bool crl__lib__core__str___coral_str_isEmpty(_coral_str* self) {
    return ((self->len == 0));
}
_Bool crl__lib__core__str___coral_str_equals(_coral_str* self, _coral_str other) {
    if (((self->len != other.len)))
    {
        return 0;
    }
    return ((memcmp(((void* const)(self->ptr)), ((void* const)(other.ptr)), ((uint64_t)(self->len))) == 0));
}
int32_t crl__lib__core__str___coral_str_compare(_coral_str* self, _coral_str other) {
    size_t minLen = ((((self->len < other.len))) ? (self->len) : (other.len));
    int32_t r = memcmp(((void* const)(self->ptr)), ((void* const)(other.ptr)), ((uint64_t)(minLen)));
    if (((r != 0)))
    {
        return r;
    }
    if (((self->len < other.len)))
    {
        return (-(1));
    }
    if (((self->len > other.len)))
    {
        return 1;
    }
    return 0;
}
_Bool crl__lib__core__str___coral_str_startsWith(_coral_str* self, _coral_str prefix) {
    if (((prefix.len > self->len)))
    {
        return 0;
    }
    return ((memcmp(((void* const)(self->ptr)), ((void* const)(prefix.ptr)), ((uint64_t)(prefix.len))) == 0));
}
_Bool crl__lib__core__str___coral_str_endsWith(_coral_str* self, _coral_str suffix) {
    if (((suffix.len > self->len)))
    {
        return 0;
    }
    return ((memcmp(((void* const)(((self->ptr + ((self->len - suffix.len)))))), ((void* const)(suffix.ptr)), ((uint64_t)(suffix.len))) == 0));
}
_Bool crl__lib__core__str___coral_str_containsView(_coral_str* self, _coral_str needle) {
    return ((crl__lib__core__str___coral_str_findView(self, needle) < self->len));
}
_Bool crl__lib__core__str___coral_str_containsByte(_coral_str* self, uint8_t c) {
    return ((crl__lib__core__str___coral_str_findByte(self, c) < self->len));
}
size_t crl__lib__core__str___coral_str_findByte(_coral_str* self, uint8_t c) {
    for (
    size_t i = 0;((i < self->len));((i)++))
    {
        if (((self->ptr[i] == c)))
        {
            return i;
        }
    }
    return self->len;
}
size_t crl__lib__core__str___coral_str_rfindByte(_coral_str* self, uint8_t c) {
    size_t i = self->len;
    while (((i > 0)))
    {
        ((i)--);
        if (((self->ptr[i] == c)))
        {
            return i;
        }
    }
    return self->len;
}
size_t crl__lib__core__str___coral_str_findView(_coral_str* self, _coral_str needle) {
    if (((needle.len == 0)))
    {
        return 0;
    }
    if (((needle.len > self->len)))
    {
        return self->len;
    }
    for (
    size_t i = 0;((i <= ((self->len - needle.len))));((i)++))
    {
        if (((memcmp(((void* const)(((self->ptr + i)))), ((void* const)(needle.ptr)), ((uint64_t)(needle.len))) == 0)))
        {
            return i;
        }
    }
    return self->len;
}
_coral_str crl__lib__core__str___coral_str_slice(_coral_str* self, size_t start, size_t end) {
    assert(((start <= end)));
    0;
    assert(((end <= self->len)));
    0;
    return crl__lib__core__str___coral_str_fromParts(((self->ptr + start)), ((end - start)));
}
_coral_str crl__lib__core__str___coral_str_trimStart(_coral_str* self) {
    size_t i = 0;
    while (((((i < self->len))) && (((((((((self->ptr[i] == ((uint8_t)(' '))))) || (((self->ptr[i] == ((uint8_t)((uint32_t)9))))))) || (((self->ptr[i] == ((uint8_t)((uint32_t)10))))))) || (((self->ptr[i] == ((uint8_t)((uint32_t)13)))))))))
    {
        ((i)++);
    }
    return crl__lib__core__str___coral_str_fromParts(((self->ptr + i)), ((self->len - i)));
}
_coral_str crl__lib__core__str___coral_str_trimEnd(_coral_str* self) {
    size_t e = self->len;
    while (((((e > 0))) && (((((((((self->ptr[((e - 1))] == ((uint8_t)(' '))))) || (((self->ptr[((e - 1))] == ((uint8_t)((uint32_t)9))))))) || (((self->ptr[((e - 1))] == ((uint8_t)((uint32_t)10))))))) || (((self->ptr[((e - 1))] == ((uint8_t)((uint32_t)13)))))))))
    {
        ((e)--);
    }
    return crl__lib__core__str___coral_str_fromParts(self->ptr, e);
}
_coral_str crl__lib__core__str___coral_str_trim(_coral_str* self) {
    _coral_str t = crl__lib__core__str___coral_str_trimStart(self);
    return crl__lib__core__str___coral_str_trimEnd((&(t)));
}
_Bool crl__lib__core__str___coral_str_isAsciiDigit(_coral_str* self) {
    for (
    size_t i = 0;((i < self->len));((i)++))
    {
        uint8_t c = self->ptr[i];
        if ((!(crl__lib__core__char_util__isDigit(c))))
        {
            return 0;
        }
    }
    return ((self->len > 0));
}
_Bool crl__lib__core__str___coral_str_isAsciiAlpha(_coral_str* self) {
    for (
    size_t i = 0;((i < self->len));((i)++))
    {
        uint8_t c = self->ptr[i];
        if ((!(crl__lib__core__char_util__isAlpha(c))))
        {
            return 0;
        }
    }
    return ((self->len > 0));
}
_Bool crl__lib__core__str___coral_str_isAsciiAlnum(_coral_str* self) {
    for (
    size_t i = 0;((i < self->len));((i)++))
    {
        uint8_t c = self->ptr[i];
        if ((!(crl__lib__core__char_util__isAlnum(c))))
        {
            return 0;
        }
    }
    return ((self->len > 0));
}