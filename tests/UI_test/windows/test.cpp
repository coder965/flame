extern "C" {
	_declspec(dllexport) const char *say_hello()
	{
		return "Hello, My Friend !\n";
	}
}

/*

cl test.cpp -LD -MD "D:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\V
C\Tools\MSVC\14.14.26428\lib\x64\msvcrt.lib" "D:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MS
VC\14.14.26428\lib\x64\oldnames.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17134.0\um\x64\kernel32.lib" "D:\P
rogram Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.14.26428\lib\x64\vcruntime.lib" "C:\Program
Files (x86)\Windows Kits\10\Lib\10.0.17134.0\ucrt\x64\ucrt.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17134.0
\um\x64\uuid.lib"

*/