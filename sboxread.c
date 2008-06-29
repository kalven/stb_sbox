// sBOX file reading code
//   Sean Barrett

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sboxread.h"
#include "sboxtype.h"

/////
//
// general tools
//

// SBOX files work with 32-bits:

#define INTSIZE       4
#define INTMOD        (INTSIZE-1)

static char *magic = "sb0X";

#define test_magic(str)  (!memcmp(magic, (str), INTSIZE))

// convert a uchar* pointing to a little-endian integer into a native integer
#define little_int(x)    ((((x)[3]*256+(x)[2])*256+(x)[1])*256+(x)[0])

/////
//
// error handling
//

// local shorthand names for result codes
#define HEADER       SBOX_INVALID_HEADER
#define TAIL         SBOX_INVALID_TAIL
#define INV_DIROFF   SBOX_INVALID_DIROFF
#define DIRECTORY    SBOX_INVALID_DIRECTORY
#define OOM          SBOX_OUT_OF_MEMORY

enum ErrorCodes
{
   MAGIC1= 1, DIROFF  = 11, DIRINDEX_MEM  = 21,
   MAGIC2= 2, DIRSIZE = 12, HANDLE_MEM    = 22,
   MAGIC3= 3, NAMESIZE= 13, OUT_OF_RANGE  = 23,
   SHORT = 4, DIR_MEM = 14, DIRSIZE_MATCH = 24,
   FREAD = 5, NO_FILE = 15, BAD_SIGNATURE = 25,
   FSEEK = 6, FWRITE  = 16,
};

static struct { int code; char *str; } read_error_strings[] =
{
#ifdef ERROR_STRINGS
   { DIR_MEM       , "Out of memory for directory" },
   { DIRINDEX_MEM  , "Out of memory for directory index" },
   { DIROFF        , "Invalid directory offset" },
   { DIRSIZE       , "Invalid directory size" },
   { DIRSIZE_MATCH , "Directory size did not match" },
   { FREAD         , "fread() on file failed" },
   { FSEEK         , "fseek() on file failed" },
   { HANDLE_MEM    , "Out of memory for file handle" },
   { MAGIC1        , "Magic number not present in header" },
   { MAGIC2        , "Magic number not present in tail"   },
   { MAGIC3        , "Magic number not present in directory" },
   { BAD_SIGNATURE , "File signature not found" },
   { NAMESIZE      , "Name in directory has invalid size" },
   { NO_FILE       , "Couldn't open file" },
   { OUT_OF_RANGE  , "Item outside of range" },
   { SHORT         , "File too short to contain header" },
#else
   { 0             , NULL }      // dummy entry to avoid 0-length array
#endif
};

int   sbox_read_error_code;
char *sbox_read_error_message;

static SboxResultCode ERROR(SboxResultCode val, int error)
{
   int i, n = sizeof(read_error_strings)/sizeof(read_error_strings[0]);
   sbox_read_error_code    = error;
   sbox_read_error_message = NULL;

   for (i=0; i < n; ++i)
      if (read_error_strings[i].code == error)
         sbox_read_error_message = read_error_strings[i].str;

#ifdef PRINT_ERRORS
   if (sbox_read_error_message != NULL)
      fprintf(stderr, "sbox error: %s\n", sbox_read_error_message);
   else
      fprintf(stderr, "sbox error: %d\n", sbox_read_error_code);
#ifdef EXIT_ON_ERROR
   exit(val);
#endif
#endif

   return val;
}

static void sbox_cleanup(SboxHandle *sbox)
{
   // sbox->free me holds:
   //  o if the directory is stored in memory, holds the address
   //    of the directory (which needs to be freed on close)
   //  o if the directory is not stored in memory, holds the
   //    last temporary pointer allocated for reporting data to
   //    the client.  Causes peril if the application holds onto a
   //    data value across multiple calls, but massively simplifies
   //    storage management.

   if (sbox->free_me) {
      free(sbox->free_me);
      sbox->free_me = NULL;
   }
}

/////
//
// file reading tools
//

// pretend the chunk the sbox is in is the whole file
static int sbox_seek(SboxHandle *sbox, uint32 offset)
{
   fseek(sbox->f, sbox->start+offset, SEEK_SET);
   return 0;
}

/////
//
// parse header
//

typedef struct
{
   uint32 diroff;
   uint32 dirsize;
} SboxDirectoryInfo;

