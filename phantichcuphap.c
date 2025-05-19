#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phantichcuphap.h" 

/*
Các sản xuất: 
1. program -> PROGRAM Ident ; Block . 
- block:
2. Block ? ConstDecl VarDecl ProcDecls BEGIN StatementList END 
3. ConstDecl ? CONST ConstAssignList ; | e 
4. ConstAssignList ? ConstAssign ConstAssignList' 
5. ConstAssignList' ? , ConstAssign ConstAssignList' | e 
6. ConstAssign ? Ident = Number 
7. VarDecl ? VAR VarDeclList ; | e 
8. VarDeclList ? VarItem VarDeclList' 
9. VarDeclList' ? , VarItem VarDeclList' | e 
10. VarItem ? Ident | Ident [ Number ] 
11. ProcDecls ? ProcDecl ProcDecls | e 
12. ProcDecl ? PROCEDURE Ident ProcParamsOpt ; Block ; 
13. ProcParamsOpt ? ( ParamDeclList ) | e 
14. ParamDeclList ? ParamDecl ParamDeclList' 
15. ParamDeclList' ? ; ParamDecl ParamDeclList' | e 
16. ParamDecl ? VAR Ident | Ident
// 17. StatementList ? Statement StatementList' 
// 18. StatementList' ? ; Statement StatementList' | e 
- statement:
17. statement -> assignment  
18. statement -> callStmt  
19. statement -> compoundStmt  
20. statement -> ifStmt  
21. statement -> whileStmt  
22. statement -> forStmt  
23. assignment -> Ident LHS := Expression 
24. LHS -> [ Expression ]  
25. LHS -> epsilon 
26, callStmt -> CALL Ident argList 
27, argList -> (Expression moreArgs) 
28, argList -> epsilon 
29. moreArgs -> , Expression moreArgs  
30. moreArgs -> epsilon  
31. compoundStmt -> BEGIN stmtList END  
32. stmtList -> statement stmtListTail  
33. stmtListTail -> ; statement stmtListTail  
34. stmtListTail -> epsilon  
35. ifStmt -> IF Condition THEN statement elsePart  
36. elsePart -> ELSE statement  
37. elsePart -> epsilon 
38. whileStmt -> WHILE Condition DO statement  
39. forStmt -> FOR Ident := Expression TO Expression DO statement 
- factor: 
40, Factor -> Ident A 
41, A -> [ Expression ] | epsilon 
42, Factor -> Number 
43, Factor -> ( Expression ) 
- condition:
44, Condition -> ODD Expression 
45, Condition -> Expression B Expression 
46, B -> = | > | >= | < | <= | <> 
- expression
47. expression ? A expression' 
48. A ? + term | - term 
49. A ? term 
50. expression' ? + term expression' | - term expression' 
51. expression' ? e 
- term
52. term ? factor term' 
53. term' ? * factor term' | / factor term' | % factor term' 
54. term' ? e
*/

typedef enum {
    NONE, IDENT, NUMBER, KEYWORD,
    BEGIN, CALL, CONST, DO, ELSE, END, FOR, IF, ODD, PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE,
    PLUS, MINUS, TIMES, SLASH, EQU, NEQ, LSS, LEQ, GTR, GEQ,
    LPARENT, RPARENT, LBRACK, RBRACK, PERIOD, COMMA, SEMICOLON, ASSIGN, PERCENT,
    COMMENT
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
    int line; // Dòng trong file nguồn 
} Token;

#define MAX_TOKENS 1000
Token tokens[MAX_TOKENS];
int tokenCount = 0;
int current = 0;

// Forward declarations cho các hàm phân tích ngữ nghĩa
void initSemantic();
void addSemanticError(const char* message, int line);
int findVariable(const char* name);
int findProcedure(const char* name);
void checkAssignment(const char* varName, int isArray, int line);
void checkCallStatement(const char* procName, int line);
void checkIfStatement(int line);
void checkWhileStatement(int line);
void checkForStatement(const char* varName, int line);
void checkCondition(int line);
void checkExpression(int line);
void printSemanticErrors();
void checkStatementSemantics(TokenType stmtType, const char* varName, int line);

