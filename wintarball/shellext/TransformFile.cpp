#include <memory>
#include "ExtensionCompare.hpp"
#include "IUINotification.hpp"
#include "TransformFile.hpp"


////////////////////////////////////////////////////////////////////////////////

void Bzip2FilenameTransform(
    std::string& output_name,
    const char* source)
{
    output_name = source;
    output_name += ".bz2";
}

void Unbzip2FilenameTransform(
    std::string& output_name,
    const char* source)
{
    if (ExtensionCompare(source, ".bz2")) {
        output_name = source;
        output_name.resize(output_name.length() - 4);
    } else {
        output_name = source;
        output_name += ".unbz2";
    }
}

void GzipFilenameTransform(
    std::string& output_name,
    const char* source)
{
    output_name = source;
    output_name += ".gz";
}

void UngzipFilenameTransform(
    std::string& output_name,
    const char* source)
{
    if (ExtensionCompare(source, ".gz")) {
        output_name = source;
        output_name.resize(output_name.length() - 3);
    } else {
        output_name = source;
        output_name += ".ungz";
    }
}

////////////////////////////////////////////////////////////////////////////////

static bool DoTransformFile(
    IUINotification* notify,
    const char* filename,
    FilenameTransform fn_transform,
    InputFactory input_factory,
    OutputFactory output_factory,
    const char* action_message)
{
    static const int BUFFER_SIZE = 4096;

    // let the user know what we're doing
    std::string message = action_message;
    message += filename;
    notify->SetMessage(message.c_str());

    // calculate the output filename
    std::string output_name;
    fn_transform(output_name, filename);
    if (file_exists(output_name.c_str())) {
        std::string message =
            "File '" + output_name + "' already exists.  Overwrite?";
        UIResult r = notify->YesNoCancel(message.c_str(), UI_WARNING);
        if (r == UI_NO) {
            return true;
        } else if (r == UI_CANCEL) {
            return false;
        }
    }

    // open input file
    std::auto_ptr<IInputFile> in(input_factory(filename));
    if (!in.get()) {
        std::string message = "Can't open input file '";
        message += filename;
        message += "'.  Continue?";
        UIResult r = notify->YesNo(message.c_str(), UI_ERROR);
        return (r == UI_YES);
    }

    // open output file
    std::auto_ptr<IOutputFile> out(output_factory(output_name.c_str()));
    if (!out.get()) {
        std::string message =
            "Can't open output file '" + output_name + ".  Continue?";
        UIResult r = notify->YesNo(message.c_str(), UI_ERROR);
        return (r == UI_YES);
    }

    int progress = 0;
    int dp = 1;

    // do the file transformation
    char buffer[BUFFER_SIZE];
    bool done = false;
    while (!done) {
    
        int read = in->Read(buffer, BUFFER_SIZE);
        int sent = out->Write(buffer, read);

        if (sent != read) {
            delete out.release(); // close the output file
            remove(output_name.c_str());  // try to remove it
            
            std::string message =
                "Can't write to file '" + output_name + "'.  Continue?";
            UIResult r = notify->YesNo(message.c_str(), UI_ERROR);
            return (r == UI_YES);
            done = true;
        } else {
            done = (read != BUFFER_SIZE);
        }

        // update the progress bar
        notify->SetProgress(progress);
        progress = (progress + 256 + dp) % 256;
        if (progress == 0 && dp == 1) {
            dp = -1;
            progress = 255;
        } else if (progress == 255 && dp == -1) {
            dp = 1;
            progress = 0;
        }

        // check to see if we should stop
        if (notify->ShouldCancel()) {

            // close files
            delete out.release();
            delete in.release();

            // attempt to remove the output file
            remove(output_name.c_str());
            return false;;
        }
    }

    // if we got here, everything's okay
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TransformFile(
    IUINotification* notification,
    const char* filename,
    const Transform& t)
{
    return DoTransformFile(
        notification,
        filename,
        t.filename_transform,
        t.input_factory,
        t.output_factory,
        t.action_name);
}

////////////////////////////////////////////////////////////////////////////////
