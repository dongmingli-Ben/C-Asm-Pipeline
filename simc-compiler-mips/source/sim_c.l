%{
#include "utils.h"
#include "sim_c.parser.h"
#include <stdio.h>
#include <string.h>
%}

%option noyywrap
%%
[ \t\r\n]+              {}
"int"                   { return INT; }
"main"                  { return MAIN; }
"void"                  { return VOID; }
"break"                 { return BREAK; }
"do"                    { return DO; }
"else"                  { return ELSE; }
"if"                    { return IF; }
"while"                 { return WHILE; }
"return"                { return RETURN; }
"scanf"                 { return READ; }
"printf"                { return WRITE; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"["                     { return LSQUARE; }
"]"                     { return RSQUARE; }
"("                     { return LPAR; }
")"                     { return RPAR; }
";"                     { return SEMI; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MUL_OP; }
"/"                     { return DIV_OP; }
"&"                     { return AND_OP; }
"|"                     { return OR_OP; }
"!"                     { return NOT_OP; }
"="                     { return ASSIGN; }
"<"                     { return LT; }
">"                     { return GT; }
"<<"                    { return SHL_OP; }
">>"                    { return SHR_OP; }
"=="                    { return EQ; }
"!="                    { return NOTEQ; }
"<="                    { return LTEQ; }
">="                    { return GTEQ; }
"&&"                    { return ANDAND; }
"||"                    { return OROR; }
","                     { return COMMA; }
[a-zA-Z]+[0-9a-zA-Z_]*  {
                            strcpy(yylval.name, yytext);
                            return ID;
                        }
[0-9]+                  { 
                            yylval.value = atoi(yytext);
                            return INT_NUM;
                        }
.                       { return LEX_ERR; }

%%
