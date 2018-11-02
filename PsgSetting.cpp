/******************************************************************************
Ootake

 [PsgSetting.c]
	���������̃N�I���e�B(�Č��𑜓x)��ݒ肷��t�H�[��

Copyright(C)2006-2017 Kitao Nakamura.
	�����ŁE��p�ł����J�Ȃ���Ƃ��͕K���\�[�X�R�[�h��Y�t���Ă��������B
	���̍ۂɎ���ł��܂��܂���̂ŁA�ЂƂ��Ƃ��m�点����������ƍK���ł��B
	���I�ȗ��p�͋ւ��܂��B
	���Ƃ́uGNU General Public License(��ʌ��O���p�����_��)�v�ɏ����܂��B

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


static HBRUSH		_hMyb; //����u���V�F
static HFONT		_hFontB; //�{�^���p�t�H���g

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" PSG Quality setting";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;

static Sint32		_PsgQuality = -1; //�߂�l(���肵���ꍇ1�ȏ�̒l���Ԃ����)�B�L�����Z���Ȃ�-1�B���ݒ蒆��0�B
static Sint32*		_pPsgQuality;


/* �t�H���g�̍������擾���� */
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* �Œ�s�b�`�t�H���g */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)(tm.tmHeight);
}

/* �t�H���g�̉������擾���� */
static Uint32
get_font_width(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* �Œ�s�b�`�t�H���g */
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

	/* �`�揀�� */
	hDC = BeginPaint(hWnd, &ps);
	SetBkMode(hDC, OPAQUE);	//�����̔w�i��h��Ԃ�
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
	TextOut(hDC, x, y, "�o�b�d���������̍Č��N�H���e�B��I�����Ă��������B", 50);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "�Q�[�������̂o�b�̏ꍇ�A�P�ԏ�̃{�^���𐄏��B", 46);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "�p���[�̏��Ȃ��o�b�̏ꍇ�A�Q���R�Ԗڂ̃{�^���𐄏��B", 52);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "�����ꂩ�̃{�^���������ƁA�����������J�n����܂��̂�", 52);
	y += _FontHeight +2;
	TextOut(hDC, x, y, "���̂܂܂�60�b�Ԃ��҂����������B", 34);

	/* �I������ */
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
		EnableWindow(OPTION_GetHwnd(), FALSE);//�I�v�V�����E�B���h�E�𖳌������ă��[�_���ɁB
		_hFontB = CreateFont(  0,						// �����B0 = �f�t�H���g
		                       0,						// ���B0�Ȃ獂���ɍ�������
    		                   0,						// �p�x
        		               0,						// �x�[�X���C���Ƃ̊p�x
            		           FW_NORMAL,				// ����
                		       FALSE,					// �C�^���b�N
	                    	   FALSE,					// �A���_�[���C��
		                       FALSE,					// �ł�������
    		                   0,						// ���{�����舵���Ƃ���SHIFTJIS_CHARSET�ɂ���B
        		               0,						// �o�͐��x
            		           0,						// �N���b�s���O���x
                		       0,						// �o�͕i��
                    		   0,						// �s�b�`�ƃt�@�~���[
		                       ""						// ���̖�
							); //�p��̃f�t�H���g�t�H���g�ɐݒ�
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
				_PsgQuality = 4; //�߂�l��ݒ�
				PSGSETTING_Deinit();
				return 0;
			case BUTTON_ALITTLE:
				_PsgQuality = 2; //�߂�l��ݒ�
				PSGSETTING_Deinit();
				return 0;
			case BUTTON_LIGHT:
				_PsgQuality = 1; //�߂�l��ݒ�
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
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //�A�C�R����ǂݍ��݁Bv2.00�X�V
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	_hMyb = CreateSolidBrush(RGB(64,128,64)); //�u���V�����
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

	//HQ�{�^�����쐬
	x = _FontWidth*2;
	y = (_FontHeight/2 +4) + (_FontHeight +2)*12 + (_FontHeight/2);
	hWndTmp = CreateWindow(
		"BUTTON", "High Quality PSG (Default)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_HQ, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//�t�H���g��ݒ�

	//ALITTLE�{�^�����쐬
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "A Little Light PSG",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_ALITTLE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//�t�H���g��ݒ�

	//LIGHT�{�^�����쐬
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "Light PSG (for not fast PC)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_LIGHT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//�t�H���g��ݒ�

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	ImmAssociateContext(_hWnd, 0); //IME�𖳌��ɂ���

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
	_PsgQuality = -1;//�L�����Z��

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
		
		DeleteObject(_hFontB); //�{�^���p�t�H���g���J��
		DeleteObject(_hMyb); //�u���V���J��
		
		//�I�v�V�����E�B���h�E�Ƀt�H�[�J�X��߂��O�ʂɁB
		EnableWindow(OPTION_GetHwnd(), TRUE);
		SetForegroundWindow(OPTION_GetHwnd());
		
		*_pPsgQuality = _PsgQuality; //�߂�l��ݒ�B���̏u�Ԃ�App.c �ɐ��䂪�߂�B
	}
}

