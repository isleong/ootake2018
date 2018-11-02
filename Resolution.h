/******************************************************************************
Ootake

 [Resolution.h]
	フルスクリーンの解像度を設定するフォーム

Copyright(C)2006-2016 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef RESOLUTION_H_INCLUDED
#define RESOLUTION_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
RESOLUTION_Init(
	HINSTANCE	hInstance,
	Sint32		setNum,
	Sint32*		resolutionWidth,
	Sint32*		resolutionHeight);

void
RESOLUTION_Deinit();


#endif /* RESOLUTION_H_INCLUDED */
