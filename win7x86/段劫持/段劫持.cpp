// �νٳ�.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>

typedef int (__cdecl *DbgPrintX)(_In_z_ _Printf_format_string_ const char * _Format, ...);
DbgPrintX DbgPrint;
char* daitouyu=NULL;

void _declspec(naked) test(){
	__asm{
		sub esp,8;//����������ջ�ռ�
		lea eax,haha;
		mov [esp],eax;//��һ����ջ�ռ�ŵ�ַ
		mov [esp+4],0x8;//�ڶ�����ջ�ռ�Ŷ�ѡ����,0x8��0���Ķ�ѡ����
		jmp fword ptr [esp];//��ת��haha
haha:
		add esp,8;//�������ջƽ��
		push fs;
		push 0x30;//����R0֮��,fs��Ҫ��Ϊ0x30
		pop fs;
		mov eax,[daitouyu];
		push eax;
		call DbgPrint;
		add esp,4;
		pop fs;
		mov eax,0x83e5e800; //int3�ĵ�ַ
		jmp eax;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	DbgPrint=(DbgPrintX)0x83e1b271; //dbgprint�ĵ�ַ
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