// Forward declarations cho các hàm parse
void parseProgram();
void parseBlock();
void parseConstDecl();
void parseConstAssign();
void parseConstAssignListPrime();
void parseVarDecl();
void parseVarItem();
void parseVarDeclListPrime();
void parseProcDecls();
void parseProcParamsOpt(); 
void parseParamDeclList();
void parseParamDeclListPrime(); 
void parseParamDecl(); 
void parseStatementList();
void parseStatementListPrime(); 
void parseProcDecl();
void parseStatement();
void parseIfStmt(); 
void parseElsePart(); 
void parseForStmt(); 
void parseAssignment();
void parseLHS(); 
void parseCallStmt();
void parseArgList();
void parseMoreArgs();
void parseCompoundStmt();
void parseWhileStmt();
void parseExpression();
void parseTerm();
void parseFactor();
void parseCondition();

char production_trace[1000] = "";
void record(int prod_number) {
    char temp[10];
    sprintf(temp, "%d ", prod_number);
    strcat(production_trace, temp);
}

void loadTokens(const char* filename) {
    FILE* file = fopen(filename, "r");
    char typeStr[50], lexeme[100];
    int lineNumber = 1; // Khởi tạo số dòng 
    while (!feof(file)) {
        char line[200];
        if (fgets(line, sizeof(line), file) == NULL) break;
        if (strlen(line) < 3) {
            lineNumber++; // Tăng số dòng 
            continue; // Bỏ qua dòng trắng/ngắn
        }    
        if (sscanf(line, "%[^:]: %[^\n]", typeStr, lexeme) == 2) {
            Token t;
            if (strcmp(typeStr, "KEYWORD") == 0) {
                if (strcmp(lexeme, "program") == 0) t.type = PROGRAM;
                else if (strcmp(lexeme, "const") == 0) t.type = CONST;
                else if (strcmp(lexeme, "var") == 0) t.type = VAR;
                else if (strcmp(lexeme, "procedure") == 0) t.type = PROCEDURE;
                else if (strcmp(lexeme, "begin") == 0) t.type = BEGIN;
                else if (strcmp(lexeme, "end") == 0) t.type = END;
                else if (strcmp(lexeme, "while") == 0) t.type = WHILE;
                else if (strcmp(lexeme, "do") == 0) t.type = DO;
                else if (strcmp(lexeme, "call") == 0) t.type = CALL;
                else if (strcmp(lexeme, "if") == 0) t.type = IF;
                else if (strcmp(lexeme, "then") == 0) t.type = THEN;
                else if (strcmp(lexeme, "else") == 0) t.type = ELSE;
                else if (strcmp(lexeme, "odd") == 0) t.type = ODD;
                else if (strcmp(lexeme, "for") == 0) t.type = FOR;
                else if (strcmp(lexeme, "to") == 0) t.type = TO;
                else t.type = KEYWORD;
            } else if (strcmp(typeStr, "IDENT") == 0) {
                t.type = IDENT;
            } else if (strcmp(typeStr, "NUMBER") == 0) {
                t.type = NUMBER;
            } else if (strcmp(typeStr, "SEMICOLON") == 0) {
                t.type = SEMICOLON;
            } else if (strcmp(typeStr, "PERIOD") == 0) {
                t.type = PERIOD;
            } else if (strcmp(typeStr, "EQU") == 0) {
                t.type = EQU;
            } else if (strcmp(typeStr, "ASSIGN") == 0) {
                t.type = ASSIGN;
            } else if (strcmp(typeStr, "COMMA") == 0) {
                t.type = COMMA;
            } else if (strcmp(typeStr, "PLUS") == 0) {
                t.type = PLUS;
            } else if (strcmp(typeStr, "COMMENT") == 0) {
                t.type = COMMENT;
            } 
            else if (strcmp(typeStr, "MINUS") == 0) {
                t.type = MINUS;
            } else if (strcmp(typeStr, "TIMES") == 0) {
                t.type = TIMES;
            } else if (strcmp(typeStr, "SLASH") == 0) {
                t.type = SLASH;
            } else if (strcmp(typeStr, "LPARENT") == 0) {
                t.type = LPARENT;
            } else if (strcmp(typeStr, "RPARENT") == 0) {
                t.type = RPARENT;
            } else if (strcmp(typeStr, "LBRACK") == 0) {
                t.type = LBRACK;
            } else if (strcmp(typeStr, "RBRACK") == 0) {
                t.type = RBRACK;
            } else if (strcmp(typeStr, "PERCENT") == 0) {
                t.type = PERCENT;
            }
            else if (strcmp(typeStr, "NEQ") == 0) {
                t.type = NEQ;
            } else if (strcmp(typeStr, "LSS") == 0) {
                t.type = LSS;
            } else if (strcmp(typeStr, "LEQ") == 0) {
                t.type = LEQ;
            } else if (strcmp(typeStr, "GTR") == 0) {
                t.type = GTR;
            } else if (strcmp(typeStr, "GEQ") == 0) {
                t.type = GEQ;
            }
            else {
                t.type = NONE;
            }
        
            strcpy(t.lexeme, lexeme);
            t.line = lineNumber; // Lưu số dòng
            tokens[tokenCount++] = t;
            lineNumber++; // Tăng số dòng 
        }
    }
    fclose(file);
}

