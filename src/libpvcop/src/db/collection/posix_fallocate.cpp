#include <pvcop/db/posix_fallocate.h>

#include <unistd.h>
#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include <pvlogger.h>

// see https://hg.mozilla.org/mozilla-central/file/3d846420a907/xpcom/glue/FileUtils.cpp#l72

int posix_fallocate(int fd, off_t offset, off_t len)
{    
    len = offset + len;
#ifdef _WIN32
    if (offset < 0 || len < 0) {
        return EINVAL;
    }

    HANDLE hFile = (HANDLE)_get_osfhandle(fd);
    if (hFile == INVALID_HANDLE_VALUE) {
        return EBADF;
    }

    LARGE_INTEGER fileSize;
    fileSize.QuadPart = len;

    LARGE_INTEGER currentSize;
    if (!GetFileSizeEx(hFile, &currentSize)) {
        return EIO;
    }

    // Only extend the file if needed
    if (fileSize.QuadPart > currentSize.QuadPart) {
        // Save current file pointer position
        LARGE_INTEGER originalPos;
        originalPos.QuadPart = 0;
        if (!SetFilePointerEx(hFile, originalPos, &originalPos, FILE_CURRENT)) {
            return EIO;
        }

        // Extend the file
        if (!SetFilePointerEx(hFile, fileSize, NULL, FILE_BEGIN)) {
            return EIO;
        }

        if (!SetEndOfFile(hFile)) {
            return EIO;
        }

        // Restore original file pointer position
        SetFilePointerEx(hFile, originalPos, NULL, FILE_BEGIN);
    }

    return 0;

#elifdef __APPLE__
    struct stat buf;
    if (fstat(fd, &buf)) {
        return 1;
    }

    if (buf.st_size >= len) {
        return 0;
    }

    const int block_size = buf.st_blksize;
    if (not block_size) {
        return 3;
    }

    if (ftruncate(fd, len)) {
        return 4;
    }

    FILE* fp = fdopen(fd, "w");

    int write_count;
    auto write_offset = ((buf.st_size + 2 * block_size - 1) / block_size) * block_size - 1;
    do {
        write_count = 0;
        if (fseek(fp, write_offset, SEEK_SET) == write_offset) {
            write_count = fwrite("", 1, 1, fp);
        }
        write_offset += block_size;
    } while (write_count == 0 and write_offset < len);

    return write_count != 0;
#endif
}