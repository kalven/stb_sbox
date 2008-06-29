// BOX.C
// sBOX testbed

// The testbed simply provides a scripting toolkit that
// allows insertion, deletion, output, etc., layered over
// the basic SBOX library code.

// The library code must be compiled with the flags:
//     PRINT_ERRORS
//     EXIT_ON_ERROR
// Setting the flag
//     ERROR_STRINGS
// is highly recommended.  All three can be accomplished by
// simply setting the flag "SBOX_TOOL" when compiling the library.

#include <stdlib.h>
#include <string.h>

#include "sboxread.h"
#include "sboxwrit.h"

void VerboseListing(char *filename)
{
   char signature[16];
   uint32 i,n;
   SboxHandle *f;
   SboxReadOpenFilename(&f, filename, NULL);
   SboxSignature(signature, f);
   SboxNumItems(&n, f);

   printf("FILE SIGNATURE: %16c  [ ", signature);
   for (i=0; i < 16; ++i) {
      printf("%02X ", signature[i]);
   }
   printf("]\n\n");

   for (i=0; i < n; ++i) {
      uint32 sz, nsz;
      char *str;
      SboxItemSize(&sz, f, i);
      SboxNameSize(&nsz, f, i);
      SboxNameData(&str, f, i);
      printf("%8d  \"%.*s\"\n", sz, nsz, str);
   }
   printf("%d entries\n", n);

   SboxReadClose(f);
}

#define strMatch(str, mem, sz)   (strlen(str) == (sz) && !memcmp(str, mem, sz))

/*
 *  copy data from one sbox to another, optionally
 *  deleting and/or renaming an entry
 */
void CopySbox(SboxWriteHandle *out, SboxHandle *in, char *remove, char *rename_from, char *rename_to)
{
   uint32 i,n;
   SboxNumItems(&n, in);
   for (i=0; i < n; ++i) {
      uint32 sz;
      char *str;
      sz = 0;
      SboxNameSize(&sz, in, i);
      SboxNameData(&str, in, i);
      /* If we're supposed to delete this item, skip to next */
      if (strMatch(remove, str, sz)) continue;

      /* If we're supposed to rename this item, output it with the new name */
      if (strMatch(rename_from, str, sz))
         SboxWriteStartItemNamed(out, rename_to, strlen(rename_to));
      else
         SboxWriteStartItemNamed(out, str, sz);

      /* copy the data in an extremely naive way */
      sz = 0;
      SboxItemSize(&sz, in, i);
      if (sz != 0) {
         str = malloc(sz);
         if (!str) { fprintf(stderr, "Out of memory.\n"); exit(1); }
         SboxReadItem(str, sz, in, i, 0);
         SboxWriteData(out, str, sz);
         free(str);
      }
      SboxWriteEndItem(out);
   }
}

void CreateSbox(char *filename, char *sig)
{
   SboxWriteHandle *f;
   SboxWriteOpenFilename(&f, filename, sig);
   SboxWriteClose(f);
}

void CopyFileToSbox(SboxWriteHandle *out, FILE *f)
{
   char buffer[4096];
   uint32 size;
   while (!feof(f)) {
      size = fread(buffer, 1, 4096, f);
      if (size) {
         SboxWriteData(out, buffer, size);
      }
   }
}

void AddEntry(char *infile, char *name, char *datafile, char *outfile)
{
   char signature[16];
   FILE *h = fopen(datafile, "rb");
   SboxHandle *f;
   SboxWriteHandle *g;
   if (!h) { fprintf(stderr, "Couldn't find datafile '%s'\n", datafile); exit(1); }
   SboxReadOpenFilename(&f, infile, NULL);
   SboxSignature(signature, f);
   SboxWriteOpenFilename(&g, outfile, signature);
   CopySbox(g,f, "i hate you", "i hate you", "i hate you");

   SboxWriteStartItemNamed(g, name, strlen(name));
   CopyFileToSbox(g, h);
   SboxWriteEndItem(g);

   SboxWriteClose(g);
   SboxReadClose(f);
}

