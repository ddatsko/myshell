#include <iostream>
#include "mycat/io/io.h"
#include <unistd.h>


ssize_t readBuf(int fd, char *buf, ssize_t size, int *status) {
    ssize_t readBytes = 0;
#ifdef HIGH_LEVEL
    FILE *readFile = fdopen(fd, "r");

    if (readFile == nullptr) return -4;
    if ((readBytes = fread(buf, sizeof(char), size, readFile)) != size) {
        if (feof(readFile)) {
            return readBytes;
        }
        return -5;
    }
    return readBytes;
#endif
    if (buf == nullptr || status == nullptr) return -1;
    while (readBytes < size) {
        ssize_t nRead = read(fd, buf + readBytes, size - readBytes);
        if (nRead == -1) {
            if (errno == EINTR) continue;
            *status = errno;
            return -1;
        }
        if (nRead == 0) {
            return readBytes;
        }
        readBytes += nRead;
    }
    return readBytes;
}

int writeBuf(int fd, const char *buf, int size) {
    ssize_t writtenBytes = 0;
#ifdef HIGH_LEVEL
    FILE *writeFile = fdopen(fd, "w");
    if (writeFile == nullptr) return -6;
    if ((writtenBytes = fwrite(buf, sizeof(char), size, writeFile)) != size) {
        return -7;
    }
    return 0;

#endif
    if (buf == nullptr) return -1;
    while (writtenBytes < size) {
        ssize_t writtenNow = write(fd, buf + writtenBytes, size - writtenBytes);
        if (writtenNow == -1) {
            if (errno == EINTR) continue;
            else {
                return errno;
            }
        }
        writtenBytes += writtenNow;
    }
    return 0;
}


unsigned char hexDigit(unsigned char x) {
    return x < 10 ? '0' + x : 'A' + (x - 10);
}

void writeHex(char *buf, unsigned char c) {
    buf[0] = '\\';
    buf[1] = 'x';
    buf[2] = static_cast<char>(hexDigit(c / 16));
    buf[3] = static_cast<char>(hexDigit(c % 16));
}

int convertInvisibleToHex(char *inBuf, char *outBuf, int size) {
    int outputSize = 0;
    for (int i = 0; i < size; i++) {
        if (inBuf[i] != '\n' && not isprint(inBuf[i])) {
            writeHex(outBuf + outputSize, inBuf[i]);
            outputSize += 4;
        } else {
            outBuf[outputSize++] = inBuf[i];
        }
    }
    return outputSize;
}


int readPrintFile(int fd, bool displayInvisible) {
    static char readingBuf[readBufSize];
    static char writingBuf[readBufSize * 4];
    int readStatus = 0, writeStatus;
    ssize_t readBytes;
    while ((readBytes = readBuf(fd, readingBuf, readBufSize, &readStatus)) > 0) {
        if (displayInvisible) {
            int newSize = convertInvisibleToHex(readingBuf, writingBuf, readBytes);
            writeStatus = writeBuf(1, writingBuf, newSize);
        } else {
            writeStatus = writeBuf(1, readingBuf, readBytes);
        }
        if (writeStatus < 0) {
            return -1;
        }
    }
    if (readStatus != 0) {  // If loop crashed and read status changed
        return readStatus;
    }
    return 0;
}
