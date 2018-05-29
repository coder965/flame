$f = read-host filename
#$n = read-host namespace
$n = "flame"
"#pragma once`r`n`r`nnamespace $n`r`n{`r`n}`r`n" | out-file "${f}.h"
"#include `"${f}.h`"`r`n`r`nnamespace $n`r`n{`r`n}`r`n" | out-file "${f}_private.cpp"
"#include `"${f}_private.h`"`r`n`r`nnamespace $n`r`n{`r`n}`r`n" | out-file "${f}.cpp"
