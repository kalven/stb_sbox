// sboxkit.c
//    toolkit over core sboxlib
//    see sboxkit.h for usage documentation

#include <stdlib.h>
#include <string.h>

#include "sbox.h"
#include "sboxread.h"
#include "sboxwrit.h"
#include "sboxkit.h"

////////////////////////////////////////////////////////////////////////////
//
//  processing by item name instead of item index

uint32 SboxkitFindName(SboxHandle *sbox, void *name, uint32 namelen)
{
   static uint32 cache = 0;
   uint32 i,n;
   void *p;

   n = SboxkitNumItems(sbox);

   // speed up repeated tests of the same name (happens
   // if you use the name-based interfaces exclusively)
   if (cache < n && SboxkitNameSize(sbox, cache) == namelen) {
      p = SboxkitNameData(sbox, cache);
      if (p && memcmp(p, name, namelen) == 0)
         return cache;
   }

   for (i=0; i < n; ++i) {
      if (SboxkitNameSize(sbox, i) == namelen) {
         p = SboxkitNameData(sbox, i);
         if (p && memcmp(p, name, namelen)==0) {
            cache = i;
            return i;
         }
      }
   }
   return SBOXKIT_NOTFOUND;
}

uint32 SboxkitFindString(SboxHandle *sbox, char *str)
{
   return SboxkitFindName(sbox, str, strlen(str));
}

uint32 SboxkitSizeByString(SboxHandle *sbox, char *str)
{
   uint32 n = SboxkitFindString(sbox, str);
   return SboxkitItemSize(sbox, n);
}

static void *cur_item=NULL;

void *SboxkitItemByString(SboxHandle *sbox, char *str)
{
   uint32 n = SboxkitFindString(sbox, str);
   uint32 size = SboxkitItemSize(sbox, n);
   if (size == 0) return NULL;
   if (cur_item != NULL) { SboxkitFreeItem(cur_item); }
   cur_item = malloc(size);
   if (cur_item == NULL) return NULL;
   if (SboxReadItem(cur_item, size, sbox, n, 0) != size)
      SboxkitFreeItem(cur_item); // sets cur_item = NULL     
   return cur_item;        
}

void SboxkitFreeItem(void *p)
{
   if (p == NULL) return;
   if (cur_item != p) {
      fprintf(stderr, "Attempted to free item that was not most recent\n");
      exit(1);
   }
   free(cur_item);
   cur_item = NULL;
}

void SboxkitStealItem(void *p)
{
   if (p == NULL) return;
   if (cur_item != p) {
      fprintf(stderr, "Attempted to steal item that was not most recent\n");
      exit(1);
   }
   cur_item = NULL;
}

uint32 SboxkitGetByString(void **p, SboxHandle *f, char *itemname)
{
   uint32 n;
   *p = NULL;
   if (!f) return 0;
   n = SboxkitSizeByString(f, itemname);
   if (n == 0) return 0;
   *p = SboxkitItemByString(f, itemname);
   if (*p == NULL) n = 0;
   return n;
}

////////////////////////////////////////////////////////////////////////////
//
//  handle multiple instances of the same name
//

uint32 SboxkitCountName(SboxHandle *sbox, void *name, uint32 namelen)
{
   uint32 i,n,count=0;
   void *p;

   n = SboxkitNumItems(sbox);
   for (i=0; i < n; ++i) {
      if (SboxkitNameSize(sbox, i) == namelen) {
         p = SboxkitNameData(sbox, i);
         if (p && memcmp(p, name, namelen)==0) {
            ++count;
         }
      }
   }
   return count;
}

uint32 SboxkitFindDuplicateName(SboxHandle *sbox, void *name, uint32 namelen, uint32 index)
{
   uint32 i,n,count=0;
   void *p;

   n = SboxkitNumItems(sbox);
   for (i=0; i < n; ++i) {
      if (SboxkitNameSize(sbox, i) == namelen) {
         p = SboxkitNameData(sbox, i);
         if (p && memcmp(p, name, namelen)==0) {
            if (count == index) return i;
            ++count;
         }
      }
   }
   return SBOXKIT_NOTFOUND;
}

uint32 SboxkitCountString(SboxHandle *sbox, char *name)
{
   return SboxkitCountName(sbox, name, strlen(name));
}

uint32 SboxkitFindDuplicateString(SboxHandle *sbox, char *name, uint32 index)
{
   return SboxkitFindDuplicateName(sbox, name, strlen(name), index);
}

////////////////////////////////////////////////////////////////////////////
//
//  processing by file name instead of file handle,
//   (for very simple quick-n-dirty applications)

#define SBOX_HANDLE_CACHE   4

static SboxHandle *sbox_cache[SBOX_HANDLE_CACHE];
static char       *sbox_name[SBOX_HANDLE_CACHE];

