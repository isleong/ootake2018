/******************************************************************************
Ootake

 [PsgSetting.h]
	内蔵音源のクオリティ(再現解像度)を設定するフォーム

Copyright(C)2006-2017 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef PSGSETTING_H_INCLUDED
#define PSGSETTING_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
PSGSETTING_Init(
	HINSTANCE	hInstance,
	Sint32*		psgSetting);

void
PSGSETTING_Deinit();


#endif /* PSGSETTING_H_INCLUDED */
