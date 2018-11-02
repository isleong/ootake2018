/******************************************************************************
Ootake

 [PsgSetting.c]
	内蔵音源のクオリティ(再現解像度)を設定するフォーム

Copyright(C)2006-2017 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "PsgSetting.h"
#include "Resource.h"
#include "WinMain.h"
#include "App.h"
#include "Option.h"

#define LINE_LEN	56
#define N_LINES 	21

#define BUTTON_HQ		1
#define BUTTON_ALITTLE	2
#define BUTTON_LIGHT	3


static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" PSG Quality setting";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;

static Sint32		_PsgQuality = -1; //戻り値(決定した場合1以上の値が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pPsgQuality;


/* フォントの高さを取得する */
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* 固定ピッチフォント */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)(tm.tmHeight);
}

/* フォントの横幅を取得する */
static Uint32
get_font_width(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* 固定ピッチフォント */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)tm.tmAveCharWidth;
}


static void
set_window_size(
	HWND			hWnd)
{
	RECT		rc;
	Uint32		wndW = _FontWidth  * LINE_LEN +2;
	Uint32		wndH = _FontHeight * N_LINES -2 + _FontHeight/2;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(OPTION_GetHwnd(), &rc);
	MoveWindow(hWnd, rc.left, rc.top, wndW, wndH, TRUE);
}


static void
update_window(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	PAINTSTRUCT 	ps;
	Uint32			x;
	Uint32			y;

	/* 描画準備 */
	hDC = BeginPaint(hWnd, &ps);
	SetBkMode(hDC, OPAQUE);	//文字の背景を塗りつぶす
	SetBkColor(hDC, RGB(64,128,64));
	SetTextColor(hDC, RGB(240,240,240));
	hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	x = _FontWidth*2 +1;
	y = _FontHeight/2 +4;
	TextOut(hDC, x, y, "Select PSG(PCE built-in sound) Quality.", 39);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "First Button - for Gaming PC (Recommend)", 40);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "2nd or 3rd Button - for Slow PC", 31);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "Push one of the Button, and automatic", 37);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "adjustment is started. It takes 60 seconds.", 43);
	y += _FontHeight + _FontHeight/2 +2;
	TextOut(hDC, x, y, "In Japanese language", 20);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "ＰＣＥ内蔵音源の再現クォリティを選択してください。", 50);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "ゲーム向けのＰＣの場合、１番上のボタンを推奨。", 46);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "パワーの少ないＰＣの場合、２か３番目のボタンを推奨。", 52);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "いずれかのボタンを押すと、自動調整が開始されますので", 52);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "そのままで60秒間お待ちください。", 34);

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static LRESULT CALLBACK
psgSetting_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(OPTION_GetHwnd(), FALSE);//オプションウィンドウを無効化してモーダルに。
		_hFontB = CreateFont(  0,						// 高さ。0 = デフォルト
		                       0,						// 幅。0なら高さに合った幅
    		                   0,						// 角度
        		               0,						// ベースラインとの角度
            		           FW_NORMAL,				// 太さ
                		       FALSE,					// イタリック
	                    	   FALSE,					// アンダーライン
		                       FALSE,					// 打ち消し線
    		                   0,						// 日本語を取り扱うときはSHIFTJIS_CHARSETにする。
        		               0,						// 出力精度
            		           0,						// クリッピング精度
                		       0,						// 出力品質
                    		   0,						// ピッチとファミリー
		                       ""						// 書体名
							); //英語のデフォルトフォントに設定
		_FontWidth	= get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_PAINT:
		update_window(hWnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PSGSETTING_Deinit();
			return 0;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case BUTTON_HQ:
				_PsgQuality = 4; //戻り値を設定
				PSGSETTING_Deinit();
				return 0;
			case BUTTON_ALITTLE:
				_PsgQuality = 2; //戻り値を設定
				PSGSETTING_Deinit();
				return 0;
			case BUTTON_LIGHT:
				_PsgQuality = 1; //戻り値を設定
				PSGSETTING_Deinit();
				return 0;
		}
		break;

	case WM_CLOSE:
		PSGSETTING_Deinit();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
psgSetting_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	HWND		hWndTmp;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = psgSetting_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	_hMyb = CreateSolidBrush(RGB(64,128,64)); //ブラシを作る
	wc.hbrBackground = _hMyb;
	wc.lpszMenuName  = "";
	wc.lpszClassName = _pCaption;

	if (RegisterClass(&wc) == 0)
		return FALSE;

	hWnd = CreateWindow(
		_pCaption,
		_pCaption,
		WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		OPTION_GetHwnd(),
		NULL,
		_hInstance,
		NULL
	);

	if (hWnd == NULL)
		return FALSE;

	_hWnd = hWnd;

	//HQボタンを作成
	x = _FontWidth*2;
	y = (_FontHeight/2 +4) + (_FontHeight +2)*12 + (_FontHeight/2);
	hWndTmp = CreateWindow(
		"BUTTON", "High Quality PSG (Default)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_HQ, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//ALITTLEボタンを作成
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "A Little Light PSG",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_ALITTLE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//LIGHTボタンを作成
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "Light PSG (for not fast PC)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_LIGHT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	ImmAssociateContext(_hWnd, 0); //IMEを無効にする

	return TRUE;
}


BOOL
PSGSETTING_Init(
	HINSTANCE	hInstance,
	Sint32*		psgQuality)
{
	if (_hInstance != NULL)
		PSGSETTING_Deinit();

	_hInstance = hInstance;

	_pPsgQuality = psgQuality;
	_PsgQuality = -1;//キャンセル

	return psgSetting_main();
}


void
PSGSETTING_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//オプションウィンドウにフォーカスを戻し前面に。
		EnableWindow(OPTION_GetHwnd(), TRUE);
		SetForegroundWindow(OPTION_GetHwnd());
		
		*_pPsgQuality = _PsgQuality; //戻り値を設定。この瞬間にApp.c に制御が戻る。
	}
}

