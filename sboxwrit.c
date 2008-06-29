// sBOX file writing code
//   Sean Barrett
//
// @TODO:
//    Need to rewrite the code to remove the 'cur_item' variable,
//    which is redundant to the current file pointer offset anyway.
//    (In cases where it's not redundant, it's because the app is
//    writing extra data into the file before calling 'StartItem',
//    and the current way we count that data as part of the next
//    item anyway; if we just use the file pointer, we will ignore
//    that data, allowing the app to write data into the non-SBOX
//    parts of the file, but that seems more consistent anyway.
//    Alternately, we can get rid of start/end semantics, and always
//    end the previous when we start the next (and also when we close
//    the file).


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "sbox.h"
#include "sboxtype.h"

/////
//
// general tools
//

// For 32-bit SBOX files:

#define INTSIZE       4
#define INTMOD        (INTSIZE-1)

static char *magic = "sb0X";

#define little_int(x)    ((((x)[3]*256+(x)[2])*256+(x)[1])*256+(x)[0])
static void make_little_int(unsigned char *buffer, uint32 value)
{
   buffer[0] = value >>  0;
   buffer[1] = value >>  8;
   buffer[2] = value >> 16;
   buffer[3] = value >> 24;
}

static int write_little_int(FILE *f, uint32 value)
{
   unsigned char buffer[4];
   make_little_int(buffer, value);
   return fwrite(buffer, 4, 1, f);
}

/////
//
// error handling
//  these codes are all identical to the read codes for now

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
   FREAD = 5, NO_FILE = 15,
   FSEEK = 6, FWRITE  = 16,
};

static struct { int code; char *str; } write_error_strings[] =
{
#ifdef ERROR_STRINGS
   { DIR_MEM       , "Out of memory for directory" },
   { DIRINDEX_MEM  , "Out of memory for directory index" },
   { FWRITE        , "fwrite() on file failed, maybe out of disk space" },
   { FSEEK         , "fseek() on file failed" },
   { HANDLE_MEM    , "Out of memory for file handle" },
   { NO_FILE       , "Couldn't open file" },
#else
   { 0             , NULL }      // can't have 0-length array
#endif
};

static int sbox_old_error;

int   sbox_write_error_code;
char *sbox_write_error_message;

static SboxResultCode ERROR(SboxResultCode val, int error)
{
   int i, n = sizeof(write_error_strings)/sizeof(write_error_strings[0]);
   sbox_old_error           = val;
   sbox_write_error_code    = error;
   sbox_write_error_message = NULL;
   for (i=0; i < n; ++i)
      if (write_error_strings[i].code == error)
         sbox_write_error_message = write_error_strings[i].str;
#ifdef PRINT_ERRORS
   if (sbox_write_error_message != NULL)
      fprintf(stderr, "sbox error: %s\n", sbox_write_error_message);
   else
      fprintf(stderr, "sbox error: %d\n", sbox_write_error_code);
#endif
   return val;
}

/////
//
// simple SBOX writer
//
// This writer is a streaming writer--it writes
// out continuous, consecutive data, so it can
// write into the middle of a file, e.g. another
// SBOX file

static void compute_item_offset(SboxWriteHandle *h)
{
   h->cur_item = ftell(h->f) - h->start;
}

static int grow_directory(SboxWriteHandle *h)
{
   SboxDirectoryItem **dir;

   h->max_items *= 2;

   dir = realloc(h->directory, sizeof(h->directory[0]) * h->max_items);
   if (!dir) return 1;

   h->directory = dir;
   return 0;
}

static SboxResultCode prep_item(SboxWriteHandle *h, char *name, int namesize)
{
   int n;
   if (h->error) return sbox_old_error;
   if (h->num_items >= h->max_items) {
      if (grow_directory(h)) {
         h->error = 1;
         return ERROR(OOM, DIR_MEM);
      }
   }
   n = h->num_items;
   h->directory[n] = malloc(INTSIZE*3 + ((namesize+3)&~3));
   if (h->directory[n] == NULL)
      return ERROR(OOM, DIR_MEM);

   h->directory[n]->offset   = h->cur_item;
   h->directory[n]->size     = 0;
   h->directory[n]->namesize = namesize;
   memcpy(h->directory[n]->name, name, namesize);
   if (namesize & 3)
      memset(h->directory[n]->name+namesize, 0, (-namesize) & 3);

   ++h->num_items;
   return SBOX_OK;
}

// use the current file pointer to determine the size
// of the previous item, and to record the location of
// the new item
static void end_item(SboxWriteHandle *h)
{
   int n = h->num_items - 1;
   compute_item_offset(h);
   h->directory[n]->size = h->cur_item - h->directory[n]->offset;
}

// use the explicitly provided size as the size of the
// next item
static void early_end_item(SboxWriteHandle *h, uint32 size)
{
   int n = h->num_items - 1;
   h->cur_item += size;
   h->directory[n]->size = size;
}

SboxResultCode SboxWriteItem(SboxWriteHandle *h, char *name,
                                    uint32 namesize, uint32 datasize)
{
   SboxResultCode result = prep_item(h, name, namesize);
   if (result == SBOX_OK)
      early_end_item(h, datasize);
   return result;
}

