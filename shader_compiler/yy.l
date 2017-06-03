%{
#define YY_NO_UNISTD_H
#include "yy_def.h"
int yywrap(void);
%}

%%
"ERROR: temp.glsl:" return YY_ERROR;
"Uniform reflection:" return YY_UR_MK;
"Uniform block reflection:" return YY_UBR_MK;
"Vertex attribute reflection:" return YY_VAR_MK;
[_a-zA-Z][_a-zA-Z0-9.]* return YY_IDENTIFIER;
[-0-9][-a-zA-Z0-9]* return YY_VALUE;
":" return YY_COLON;
. ;
%%

int yywrap(void)
{
	return 1;
}
