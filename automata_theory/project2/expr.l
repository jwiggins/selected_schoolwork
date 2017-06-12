%%
[0-9]+  {yylval = atoi(yytext); return(NUMBER);}
invert	{return(INVERT);}
square	{return(SQUARE);}
[-+/*()] {return(yytext[0]); }
\n       {return(-1);}
