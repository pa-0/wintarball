#ifndef TRANSFORM_FILE_HPP
#define TRANSFORM_FILE_HPP


#include <string>
#include "Files.hpp"


class IUINotification;


typedef void (*FilenameTransform)(
    std::string& output_name,
    const char* source);


extern void Bzip2FilenameTransform  (std::string&, const char*);
extern void Unbzip2FilenameTransform(std::string&, const char*);
extern void GzipFilenameTransform   (std::string&, const char*);
extern void UngzipFilenameTransform (std::string&, const char*);


struct Transform {
    FilenameTransform filename_transform;
    InputFactory      input_factory;
    OutputFactory     output_factory;
    const char*       action_name;
};

static const Transform Bzip2Transform = {
    Bzip2FilenameTransform,
    OpenANSIInputFile,
    OpenBzip2OutputFile,
    "Compressing ",
};

static const Transform Unbzip2Transform = {
    Unbzip2FilenameTransform,
    OpenBzip2InputFile,
    OpenANSIOutputFile,
    "Decompressing ",
};

static const Transform GzipTransform = {
    GzipFilenameTransform,
    OpenANSIInputFile,
    OpenGzipOutputFile,
    "Compressing ",
};

static const Transform UngzipTransform = {
    UngzipFilenameTransform,
    OpenGzipInputFile,
    OpenANSIOutputFile,
    "Decompressing ",
};


// returns true if shell should go on to the next file
// returns false if it should stop all processing
extern bool TransformFile(
    IUINotification* notification,
    const char* filename,
    const Transform& t);


#endif
