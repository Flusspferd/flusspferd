#ifndef ICONV_H_INCLUDED
#define ICONV_H_INCLUDED 1
#ifdef __cplusplus
extern "C" {
#endif

#if defined(HIPPOENV_ICONV_SHARED_BUILD)
#   define ICONV_PUBLIC_API(x) __declspec(dllexport) x
#elif defined(HIPPOENV_ICONV_SHARED)
#   define ICONV_PUBLIC_API(x) __declspec(dllimport) x
#else
#   define ICONV_PUBLIC_API(x) x
#endif

typedef void* iconv_t;
iconv_t iconv_open(const char *tocode, const char *fromcode);
int iconv_close(iconv_t cd);
unsigned iconv(iconv_t cd, const char **inbuf, unsigned *inbytesleft, char **outbuf, unsigned *outbytesleft);
#ifdef __cplusplus
}
#endif
#endif
