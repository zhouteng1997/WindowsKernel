// 中断门.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>


void _declspec(naked) test(){
	__asm{
		int 3;
		iretd;
	}
}

void _declspec(naked) test1(){
	__asm{
		int 3;
		iretd;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("function addr =%x \r\n",test1);
	system("pause");
	__asm{
		int 32;
		push 0x3b;
		pop fs;
	}
	system("pause");
	return 0;
}



