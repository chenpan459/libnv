#ifndef _NV_MATH_H_INCLUDED_
#define _NV_MATH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include "../nv_base_include.h"
#include <math.h>
#include <stdio.h>

// 加法
double nv_add(double a, double b);

// 减法
double nv_subtract(double a, double b);

// 乘法
double nv_multiply(double a, double b);

// 除法
double nv_divide(double a, double b);

// 平方根
double nv_sqrt(double x);

// 幂运算
double nv_pow(double base, double exponent);

// 整数绝对值
int nv_abs(int num);

// 浮点数绝对值
double nv_fabs(double num);


#ifdef __cplusplus
}
#endif

#endif 