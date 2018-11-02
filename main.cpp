/******************************************************************************
Ootake
�E��d�N��������Ɩ�肪�N����ꍇ������̂ŁA��d�N���͖h�~����悤�ɂ����B
�ECOM�̏������ƊJ���������ł��悤�ɂ����BVista�Ńt�@�C���_�C�A���O�g�p����
  ���������Ă����Ȃ��ƕs���肾�����Bv1.05
�E�}���`���f�B�A�^�C�}�̐��x�������ŏグ�Ă����悤�ɂ����Bv1.55
�EShift�L�[���Q�[������ň�����悤�ɁAShift�L�[��5�񉟂��Č����Œ�L�[�@
  �\�𖳌��ɂ���悤�ɂ����Bv2.87

Copyright(C)2006-2017 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	�����ŁE��p�ł����J�Ȃ���Ƃ��͕K���\�[�X�R�[�h��Y�t���Ă��������B
	���̍ۂɎ���ł��܂��܂���̂ŁA�ЂƂ��Ƃ��m�点����������ƍK���ł��B
	���I�ȗ��p�͋ւ��܂��B
	���Ƃ́uGNU General Public License(��ʌ��O���p�����_��)�v�ɏ����܂��B

*******************************************************************************
	[main]
		�{�v���W�F�N�g�̃��C���֐��ł��D

		The main function of the project.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/

#define _WIN32_DCOM //v2.17�X�V

#include <objbase.h>
#include "App.h"

//Kitao�ǉ��Bv2.87
STICKYKEYS	g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
TOGGLEKEYS	g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
FILTERKEYS	g_StartupFilterKeys = {sizeof(FILTERKEYS), 0}; 

//Kitao�ǉ��Bv2.87
void
AllowAccessibilityShortcutKeys(
	bool	bAllowKeys)
{
	if (bAllowKeys)
	{
		// Restore StickyKeys/etc to original state and enable Windows key
		STICKYKEYS sk = g_StartupStickyKeys;
		TOGGLEKEYS tk = g_StartupToggleKeys;
		FILTERKEYS fk = g_StartupFilterKeys;

		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
		// then leave the settings alone as its probably being usefully used
		STICKYKEYS skOff = g_StartupStickyKeys;
		if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
		}

		TOGGLEKEYS tkOff = g_StartupToggleKeys;
		if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
		}

		FILTERKEYS fkOff = g_StartupFilterKeys;
		if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
		}
	}
}

// gcc �� main ����`����Ă����
// WinMain ������� main ���Ă�ł��܂��炵���D�D�D
// __main__ �Ƃ����̂͂��� workaround.
int
__main__(
	int			argc,
	char**		argv)
{
	HANDLE		hMutex;
	TIMECAPS	tc;
	BOOL		bOk;

	//Kitao�ǉ��B��d�N����h�~
	hMutex = CreateMutex(NULL, TRUE, "Ootake Emulator"); //�~���[�e�b�N�X�̍쐬
	if (GetLastError() == ERROR_ALREADY_EXISTS) //���ł�Ootake���N�����Ă�����
		return 0; //�N�������ɏI��

	CoInitializeEx(NULL, COINIT_MULTITHREADED); //Kitao�ǉ��Bv2.17�X�V�B�Q�l�F�A�p�[�g�����g(COINIT_APARTMENTTHREADED)���Ɖ��������d�������ɂȂ�(�����炭����MTA���STA�̂ق��������Ԋu������)�Bv2.19�L
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin); //Kitao�ǉ��B�^�C�}���x�������ŏグ�Ă����悤�ɂ����B

	//Shift�L�[��5�񉟂��Č����Œ�L�[�@�\���𖳌��ɁBv2.87
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	AllowAccessibilityShortcutKeys(FALSE);//����

	bOk = APP_Init(argc, argv);
	if (bOk)
	{
		while (APP_ProcessEvents() != APP_QUIT);
		APP_Deinit();
	}

	//Shift�L�[��5�񉟂��Č����Œ�L�[�@�\�������ɖ߂��Bv2.87
	AllowAccessibilityShortcutKeys(TRUE);//���ɖ߂�

	timeEndPeriod(tc.wPeriodMin); //Kitao�ǉ�
	CoUninitialize(); //Kitao�ǉ�

	if (bOk)
		return 0;
	else
		return -1;
}