SboxResultCode SboxWriteStartItemNamed(SboxWriteHandle *h, char *name, int namesize)
{
   SboxResultCode result = prep_item(h, name, namesize);
   return result;
}

SboxResultCode SboxWriteStartItem(SboxWriteHandle *h)
{
   if (h->error) return sbox_old_error;
   return SBOX_OK;
}

SboxResultCode SboxWriteEndItemNamed(SboxWriteHandle *h, char *name, int namesize)
{
   SboxResultCode result = prep_item(h, name, namesize);
   if (result != SBOX_OK) return result;
   end_item(h);
   return SBOX_OK;
}

SboxResultCode SboxWriteEndItem(SboxWriteHandle *h)
{
   if (h->error) return sbox_old_error;
   end_item(h);
   return SBOX_OK;
}

SboxResultCode SboxWriteData(SboxWriteHandle *h, void *data, uint32 datasize)
{
   if (fwrite(data, datasize, 1, h->f) == 1)
      return SBOX_OK;
   return ERROR(SBOX_INVALID_ITEM, FWRITE);
}

static int write_header(SboxWriteHandle *h, char *signature)
{
   if (fwrite(signature, 1, 16, h->f) != 16) return 1;
   if (fwrite(magic, 4, 1, h->f) != 1) return 1;
   if (write_little_int(h->f, 0) != 1) return 1;

   return 0;
}

static int write_directory_header(SboxWriteHandle *h)
{
   uint32 dirsize=0;
   uint32 i;

   // compute dirsize
   for (i=0; i < h->num_items; ++i)
      dirsize += INTSIZE*3 + ((h->directory[i]->namesize + 3) & ~3);

   // write out the header
   if (fwrite(magic, 4, 1, h->f)       != 1) return 1;
   if (write_little_int(h->f, dirsize) != 1) return 1;
   return 0;
}

static int write_directory_item(SboxWriteHandle *h, int i)
{
   if (write_little_int(h->f, h->directory[i]->offset  ) != 1) return 1;
   if (write_little_int(h->f, h->directory[i]->size    ) != 1) return 1;
   if (write_little_int(h->f, h->directory[i]->namesize) != 1) return 1;
   if (fwrite(h->directory[i]->name,
              (h->directory[i]->namesize+3)&~3, 1, h->f) != 1) return 1;
   return 0;
}

static SboxResultCode write_directory_and_tail(SboxWriteHandle *h)
{
   uint32 i;
   uint32 dirloc = ftell(h->f) - h->start;

   // align
   while (dirloc & 3) {
      char buf = 0;
      if (fwrite(&buf, 1, 1, h->f) != 1)    return ERROR(TAIL, FWRITE);
      ++dirloc;
   }

   // directory
   if (write_directory_header(h))      return ERROR(DIRECTORY, FWRITE);
   for (i=0; i < h->num_items; ++i)
      if (write_directory_item(h, i))  return ERROR(DIRECTORY, FWRITE);

   assert(((ftell(h->f) - h->start) & 3) == 0);

   // tail
   if (write_little_int(h->f, dirloc) != 1) return ERROR(TAIL, FWRITE);
   if (fwrite(magic, 4, 1, h->f)      != 1) return ERROR(TAIL, FWRITE);

   return SBOX_OK;
}

SboxResultCode SboxWriteClose(SboxWriteHandle *handle)
{
   SboxResultCode result = SBOX_OK;
   uint32 i;
   if (!handle->error)
      result = write_directory_and_tail(handle);
  
   if (handle->close_file)
      fclose(handle->f);

   if (handle->directory) {
      for (i=0; i < handle->num_items; ++i)
         free(handle->directory[i]);
      free(handle->directory);
   }
    
   return result;
}

// create a new SBOX file starting at the current location of f
SboxResultCode SboxWriteOpenFromFile(SboxWriteHandle **handle, FILE *f, int close, char *sig)
{
   SboxWriteHandle *h;
   if (!f)               return ERROR(SBOX_INVALID_FILE_OPEN, NO_FILE);
   h = malloc(sizeof(SboxWriteHandle));
   if (!h)  { fclose(f); return ERROR(OOM, HANDLE_MEM); }

   h->f         = f;
   h->start     = ftell(f);
   h->num_items = 0;
   h->max_items = 16;
   h->close_file = close;
   h->error     = 0;

   h->directory = malloc(sizeof(h->directory[0]) * h->max_items);
   if (!h->directory) {
      free(h);
      if (close) fclose(f);
      return ERROR(OOM, DIR_MEM);
   }

   if (write_header(h, sig)) {
      free(h->directory);
      free(h);
      if (close) fclose(f);
      return ERROR(HEADER, FWRITE);
   }

   compute_item_offset(h);

   *handle = h;
   return SBOX_OK;
}

// create a new SBOX file 
SboxResultCode SboxWriteOpenFilename(SboxWriteHandle **handle, char *filename, char *sig)
{
   return SboxWriteOpenFromFile(handle, fopen(filename, "wb"), 1, sig);
}

FILE *SboxWriteFileHandle(SboxWriteHandle *sbox)
{
   return sbox->f;
}
