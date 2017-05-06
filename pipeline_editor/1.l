%{
#define YY_NO_UNISTD_H
#include "def.h"
int yywrap(void);
%}

%%
"ERROR:" return ERROR_MK;
"Uniform reflection:" return UNIFORM_REFLECTION_MK;
. ;
%%

int yywrap(void)
{
	return 1;
}