static SboxResultCode locate_directory(SboxHandle *sbox, SboxDirectoryInfo *sd, char *sig)
{
   FILE *f = sbox->f;
   uint32 diroff, dirsize;
   unsigned char buffer[16 + INTSIZE*2];

   // find and parse header

   if (sbox->length < 16+INTSIZE*2)         return ERROR(HEADER, SHORT);

   if (sbox_seek(sbox, 0) != 0)             return ERROR(HEADER, FREAD);
   if (fread(buffer,16+INTSIZE*2, 1, f)!=1) return ERROR(HEADER, FREAD);
   if (sig && memcmp(sig, buffer, 16))      return ERROR(HEADER, BAD_SIGNATURE);
   if (!test_magic(buffer+16))              return ERROR(HEADER, MAGIC1);

   diroff = little_int(buffer+16+INTSIZE);
   if (diroff != 0) {
      if (diroff & INTMOD)                  return ERROR(HEADER, DIROFF);
      if (diroff < INTSIZE*2)               return ERROR(HEADER, DIROFF);
      if (diroff > sbox->length-INTSIZE*3)  return ERROR(HEADER, DIROFF);
   }

   // find and parse tail

   if (sbox_seek(sbox, sbox->length-INTSIZE*2) != 0) return ERROR(TAIL,FREAD);
   if (fread(buffer, INTSIZE, 2, f) != 2)   return ERROR(TAIL, FREAD);
   if (!test_magic(buffer+INTSIZE))         return ERROR(TAIL, MAGIC2);

   if (diroff == 0) {
      diroff = little_int(buffer);
      if (diroff == 0)                      return ERROR(TAIL, DIROFF);
      if (diroff & INTMOD)                  return ERROR(TAIL, DIROFF);
      if (diroff < INTSIZE*2)               return ERROR(TAIL, DIROFF);
      if (diroff > sbox->length-INTSIZE*4)  return ERROR(TAIL, DIROFF);
   }

   // find and parse directory header

   if (sbox_seek(sbox, diroff) != 0)        return ERROR(DIRECTORY, FREAD);
   if (fread(buffer, INTSIZE, 2, f) != 2)   return ERROR(DIRECTORY, FREAD);
   if (!test_magic(buffer))                 return ERROR(DIRECTORY, MAGIC3);

   dirsize = little_int(buffer+INTSIZE);
   if (dirsize & INTMOD)                    return ERROR(DIRECTORY, DIRSIZE);
   if (diroff+dirsize>sbox->length-INTSIZE) return ERROR(DIRECTORY, DIRSIZE);
   
   sd->diroff  = diroff + 2*INTSIZE;
   sd->dirsize = dirsize;

   return SBOX_OK;
}

///////////////////////
//
// load directory into system memory
//

static uint32 offset_to_next_item(uint32 namesize)
{
   return INTSIZE*3 + namesize + ((uint32) (0-namesize) & 3);
}

static SboxResultCode endian_fix_and_count_directory(
                              unsigned char *dir, uint32 size, uint32 *count)
{
   int item_count = 0;
   uint32 offset=0;

   while (offset < size) {
      SboxDirectoryItem *item;

      assert((offset & INTMOD) == 0);
      item = (SboxDirectoryItem *) &dir[offset];

      // endian fix... this code would be clearer if it were
      // item->offset = little_int(&item->offset)
      // this way makes explicit the disk-order dependency...
      // on the other hand, if the items in the struct were
      // in a different order, it might look like it would work,
      // but it really wouldn't (it would if it weren't in-place)

      item->offset   = little_int(&dir[offset]);
      item->size     = little_int(&dir[offset+INTSIZE]);
      item->namesize = little_int(&dir[offset+INTSIZE*2]);

      // validate
      if (offset + item->namesize < offset || offset + item->namesize > size)
         return ERROR(DIRECTORY, NAMESIZE);

      offset += offset_to_next_item(item->namesize);
      ++item_count;
   }
   if (offset != size)
      return ERROR(DIRECTORY, DIRSIZE_MATCH);

   *count = item_count;
   return SBOX_OK;
}

static SboxResultCode load_directory(SboxHandle *sbox, uint32 diroff, uint32 size)
{
   SboxResultCode result;
   uint32 i,offset;
   unsigned char *dir;

   dir = malloc(size);
   if (!dir)                                  return ERROR(OOM, DIR_MEM);
   assert(sbox->free_me == NULL);
   sbox->free_me = dir;

   sbox_seek(sbox, diroff);
   if (fread(dir, size, 1, sbox->f) != 1)     return ERROR(DIRECTORY, FREAD);

   result = endian_fix_and_count_directory(dir, size, &sbox->num_items);
   if (result != SBOX_OK)                     return result;

   sbox->directory = malloc(sbox->num_items * sizeof(sbox->directory[0]));
   if (sbox->directory == NULL)               return ERROR(OOM, DIRINDEX_MEM);

   offset = 0;
   for (i=0; i < sbox->num_items; ++i) {
      assert((offset & INTMOD) == 0);
      sbox->directory[i] = (SboxDirectoryItem *) &dir[offset];
      offset += offset_to_next_item(sbox->directory[i]->namesize);
   }
   assert(offset == size);
   return SBOX_OK;
}

