#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP


struct Configuration
{
    int  bzip2_block_size;       // 1-9
    bool bzip2_less_memory;
    int  gzip_compression_level; // 1-9

    Configuration() {
      // defaults
      bzip2_block_size       = 9;
      bzip2_less_memory      = false;
      gzip_compression_level = 6;
    }
    
    bool Load();
    bool Save();
};


// a singleton would be better...
extern Configuration g_Configuration;


#endif
