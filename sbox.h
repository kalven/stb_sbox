#ifndef INCLUDE_SBOX_H               // NOT_IN_SBOXLIB
#define INCLUDE_SBOX_H               // NOT_IN_SBOXLIB

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

#ifdef __cplusplus
}
#endif

#endif                        // NOT_IN_SBOXLIB
