sboxlib                          This file documents "sboxlib", and should come
                                 in a distribution file called "sboxlib.zip".
a reference implementation       If it it did not, or for more information, see
of the sBOX file format          http://world.std.com/~buzzard/sbox/sbox.html


This distribution contains the free, freely
redistributable source code to the reference
implementation for sBOX file reading and writing.

1.   MANIFEST
2.   BUILD INSTRUCTIONS
2.1.    PORTING
2.2.    COMPILE-TIME OPTIONS
2.3.    VAGUE LIBRARY HOW-TO
3.   BUGS AND TESTING STATUS
4.   IMPLEMENTATION FEATURES
5.   QUICKSTART DOCUMENTATION
6.   REFERENCE

-----------------------------------------------------------------------------

1. MANIFEST

sboxlib includes the following files:

sboxread.c      A prototypical sBOX reading codebase (standalone)
sboxwrit.c      A prototypical sBOX writing codebase (standalone)
sboxkit.c       A toolkit layered over sboxread and sboxwrit
box.c           A demonstration program using sboxread and sboxwrit

sbox.h          General shared definitions
sboxtype.h      Internal-to-library definitions
sboxread.h      Functions exposed by sboxread.c
sboxwrit.h      Functions exposed by sboxwrit.c
sboxkit.h       Functions exposed by sboxkit.c
sboxlib.h       A single header file providing all entry points to above

readme.txt      This file (library documentation)

The sBOX specification is available separately; the current version
as of 1999-03-29 is preliminary version 0.1, and can be found at:
   http://world.std.com~/buzzard/sbox

-----------------------------------------------------------------------------

2.   VAGUE BUILDING-THE-LIBRARY INSTRUCTIONS

2.1.   PORTING

sboxlib requires you to do a single thing in two header files:
the typedef of "uint32" should be set to a type which is
unsigned and 32 bits long; the typedef is found in sbox.h
and in sboxlib.h, and a default value that works for most 32-bit
processors is provided.  (sbox.h is used internally when compiling
the library; sboxlib.h is used by clients of the library.)

Ideally sboxlib would work automatically as long as uint32
is *at least* 32 bits long, because it can use explicit
conversions from byte streams to values, but I didn't think
of it when I wrote it.

sboxlib automatically deals with endianness.  Its solution
is not maxmimally efficient for little-endian processors, but
sboxlib's tasks should be so I/O dominated that this will not
be an issue.  (In an ideal world, compilers would recognize
the standard assemble-bytes-into-integers sequences and output
efficient code for them.)

2.2.   COMPILE-TIME OPTIONS

