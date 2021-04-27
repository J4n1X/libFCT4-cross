# libFCT4-cross
File Archival Software with a self-designed format.

## The FCT file container system
Files can be added and removed. Appending adds to the end of file, however, which can make the extraction order confusing when appending files into an existing folder. This might be fixed in the future.

There is a folder system in place, which is based on the file path. 

Archives can also be listed, of course.

## Format explanation
### The Archive Header
The archive header is simple:
 - Identification String "FCT" (3 bytes)
 - ChunkSize of Archive (2 bytes)

### The File Header
The Header size is dynamic, reducing unused space. This does not apply to the file data chunks.

The maximum header size in bytes is 65535 bytes.

The format was designed to be simple to program for, hence it might not be the most efficient.

The format will be as follows:
 - ChunkCount (4 bytes)
 - LastChunkContentSize (2 bytes)
 - FilePathSize (2 bytes)
 - FormattedFilePath (up to 65526 bytes)
 
 ## Compilation and usage
 This program was written in Visual Studio 2019 using the CMake feature. 
 Therefore, to build on Linux, simply run `cmake .` and then `make`
