#ifndef FILES_HPP
#define FILES_HPP


#include <stdio.h>
#include <zlib.h>
#include <bzlib.h>


// INTERFACES

class IInputFile {
public:
    virtual ~IInputFile() { }  // close
    virtual unsigned Read(void* buffer, unsigned size) = 0;
};

class IOutputFile {
public:
    virtual ~IOutputFile() { }  // close
    virtual unsigned Write(void* buffer, unsigned size) = 0;
};


// FACTORIES

typedef IInputFile* (*InputFactory)(const char* filename);
typedef IOutputFile* (*OutputFactory)(const char* filename);

// ansi

extern IInputFile* OpenANSIInputFile(const char* filename);
extern IOutputFile* OpenANSIOutputFile(const char* filename);

// gzip

extern IInputFile* OpenGzipInputFile(const char* filename);
extern IOutputFile* OpenGzipOutputFile(const char* filename);

// bzip2

extern IInputFile* OpenBzip2InputFile(const char* filename);
extern IOutputFile* OpenBzip2OutputFile(const char* filename);


#endif