sboxlib provides three major compile time flags which can be
set when building the library.  (Setting the flags does not
affect the header file or client code, so the client need not
worry about being 'in sync' with the library's setting.)

These flags are designed to simplify life for programmers creating
tools for software developers, or during testing and debugging.
For this reason, all three flags can be turned on by defining the
single flag SBOX_TOOL.

ERROR_STRINGS

    If ERROR_STRINGS is enabled, then in addition to generating
    result codes in response to errors, a result string is placed
    in sbox_read_error_message or sbox_write_error_message.
    These messages are aimed at programmers developing sBOX-derived
    formats, and provide explicit feedback about what in the sBOX
    file went wrong.  The messages will not be useful to end users.
    Examples: "Out of memory for directory", "Invalid directory offset",
    "fread() on file failed".

    Disabling ERROR_STRINGS will save a little memory.

PRINT_ERRORS

    If an error occurs during sbox reading or writing, a message
    will be printed to stderr containing the error code (or error
    string if ERROR_STRINGS is enabled).

EXIT_ON_ERROR

    If an error occurs during sbox reading or writing, the program
    will automatically exit if and only if PRINT_ERRORS is defined.
    (Exitting silently doesn't seem like a very useful thing.)
    Programs aimed at end users will want greater resiliency to
    input errors and failure conditions; however, this can simplify
    early development, and is appropriate for certain kinds of
    in-house tools.

Note that sboxkit provides a separate run-time system for error
response and reporting.  The intention here is that SBOX_TOOL will
do exactly what some people need, and save them the effort of
learning the details of how sboxkit's error-handling works.
(sboxkit's exit-on-error processing will *not* print out the error
message if PRINT_ERRORS is defined, on the assumption that the
underlying sboxread or sboxwrit call already did.)

2.3.  VAGUE LIBRARY HOW-TO

The simplest and most effective way of using sboxlib is to
compile all three of the source code files into a single library:

   sboxread.c
   sboxwrit.c
   sboxkit.c

Then use the resulting library file (e.g. sboxlib.lib) and
the header file 'sboxlib.h' in other projects directly.

You may want to compile two different versions of the library,
one with SBOX_TOOL defined and the other with all the flags
undefined.  These should probably be named different things,
not considered 'debug' versus 'release', because they behave
radically differently in the face of errors which would
undermine debugging.

-----------------------------------------------------------------------------

3.  BUGS AND TESTING STATUS

3.1  PORTABILITY

At this date, 1999-03-29, sboxlib has only been tested with
one compiler and one platform: MSVC 4.2 under Win98.

However, sboxlib's dependency on the platform is limited
to the FILE * interface from the stdio library, which should
be being used in a strictly portable fashion.

It is possible that an endian dependency or a sizeof(int)
dependency crawled into the code despite my precautions.
If you have a mysterious failure or an inconsistency between
systems, check if the bad system is big endian or if
sizeof(int) != 4 first thing.

I was writing extremely portable unix applications in 1990,
so hopefully I haven't dropped the ball now that I'm only
developing under Windows--but I can't make any guarantees
until I get some feedback.

3.2  BUGS

sboxlib was developed for my own personal use.  It has not
undergone a thorough QA process, nor even an appropriate
amount of programmer-testing.

Notably undertested:

   The support for scanning directories on disk (instead of
   reading them entirely into memory) is entirely untested
   (and hence almost certainly buggy).

   Many of the sboxkit interfaces haven't been tested.

   Some of the sboxwrit interfaces haven't been tested.

Also:

   I *tried* to write very bulletproof code, with totally
   thorough error-checking everywhere.  (But parameter
   validation is not done for pointers arguments.)  But
   I probably missed some cases.

All of this means that things may well suck if you're not willing
to debug my code.  Yeah, my code is pretty well structured,
and it's not much code, and it's a layered API so you don't
have to understand the innards of sboxread/sboxwrit to debug
sboxkit, but it opens up a whole can of worms if you use it.

However, you should realize that I haven't debugged this
stuff not because it's hard, but because writing test code
to debug it is a giant waste of my time if nobody is ever
going to use it.  You should think of it as 'lazy evaluation'
testing.

In other words, any bugs (as opposed to feature requests)
that you find are probably trivial for me to fix (if not
for you), and I will make them my number one priority.

On the other hand, if you find and fix bugs, I will gladly
incorporate them into my codebase.  On the other other hand,
I am leery about adding new features, as I would like to
keep the sboxlib codebase relatively concise and easy to
comprehend, so that the source itself is useful for more
than just compiling.

-----------------------------------------------------------------------------

4.   IMPLEMENTATION FEATURES

sboxlib exposes a very simple, hopefully stable, layered API.

-- SBOXREAD & SBOXWRIT --

The bottom layer of the library consists of sboxread and sboxwrit.
Each of these files provides the basic mechanisms for reading and
writing sbox files.  They do a very careful, hopefully thorough
job of error detection and reporting.  sboxread provides the bare
minimum functionality necessary 'to get the job done', and relies
on the layered API to achieve flexibility.  (sboxwrit attempts to
subscribe to the same philosophy, but is forced to provide several
different accessing mechanisms due to it being harder to layer
correctly than just provide them all in the first place.)

Notable features:

   Full error detection and reporting, compile-time controllable
   using the preprocessor flags discussed in section 2.2.  (Check
   out the function locate_directory() in sboxread.c for an example
   of why error checking takes up more code in sboxread than does
   actual processing.  This level of detail is mainly useful for
   debugging flawed sBOX writers.)

   sboxread supports a 'max memory directory size'; if a directory
   is smaller than this size, sboxread reads the entire directory
   into memory for later use.  If it is larger than this size, then
   it is left on disk.  sboxread provides accessing methods for
   reading directory entries that automatically use whichever mechanism
   was used to open the file.

   sboxread can parse an arbitrary subregion of a file as an sbox
   file, so, for example, it can directly handle nested sbox files.

   sboxwrit writes to a FILE * sequentially, and thus can be used to
   generate a subregion of a file.  (It explicitly generates offsets
   relative to the start of its subregion, as opposed to explicit
   global file offsets.)  Additionally, data-writing is done by
   getting a FILE * from sboxwrit and writing the data for a block
   to it (typically sequentially); therefore, it is possible (and
   straightforward) to use sboxwrit to generate nested sbox files.

   sboxread and sboxwrit are thread-safe if multiple threads don't operate
   on the same file (because there is no global state), except for extended
   error reporting which occurs through global variables.  Multiple
   threads cannot safely read or write to the same file (at least
   not through the same SboxHandle; while multi-thread writing is
   entirely impossible, multi-thread reading is possible if each
   thread uses its own SboxHandle built on top of independent
   FILE * handles).

   sboxwrit currently won't work if you lie to it about how much data
   you wrote into the FILE * (if you use the interface that requires
   explicit lengths).  I could make some straightforward changes
   to it (actually, simplify the design quite a bit!), and make it so
   that you can write into the FILE * freely, generating "dead data"
   as well as the sbox data.  I didn't need it or want it, so I designed
   it differently, but it could be changed.  (It would probably make
   sboxwrit behave more predictably, although the current design was
   intended to basically make sure you couldn't accidentally write data
   that didn't end up in some data block.)

-- SBOXKIT --

The top layer of the library is sboxkit.  sboxkit is a pure layer
over sboxread and sboxwrit--it doesn't access any internal data
structures, just the pure functional interface.  [The file sboxtype.h
defines the internal structures used by sboxread and sboxwrit, but
it isn't exported in sboxlib.h, and it's not used by sboxkit.]

sboxkit provides some very important functionality, such as directory
searching.  It also provides a large number of 'wrapper' functions
which simplify the usage of the raw functionality provided by sboxread.
It includes automatic memory allocation for reading data items,
interfaces for using C-style strings as names (that is, not needing
to supply an explicit length field for names), a set of interfaces
which squish the result codes into other return values, and support for
automatic file opening and closing for super-simple one-shot access.

Notable issues:

   sboxkit just linear searches the directory when asked to find an
   item.  This will need to change at some point to a smarter data
   structure (e.g. a hash table or a binary tree).  To do so without
   changing the interface will require some cleverness, however, as
   sboxkit does not wrap the file handles provided by sboxread, and
   therefore has nowhere to store any derived data (like a hash table).
   This is the number one feature that will definitely go into sboxkit
   the minute it is being used for any serious applications, and it
   will NOT require any changes to the API exposed to the client.
   [The solution: sboxread will be extended to provide a void * which
   it stores in the file handle on behalf of a client; sboxkit can
   hang its data off of there.  Thus the sboxread API will be extended,
   sboxkit will use the new API, and the API will be unchanged as
   perceived by clients.  Also, since these files are read-only once
   they're accessed by sboxread, an easy fast approach would be to
   make an index array that's sorted, and binary search it.  This involves
   lg N string compares, which could suck with long strings, but simple
   and relatively efficient.  It also would handle the current 'repeated
   item' interface cleanly.  Plus, 'simple and easy' means 'bug free'--
   use qsort, spend a little effort on "proving" the binary search, and
   it's basically done.]

   sboxkit is not multi-thread friendly, since it uses a lot of global
   data to keep track of temporarily allocated data items and temporarily
   opened files.

-----------------------------------------------------------------------------

5. QUICKSTART DOCUMENTATION

  sboxlib provides a wide variety of functions to maximize functionality
  and improve ease of use.  That means it would take a while to come up
  to speed on the entire API.  Therefore, here is an extremely simple
  set of functions you can use to start with.

5.1  QUICKSTART: READING SBOX FILES

Prototypes:

#   SboxHandle *SboxkitReadOpenFilename(char *filename, char *signature);

      Creates a new sbox file handle for reading.  Fails if the
      file doesn't exist or doesn't begin with the 16-byte signature
      pointed to by the argument signature.  If signature is a NULL,
      any sBOX file will be accepted.

#   SRCode SboxReadClose(SboxHandle *sbox);

      Closes such a file handle.

#   uint32 SboxkitGetByString(void **p, SboxHandle *sbox, char *name);

      Looks for a <name, value> pair that matches the name, and loads the
      corresponding value into a temporary memory buffer, returning the length.

Sample code:

  #include "sboxlib.h"
  #define MY_NAME   "my_name"

  int main(int argc, char **argv)
  {
     SboxHandle *sbox;
     uint32 length;
     unsigned char *value;

     sbox = SboxkitReadOpenFilename(argv[1]);

     length = SboxkitGetByString(&value, sbox, MY_NAME);

     if (value)
        printf(MY_NAME ": length %d, first byte %d\n", length, value[0]);
     else
        printf(MY_NAME " was not present or was zero-length\n");
     
     // NOTE: if we did another SboxkitGetByString(), 'value' would
     // no longer be pointing to a valid buffer--it's a temporary
     // short-lifetime buffer

     SboxReadClose(sbox);
  }

5.2  QUICKSTART: WRITING SBOX FILES

Prototypes:

#   SboxWriteHandle *SboxkitWriteOpenFromFilename(char *filename, char *sig);

      Creates a new sbox file for writing.  (Truncates an existing file.)
      Uses the first 16 bytes pointed to by "sig" as the 16-byte file
      signature for the sBOX file.

#   SRCode SboxWriteClose(SboxWriteHandle *handle);

      Closes an active sboxfile being written, freezing its contents
      (the sBOX directory is written out at this time).

#   SRCode SboxWriteStartItemNamed(SboxWriteHandle *h,
#                                                char *name, int namesize);

      Begin adding an item named 'name' to the sbox file.  'name' need
      not be nulll terminated, and should be of length 'namesize'. (Sorry
      for the lack of parallelism with the read code here, but sboxkit
      wrappers were focused on simplifying reading, not writing.)

#   SRCode SboxWriteEndItem(SboxWriteHandle *h);

      Indiciate the end of the value of the item started previously
      with SboxWriteStartItemNamed().

#   SRCode SboxWriteData(SboxWriteHandle *h, void *data, uint32 datasize);

      Write some of the value into the current <name, value> pair.
      Explicitly: append datasize bytes to the current value.  Must
      be called between SboxWriteStartItemNamed() and SboxWriteEndItem()
      or equivalent functions.

Sample code:

  #include "sboxlib.h"
  #define MY_NAME   "my_name"

  int main(int argc, char **argv)
  {
     SboxWriteHandle *sbox;

     sbox = SboxkitWriteOpenFilename(argv[1]);

     SboxWriteStartItemNamed(sbox, MY_NAME, strlen(MY_NAME));
     SboxWriteData(sbox, "some junky data",  15);
     SboxWriteData(sbox, " plus some more!", 16);
     SboxWriteEndItem();

     SboxWriteClose(sbox);
     // creates a single item:  <"my_name", "some junky data plus some more!">
  }

-----------------------------------------------------------------------------

6. REFERENCE

  It may look like there are a lot of functions here.  A lot of
  this is merely to provide flexibility.

6.0   ARGUMENT ORDER CONVENTIONS

  All sbox interfaces use a consistent argument ordering convention
  to try to make it easier to remember them without looking them up:

     1.  All output items appear first (an output item is a
         pointer through which the called function will write)
     2.  Buffers are always named with the pointer and then the size;
         if the buffer is a output item, then the size appears next,
         even if the size is not a output item.
     3.  The SboxHandle * appears as early in the list as is possible
         without conflicting with the above rules.  (In other words,
         SboxHandle * appears immediately after all output items, unless
         it is an output (e.g. SboxHandle **), in which case it appears
         *first*.)
     4.  offset/size pairs always appear in that order
     5.  boolean flags come last

  Boolean flags are typed with 'int' to avoid system dependent
  conflicts and conflicts with C++.

6.1   READING SBOX FILES

6.1.1.  ERROR CODES

  Almost all functions in sboxread return a value of type SboxResultCode,
  indicating whether an operation succeeded (indicated by SBOX_OK == 0),
  or whether something went wrong.  In the remainder of this file,
  SboxResultCode will be abbreviated SRCode.

  The returned error code indicates either a general class of errors
  (such as SBOX_OUT_OF_MEMORY), or indicates where in the sBOX file something
  went wrong (header, tail, directory, data block), or whether an invalid
  item offset was provided (SBOX_INVALID_ITEM).

  Extended error information giving more details are exposed via global
  variables, depending on options defined at compile time:

#    int   sbox_read_error_code;
#    char *sbox_read_error_message;

  See sboxread.c for the values of sbox_read_error_code, and section 2.2
  to learn how to get non-NULL values for sbox_read_error_message.

  Many sboxkit functions hide the detailed error values and provide
  either a single value, or none at all.  You can enable at run time
  whether errors will cause sboxkit functions to terminate the application
  by setting

#    int sboxkit_exit_on_error;

  to TRUE.

6.1.2.   MEMORY USAGE

  sBOX provides a global variable whose value is used to determine
  whether or not to load an sBOX directory into memory, or to search
  it on disk.

#     unsigned long  sbox_max_memory_directory;

  Set this to the maximum size (in bytes) of an sBOX directory which
  should be stored in memory.  The actual storage used depends on the
  sBOX file; and every open sBOX file can use this amount independently.
  The value only affects files which are opened subsequently.

  Set it to SBOX_DIRECTORY_ALWAYS_IN_MEMORY to have directories always
  read into memory (the default).  Set it to SBOX_DIRECTORY_NEVER_IN_MEMORY
  to have it never be read into memory.

6.1.3   OPENING FILES FOR READ

  sBOX files are read through the use of SboxHandle *, which
  somewhat resemble the FILE * interfaces provided by stdio.h.

  There are three interfaces for opening an sbox file for reading,
  depending on how it should get at the underlying file.  Each of these
  three interfaces has two variants.  The first variant returns a
  result code and requires a pointer to an SboxHandle *.  The second
  variant returns an SboxHandle *, and returns NULL if there is an
  error.  The former case allows you to distinguish between a file
  not existing, not being an sBOX file, or being a corrupted sBOX
  file; the second variant is a little easier to use but makes it
  clumsier to find out what went wrong.

#   SRCode      SboxReadOpenFilename(SboxHandle **, char *filename, char *sig);
#   SboxHandle *SboxkitReadOpenFilename(char *filename, char *sig);

       Given a filename, fopen() it and parse it as an sbox file.  Checks
       that it begins with the signature specified as "sig", unless sig
       is NULL, in which case any file is accepted.

#   SRCode SboxReadOpenFromFile(SboxHandle **, FILE *f, int close, char *sig);
#   SboxHandle *SboxkitReadOpenFromFile(FILE *f, int close, char *sig);

       Given an open file f, rewind to the beginning and parse it as
       an sbox file.  If 'close' is true, sboxlib will fclose()
       the FILE * when the sbox file is closed.

#   SRCode      SboxReadOpenFromFileBlock(SboxHandle **, FILE *f,
#                         uint32 offset, uint32 size, int close, char *sig);
#   SboxHandle *SboxkitReadOpenFromFileBlock(FILE *f,
#                         uint32 offset, uint32 size, int close, char *sig);

       Given an open file f, treat the subregion of it starting at 'offset'
       and of length 'size' as an sbox file.  If 'close' is true, sboxlib
       will fclose() the FILE * when the sbox file is closed.

  Note: the following calls are essentially equivalent:
      SboxReadOpenFilename(&sbox, filename, sig)
      SboxReadOpenFromFile(&sbox, fopen(filename, "rb"), TRUE, sig);
      SboxReadOpenFromFileBlock(&sbox, fopen(filename, "rb"),
                    0, function_returning_file_length(filename), TRUE, sig);
  
6.1.4   CLOSING FILES FOR READ

#   SRCode SboxReadClose(SboxHandle *sbox);

    When you have finished reading from an sbox file, call SboxReadClose
    to release system resources.  (Mainly memory consumption, and a file
    handle if the file was opened with 'close' set TRUE.)

6.1.5   SIMPLE ITEM FETCHING

#   uint32 SboxkitGetByString(void **p, SboxHandle *sbox, char *name);
#   uint32 SboxkitSizeByString(SboxHandle *sbox, char *name);
#   void *SboxkitItemByString(SboxHandle *sbox, char *name);

    Given a name represented by a C string (zero-terminated, but
    the one in the sbox file is not explicitly zero-terminated),
    these functions return the size of the correspondin data item,
    and a pointer to a temporary buffer holding all of the data
    respectively.  Only one such buffer is available at a time,
    globally across all sbox files.

    The first function returns both values; the other two functions
    each return a single value.  A length of 0 and a pointer of NULL
    are used to indicate that a name is not present.  (Note that if a
    name is present but has a 0-length value, it will also be reported
    with a length of 0 and a pointer of NULL; to test for the presence
    of a data item, use another function, such as SboxkitFindString.)

#   void SboxkitStealItem(void *p);
#   void SboxkitFreeItem(void *p);

    The previous functions put the item in a temporary buffer, and
    one of the item is available at a time.  If you want to preserve
    the item without copying it, call SboxkitStealItem().  This informs
    sboxkit that you are taking responsibility for managing the lifetime
    of the item.  It will remain valid until you release it, which you
    do by calling SboxkitFreeItem().  

  This somewhat clumsy solution allows for extremely convenient
  automatic storage management in simple situations, and a facility
  for overriding it in special circumstances.  If you want to do
  the storage managment yourself, then you need to get an id for
  the item (see section 6.1.6), and then use SboxReadItem(),
  described in section 6.1.7.  If you want to use names which are
  not 0-terminated--i.e. they have embedded 0s in the names, or
  the representation you have for them in code does not have a
  terminating 0, then you must use the id interface as well.

6.1.6   DIRECTORY LOOKUP

  Items in an sboxfile are referred to by integer ids, starting from 0.
  You can find the index of a <name, value> pair given only the name
  (assuming it is unique):

#   uint32 SboxkitFindName(SboxHandle *sbox, void *name, uint32 namelen);
#   uint32 SboxkitFindString(SboxHandle *sbox, char *name);

    The second function takes a 0-terminated string.  (It matches a string
    of the length strlen(str) in the sbox file--the name in the sbox file
    must not be zero-terimated.)  These functions return SBOXKIT_NOTFOUND
    if the name is not present.  [Note that 0 is a legal item id.]

  There is also extremely primitive support for handling files in
  which the same name appears multiple times.  If iterating over the
  entire directory is not an appropriate solution, please consult sboxkit.h
  for brief documentation on these functions:

#        SboxkitCountName
#        SboxkitCountString
#        SboxkitFindDuplicateName
#        SboxkitFindDuplicateString

6.1.7   READING ITEMS 

  Given an item id generated from the functions in section 6.1.6, you
  can read the data directly.

#   uint32 SboxReadItem(void *buffer, uint32 bufsize,
                        SboxHandle *sbox, uint32 n, uint32 offset);

    This function reads as much as 'bufsize' data into the buffer
    provided by the caller.  The data is taken from the n'th item's
    data field, starting at 'offset' bytes within it.  The return
    value is the number of bytes read, or 0 if there is no such item
    or if the offset specified is outside the legal range for that item.
    This is roughly equivalent to an fseek() followed by an fread().

#   SRCode SboxSeekItem(  SboxHandle *sbox, uint32 n, uint32 offset);
#   FILE  *SboxFileHandle(SboxHandle *sbox);

    You can read the data field manually.  SboxSeekItem()
    will wind the file pointed to by SboxFileHandle to the offset'th
    byte of the n'th item.  SboxFileHandle() will return the file handle
    which can then be used to fread() the data directly.  This allows
    the data to be supplied to other libraries which want to stream the
    data directly from a file, but it is not recommended for general use.

6.1.8   RAW DIRECTORY ACCESS

  Given an item id, you can directly access all the information
  about the corresponding <name, value> pair; for example, you
  can explicitly iterate through id's and examine <name, value>
  pairs this way.

#   SRCode SboxNumItems(uint32 *value, SboxHandle *sbox);
#   uint32 SboxkitNumItems(SboxHandle *sbox);

    Reports the number of <name, value> pairs in the file.

#   SRCode SboxNameSize(uint32 *value, SboxHandle *sbox, uint32 n);
#   uint32 SboxkitNameSize(SboxHandle *sbox, uint32 n);

    Reports the length of the <name> field of the n'th item (numbered from 0).

#   SRCode SboxNameData  (void **value, SboxHandle *sbox, uint32 n);
#   void *SboxkitNameData(SboxHandle *sbox, uint32 n);

    Returns the value of the <name> field of the n'th item (numbered from 0).
    This data is malloc'd and will become invalid on the next Sbox{kit}NameData
    operation on this sbox file.

#   SRCode SboxNameBuffer(void *buffer, uint32 bufsize,
#                                          SboxHandle *sbox, uint32 n);

    Puts up to 'bufsize' bytes of the name of the n'th item into buffer.
    The total number of bytes placed is the smaller of bufsize and the
    length of the name.

#   SRCode SboxItemSize(uint32 *value, SboxHandle *sbox, uint32 n);
#   uint32 SboxkitItemSize(SboxHandle *sbox, uint32 item);

    Reports the length of the <value> field of the n'th item (numbered from 0).

#   SRCode SboxItemLoc (uint32 *value, SboxHandle *sbox, uint32 n);

    Reports the file offset (relative to the start of the sbox file)
    of the data for the n'th item (numbered from 0).  (Note that you
    cannot just directly fseek() SboxFileHandle() to this location
    if the sbox file is formed from a subregion.  The information is
    provided for completeness, but you probably should never use it.)
 
6.1.9   UTILITY EASY READER

  For extra ease of use, sboxkit provides some mechanisms for
  reading from sbox files without ever creating sbox handles.
  (Behind the scenes, these mechanisms are less efficient, and
  may leave files open for longer than you might want, but they're
  convenient for quick-and-dirty situations):

#   uint32 SboxkitGet(void **p, char *filename, char *itemname);
#   uint32 SboxkitFilenameItemSize(char *filename, char *itemname);
#   void *SboxkitFilenameItem(char *filename, char *itemname);

    SboxkitGet is a single easy-to-use entry point.  The file named
    by 'filename' is scanned for an item named 'itemname'; if it is
    found, its value is loaded into memory, and *p is made to point
    to it; the length of the item is returned.  This value is only
    guaranteed to last until the next SboxkitGet or equivalent; see
    section 6.1.5 for information on how to 'acquire' the storage
    management of one of these items.

    The other two functions provide each of these items independently.

    There is no interface to easily detect the presence of a 0-length
    item.

6.2   WRITING SBOX FILES

  The provided codebase in sboxwrit allows the creation of sbox files
  with certain restrictions.  The entire directory must be able to fit
  in main memory.  The <name, value> pairs must be written out in order.
  No provision is made for providing a FILE handle to allow the use of
  libraries to write out the data.  You cannot output the canonical format.
  You *can* directly write nested sbox files--that is, allow the <value>
  field to contain one (or more!) sbox files which are also created on
  the fly.

6.2.1  ERROR CODES

  All functions in sboxwrit return a value of type SboxResultCode,
  indicating whether an operation succeeded (indicated by SBOX_OK == 0),
  or whether something went wrong.  In the remainder of this file,
  SboxResultCode will be abbreviated SRCode.

  The returned error code indicates either a general class of errors
  (such as SBOX_OUT_OF_MEMORY), or indicates where in the sBOX file something
  went wrong (header, tail, directory, data block), or whether an invalid
  item offset was provided (SBOX_INVALID_ITEM).

  Extended error information giving more details are exposed via global
  variables, depending on options defined at compile time:

#    int   sbox_write_error_code;
#    char *sbox_write_error_message;

  See sboxwrit.c for the values of sbox_write_error_code, and section 2.2
  to learn how to get non-NULL values for sbox_write_error_message.

  Many sboxkit functions hide the detailed error values and provide
  either a single value, or none at all.  You can enable at run time
  whether errors will cause sboxkit functions to terminate the application
  by setting

#    int sboxkit_exit_on_error;

  to TRUE.

6.2.2  OPENING FILES
   
  sBOX files are written through the use of SboxWriteHandle *, which
  somewhat resemble the FILE * interfaces provided by stdio.h.

  Note that SboxHandle (for reading) and SboxWrithandle (for writing)
  are not interchangeable, and there is no mechanism supporting
  both read and write to the same file.  sboxread.c and sboxwrit.c are
  entirely separate files and work entirely independently of each other.

  New writeable files can be created through one of two interfaces,
  each of which has two variants, one of which simplifies error handling:

#   SRCode SboxWriteOpenFilename(SboxWriteHandle **handle,
                                      char *filename, char *sig);
#   SboxWriteHandle *SboxkitWriteOpenFromFilename(char *filename, char *sig);

    Open the specified filename, creating or truncating it as necessary,
    to create a new sBOX file.  "sig" must point to a 16-byte array of
    characters which is used as the file signature.

#   SRCode SboxWriteOpenFromFile(SboxWriteHandle **handle,
                                            FILE *f, int close, char *sig);