///////////////////////
//
// build an index of the directory in system memory
//

static void grow_directory(uint32 **dir, uint32 *size)
{
   uint32 *new_dir;
   *size *= 2;
   new_dir = realloc(*dir, *size * sizeof(**dir));
   if (new_dir == NULL)
      free(*dir);
   *dir = new_dir;
}

static SboxResultCode scan_directory(SboxHandle *sbox,
               uint32 diroff, uint32 size)
{
   unsigned char buffer[INTSIZE*3];
   uint32 *directory_index;
   uint32 items, namesize;
   uint32 directory_size;
   uint32 offset=0;

   directory_size = 16;
   directory_index = malloc(directory_size * sizeof(directory_index[0]));
   if (directory_index == NULL)
            return ERROR(OOM, DIRINDEX_MEM);
   sbox->directory_index = directory_index;

   items = 0;
   while (offset < size) {
      assert((offset & INTMOD) == 0);
      if (items >= directory_size) {
         grow_directory(&directory_index, &directory_size);
         if (directory_index == NULL)
            return ERROR(OOM, DIRINDEX_MEM);
         sbox->directory_index = directory_index;
      }
      directory_index[items] = diroff+offset;
      sbox_seek(sbox, diroff+offset);
      if (fread(buffer, INTSIZE, 3, sbox->f) != 3)
         return ERROR(DIRECTORY, FREAD);
      namesize = little_int(buffer+INTSIZE*2);
  
      // validate result
      if (offset + namesize < offset || offset + namesize > size)
         return ERROR(DIRECTORY, NAMESIZE);

      offset += offset_to_next_item(namesize);
   }
   if (offset != size)
      return ERROR(DIRECTORY, DIRSIZE_MATCH);

   sbox->num_items = items;
   return SBOX_OK;
}

/////
//
// locate and read the directory

unsigned long sbox_max_memory_directory = SBOX_DIRECTORY_ALWAYS_IN_MEMORY;

static SboxResultCode read_directory(SboxHandle *sbox, char *sig)
{
   SboxDirectoryInfo sd;
   SboxResultCode result;

   result = locate_directory(sbox, &sd, sig);
   if (result != SBOX_OK) return result;

   if (sd.dirsize == 0) {
      sbox->num_items = 0;
      return SBOX_OK;
   }

   if (sd.dirsize > sbox_max_memory_directory)
      return scan_directory(sbox, sd.diroff, sd.dirsize);
   else
      return load_directory(sbox, sd.diroff, sd.dirsize);
}

/////
//
// return information from the directory
//

SboxResultCode SboxNumItems(uint32 *value, SboxHandle *sbox)
{
   *value = sbox->num_items;
   return SBOX_OK;
}

static SboxResultCode dirfield(uint32 *value, SboxHandle *sbox, uint32 item, int field)
{
   if (item >= sbox->num_items)
      return ERROR(SBOX_INVALID_ITEM, OUT_OF_RANGE);

   if (sbox->directory) {
      *value = (&sbox->directory[item]->offset)[field];
   } else {
      unsigned char buffer[4];
      sbox_seek(sbox, sbox->directory_index[item] + field*INTSIZE);
      if (fread(buffer, 4, 1, sbox->f) != 1)
         return ERROR(SBOX_INVALID_ITEM, FREAD);
      *value = little_int(buffer);
   }
   return SBOX_OK;
}

SboxResultCode SboxItemLoc(uint32 *value, SboxHandle *sbox, uint32 item)
{
   return dirfield(value, sbox, item, 0);
}

SboxResultCode SboxItemSize(uint32 *value, SboxHandle *sbox, uint32 item)
{
   return dirfield(value, sbox, item, 1);
}

SboxResultCode SboxNameSize(uint32 *value, SboxHandle *sbox, uint32 item)
{
   return dirfield(value, sbox, item, 2);
}

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

SboxResultCode SboxNameData(void **value, SboxHandle *sbox, uint32 item)
{
   if (item >= sbox->num_items)
      return ERROR(SBOX_INVALID_ITEM, OUT_OF_RANGE);

   if (sbox->directory) {
      *value = &sbox->directory[item]->name;
   } else {
      uint32 size;
      SboxResultCode result;
      result = SboxNameSize(&size, sbox, item);
      if (result != SBOX_OK) return result;
      if (size == 0) {
         *value = NULL;
      } else {
         // allocate storage--cleanup any existing one
         sbox_cleanup(sbox);
         sbox->free_me = malloc(size);
         if (sbox->free_me == NULL)    return ERROR(OOM, DIR_MEM);
         sbox_seek(sbox, sbox->directory_index[item] + 3*INTSIZE);
         if (fread(sbox->free_me, size, 1, sbox->f) != 1)
            return ERROR(DIRECTORY, FREAD);
         *value = sbox->free_me;
      }
   }
   return SBOX_OK;
}

