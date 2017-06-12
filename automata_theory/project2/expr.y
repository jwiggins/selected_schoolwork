
%{
#include <string.h>
int yydebug = 1;
%}
%token NUMBER
%token INVERT
%token SQUARE
%left '+' '-'
%left '*' '/'
%%
Line    :       expr    {printf("%d\n",$1);}
		|		error '\n' {yyclearin; yyerrok;}
        ;

expr    :       NUMBER
        |     expr '+' expr  {$$=$1+$3;}
        |     expr '-' expr  {$$=$1-$3;}
        |     expr '*' expr  {$$=$1*$3;}
        |     expr '/' expr  {$$=$1/$3;}
        |     '(' expr ')'   {$$=$2;}
        |     '-' expr %prec '*' {$$=-$2;}
        |     invert '(' expr ')' {$$=-$3;}
        |     square '(' expr ')' {$$=$3*$3;}  
      ;

invert	:		INVERT
		;

square	:		SQUARE
		;
%%
#include "lex.yy.c"
main()
{
printf("? ");
while(yyparse()) ;
}
yyerror() {
printf("syntax error. try again.\n? ");
}
yywrap()
{
}

