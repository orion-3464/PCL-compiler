#ifndef __LIBPCL_H__
#define __LIBPCL_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define STDIN 0
#define STDOUT 1

typedef int32_t integer;
typedef bool boolean;
typedef double real;

// IO

void writeInteger(integer n);
void writeBoolean(boolean b);
void writeChar(char c);
void writeReal(real r);
void writeString(char* s);

integer readInteger();
boolean readBoolean();
char readChar();
real readReal();
void readString(integer size, char* s);

// Math

integer abs(integer n);
real fabs(real r);

real pcl_sqrt(real r);
real sin(real r);
real cos(real r);
real tan(real r);
real arctan(real r);
real pcl_exp(real r);
real ln(real r);
real pi();

// Transformation

integer trunc(real r);
integer round(real r);

integer ord(char c);
char chr(integer n);

#endif // __LIBPCL_H__

