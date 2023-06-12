%{
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utils.h"

void yyerror(const char *);
int yylex();
extern FILE *yyin;
extern char *yytext;
%}
/* %define parse.trace for debug */
/* %define parse.trace */

%union {
    int value;
    char name[33];
    /* 
     * represents a block of instructions that functions and put 
     * the output value to $t0 register (if there is a output)
     */
    struct temp_value_struct temp_value_struct; 
    struct declaration_struct declaration_struct; 
}
/* terminal tokens */
%token <value> INT_NUM
%token <name> ID
%token INT
%token MAIN
%token VOID
%token BREAK
%token DO
%token ELSE
%token IF
%token WHILE
%token RETURN
%token READ
%token WRITE
%token LBRACE
%token RBRACE
%token LSQUARE
%token RSQUARE
%token LPAR
%token RPAR
%token SEMI
%token PLUS
%token MINUS
%token MUL_OP
%token DIV_OP
%token AND_OP
%token OR_OP
%token NOT_OP
%token ASSIGN
%token LT
%token GT
%token SHL_OP
%token SHR_OP
%token EQ
%token NOTEQ
%token LTEQ
%token GTEQ
%token ANDAND
%token OROR
%token COMMA
/* scanner error */
%token LEX_ERR

%type <temp_value_struct> program
%type <temp_value_struct> assign_statement
%type <temp_value_struct> code_block
%type <temp_value_struct> control_statement
%type <declaration_struct> declaration
%type <declaration_struct> declaration_list
%type <temp_value_struct> do_while_statement
%type <temp_value_struct> exp
%type <temp_value_struct> exp_level_1
%type <temp_value_struct> exp_level_2
%type <temp_value_struct> exp_level_3
%type <temp_value_struct> exp_level_4
%type <temp_value_struct> exp_level_5
%type <temp_value_struct> exp_level_6
%type <temp_value_struct> exp_level_7
%type <temp_value_struct> exp_level_8
%type <temp_value_struct> exp_level_9
%type <temp_value_struct> exp_level_10
%type <temp_value_struct> exp_level_11
%type <temp_value_struct> exp_level_12
%type <temp_value_struct> if_statement
%type <temp_value_struct> if_stmt
%type <temp_value_struct> read_statement
%type <temp_value_struct> read_write_statement
%type <temp_value_struct> return_statement
%type <temp_value_struct> statement
%type <temp_value_struct> statements
%type <declaration_struct> var_declaration
%type <temp_value_struct> var_declaration_section  /* contains codes for variable initialization */
%type <declaration_struct> var_declarations
%type <temp_value_struct> while_statement
%type <temp_value_struct> write_statement

%nonassoc IFX
%nonassoc ELSE

%start program
%%
program : var_declaration_section statements { 
        append_codes(&($$), $1, $2);
        register_instructions_to_codes($$); 
    } ;

var_declaration_section 
    : var_declarations { 
        register_symbol_table($1);  /* also update symbol offsets */
        initialize_symbols(&($$), $1); /* generate codes to initialize the variables */
    }
    | /* empty */ { 
        initialize_empty_symbol_table();
        initialize_empty_codes(&($$)); 
    }
    ;

var_declarations 
    : var_declarations var_declaration { append_symbols(&($$), $1, $2); }
    | var_declaration { transfer_symbols(&($$), $1); }
    ;

/* because all variables are int, a simple transfer is enough */
var_declaration : INT declaration_list SEMI { transfer_symbols(&($$), $2); } ;

declaration_list 
    : declaration_list COMMA declaration { append_symbols(&($$), $1, $3); }
    | declaration { transfer_symbols(&($$), $1); }
    ;

declaration 
    : ID ASSIGN INT_NUM { initialized_declaration(&($$), $1, $3); }
    | ID ASSIGN MINUS INT_NUM { initialized_declaration(&($$), $1, -$4); }
    | ID LSQUARE INT_NUM RSQUARE { array_declaration(&($$), $1, $3); }
    | ID { uninitialized_declaration(&($$), $1); }
    ;

code_block 
    : statement { transfer_codes(&($$), $1); }
    | LBRACE statements RBRACE { transfer_codes(&($$), $2); }
    ;

statements 
    : statements statement { append_codes(&($$), $1, $2); }
    | statement { transfer_codes(&($$), $1); }
    ;

statement 
    : assign_statement SEMI { transfer_codes(&($$), $1); }
    | control_statement { transfer_codes(&($$), $1); }
    | read_write_statement SEMI { transfer_codes(&($$), $1); }
    | SEMI { initialize_empty_codes(&($$)); }
    ;

control_statement 
    : if_statement { transfer_codes(&($$), $1); }
    | while_statement { transfer_codes(&($$), $1); }
    | do_while_statement SEMI { transfer_codes(&($$), $1); }
    | return_statement SEMI { transfer_codes(&($$), $1); }
    ;

read_write_statement 
    : read_statement { transfer_codes(&($$), $1); }
    | write_statement { transfer_codes(&($$), $1); }
    ;