SboxHandle *SboxkitCachedFilehandle(char *filename)
{
   static int next=0;
   int i;
   for (i=0; i < SBOX_HANDLE_CACHE; ++i)
      if (sbox_name[i] != NULL && strcmp(sbox_name[i], filename) == 0)
         return sbox_cache[i];

   i = next;
   next = (next + 1) % SBOX_HANDLE_CACHE;
   if (sbox_name[i]) {
      free(sbox_name[i]);
      if (sbox_cache[i])
         SboxReadClose(sbox_cache[i]);
   }
   sbox_name[i] = malloc(strlen(filename)+1);
   if (sbox_name[i] == NULL) return NULL;
   strcpy(sbox_name[i], filename);
   sbox_cache[i] = SboxkitReadOpenFilename(filename, NULL);
   return sbox_cache[i];
}

uint32 SboxkitFilenameItemSize(char *filename, char *itemname)
{
   SboxHandle *f = SboxkitCachedFilehandle(filename);
   if (!f) return 0;
   return SboxkitSizeByString(f, itemname);
}

void *SboxkitFilenameItem(char *filename, char *itemname)
{
   SboxHandle *f = SboxkitCachedFilehandle(filename);
   if (!f) return NULL;
   return SboxkitItemByString(f, itemname);
}

uint32 SboxkitGet(void **p, char *filename, char *itemname)
{
   SboxHandle *f = SboxkitCachedFilehandle(filename);
   return SboxkitGetByString(p, f, itemname);
}

SboxResultCode SboxkitStringPut(SboxWriteHandle *h, char *name,
      void *data, uint32 datasize)
{
   SboxResultCode result;
   result = SboxWriteItem(h, name, strlen(name), datasize);
   if (result != SBOX_OK) return result;
   result = SboxWriteData(h, data, datasize);
   return result;
}

////////////////////////////////////////////////////////////////////////////
//
//  interfaces without result codes (mainly useful for tools)

int sboxkit_exit_on_error;

enum error_mode { ERR_READ, ERR_WRITE };

static void handle_error(enum error_mode err)
{
   if (sboxkit_exit_on_error) {
#ifndef PRINT_ERRORS
      // if we didn't already print the message in the library
      if (err == ERR_READ && sbox_read_error_message)
         fprintf(stderr, "Error during read: %s\n", sbox_read_error_message);
      else if (err == ERR_WRITE && sbox_write_error_message)
         fprintf(stderr, "Error during write: %s\n", sbox_write_error_message);
      else if (err == ERR_READ && sbox_read_error_code)
         fprintf(stderr, "Error during read; %d\n", sbox_read_error_code);
      else if (err == ERR_WRITE && sbox_write_error_code)
         fprintf(stderr, "Error during write: %d\n", sbox_write_error_code);
      else
         fprintf(stderr, "Unknown error\n");
#endif
      exit(1);
   }
}

#define ERROR()   handle_error(ERR_READ)
#define WRITE_ERROR()  handle_error(ERR_WRITE)

SboxHandle *SboxkitReadOpenFilename(char *filename, char *sig)
{
   SboxHandle *f = NULL;
   if (SboxReadOpenFilename(&f, filename, sig) != SBOX_OK) ERROR();
   return f;
}

SboxHandle *SboxkitReadOpenFromFile(FILE *f, int close, char *sig)
{
   SboxHandle *sbox = NULL;
   if (SboxReadOpenFromFile(&sbox, f, close, sig) != SBOX_OK)  ERROR();
   return sbox;
}

SboxHandle *SboxkitReadOpenFromFileBlock(FILE *f, uint32 offset,
                                                   uint32 size, int close, char *sig)
{
   SboxHandle *sbox = NULL;
   if (SboxReadOpenFromFileBlock(&sbox, f, offset, size, close, sig)!=SBOX_OK) ERROR();
   return sbox;
}

uint32 SboxkitNumItems(SboxHandle *sbox)
{
   uint32 count=0;
   if (SboxNumItems(&count, sbox) != SBOX_OK) ERROR();
   return count;
}

uint32 SboxkitItemSize(SboxHandle *sbox, uint32 item)
{
   uint32 size=0;
   if (SboxItemSize(&size, sbox, item) != SBOX_OK) ERROR();
   return size;
}

uint32 SboxkitNameSize(SboxHandle *sbox, uint32 item)
{
   uint32 size=0;
   if (SboxNameSize(&size, sbox, item) != SBOX_OK) ERROR();
   return size;
}

void *SboxkitNameData(SboxHandle *sbox, uint32 item)
{
   void *p=NULL;
   if (SboxNameData(&p, sbox, item) != SBOX_OK) ERROR();
   return p;
}

SboxWriteHandle *SboxkitWriteOpenFilename(char *filename, char *sig)
{
   SboxWriteHandle *sbox=NULL;
   if (SboxWriteOpenFilename(&sbox, filename, sig) != SBOX_OK) WRITE_ERROR();
   return sbox;
}

SboxWriteHandle *SboxkitWriteOpenFromFile(FILE *f, int close, char *sig)
{
   SboxWriteHandle *sbox=NULL;
   if (SboxWriteOpenFromFile(&sbox, f, close, sig) != SBOX_OK) WRITE_ERROR();
   return sbox;
}