#   SboxWriteHandle *SboxkitWriteOpenFromFile(FILE *f, int close, char *sig);

    Start creating an sBOX file at the current location pointed to by
    the provided FILE *.

  Note that SboxWriteOpenFromFile() and SboxReadOpenFromFile() have
  radically different syntaces.  SboxReadOpenFromFile() always seeks
  to the beginning of the file before opening; if you want to read
  a subblock, you have to specify its location and length explicitly
  using SboxReadOpenFromFileBlock().  SboxReadOpenFromFile() has no
  way of knowing where the 'current block' would end.  However,
  SboxWriteOpenFromFile() simply starts writing at the current location;
  the 'current block' (the sub-sBOX file) ends whereever is necessary
  to write out all the data when it is eventually closed.

6.2.2  CLOSING FILES

#   SRCode SboxWriteClose(SboxWriteHandle *handle);

    Closes the file and causes sboxwrit to write out the directory.
    Failing to close an sbox file opened for write will result in a
    corrupted sbox file.

6.2.3  WRITING ITEMS

  sboxwrit provides a number of different mechanisms for specifying
  <name, value> pairs to be written into the sbox file.
  
#    SRCode SboxkitStringPut(SboxWriteHandle *h, char *name,
#                                             void *data, uint32 datasize);

     Add a <name, value> pair to the sbox file where the selected name
     is specified by 'char *name'--the output name will not be null
     terminated, and will have length strlen(name)--and the value is
     in the buffer 'data' and has length 'datasize'.

  The remaining mechanisms allow you to write the data out to the file
  a little at a time.  There are three ways for specifying
  the name, start, and end of the value; the first two functions in this
  list represent one way, the second two functions the second way,
  and the fifth function the third way.

