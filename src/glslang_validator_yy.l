%{
#define YY_NO_UNISTD_H
#define ECHO
#include "glslang_validator_yy.h"
int yywrap(void);
%}

%%
"glslangValidator: Error unable to open input file" return glslang_validator_yy_ERROR_FILE_NOT_FOUND;
"ERROR: temp.glsl:" return glslang_validator_yy_ERROR;
"Uniform reflection:" return glslang_validator_yy_UR_MK;
"Uniform block reflection:" return glslang_validator_yy_UBR_MK;
"Vertex attribute reflection:" return glslang_validator_yy_VAR_MK;
[_a-zA-Z][_a-zA-Z0-9.]* return glslang_validator_yy_IDENTIFIER;
[-0-9][-a-zA-Z0-9]* return glslang_validator_yy_VALUE;
":" return glslang_validator_yy_COLON;
. ;
%%

int yywrap(void)
{
	return 1;
}
