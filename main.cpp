/******************************************************************************
Ootake
・二重起動をすると問題が起こる場合があるので、二重起動は防止するようにした。
・COMの初期化と開放をここでやるようにした。Vistaでファイルダイアログ使用時に
  初期化しておかないと不安定だった。v1.05
・マルチメディアタイマの精度をここで上げておくようにした。v1.55
・Shiftキーをゲーム操作で扱えるように、Shiftキーを5回押して現れる固定キー機
  能を無効にするようにした。v2.87

Copyright(C)2006-2017 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[main]
		本プロジェクトのメイン関数です．

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

#define _WIN32_DCOM //v2.17更新

#include <objbase.h>
#include "App.h"

//Kitao追加。v2.87
STICKYKEYS	g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
TOGGLEKEYS	g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
FILTERKEYS	g_StartupFilterKeys = {sizeof(FILTERKEYS), 0}; 

//Kitao追加。v2.87
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

// gcc は main が定義されていると
// WinMain よりも先に main を呼んでしまうらしい．．．
// __main__ というのはその workaround.
int
__main__(
	int			argc,
	char**		argv)
{
	HANDLE		hMutex;
	TIMECAPS	tc;
	BOOL		bOk;

	//Kitao追加。二重起動を防止
	hMutex = CreateMutex(NULL, TRUE, "Ootake Emulator"); //ミューテックスの作成
	if (GetLastError() == ERROR_ALREADY_EXISTS) //すでにOotakeを起動していたら
		return 0; //起動せずに終了

	CoInitializeEx(NULL, COINIT_MULTITHREADED); //Kitao追加。v2.17更新。参考：アパートメント(COINIT_APARTMENTTHREADED)だと音が少し硬い感じになる(おそらく直のMTAよりSTAのほうが処理間隔が長い)。v2.19記
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin); //Kitao追加。タイマ精度をここで上げておくようにした。

	//Shiftキーを5回押して現れる固定キー機能等を無効に。v2.87
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_StartupStickyKeys, 0);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &g_StartupToggleKeys, 0);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_StartupFilterKeys, 0);
	AllowAccessibilityShortcutKeys(FALSE);//無効

	bOk = APP_Init(argc, argv);
	if (bOk)
	{
		while (APP_ProcessEvents() != APP_QUIT);
		APP_Deinit();
	}

	//Shiftキーを5回押して現れる固定キー機能等を元に戻す。v2.87
	AllowAccessibilityShortcutKeys(TRUE);//元に戻す

	timeEndPeriod(tc.wPeriodMin); //Kitao追加
	CoUninitialize(); //Kitao追加

	if (bOk)
		return 0;
	else
		return -1;
}
