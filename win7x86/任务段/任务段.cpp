// 任务段.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
//0x2024 bytes (sizeof)
struct _KiIoAccessMap
{
    UCHAR DirectionMap[32];                                                 //0x0
    UCHAR IoMap[8196];                                                      //0x20
}; 
//0x20ac bytes (sizeof)
typedef struct _KTSS
{
    USHORT Backlink;                                                        //0x0
    USHORT Reserved0;                                                       //0x2
    ULONG Esp0;                                                             //0x4
    USHORT Ss0;                                                             //0x8
    USHORT Reserved1;                                                       //0xa
    ULONG NotUsed1[4];                                                      //0xc
    ULONG CR3;                                                              //0x1c
    ULONG Eip;                                                              //0x20
    ULONG EFlags;                                                           //0x24
    ULONG Eax;                                                              //0x28
    ULONG Ecx;                                                              //0x2c
    ULONG Edx;                                                              //0x30
    ULONG Ebx;                                                              //0x34
    ULONG Esp;                                                              //0x38
    ULONG Ebp;                                                              //0x3c
    ULONG Esi;                                                              //0x40
    ULONG Edi;                                                              //0x44
    USHORT Es;                                                              //0x48
    USHORT Reserved2;                                                       //0x4a
    USHORT Cs;                                                              //0x4c
    USHORT Reserved3;                                                       //0x4e
    USHORT Ss;                                                              //0x50
    USHORT Reserved4;                                                       //0x52
    USHORT Ds;                                                              //0x54
    USHORT Reserved5;                                                       //0x56
    USHORT Fs;                                                              //0x58
    USHORT Reserved6;                                                       //0x5a
    USHORT Gs;                                                              //0x5c
    USHORT Reserved7;                                                       //0x5e
    USHORT LDT;                                                             //0x60
    USHORT Reserved8;                                                       //0x62
    USHORT Flags;                                                           //0x64
    USHORT IoMapBase;                                                       //0x66
    struct _KiIoAccessMap IoMaps[1];                                        //0x68
    UCHAR IntDirectionMap[32];                                              //0x208c
}KTSS; 

void _declspec(naked) test(){
	__asm{
		int 3;
		pushfd;
		pop eax;
		or eax,0x4000;
		push eax;
		popfd;
		iretd;
	}
}

KTSS tss1={0};
char bufEsp0[0x2000]={0};
char bufEsp3[0x2000]={0};

int _tmain(int argc, _TCHAR* argv[])
{
	short trss=0;
	__asm{
		str trss;//获取当前任务段的段选择子 r tr
	}

	memset(bufEsp0,0,0x2000);
	memset(bufEsp3,0,0x2000);
	tss1.Esp0=(ULONG)bufEsp0+0x1FF0;
	tss1.Esp=(ULONG)bufEsp3+0x1FF0;
	tss1.Ss0=0x10;
	tss1.Ss=0x10;
	tss1.Cs=0x8;
	tss1.Ds=0x23;
	tss1.Es=0x23;
	tss1.Fs=0x30;
	tss1.EFlags=2;
	tss1.Eip=(ULONG)test;
	tss1.IoMapBase=0x20ac;

	printf("请输入你的CR3:");
	DWORD dwCr3=0;
	scanf("%x",&dwCr3);
	tss1.CR3=dwCr3;
	printf("%x \r\n",&tss1);
	system("pause");

	char bufcode[]={0,0,0,0,0x48,0};

	__asm{
		call fword ptr bufcode;
	}
	system("pause");
	return 0;
}

