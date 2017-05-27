%{
#define YY_NO_UNISTD_H
#include "stage_editor_yy_def.h"
int yywrap(void);
%}

%%
"ERROR: temp.glsl:" return STAGE_EDITOR_YY_ERROR;
"Uniform reflection:" return STAGE_EDITOR_YY_UR_MK;
"Uniform block reflection:" return STAGE_EDITOR_YY_UBR_MK;
"Vertex attribute reflection:" return STAGE_EDITOR_YY_VAR_MK;
[_a-zA-Z][_a-zA-Z0-9.]* return STAGE_EDITOR_YY_IDENTIFIER;
[-0-9][-a-zA-Z0-9]* return STAGE_EDITOR_YY_VALUE;
":" return STAGE_EDITOR_YY_COLON;
. ;
%%

int yywrap(void)
{
	return 1;
}
