#ifndef INCLUDE_SBOXLIB_H
#define INCLUDE_SBOXLIB_H

#include <stdio.h>     // FILE *

#ifdef SBOX_TOOL
   #define ERROR_STRINGS
   #define EXIT_ON_ERROR
   #define PRINT_ERRORS
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint32; // @PORT: fix this line as appropriate

typedef struct st_SboxHandle      SboxHandle;
typedef struct st_SboxWriteHandle SboxWriteHandle;

typedef enum
{
   SBOX_OK=0,
   SBOX_OUT_OF_MEMORY,
   SBOX_INVALID_FILE_OPEN,
   SBOX_INVALID_HEADER,
   SBOX_INVALID_TAIL,
   SBOX_INVALID_DIRECTORY_OFFSET,
   SBOX_INVALID_DIRECTORY,
   SBOX_INVALID_ITEM,
} SboxResultCode;



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



typedef struct st_SboxWriteHandle SboxWriteHandle;

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



/////////////////////////////////////////////////////////////////////////
//
//  SBOXKIT wrapper functions
//    functions from sboxread and sboxwrit without result codes

// set to true if you want sboxkit wrapper functions to exit on errors

extern int sboxkit_exit_on_error;

// open files, return NULL on error

extern SboxHandle *SboxkitReadOpenFilename(char *filename, char *sig);
extern SboxHandle *SboxkitReadOpenFromFile(FILE *f, int close, char *sig);
extern SboxHandle *SboxkitReadOpenFromFileBlock(FILE *f, uint32 offset,
                                                   uint32 size, int close, char *sig);
extern SboxWriteHandle *SboxkitWriteOpenFromFilename(char *filename, char *sig);
extern SboxWriteHandle *SboxkitWriteOpenFromFile(FILE *f, int close, char *sig);

// access data about file

extern uint32 SboxkitNumItems(SboxHandle *sbox);               // 0 if error

// access data about items

extern uint32 SboxkitItemSize(SboxHandle *sbox, uint32 item);  // 0 if error
extern uint32 SboxkitNameSize(SboxHandle *sbox, uint32 item);  // 0 if error
extern void *SboxkitNameData(SboxHandle *sbox, uint32 item);   // NULL if error

//////////////////////////////////////////////////////////////////////////
//
//  simplified accessing methods, from easiest to most complex
//     most require that the names being looked up be strings (i.e.
//     terminated with a 0-byte) as passed in, but do not have
//     terminating 0s on disk

// look in sbox file 'filename' for the first item named 'itemname';
// return its length and return a pointer to a buffer containing
// the data in *p; *p = NULL if not present or OOM
extern uint32 SboxkitGet(void **p, char *filename, char *itemname);

// as above, but only returns the length, 0 if not found
extern uint32 SboxkitFilenameItemSize(char *filename, char *itemname);

// as above, but only returns the pointer to the data (or NULL)--no size info
extern void *SboxkitFilenameItem(char *filename, char *itemname);

// as SboxkitGet, but from an SboxHandle, not a filename
extern uint32 SboxkitGetByString(void **p, SboxHandle *f, char *itemname);

// as above, but only reeturns the length, 0 if not found
extern uint32 SboxkitSizeByString(SboxHandle *sbox, char *str);

// as above, but only returns the pointer to the data (or NULL)--no size info
extern void *SboxkitItemByString(SboxHandle *sbox, char *str);

// above functions which return an item only keep one available
// at a time, and automatically storage manage.  If you want to
// keep an item, call SboxkitStealItem(); at this point, you are
// responsible for storage management of it, and must call
// SboxkitFreeItem() when you are done.

extern void SboxkitStealItem(void *p);
extern void SboxkitFreeItem(void *p);

// Given an open sbox file and a name (represented as a pointer and a length),
// find the first explicit match, and return the 'directory index' of that

extern uint32 SboxkitFindName(SboxHandle *sbox, void *name, uint32 namelen);

// Given an open sbox file and a name represented as a string, find the
// first explicit match and return the 'directory index' of that

extern uint32 SboxkitFindString(SboxHandle *sbox, char *str);

////////////////////////
//
// Write functions

extern SboxResultCode SboxkitStringPut(SboxWriteHandle *h, char *name,
      void *data, uint32 datasize);

//////////////////////////////////////////////////////////////////////////
//
//  simple support for repeated data items

// given a name (pointer and length), count how many times it appears
extern uint32 SboxkitCountName(SboxHandle *sbox, void *name, uint32 namelen);

// given a name (pointer and length) and an 'index', return the index'th
// instance of that name (or, rather, the directory index of that entry)
extern uint32 SboxkitFindDuplicateName(SboxHandle *sbox, void *name,
                                       uint32 namelen, uint32 index);

// as above, but for names represented as strings

extern uint32 SboxkitCountString(SboxHandle *sbox, char *name);
extern uint32 SboxkitFindDuplicateString(SboxHandle *sbox, char *name, uint32 index);
#define SBOXKIT_NOTFOUND  ((uint32) -1)

#ifdef __cplusplus
}
#endif

#endif