SboxResultCode SboxNameBuffer(void *buffer, uint32 bufsize, SboxHandle *sbox, uint32 item)
{
   if (item >= sbox->num_items)
      return ERROR(SBOX_INVALID_ITEM, OUT_OF_RANGE);

   if (sbox->directory) {
      bufsize = min(bufsize, sbox->directory[item]->namesize);
      memcpy(buffer, sbox->directory[item]->name, bufsize);
   } else {
      SboxResultCode result;
      uint32 size;
      result = SboxNameSize(&size, sbox, item);
      if (result != SBOX_OK) return result;
      bufsize = min(bufsize, size);
      sbox_seek(sbox, sbox->directory_index[item] + 3*INTSIZE);
      if (fread(buffer, bufsize, 1, sbox->f) != 1)
         return ERROR(DIRECTORY, FREAD);
   }
   return SBOX_OK;
}

SboxResultCode SboxSeekItem(SboxHandle *sbox, uint32 item, uint32 offset)
{
   uint32 where;
   SboxResultCode result;

   result = SboxItemLoc(&where, sbox, item);
   if (result != SBOX_OK) return result;
   sbox_seek(sbox, where+offset);
   return result;
}

FILE *SboxFileHandle(SboxHandle *sbox)
{
   return sbox->f;
}

uint32 SboxReadItem(void *buffer, uint32 bufsize,
                            SboxHandle *sbox, uint32 item, uint32 offset)
{
   uint32 size;
   SboxResultCode result = SboxItemSize(&size, sbox, item);
   if (result != SBOX_OK) return 0;

   // check that offset is not past end of item
   if (offset >= size)    return 0;

   // determine how much data is left to be read, and reduce bufsize to match
   bufsize = min(bufsize, size - offset);

   // seek to the data
   result = SboxSeekItem(sbox, item, offset);
   if (result != SBOX_OK) return 0;

   // read it and return number of bytes read
   return fread(buffer, 1, bufsize, sbox->f);
}

static void sbox_initialize(SboxHandle *sbox)
{
   sbox->num_items       = 0;

   sbox->directory       = NULL;
   sbox->directory_index = NULL;
   sbox->free_me         = NULL;
   sbox->f               = NULL;
}

static void sbox_free(SboxHandle *sbox)
{
   if (sbox->directory)        free(sbox->directory);
   if (sbox->directory_index)  free(sbox->directory_index);
   if (sbox->free_me)          free(sbox->free_me);
   sbox_initialize(sbox);
   free(sbox);
}

SboxResultCode SboxReadClose(SboxHandle *sbox)
{
   if (sbox->close_file)
      fclose(sbox->f);
   sbox_free(sbox);
   return SBOX_OK;
}

SboxResultCode SboxReadOpenFromFileBlock(SboxHandle **handle,
          FILE *f, uint32 offset, uint32 size, int close, char *sig)
{
   SboxResultCode result;
   SboxHandle *sbox;
   if (f == NULL)
      return ERROR(SBOX_INVALID_FILE_OPEN, NO_FILE);

   sbox = malloc(sizeof(SboxHandle));
   if (!sbox) {
      if (close) fclose(f);
      return ERROR(OOM, HANDLE_MEM);
   }

   sbox_initialize(sbox);

   sbox->start      = offset;
   sbox->length     = size;
   sbox->f          = f;
   sbox->close_file = close;

   result = read_directory(sbox, sig);
   if (result != SBOX_OK) {
      SboxReadClose(sbox);
      return result;
   }
   *handle = sbox;
   return SBOX_OK;
}

SboxResultCode SboxReadOpenFromFile(SboxHandle **handle, FILE *f, int close, char *sig)
{
   if (f == NULL)
      return ERROR(SBOX_INVALID_FILE_OPEN, NO_FILE);
   fseek(f, 0, SEEK_END);
   return SboxReadOpenFromFileBlock(handle, f, 0, ftell(f), close, sig); 
}

SboxResultCode SboxReadOpenFilename(SboxHandle **handle, char *filename, char *sig)
{
   return SboxReadOpenFromFile(handle, fopen(filename, "rb"), 1, sig);
}

SboxResultCode SboxSignature(char *signature, SboxHandle *sbox)
{
   sbox_seek(sbox, 0);
   if (fread(signature, 1, 16, sbox->f) != 16) return ERROR(HEADER, BAD_SIGNATURE);
   return SBOX_OK;
}