Token peek() {
    while (current < tokenCount && tokens[current].type == COMMENT) {
        current++; // Bỏ qua COMMENT 
    }
    if (current < tokenCount) return tokens[current];
    Token none = { NONE, "" };
    return none;
}

Token advance() {
    while (current < tokenCount && tokens[current].type == COMMENT) {
        current++; // Bỏ qua COMMENT
    }
    if (current < tokenCount) return tokens[current++];
    Token none = { NONE, "" };
    return none;
}

int match(TokenType expected) {
    if (peek().type == expected) {
        advance();
        return 1;
    }
    return 0;
}

void error(const char* msg) {
    Token t = peek();
    printf("Syntax error: %s at token '%s' (line %d)\n\n", msg, peek().lexeme, t.line);
    exit(1);
}

// Cấu trúc lưu trữ thông tin biến
typedef struct {
    char name[100];
    int isArray;
    int arraySize;
    int isParam;
    int isVarParam; // true nếu là tham số VAR
} Variable;

// Cấu trúc lưu trữ thông tin procedure
typedef struct {
    char name[100];
    Variable params[100];
    int paramCount;
} Procedure;

// Cấu trúc lưu trữ thông tin lỗi ngữ nghĩa
typedef struct {
    char message[200];
    int line;
} SemanticError;

// Danh sách biến và procedure cho phân tích ngữ nghĩa
Variable variables[1000];
int varCount = 0;
Procedure procedures[100];
int procCount = 0;
SemanticError semanticErrors[100];
int errorCount = 0;

// Các hàm phân tích ngữ nghĩa
void initSemantic() {
    varCount = 0;
    procCount = 0;
    errorCount = 0;
}

void addSemanticError(const char* message, int line) {
    if (errorCount < 100) {
        strcpy(semanticErrors[errorCount].message, message);
        semanticErrors[errorCount].line = line;
        errorCount++;
    }
}

