#ifndef INCLUDE_SBOXTYPE_H
#define INCLUDE_SBOXTYPE_H

#include "sbox.h"

typedef struct
{
   uint32 offset;
   uint32 size;
   uint32 namesize;
   unsigned char name[4];
} SboxDirectoryItem;

struct st_SboxHandle
{
   FILE   *f;
   uint32 start;
   uint32 length;
   uint32 num_items;                   // number of items in directory
   SboxDirectoryItem **directory;      // if we can just load it into memory
   uint32 *directory_index;            // if we have to refer to it on disk
   void   *free_me;                    // storage to free when done
   int    close_file;                  // whether we should close the file
};

struct st_SboxWriteHandle
{
   FILE   *f;
   uint32 start;
   uint32 cur_item;
   uint32 num_items;
   uint32 max_items;
   SboxDirectoryItem **directory;
   int    close_file;                  // if we must close the file when done
   int    error;                       // if there was an error creating it
};

#endif
