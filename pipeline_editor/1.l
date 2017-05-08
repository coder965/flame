%{
#define YY_NO_UNISTD_H
#include "def.h"
int yywrap(void);
%}

%%
"Uniform reflection:" return UR_MK;
"Uniform block reflection:" return UBR_MK;
"Vertex attribute reflection:" return VAR_MK;
[_a-zA-Z][_a-zA-Z0-9]* return IDENTIFIER;
[-0-9][-a-zA-Z0-9]* return VALUE;
":" return COLON;
. ;
%%

int yywrap(void)
{
	return 1;
}
