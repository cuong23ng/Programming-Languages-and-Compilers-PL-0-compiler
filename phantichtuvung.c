#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "phantichtuvung.h"

#define MAX_TOKEN_LEN 100

// Danh sách các từ khóa trong PL/0
const char *keywords[] = {"begin", "call", "const", "do", "else", "end", "for", "if", 
                        "odd", "procedure", "program", "then", "to", "var", "while"};
#define NUM_KEYWORDS (sizeof(keywords) / sizeof(keywords[0]))

// Định nghĩa các loại token
typedef enum {
    NONE, IDENT, NUMBER, KEYWORD,
    BEGIN, CALL, CONST, DO, ELSE, END, FOR, IF, ODD, PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE,
    PLUS, MINUS, TIMES, SLASH, EQU, NEQ, LSS, LEQ, GTR, GEQ,
    LPARENT, RPARENT, LBRACK, RBRACK, PERIOD, COMMA, SEMICOLON, ASSIGN, PERCENT,
    COMMENT
} TokenType;

// Đọc mã nguồn từ file
void readSourceFromFile(const char *filename, char *buffer, int maxSize) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("khong the mo file");
        exit(1);
    }
    size_t bytesRead = fread(buffer, 1, maxSize - 1, file);
    buffer[bytesRead] = '\0'; // Đảm bảo chuỗi luôn kết thúc
    fclose(file);
}

// Lưu kết quả đã phân tích từ vựng vào file khác
void saveTokenToFile(const char *filename, const char *format, ...) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        perror("Khong the mo file de luu");
        exit(1);
    }
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fclose(file);
}

// Cấu trúc token
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

// Kiểm tra từ có phải từ khóa không
TokenType getKeywordType(char *str) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return KEYWORD;
        }
    }
    return IDENT;
}

// Nhận diện token từ mã nguồn
void tokenize(char *source, const char *outputFile) {
    int i = 0;
    char buffer[MAX_TOKEN_LEN];
    int in_comment = 0;
    FILE *file = fopen(outputFile, "w"); // Xóa nội dung file cũ
    fclose(file);

    while (source[i] != '\0') {
        if (in_comment) {
            if (source[i] == '*' && source[i + 1] == '/') {
                in_comment = 0;
                i += 2;
            } else {
                i++;
            }
            continue;
        }

        if (isspace(source[i])) {
            i++;
            continue;
        }

        // Bắt đầu chú thích dạng /* ... */
        if (source[i] == '/' && source[i + 1] == '*') {
            in_comment = 1;
            i += 2;
            saveTokenToFile(outputFile, "COMMENT: /*...*/\n");
            continue;
        }

        // Bắt đầu chú thích dòng đơn //
        if (source[i] == '/' && source[i + 1] == '/') {
            while (source[i] != '\n' && source[i] != '\0') {
                i++;
            }
            saveTokenToFile(outputFile, "COMMENT: //...\n");
            continue;
        }
        
        // Nhận diện số
        if (isdigit(source[i])) {
            int j = 0;
            while (isdigit(source[i])) {
                buffer[j++] = source[i++];
            }
            buffer[j] = '\0';
            saveTokenToFile(outputFile, "NUMBER: %s\n", buffer);
            continue;
        }
        
        // Nhận diện identifier hoặc keyword
        if (isalpha(source[i])) {
            int j = 0;
            while (isalnum(source[i])) {
                buffer[j++] = source[i++];
            }
            buffer[j] = '\0';
            TokenType type = getKeywordType(buffer);
            if (type == KEYWORD) {
                saveTokenToFile(outputFile, "KEYWORD: %s\n", buffer);
            } else {
                saveTokenToFile(outputFile, "IDENT: %s\n", buffer);
            }
            continue;
        }
        
        // Nhận diện ký hiệu đặc biệt
        switch (source[i]) {
            case '+': saveTokenToFile(outputFile, "PLUS: +\n"); break;
            case '-': saveTokenToFile(outputFile, "MINUS: -\n"); break;
            case '*': saveTokenToFile(outputFile, "TIMES: *\n"); break;
            case '/': saveTokenToFile(outputFile, "SLASH: /\n"); break;
            case '=': saveTokenToFile(outputFile, "EQU: =\n"); break;
            case '<': saveTokenToFile(outputFile, (source[i+1] == '=') ? "LEQ: <=\n" : "LSS: <\n"); 
                i += (source[i+1] == '='); break;
            case '>': saveTokenToFile(outputFile, (source[i+1] == '=') ? "GEQ: >=\n" : "GTR: >\n"); 
                i += (source[i+1] == '='); break;
            case '(': saveTokenToFile(outputFile, "LPARENT: (\n"); break;
            case ')': saveTokenToFile(outputFile, "RPARENT: )\n"); break;
            case '[': saveTokenToFile(outputFile, "LBRACK: [\n"); break;
            case ']': saveTokenToFile(outputFile, "RBRACK: ]\n"); break;
            case '.': saveTokenToFile(outputFile, "PERIOD: .\n"); break;
            case ',': saveTokenToFile(outputFile, "COMMA: ,\n"); break;
            case ';': saveTokenToFile(outputFile, "SEMICOLON: ;\n"); break;
            case ':':
                if (source[i+1] == '=') {
                    saveTokenToFile(outputFile, "ASSIGN: :=\n");
                    i++;
                }
                break;
            case '%': saveTokenToFile(outputFile, "PERCENT: %%\n"); break;
            default: saveTokenToFile(outputFile, "NONE: %c\n", source[i]); break;
        }
        i++;
    }
}
