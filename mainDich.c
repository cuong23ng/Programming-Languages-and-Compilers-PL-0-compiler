#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "phantichtuvung.h"
#include "phantichcuphap.h"

int main(int argc, char *argv[]) {
    
    char source[1000];
    readSourceFromFile(argv[1], source, sizeof(source));
    
    tokenize(source, "token.txt");
    printf("\nPhan tich tu vung da luu vao file token.txt\n\n");

    loadTokens("token.txt"); // Đọc file tokens đã được tạo từ phantichtuvung.c 
    parseProgram(); // Phân tích cú pháp chương trình
    
    return 0;
}