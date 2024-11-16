

#include "nv_md5.h"

// MD5 算法使用的常量
#define F(x, y, z) ((x & y) | (~x & z))
#define G(x, y, z) ((x & z) | (y & ~z))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | ~z))
#define ROTATE_LEFT(x, n) ((x << n) | (x >> (32 - n)))

// MD5 算法使用的常量数组
const uint32_t K[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// MD5 上下文结构
typedef struct {
    uint32_t state[4];  // A, B, C, D
    uint32_t count[2];  // 位数计数器
    uint8_t buffer[64]; // 输入缓冲区
} MD5_CTX;

// 初始化 MD5 上下文
void MD5Init(MD5_CTX *context) {
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

// MD5 变换
void MD5Transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    for (int i = 0, j = 0; i < 16; ++i, j += 4)
        x[i] = (uint32_t)(block[j]) | ((uint32_t)(block[j+1]) << 8) |
               ((uint32_t)(block[j+2]) << 16) | ((uint32_t)(block[j+3]) << 24);

    // 主循环
    for (int i = 0; i < 64; i++) {
        uint32_t f, g;
        if (i < 16) {
            f = F(b, c, d);
            g = i;
        } else if (i < 32) {
            f = G(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = H(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            f = I(b, c, d);
            g = (7 * i) % 16;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROTATE_LEFT((a + f + K[i] + x[g]), 7);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

// 更新 MD5 上下文
void MD5Update(MD5_CTX *context, const uint8_t *input, size_t inputLen) {
    size_t i, index, partLen;

    index = (context->count[0] >> 3) & 0x3F;
    if ((context->count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((uint32_t)inputLen >> 29);

    partLen = 64 - index;

    if (inputLen >= partLen) {
        memcpy(&context->buffer[index], input, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform(context->state, &input[i]);

        index = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[index], &input[i], inputLen - i);
}

// 完成 MD5 计算
void MD5Final(uint8_t digest[16], MD5_CTX *context) {
    uint8_t bits[8];
    uint32_t index, padLen;

    for (int i = 0; i < 8; i++)
        bits[i] = (context->count[(i >= 4 ? 0 : 1)] >> ((i % 4) * 8)) & 0xFF;

    index = (context->count[0] >> 3) & 0x3f;
    padLen = (index < 56) ? (56 - index) : (120 - index);
    uint8_t padding[64] = {0x80};
    MD5Update(context, padding, padLen);
    MD5Update(context, bits, 8);

    for (int i = 0; i < 16; i++)
        digest[i] = (context->state[i >> 2] >> ((i & 0x03) * 8)) & 0xFF;
}

// 计算字符串的 MD5 值
void nv_MD5(const char *input, char *output) {
    MD5_CTX context;
    uint8_t digest[16];

    MD5Init(&context);
    MD5Update(&context, (const uint8_t *)input, strlen(input));
    MD5Final(digest, &context);

    for (int i = 0; i < 16; i++)
        sprintf(&output[i*2], "%02x", (unsigned int)digest[i]);
}



#define BUFFER_SIZE 4096

// 计算文件的MD5值
int nv_calculate_file_md5(const char *filename, char *output) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("无法打开文件");
        return 1;
    }

    MD5_CTX context;
    uint8_t buffer[BUFFER_SIZE];
    size_t bytes;

    MD5Init(&context);

    while ((bytes = fread(buffer, 1, BUFFER_SIZE, file)) != 0) {
        MD5Update(&context, buffer, bytes);
    }

    if (ferror(file)) {
        fclose(file);
        fprintf(stderr, "读取文件时发生错误\n");
        return 1;
    }

    uint8_t digest[16];
    MD5Final(digest, &context);

    for (int i = 0; i < 16; i++) {
        sprintf(&output[i*2], "%02x", (unsigned int)digest[i]);
    }

    fclose(file);
    return 0;
}

