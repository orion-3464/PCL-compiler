#include "libpcl.h"

// IO

void writeInteger(integer n) {
    printf("%" PRId64, n);
}

void writeBoolean(boolean b) {
    if (b) printf("true");
    else printf("false");
}

void writeChar(char c) {
    putchar(c);
}

void writeReal(real r) {
    char text[64];

    snprintf(text, sizeof(text), "%.6g", r);
    fputs(text, stdout);

    if (strspn(text, "+-0123456789") == strlen(text)) {
        fputs(".0", stdout);
    }
}

void writeString(char *s) {
    printf("%s", s);
}

integer readInteger() {
    integer n;
    scanf("%" SCNd64, &n);
    
    // To make test cases work
    int c = getchar();
    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);
    }

    return n;
}

boolean readBoolean() {
    int b;
    scanf("%d", &b);
    return (b) ? 1 : 0;
}

char readChar() {
    char c;
    scanf(" %c", &c);
    return c;
}

real readReal() {
    real r;
    scanf("%lf", &r);
    return r;
}

void readString(integer size, char* s) {
    if (size <= 0) return;
    int c;
    integer i = 0;
    
    while (i < size - 1) {
        c = getchar();
        if (c == '\n' || c == EOF) break;        
        s[i++] = (char)c;
    }
    s[i] = '\0';
}

// Math

integer abs(integer n) {
    return __builtin_labs(n);
}

real fabs(real r) {
    return __builtin_fabs(r);
}

real pcl_sqrt(real r) {
    return __builtin_sqrt(r);
}

real sin(real r) {
    return __builtin_sinl(r);
}

real cos(real r) {
    return __builtin_cosl(r);
}

real tan(real r) {
    return __builtin_tanl(r);
}

real arctan(real r) {
    return __builtin_atanl(r);
}

real pcl_exp(real r) {
    return __builtin_exp(r);
}

real ln(real r) {
    return __builtin_log(r);
}

real pi() {
    return __builtin_acos(-1.0);
}

// Transformation

integer trunc(real r) {
    return __builtin_truncl(r);
}

integer round(real r) {
    return __builtin_llround(r);
}

integer ord(char c) {
    return (integer)(c);
}

char chr(integer n) {
    return (char)(n);
}

