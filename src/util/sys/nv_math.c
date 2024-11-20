#include "nv_math.h"

// 加法
double nv_add(double a, double b) {
    return a + b;
}

// 减法
double nv_subtract(double a, double b) {
    return a - b;
}

// 乘法
double nv_multiply(double a, double b) {
    return a * b;
}

// 除法
double nv_divide(double a, double b) {
    if (b == 0) {
        fprintf(stderr, "Error: Division by zero\n");
        return NAN;
    }
    return a / b;
}

// 平方根
double nv_sqrt(double x) {
    if (x < 0) {
        fprintf(stderr, "Error: Square root of negative number\n");
        return NAN;
    }
    return sqrt(x);
}

// 幂运算
double nv_pow(double base, double exponent) {
    return pow(base, exponent);
}

// 整数绝对值
int nv_abs(int num) {
    return abs(num);
}

// 浮点数绝对值
double nv_fabs(double num) {
    return fabs(num);
}