#include "nv_base64.h"
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char *nv_base64_encode(const unsigned char *input, size_t length) {
    if (input == NULL || length == 0) {
        return NULL;
    }

    size_t encoded_length = 4 * ((length + 2) / 3);
    char *encoded = (char *)malloc(encoded_length + 1);
    if (encoded == NULL) {
        return NULL;
    }

    int i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? (unsigned char)input[i++] : 0;
        uint32_t octet_b = i < length ? (unsigned char)input[i++] : 0;
        uint32_t octet_c = i < length ? (unsigned char)input[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    for (i = length; i % 3; i++) {
        encoded[j - 1] = '=';
    }

    encoded[encoded_length] = '\0';
    return encoded;
}


static const int base64_index[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int nv_base64_decode(const char *input, unsigned char **output) {
    if (input == NULL || output == NULL) {
        return -1;
    }

    size_t input_length = strlen(input);
    size_t padding = 0;

    // 计算填充字符的数量
    if (input[input_length - 1] == '=') padding++;
    if (input[input_length - 2] == '=') padding++;

    size_t decoded_length = (input_length / 4) * 3 - padding;
    *output = (unsigned char *)malloc(decoded_length);
    if (*output == NULL) {
        return -1;
    }

    int i, j;
    for (i = 0, j = 0; i < input_length;) {
        int sextet_a = base64_index[(unsigned char)input[i++]];
        int sextet_b = base64_index[(unsigned char)input[i++]];
        int sextet_c = i < input_length ? base64_index[(unsigned char)input[i++]] : 0;
        int sextet_d = i < input_length ? base64_index[(unsigned char)input[i++]] : 0;

        uint32_t triple = (sextet_a << 3 * 6)
                       + (sextet_b << 2 * 6)
                       + (sextet_c << 1 * 6)
                       + (sextet_d << 0 * 6);

        if (j < decoded_length) (*output)[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < decoded_length) (*output)[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < decoded_length) (*output)[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_length;
}


int nv_base64_main() {

    const char *original_data = "Hello, World!";
    size_t original_length = strlen(original_data);

    // 编码
    char *encoded_data = nv_base64_encode((const unsigned char *)original_data, original_length);
    if (encoded_data == NULL) {
        fprintf(stderr, "Encoding failed\n");
        return 1;
    }
    printf("Encoded: %s\n", encoded_data);

    // 解码
    unsigned char *decoded_data;
    int decoded_length = nv_base64_decode(encoded_data, &decoded_data);
    if (decoded_length < 0) {
        fprintf(stderr, "Decoding failed\n");
        free(encoded_data);
        return 1;
    }
    printf("Decoded: %.*s\n", decoded_length, decoded_data);

    free(encoded_data);
    free(decoded_data);
    return 0;

}