void DeleteEntry(char *infile, char *name, char *outfile)
{
   char signature[16];
   SboxHandle *f;
   SboxWriteHandle *g;
   SboxReadOpenFilename(&f, infile, NULL);
   SboxSignature(signature, f);
   SboxWriteOpenFilename(&g, outfile, signature);
   CopySbox(g, f, name, "i hate you", "i hate you");
   SboxWriteClose(g);
   SboxReadClose(f);
}

void RenameEntry(char *infile, char *old_name, char *new_name, char *outfile)
{
   char signature[16];
   SboxHandle *f;
   SboxWriteHandle *g;
   SboxReadOpenFilename(&f, infile, NULL);
   SboxSignature(signature, f);
   SboxWriteOpenFilename(&g, outfile, signature);
   CopySbox(g, f, "i hate you", old_name, new_name);
   SboxWriteClose(g);
   SboxReadClose(f);
}

void OutputEntry(char *infile, char *name, char *outfile)
{
   uint32 i,n;
   SboxHandle *f;
   FILE *g;
   if(SboxReadOpenFilename(&f, infile, "box-output-entry") != SBOX_OK) {
     fprintf(stderr, "opening sbox %s failed: %s", infile, sbox_read_error_message);
     exit(1);
   }
   SboxNumItems(&n, f);
   for (i=0; i < n; ++i) {
      uint32 sz;
      char *str;
      sz = 0;
      SboxNameSize(&sz, f, i);
      SboxNameData(&str, f, i);
      if (strMatch(name, str, sz)) {
         sz = 0;
         SboxItemSize(&sz, f, i);
         if (sz != 0) {
            str = malloc(sz);
            if (!str) { fprintf(stderr, "Out of memory.\n"); exit(1); }
            SboxReadItem(str, sz, f, i, 0);
            g = fopen(outfile, "wb");
            if (!g) { fprintf(stderr, "opening file '%s' for write failed\n", outfile); exit(1); }
            fwrite(str, 1, sz, g);
            fclose(g);
         } else {
            g = fopen(outfile, "wb");
            if (!g) { fprintf(stderr, "opening file '%s' for write failed\n", outfile); exit(1); }
            fclose(g);
         }
         return;
      }
   }
   fprintf(stderr, "Not found.\n");
   exit(1);
}

int main(int argc, char **argv)
{
   // argv[1] is the input file
   // argv[2] is the command
   // argv[3] is the output file

#if 0
   if (argc == 2 && argv[1][0] == 'i') {
      InteractiveTest();
      return 0;
   }
#endif

   if (argc < 3) {
     usage:
      fprintf(stderr, "Usage: box <boxfile> {acdorv} [other parameters]\n"
             "  box v boxfile                    list the contents of the boxfile\n"
             "  box c boxfile 16-char-signature  create an empty boxfile\n"
             "  box a boxfile name file1 file2   add the pair(name,file1) to boxfile, output to file2\n"
             "  box d boxfile name file1         delete the first entry containing (name), output to file1\n"
             "  box r boxfile name1 name2 file1  rename the item name1 to the name name2\n"
             "  box o boxfile name file1         output the data for 'name' to file1\n");
      exit(0);
   }

   switch (argv[1][0]) {
      default:  fprintf(stderr, "Unknown option to box\n"); goto usage;
      case 'v': VerboseListing(argv[2]); break;
      case 'c': CreateSbox(argv[2], argv[3]); break;
      case 'a': if (argc == 6) { AddEntry(argv[2], argv[3], argv[4], argv[5]); break; }
               BadParameters:
                fprintf(stderr, "Incorrect number of parameters for option '%c'", argv[1][0]);
                goto usage;
      case 'd': if (argc == 5) DeleteEntry(argv[2], argv[3], argv[4]); else goto BadParameters;
                break;
      case 'r': if (argc == 6) RenameEntry(argv[2], argv[3], argv[4], argv[5]); else goto BadParameters;
                break;
      case 'o': if (argc == 5) OutputEntry(argv[2], argv[3], argv[4]); else goto BadParameters;
                break;
   }
   return 0;
}
