%{
#include "def.h"
int yywrap(void);
%}

%%
"ERROR:" return ERROR_MK;
. ;
%%

int yywrap(void)
{
	return 1;
}
