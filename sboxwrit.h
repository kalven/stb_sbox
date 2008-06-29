#ifndef INCLUDE_SBOXWRIT_H     // NOT_IN_SBOXLIB
#define INCLUDE_SBOXWRIT_H     // NOT_IN_SBOXLIB

#include <stdio.h>      // NOT_IN_SBOXLIB
#include "sbox.h"       // NOT_IN_SBOXLIB

#ifdef __cplusplus
extern "C" {
#endif

/* typedef struct st_SboxWriteHandle SboxWriteHandle; DA: Already typedef'd in sbox.h */

extern int   sbox_write_error_code;
extern char *sbox_write_error_message;

#define SRC SboxResultCode

extern SRC SboxWriteItem(SboxWriteHandle *h, char *name,
                                    uint32 namesize, uint32 datasize);
extern SRC SboxWriteStartItemNamed(SboxWriteHandle *h, char *name, int namesize);
extern SRC SboxWriteStartItem(SboxWriteHandle *h);
extern SRC SboxWriteEndItemNamed(SboxWriteHandle *h, char *name, int namesize);
extern SRC SboxWriteEndItem(SboxWriteHandle *h);
extern SRC SboxWriteData(SboxWriteHandle *h, void *data, uint32 datasize);
extern SRC SboxWriteClose(SboxWriteHandle *handle);
extern SRC SboxWriteOpenFromFile(SboxWriteHandle **handle, FILE *f, int close, char *signature);
extern SRC SboxWriteOpenFilename(SboxWriteHandle **handle, char *filename, char *signature);

extern FILE *SboxWriteFileHandle(SboxWriteHandle *sbox);

#undef SRC

#ifdef __cplusplus
}
#endif

#endif     // NOT_IN_SBOXLIB
