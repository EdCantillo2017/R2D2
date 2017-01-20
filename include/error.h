#ifndef ERROR_H
#define ERROR_H

typedef int error;
typedef char byte;

namespace ERROR{
    const error NONE           = 0;
    const error NOSUCHDEVICE   = 1;
    const error NOTIMPLEMENTED = 2;
    const error NOTREADY       = 3;
    const error BADDATA        = 4;
}

#endif // ERROR_H
