#ifndef TAR_HPP
#define TAR_HPP


struct tar_posix_header
{                               /* byte offset */
    char name[100];             /*   0 */
    char mode[8];               /* 100 */
    char uid[8];                /* 108 */
    char gid[8];                /* 116 */
    char size[12];              /* 124 */
    char mtime[12];             /* 136 */
    char chksum[8];             /* 148 */
    char typeflag;              /* 156 */
    char linkname[100];         /* 157 */
    char magic[6];              /* 257 */
    char version[2];            /* 263 */
    char uname[32];             /* 265 */
    char gname[32];             /* 297 */
    char devmajor[8];           /* 329 */
    char devminor[8];           /* 337 */
    char prefix[155];           /* 345 */
                                /* 500 */
};

static const int TAR_BLOCK_SIZE = 512;

union tar_block {
    char buffer[TAR_BLOCK_SIZE];
    tar_posix_header header;
};

static int octal_to_int(char* o, int size) {
    int result = 0;
    while (--size) {
        char c = *o++;
        int value = (c >= '0' && c <= '7' ? c - '0' : 0);
        result = (result * 8) + value;
    }
    return result;
}



#endif
