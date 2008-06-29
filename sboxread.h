#ifndef INCLUDE_SBOXREAD_H     // NOT_IN_SBOXLIB
#define INCLUDE_SBOXREAD_H     // NOT_IN_SBOXLIB

#include <stdio.h>            // NOT_IN_SBOXLIB
#include "sbox.h"             // NOT_IN_SBOXLIB

#ifdef __cplusplus
extern "C" {
#endif

extern int   sbox_read_error_code;
extern char *sbox_read_error_message;

extern unsigned long  sbox_max_memory_directory;

#define SBOX_DIRECTORY_ALWAYS_IN_MEMORY    0xffffffff
#define SBOX_DIRECTORY_NEVER_IN_MEMORY     0

//////////////////////////////////////////////////////////////////////////

#define SRC  SboxResultCode

/////
//
// open an sBOX file for read

extern SRC SboxReadOpenFilename(SboxHandle **handle, char *filename, char *sig);
extern SRC SboxReadOpenFromFile(SboxHandle **handle, FILE *f, int close, char *sig);
extern SRC SboxReadOpenFromFileBlock(SboxHandle **handle,
          FILE *f, uint32 offset, uint32 size, int close, char *sig);

extern SRC SboxReadClose(SboxHandle *sbox);


/////
//
// access file signature

extern SRC SboxSignature(char *signature, SboxHandle *sbox);

/////
//
// access directory information 

extern SRC SboxNumItems(uint32 *value, SboxHandle *sbox);

extern SRC SboxItemLoc (uint32 *value, SboxHandle *sbox, uint32 item);
extern SRC SboxItemSize(uint32 *value, SboxHandle *sbox, uint32 item);
extern SRC SboxNameSize(uint32 *value, SboxHandle *sbox, uint32 item);

extern SRC SboxNameData  (void **value,                 SboxHandle *sbox, uint32 item);
extern SRC SboxNameBuffer(void *buffer, uint32 bufsize, SboxHandle *sbox, uint32 item);

/////
//
// access values

extern uint32 SboxReadItem(void *buffer, uint32 bufsize,
                             SboxHandle *sbox, uint32 item, uint32 offset);
extern SRC    SboxSeekItem(  SboxHandle *sbox, uint32 item, uint32 offset);
extern FILE  *SboxFileHandle(SboxHandle *sbox);

#undef SRC

#ifdef __cplusplus
}
#endif

#endif     // NOT_IN_SBOXLIB
