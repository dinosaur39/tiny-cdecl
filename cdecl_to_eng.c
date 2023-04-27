#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXTOKENS 100
#define MAXTOKENLEN 64
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum type_tag { IDENTIFIER, QUALIFIER, TYPE };
struct token {
    char type;
    char string[MAXTOKENLEN];
};

struct token stack[MAXTOKENS];
struct token token_buf;
int top = -1;
void push(struct token token)
{
    stack[++top] = token;
}
void pop(void)
{
    token_buf = stack[top--];
}
int is_empty(void)
{
    return (top == -1);
}

struct token keywords[] = {
    { QUALIFIER, "const" },
    { QUALIFIER, "volatile" },
    { TYPE, "void" },
    { TYPE, "char" },
    { TYPE, "signed" },
    { TYPE, "unsigned" },
    { TYPE, "short" },
    { TYPE, "int" },
    { TYPE, "long" },
    { TYPE, "float" },
    { TYPE, "double" },
    { TYPE, "struct" },
    { TYPE, "union" },
    { TYPE, "enum" },
};

enum type_tag classify_string(char *str);
int isalnum_und(char c);
void get_token(void);
void read_to_first_identifier(void);

void deal_with_function_args(void);
void deal_with_arrays(void);
void deal_with_pointers(void);
void deal_with_declarator(void);

enum type_tag classify_string(char *str)
{
    int length = ARRAY_SIZE(keywords), i;
    for (i = 0; i < length; i++) {
        if (!strcmp(str, keywords[i].string))
            return keywords[i].type;
    }
    return IDENTIFIER;
}

int isalnum_und(char c)
{
    return isalnum(c) || (c == '_');
}

void get_token(void)
{
    char *p = token_buf.string;
    while ((*p = getchar()) == ' ');
    
    if (isalnum_und(*p)) {
        while (isalnum_und(*++p = getchar()));
        ungetc(*p, stdin);
        *p = '\0';
        token_buf.type = classify_string(token_buf.string);
        return;
    }
    if (*p == '*') {
        strcpy(token_buf.string, "pointer to");
        token_buf.type = '*';
        return;
    }
    token_buf.string[1] = '\0';
    token_buf.type = *p;
    return;
}

void read_to_first_identifier(void)
{
    get_token();
    while (token_buf.type != IDENTIFIER) {
        push(token_buf);
        get_token();
    }
    printf("%s is ", token_buf.string);
    get_token();
}

void deal_with_declarator(void)
{
    switch (token_buf.type) {
        case '[':
            deal_with_arrays();
            break;
        case '(':
            deal_with_function_args();
            break;
    }
    deal_with_pointers();
    while (!is_empty()) {
        pop();
        if (token_buf.type == '(') {
            //the last token get is the right paren matching this left paren
            get_token();
            deal_with_declarator();
        } else {
            printf("%s ", token_buf.string);
        }
    }
}

void deal_with_arrays(void) 
{
    while (token_buf.type == '[') {
        printf("array ");
        get_token();
        if (isdigit(token_buf.string[0])) {
            printf("0..%d ", atoi(token_buf.string));
            get_token();//read '['
        }
        printf("of ");
        get_token(); 
    }
}

void deal_with_function_args() {
    while (token_buf.type != ')') {
        get_token();
    }
    get_token();
    printf("function returning ");
}

void deal_with_pointers() {
    while (!is_empty() && stack[top].type == '*') {
        pop();
        printf("%s ", token_buf.string);
    }
}

int main()
{
    read_to_first_identifier();
    deal_with_declarator();
    printf("\n");
    return 0;
}




