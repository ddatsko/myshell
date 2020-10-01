#ifndef MYCAT_IO_H
#define MYCAT_IO_H

const unsigned int readBufSize = 1u << 6u;

int readPrintFile(int fd, bool displayInvisible);

#endif