#    SRCode SboxWriteStartItemNamed(SboxWriteHandle *h,
#                                                char *name, int namesize);

     Adds a <name, value> pair to the sbox file where the name is pointed
     to by 'char *name' and is of length 'namesize'.  The value will be
     written using one of the functions at the end of this section, and
     then SboxWriteEndItem() will be called to indicate the end of the value.

#    SRCode SboxWriteEndItem(SboxWriteHandle *h);

     Marks the end of the value specified by SboxWriteStartItemNamed()

#    SRCode SboxWriteStartItem(SboxWriteHandle *h);

     Like SboxWriteStartItemNamed(), but doesn't specify the name yet--
     it will be specified when the value is ended with the next function:

#    SRCode SboxWriteEndItemNamed(SboxWriteHandle *h, char *name, int namesize);

     Marks the end of the value specified by SboxWriteStartItem(), and
     specifies the name of that item--pointed to by 'name' and of length
     namesize.

#    SRCode SboxWriteItem(SboxWriteHandle *h, char *name,
#                                      uint32 namesize, uint32 datasize);

     Specifies the start of a new item, with name pointed to by 'name'
     and of length 'namesize'.  Promises the item will be 'datasize' long.
     No explicit 'end of item' function needs to be called.

  In between calls to SboxWriteStartItem() and SboxWriteEndItem(),
  you write out the "value" part of the <name, value> pair by outputting
  data through the following function, which resembles fwrite:

#    SRCode SboxWriteData(SboxWriteHandle *h, void *data, uint32 datasize);

     Writes 'datasize' bytes pointed to by 'data' to the currently
     "open" <name, value> pair.  May be called multiple times; each
     item is appended to the previous item.

#    FILE  *SboxWriteFileHandle(SboxHandle *sbox);

     Provides a 'FILE *' file handle to which data can be written
     using fwrite(); this will have the same effect as the above
     function.  You can even ftell, fseek, and backpatch, even
     backpatching data from other values; just make sure you leave
     the file fseek()'d to the end of the data you want written
     for this value before calling a terminating function.

STB 1999-03-01
updated STB 2000-08-18
