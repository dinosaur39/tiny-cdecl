#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

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
#define stack_top stack[top]

void push(struct token token)
{
    stack[++top] = token;
}
void pop(void)
{
    top--;
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

void initialize(void);
void parse_next_token(void);
void parse_arr_declaration(void);
void parse_fun_params_start(void);
void parse_ptr_and_type(void);
void parse_ptr_end(void);
void parse_type_end(void);
#define end_state NULL

void print_stack(void);

void (*next_state)(void) = initialize;

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

void initialize(void)
{
    get_token();
    while (token_buf.type != IDENTIFIER) {
        push(token_buf);
        get_token();
    }
    printf("%s is ", token_buf.string);
    get_token();
    next_state = parse_next_token;
}

void parse_next_token(void)
{
    switch (token_buf.type) {
        case '[':
            next_state = parse_arr_declaration;
            break;
        case '(':
            next_state = parse_fun_params_start;
            break;
        default:
            next_state = parse_ptr_and_type;
            break;
    }
}

void parse_arr_declaration(void)
{
    while (token_buf.type == '[') {
        printf("array ");
        get_token();
        if (token_buf.type != ']') {
            printf("0..%s ", token_buf.string);
            get_token();
            assert(token_buf.type == ']');
        }
        printf("of ");
        get_token();
    }
    next_state = parse_ptr_and_type;
}

void parse_fun_params_start(void)
{
    printf("function(param ");
    push(token_buf);
    next_state = initialize;
}

void parse_ptr_and_type(void)
{
    next_state = parse_ptr_end;
    while (!is_empty() && stack_top.type != '(') {
        if (stack_top.type == TYPE)
            next_state = parse_type_end;
        printf("%s ", stack_top.string);
        pop();
    }
    if (is_empty())
        next_state = end_state;

}

void parse_ptr_end(void)
{
    pop();
    get_token();
    next_state = parse_next_token;
}

void parse_type_end(void)
{
    if (token_buf.type == ',') {
        printf("\b, param ");
        next_state = initialize;
    } else if (token_buf.type == ')') {
        pop();
        printf("\b) returning ");
        get_token();
        next_state = parse_next_token;
    } else {
        next_state = end_state;
    }
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
    } else if (*p == '*') {
        strcpy(token_buf.string, "pointer to");
        token_buf.type = '*';
    } else {
        token_buf.string[1] = '\0';
        token_buf.type = *p;
    }
}

void print_stack(void)
{
    int i;
    for (i = 0; i < top; i++) {
        printf("type:%c, string:%s \n", stack[i].type, stack[i].string);
    }
}

int main()
{
    while(next_state != end_state) {
        next_state();
    }
    printf("\n");
    return 0;
}