assign_statement 
    : ID LSQUARE exp RSQUARE ASSIGN exp { array_assignment(&($$), $1, $3, $6); }
    | ID ASSIGN exp { id_assignment(&($$), $1, $3); }
    ;

if_statement 
    : if_stmt %prec IFX { transfer_codes(&($$), $1); }
    | if_stmt ELSE code_block { if_else_statement(&($$), $1, $3); }
    ;

if_stmt : IF LPAR exp RPAR code_block { if_statement(&($$), $3, $5); } ;

while_statement : WHILE LPAR exp RPAR code_block { while_statement(&($$), $3, $5); } ;

do_while_statement : DO code_block WHILE LPAR exp RPAR { do_while_statement(&($$), $5, $2); } ;

return_statement : RETURN { return_statement(&($$)); } ;

read_statement : READ LPAR ID RPAR { read_statement(&($$), $3); } ;

write_statement : WRITE LPAR exp RPAR { write_statement(&($$), $3); } ;

exp : exp_level_12 { transfer_codes(&($$), $1); } ;

exp_level_12 
    : exp_level_12 OROR exp_level_11 { logical_or_exp(&($$), $1, $3); }
    | exp_level_11 { transfer_codes(&($$), $1); }
    ;

exp_level_11 
    : exp_level_11 ANDAND exp_level_10 { logical_and_exp(&($$), $1, $3); }
    | exp_level_10 { transfer_codes(&($$), $1); }
    ;

exp_level_10 
    : exp_level_10 OR_OP exp_level_9 { bitwise_or_exp(&($$), $1, $3); }
    | exp_level_9 { transfer_codes(&($$), $1); }
    ;

exp_level_9 : exp_level_8 { transfer_codes(&($$), $1); } ;

exp_level_8 
    : exp_level_8 AND_OP exp_level_7 { bitwise_and_exp(&($$), $1, $3); }
    | exp_level_7 { transfer_codes(&($$), $1); }
    ;

exp_level_7 
    : exp_level_7 EQ exp_level_6 { is_equal_exp(&($$), $1, $3); }
    | exp_level_7 NOTEQ exp_level_6 { not_equal_exp(&($$), $1, $3); }
    | exp_level_6 { transfer_codes(&($$), $1); }
    ;

exp_level_6 
    : exp_level_6 LT exp_level_5 { less_than_exp(&($$), $1, $3); }
    | exp_level_6 LTEQ exp_level_5 { less_or_equal_exp(&($$), $1, $3); }
    | exp_level_6 GT exp_level_5 { greater_than_exp(&($$), $1, $3); }
    | exp_level_6 GTEQ exp_level_5 { greater_or_equal_exp(&($$), $1, $3); }
    | exp_level_5 { transfer_codes(&($$), $1); }
    ;

exp_level_5 
    : exp_level_5 SHL_OP exp_level_4 { shift_left_logical_exp(&($$), $1, $3); }
    | exp_level_5 SHR_OP exp_level_4 { shift_right_arithmetic_exp(&($$), $1, $3); }
    | exp_level_4 { transfer_codes(&($$), $1); }
    ;

exp_level_4 
    : exp_level_4 PLUS exp_level_3 { plus_exp(&($$), $1, $3); }
    | exp_level_4 MINUS exp_level_3 { minus_exp(&($$), $1, $3); }
    | exp_level_3 { transfer_codes(&($$), $1); }
    ;

exp_level_3 
    : exp_level_3 MUL_OP exp_level_2 { multiply_exp(&($$), $1, $3); }
    | exp_level_3 DIV_OP exp_level_2 { divide_exp(&($$), $1, $3); }
    | exp_level_2 { transfer_codes(&($$), $1); }
    ;

exp_level_2 
    : MINUS exp_level_2 { negate_exp(&($$), $2); }
    | NOT_OP exp_level_2 { logical_not_exp(&($$), $2); }
    | exp_level_1 { transfer_codes(&($$), $1); }
    ;

exp_level_1 
    : INT_NUM { register_int_to_exp(&($$), $1); }
    | ID { register_id_to_exp(&($$), $1); }
    | ID LSQUARE exp RSQUARE { register_array_element_to_exp(&($$), $1, $3); }
    | LPAR exp RPAR { transfer_codes(&($$), $2); }
    ;

/* goal : program SCANEOF ; */
%%
void yyerror(const char *msg) {
    if (yychar == LEX_ERR)
        fprintf(stderr, "Lexical error: unkown token '%s'\n", yytext);
    else fprintf(stderr, "%s\n", msg);
}
int main(int argc, char **argv) {
/* #ifdef YYDEBUG
    yydebug = 1;
#endif */
    if (argc < 2) {
        fprintf(stderr, "Missing input file!\n");
    }
    yyin = fopen(argv[1], "r");
    if (yyparse()) {
        fprintf(stderr, "Unable to parse the input!\n");
        return 1;
    }
    if (argc >= 3) {
        char output[50];
        strcpy(output, argv[2]);
        FILE *out = fopen(output, "w");
        generate_mips_to_file(out);
        fclose(out);
    } else {
        generate_mips_to_file(stdout);
    }

    fclose(yyin);
    return 0;
}
