#pragma once
#include <windows.h>
// ��ɫ������ʾһ���ַ���
static void ColourPrintf(const char* str)
{
	// 0-�� 1-�� 2-�� 3-ǳ�� 4-�� 5-�� 6-�� 7-�� 8-�� 9-���� 10-����
	// 11-��ǳ�� 12-���� 13-���� 14-���� 15-����
	//��ɫ��ǰ��ɫ + ����ɫ*0x10
	//���磺���Ǻ�ɫ������ɫ�ǰ�ɫ���� ��ɫ + ���� = 4 + 15*0x10
	WORD color = 4 + 15 * 0x10;
	WORD colorOld;
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	colorOld = csbi.wAttributes;
	SetConsoleTextAttribute(handle, color);
	printf("%s", str);
	SetConsoleTextAttribute(handle, colorOld);
}