%{
#define YY_NO_UNISTD_H
#include "pipeline_editor_yy_def.h"
int yywrap(void);
%}

%%
"Uniform reflection:" return PIPELINE_EDITOR_YY_UR_MK;
"Uniform block reflection:" return PIPELINE_EDITOR_YY_UBR_MK;
"Vertex attribute reflection:" return PIPELINE_EDITOR_YY_VAR_MK;
[_a-zA-Z][_a-zA-Z0-9.]* return PIPELINE_EDITOR_YY_IDENTIFIER;
[-0-9][-a-zA-Z0-9]* return PIPELINE_EDITOR_YY_VALUE;
":" return PIPELINE_EDITOR_YY_COLON;
. ;
%%

int yywrap(void)
{
	return 1;
}