int findVariable(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int findProcedure(const char* name) {
    for (int i = 0; i < procCount; i++) {
        if (strcmp(procedures[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void checkAssignment(const char* varName, int isArray, int line) {
    int varIndex = findVariable(varName);
    if (varIndex == -1) {
        addSemanticError("Bien chua duoc khai bao", line);
        return;
    }
    
    if (variables[varIndex].isArray != isArray) {
        if (isArray) {
            addSemanticError("Bien khong phai la mang", line);
        } else {
            addSemanticError("Bien la mang, khong the gan truc tiep", line);
        }
    }
    
    if (variables[varIndex].isParam && !variables[varIndex].isVarParam) {
        addSemanticError("Khong the gan gia tri cho tham so khong phai VAR", line);
    }
}

void checkCallStatement(const char* procName, int line) {
    int procIndex = findProcedure(procName);
    if (procIndex == -1) {
        addSemanticError("Procedure chua duoc khai bao", line);
    }
}

void checkIfStatement(int line) {
    // Kiểm tra điều kiện trong if statement
    checkCondition(line);
}

void checkWhileStatement(int line) {
    // Kiểm tra điều kiện trong while statement
    checkCondition(line);
}

void checkForStatement(const char* varName, int line) {
    int varIndex = findVariable(varName);
    if (varIndex == -1) {
        addSemanticError("Bien trong vong lap FOR chua duoc khai bao", line);
        return;
    }
    
    if (variables[varIndex].isArray) {
        addSemanticError("Khong the su dung mang trong vong lap FOR", line);
    }
    
    if (variables[varIndex].isParam && !variables[varIndex].isVarParam) {
        addSemanticError("Khong the su dung tham so khong phai VAR trong vong lap FOR", line);
    }
}

void checkCondition(int line) {
    // Kiểm tra biểu thức điều kiện
    checkExpression(line);
}

void checkExpression(int line) {
    // Kiểm tra biểu thức
    // Trong trường hợp đơn giản, chỉ cần đảm bảo các biến được sử dụng đã được khai báo
    // TODO: Thêm kiểm tra chi tiết hơn cho biểu thức
}

void printSemanticErrors() {
    if (errorCount == 0) {
        printf("Khong co loi ngu nghia nao.\n");
        return;
    }
    
    printf("\nDanh sach loi ngu nghia:\n");
    for (int i = 0; i < errorCount; i++) {
        printf("Dong %d: %s\n", semanticErrors[i].line, semanticErrors[i].message);
    }
}

void checkStatementSemantics(TokenType stmtType, const char* varName, int line) {
    switch(stmtType) {
        case IDENT: // Assignment
            checkAssignment(varName, peek().type == LBRACK, line);
            break;
        case CALL: // Call statement
            checkCallStatement(varName, line);
            break;
        case IF: // If statement
            checkIfStatement(line);
            break;
        case WHILE: // While statement
            checkWhileStatement(line);
            break;
        case FOR: // For statement
            checkForStatement(varName, line);
            break;
    }
}

void parseFactor() {
    if (match(IDENT)) {
        record(40); // 40. factor -> Ident A
        record(41); // 41. A -> [ Expression ] | e
        return;
    }
    if (match(NUMBER)) {
        record(42); // 42. factor -> Number
        return;
    }
    if (match(LPARENT)) {
        record(43); // 43. factor -> ( Expression )
        parseExpression();
        if (!match(RPARENT)) error("Expected ')'");
        return;
    }
    error("Invalid factor");
}

void parseTerm() {
    record(52); // 52. term -> factor term'
    parseFactor();
    while (peek().type == TIMES || peek().type == SLASH || peek().type == PERCENT) {
        record(53); // 53. term' -> * factor term' | / factor term' | % factor term'
        advance();
        parseFactor();
    }
    record(54); // 54. term' -> e
}

void parseExpression() {
    record(47); // 47. expression -> A expression'
    if (peek().type == PLUS || peek().type == MINUS) {
        record(48); // 48. A -> + term | - term
        advance();
        parseTerm();
    } else {
        record(49); // 49. A -> term
        parseTerm();
    }
    while (peek().type == PLUS || peek().type == MINUS) {
        record(50); // 50. expression' -> + term expression' | - term expression'
        advance();
        parseTerm();
    }
    record(51); // 51. expression' -> e
}

void parseAssignment() {
    Token varToken = peek();
    if (!match(IDENT)) error("Expected identifier in assignment");
    parseLHS(); 
    if (!match(ASSIGN)) error("Expected ':=' in assignment");
    parseExpression();
    
    // Thêm kiểm tra ngữ nghĩa
    checkStatementSemantics(IDENT, varToken.lexeme, varToken.line);
}

void parseLHS() {
    if (match(LBRACK)) {
        record(24); // 24. LHS -> [ Expression ] 
        parseExpression();
        if (!match(RBRACK)) error("Expected ']' in array access");
    } else {
        record(25); // 25. LHS -> e 
    }
}

void parseCallStmt() {
    Token procToken = peek();
    if (!match(CALL)) error("Expected 'call'");
    if (!match(IDENT)) error("Expected procedure identifier");
    
    // Thêm kiểm tra ngữ nghĩa
    checkStatementSemantics(CALL, procToken.lexeme, procToken.line);
}

void parseArgList() {
    if (match(LPARENT)) {
        record(27); // 27. argList -> ( Expression moreArgs ) 
        parseExpression();
        parseMoreArgs();
        if (!match(RPARENT)) error("Expected ')' after arguments");
    } else {
        record(28); // 28. argList -> e 
    }
}

void parseMoreArgs() {
    if (match(COMMA)) {
        record(29); // 29. moreArgs -> , Expression moreArgs 
        parseExpression();
        parseMoreArgs();
    } else {
        record(30); // 30. moreArgs -> e 
    }
}

void parseCompoundStmt() {
    if (!match(BEGIN)) error("Expected 'begin'");
    record(31); // 31. compoundStmt -> BEGIN stmtList END
    parseStatementList();
    
    if (!match(END)) error("Expected 'end'");
}

void parseWhileStmt() {
    Token whileToken = peek();
    if (!match(WHILE)) error("Expected 'while'");
    parseCondition();
    if (!match(DO)) error("Expected 'do'");
    parseStatement();
    
    // Thêm kiểm tra ngữ nghĩa
    checkStatementSemantics(WHILE, NULL, whileToken.line);
}

void parseCondition() {
    if (match(ODD)) {
        record(44); // 44. Condition -> ODD Expression
        parseExpression();
    } else {
        record(45); // 45. Condition -> Expression B Expression
        parseExpression();
        TokenType op = peek().type;
        if (op == EQU || op == NEQ || op == LSS || op == LEQ || op == GTR || op == GEQ) {
            record(46); // 48. B -> = | > | >= | < | <= | <>
            advance();
        } else {
            error("Expected relational operator (=, <>, <, <=, >, >=)");
        }
        parseExpression();
    }
}

void parseStatement() {
    Token t = peek();
    if (t.type == IDENT && current + 1 < tokenCount && tokens[current + 1].type == ASSIGN) {
        record(17); // 17. statement -> assignment
        record(23); // 23. assignment -> Ident LHS := Expression
        parseAssignment();
    } else if (t.type == CALL) {
        record(18); // 18. statement -> callStmt
        record(26); // 26. callStmt -> CALL Ident argList
        parseCallStmt();
    } else if (t.type == BEGIN) {
        record(19); // 19. statement -> compoundStmt
        record(31); // 31. compoundStmt -> BEGIN stmtList END
        parseCompoundStmt();
    } else if (t.type == IF) {
        record(20); // 20. statement -> ifStmt
        record(35); // 35. ifStmt -> IF Condition THEN statement elsePart
        parseIfStmt();
    } else if (t.type == WHILE) {
        record(21); // 21. statement -> whileStmt
        record(38); // 38. whileStmt -> WHILE Condition DO statement
        parseWhileStmt();
    } else if(t.type == FOR) {
        record(22); // 22. statement -> forStmt
        record(39); // 39. forStmt -> FOR Ident := Expression TO Expression DO statement
        parseForStmt();
    } else {
        error("Invalid statement");
    }
}

void parseIfStmt() {
    Token ifToken = peek();
    if (!match(IF)) error("Expected 'if'");
    parseCondition();
    if (!match(THEN)) error("Expected 'then'");
    parseStatement();
    parseElsePart();
    
    // Thêm kiểm tra ngữ nghĩa
    checkStatementSemantics(IF, NULL, ifToken.line);
}

void parseElsePart() {
    if (match(ELSE)) {
        record(36); // 36. elsePart -> ELSE statement 
        parseStatement();
    } else {
        record(37); // 37. elsePart -> e 
    }
}

void parseForStmt() {
    Token varToken = peek();
    if (!match(FOR)) error("Expected 'for'");
    if (!match(IDENT)) error("Expected identifier in for loop");
    if (!match(ASSIGN)) error("Expected ':=' in for loop");
    parseExpression();
    if (!match(TO)) error("Expected 'to' in for loop");
    parseExpression();
    if (!match(DO)) error("Expected 'do' in for loop");
    parseStatement();
    
    // Thêm kiểm tra ngữ nghĩa
    checkStatementSemantics(FOR, varToken.lexeme, varToken.line);
}

void parseConstDecl() {
    if (match(CONST)) {
        record(3);  // 3. ConstDecl ? CONST ConstAssignList ; | e
        do {
            record(4);  // 4. ConstAssignList ? ConstAssign ConstAssignList' 
            if (!match(IDENT)) error("Expected identifier in const declaration");
            if (!match(EQU)) error("Expected '='");
            if (!match(NUMBER)) error("Expected number in const declaration");
        } while (match(COMMA));
        if (!match(SEMICOLON)) error("Expected ';' after const declaration");
    }
    else{
        record(3);  // 3. ConstDecl ? e
    }
}

void parseConstAssign() {
    record(6); // 6. ConstAssign ? Ident = Number 
    if (!match(IDENT)) error("Expected identifier in const assignment");
    if (!match(EQU)) error("Expected '=' in const assignment");
    if (!match(NUMBER)) error("Expected number in const assignment");
}

void parseConstAssignListPrime() {
    if (match(COMMA)) {
        record(5); // 5. ConstAssignList' ? , ConstAssign ConstAssignList' 
        parseConstAssign();
        parseConstAssignListPrime();
    } else {
        record(5); // 5. ConstAssignList' ? e 
    }
}

void parseVarDecl() {
    if (match(VAR)) {
        record(7);  // 7. VarDecl ? VAR VarDeclList ; | e
        do {
            record(8);  // 8. VarDeclList ? VarItem VarDeclList'
            if (!match(IDENT)) error("Expected identifier in var declaration");
        } while (match(COMMA));
        if (!match(SEMICOLON)) error("Expected ';' after var declaration");
    } else{
        record(7);  // 7. VarDecl ? e
    }
}

void parseVarItem() {
    record(10); // 10. VarItem ? Ident | Ident [ Number ] 
    if (!match(IDENT)) error("Expected identifier in var item");
    if (match(LBRACK)) {
        if (!match(NUMBER)) error("Expected number in array declaration");
        if (!match(RBRACK)) error("Expected ']' after number in array declaration");
    }
}

void parseVarDeclListPrime() {
    if (match(COMMA)) {
        record(9); // 9. VarDeclList' ? , VarItem VarDeclList' 
        parseVarItem();
        parseVarDeclListPrime();
    } else {
        record(9); // 9. VarDeclList' ? e 
    }
}

void parseProcDecls() {
    if (peek().type == PROCEDURE) {
        record(11); // 11. ProcDecls ? ProcDecl ProcDecls | e 
        parseProcDecl();
        parseProcDecls();
    } else {
        record(11); // 11. ProcDecls ? e 
    }
}

void parseProcParamsOpt() {
    if (match(LPARENT)) {
        record(13); // 13. ProcParamsOpt ? ( ParamDeclList ) | e 
        parseParamDeclList();
        if (!match(RPARENT)) error("Expected ')' after parameter list");
    } else {
        record(13); // 13. ProcParamsOpt ? e 
    }
}

void parseParamDeclList() {
    record(14); // 14. ParamDeclList ? ParamDecl ParamDeclList' 
    parseParamDecl();
    parseParamDeclListPrime();
}

void parseParamDeclListPrime() {
    if (match(SEMICOLON)) {
        record(15); // 15. ParamDeclList' ? ; ParamDecl ParamDeclList' 
        parseParamDecl();
        parseParamDeclListPrime();
    } else {
        record(15); // 15. ParamDeclList' ? e 
    }
}

void parseParamDecl() {
    record(16); // 16. ParamDecl ? VAR Ident | Ident
    if (match(VAR)) {
        if (!match(IDENT)) error("Expected identifier after 'var'");
    } else {
        if (!match(IDENT)) error("Expected identifier in parameter");
    }
}

void parseStatementList() {
    record(32); // 32. stmtList -> statement stmtListTail 
    if (peek().type != END) {
        parseStatement();
    }
    parseStatementListPrime();
}

void parseStatementListPrime() {
    if (match(SEMICOLON)) {
        record(33); // 33. stmtListTail -> ; statement stmtListTail   
        if (peek().type != END) {
            parseStatement();
            parseStatementListPrime();
        }
    } else {
        record(34); // 34. stmtListTail -> e  
    }
}

void parseProcDecl() {
    while (match(PROCEDURE)) {
        record(12);  // 12. ProcDecl ? PROCEDURE Ident ProcParamsOpt ; Block ;
        if (!match(IDENT)) error("Expected identifier after 'procedure'");
        if (!match(SEMICOLON)) error("Expected ';' after procedure header");
        parseBlock();
        if (!match(SEMICOLON)) error("Expected ';' after procedure block");
    }
}

void parseBlock() {
    record(2);  // 2. Block ? ConstDecl VarDecl ProcDecls BEGIN StatementList END
    parseConstDecl();
    parseVarDecl();
    parseProcDecl();
    parseCompoundStmt();
}

void parseProgram() {
    // Khởi tạo phân tích ngữ nghĩa
    initSemantic();
    
    record(1);  // 1. program -> PROGRAM Ident ; Block .
    if (!match(PROGRAM)) error("Expected 'program'");
    if (!match(IDENT)) error("Expected identifier after 'program'");
    if (!match(SEMICOLON)) error("Expected ';' after identifier");
    parseBlock();
    if (!match(PERIOD)) error("Expected '.' at the end of program");
    
    // In kết quả phân tích
    printf("Phan tich cu phap thanh cong va da luu vao file cuphap.txt\n\n");
    FILE* out = fopen("cuphap.txt", "w");
    fprintf(out, "%s\n", production_trace);
    fclose(out);
    
    // In kết quả phân tích ngữ nghĩa
    printSemanticErrors();
}

int main() {
    loadTokens("token.txt");
    parseProgram();
    return 0;
}