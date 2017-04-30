%{
#include "def.h"
int yywrap(void);
%}

%%
"REFLECTABLE" return REFLECTABLE;
"REFLv" return REFLv;
"REFLe" return REFLe;
"struct" return STRUCT;
"enum" return ENUM;
[_a-zA-Z][_a-zA-Z0-9:]* return IDENTIFIER;
. ;
%%

int yywrap(void)
{
	return 1;
}
