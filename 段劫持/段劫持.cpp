// 段劫持.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>

typedef int (__cdecl *DbgPrintX)(_In_z_ _Printf_format_string_ const char * _Format, ...);
DbgPrintX DbgPrint;
char* daitouyu=NULL;

void _declspec(naked) test(){
	__asm{
		sub esp,8;//申请两个堆栈空间
		lea eax,haha;
		mov [esp],eax;//第一个堆栈空间放地址
		mov [esp+4],0x8;//第二个堆栈空间放段选择器,0x8是0环的段选择器
		jmp fword ptr [esp];//跳转到haha
haha:
		add esp,8;//与上面堆栈平衡
		push fs;
		push 0x30;//进入R0之后,fs需要改为0x30
		pop fs;
		mov eax,[daitouyu];
		push eax;
		call DbgPrint;
		add esp,4;
		pop fs;
		mov eax,0x83e5e800; //int3的地址
		jmp eax;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	DbgPrint=(DbgPrintX)0x83e1b271; //dbgprint的地址
	printf("function addr =%x \r\n",test);
	daitouyu=(char*)malloc(30);
	memset(daitouyu,0,30);
	memcpy(daitouyu,"daitouyu\r\n",strlen("daitouyu\r\n"));
	system("pause");
	__asm{
		int 3;
	}
	system("pause");
	return 0;
}

