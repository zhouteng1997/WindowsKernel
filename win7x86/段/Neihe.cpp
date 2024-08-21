// Neihe.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>


typedef int (__cdecl *DbgPrintX)(_In_z_ _Printf_format_string_ const char * _Format, ...);
DbgPrintX DbgPrint;
char* strxx="aaaaaaaaaaaaaaaaaaaaaaaaaa";
void _declspec(naked) test(){
	__asm{
		push fs;
		push 0x30;//进入R0之后,fs需要改为0x30
		pop fs;
		mov eax,[strxx];
		push eax;
		call DbgPrint;
		add esp,4;
		pop fs;
		retf;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	char bufcode[] ={0,0,0,0,0x48,0};
	DbgPrint=(DbgPrintX)0x7750e95c;
	printf("function addr =%x \r\n",test);
	system("pause");
	__asm{
		call fword ptr bufcode;
		haha:
	}
	system("pause");
	return 0;
}

