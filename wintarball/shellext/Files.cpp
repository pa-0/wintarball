#include "Files.hpp"
#include "../common/Configuration.hpp"


// ANSI IMPLEMENTATION

class ANSIInputFile : public IInputFile {
public:
    ANSIInputFile(FILE* file) {
        m_file = file;
    }

    ~ANSIInputFile() {
        fclose(m_file);
    }

    virtual unsigned Read(void* buffer, unsigned size) {
        return fread(buffer, 1, size, m_file);
    }
    
private:
    FILE* m_file;
};

class ANSIOutputFile : public IOutputFile {
public:
    ANSIOutputFile(FILE* file) {
        m_file = file;
    }

    ~ANSIOutputFile() {
        fclose(m_file);
    }

    virtual unsigned Write(void* buffer, unsigned size) {
        return fwrite(buffer, 1, size, m_file);
    }

private:
    FILE* m_file;
};


// GZIP IMPLEMENTATION

class GzipInputFile : public IInputFile {
public:
    GzipInputFile(gzFile file) {
        m_file = file;
    }

    ~GzipInputFile() {
        gzclose(m_file);
    }

    unsigned Read(void* buffer, unsigned size) {
        return gzread(m_file, buffer, size);
    }

private:
    gzFile m_file;
};

class GzipOutputFile : public IOutputFile {
public:
    GzipOutputFile(gzFile file) {
        m_file = file;
    }

    ~GzipOutputFile() {
        gzclose(m_file);
    }

    unsigned Write(void* buffer, unsigned size) {
        return gzwrite(m_file, buffer, size);
    }

private:
    gzFile m_file;
};


// BZIP2 IMPLEMENTATION

class Bzip2InputFile : public IInputFile {
public:
    Bzip2InputFile(BZFILE* file) {
        m_file = file;
    }

    ~Bzip2InputFile() {
        BZ2_bzclose(m_file);
    }

    virtual unsigned Read(void* buffer, unsigned size) {
        return BZ2_bzread(m_file, buffer, size);
    }

private:
    BZFILE* m_file;
};

class Bzip2OutputFile : public IOutputFile {
public:
    Bzip2OutputFile(BZFILE* file) {
        m_file = file;
    }

    ~Bzip2OutputFile() {
        BZ2_bzclose(m_file);
    }

    virtual unsigned Write(void* buffer, unsigned size) {
        return BZ2_bzwrite(m_file, buffer, size);
    }

private:
    BZFILE* m_file;
};



// XXXX maybe these should be split into IFileFactory objects
// (factories could support parameters without using globals, like gzip
// compression level)

////////////////////////////////////////////////////////////////////////////////

IInputFile* OpenANSIInputFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        return new ANSIInputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

IOutputFile* OpenANSIOutputFile(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file) {
        return new ANSIOutputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

IInputFile* OpenGzipInputFile(const char* filename) {
    gzFile file = gzopen(filename, "rb");
    if (file) {
        return new GzipInputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

IOutputFile* OpenGzipOutputFile(const char* filename) {
    char mode[4] = { 'w', 'b', 0, 0 };
    mode[2] = '0' + g_Configuration.gzip_compression_level;
    gzFile file = gzopen(filename, mode);
    if (file) {
        return new GzipOutputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

IInputFile* OpenBzip2InputFile(const char* filename) {
    BZFILE* file = BZ2_bzopen(filename, "rb");
    if (file) {
        return new Bzip2InputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

IOutputFile* OpenBzip2OutputFile(const char* filename) {
    char mode[5] = { 'w', 'b', 0, 0, 0 };
    mode[2] = '0' + g_Configuration.bzip2_block_size;
    if (g_Configuration.bzip2_block_size) {
        mode[3] = 's';
    }
    BZFILE* file = BZ2_bzopen(filename, mode);
    if (file) {
        return new Bzip2OutputFile(file);
    } else {
        return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
