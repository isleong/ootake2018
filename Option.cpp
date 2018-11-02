/******************************************************************************
Ootake

 [Option.cpp]
	スタート時に設定を変更するためのフォーム

Copyright(C)2006-2016 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Option.h"
#include "resource.h"
#include "Startup.h"
#include "WinMain.h"
#include "App.h"
#include "JoyPad.h"
#include "ScreenD3D.h"
#include "PsgSetting.h"

#define LINE_LEN	47
#define N_LINES 	23

enum OptionComponent
{
	BUTTON_HELP = 1,
	BUTTON_CDDATIMING,
	BUTTON_FUNCKEY,
	BUTTON_PADCONFIG,
	BUTTON_RESUME,
	COMBOBOX_SCREENTYPE,
	COMBOBOX_DRAWMODE,
	COMBOBOX_SOUNDBUFFER,
	BUTTON_LIGHT,
	BUTTON_DEFAULT,
	BUTTON_SET,
};
static HWND			_hWndC[BUTTON_SET]; //各コンポーネントのハンドル

static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" Setting";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd = NULL;


static Sint32		_ScreenType = -1; //戻り値(決定した場合1～5が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pScreenType;
static Sint32		_DrawMode = -1; //戻り値(決定した場合1～4が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pDrawMode;
static Sint32		_SoundBuffer = -1; //戻り値(決定した場合1～9が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pSoundBuffer;


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
	Uint32		wndW = _FontWidth  * LINE_LEN +_FontWidth;
	Uint32		wndH = _FontHeight * N_LINES +_FontHeight/2 -1;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(STARTUP_GetHwnd(), &rc);
	MoveWindow(hWnd, rc.left, rc.bottom - wndH, wndW, wndH, TRUE);
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
	y = _FontHeight/2;
	TextOut(hDC, x, y, "Important Setting  重要な設定", 29);
	y += _FontHeight*9 +1;
	TextOut(hDC, x, y, "Push button, and Explanation is displayed.", 42);
	y += _FontHeight;
	TextOut(hDC, x, y, "ボタンを押すと説明も表示されます。", 34);

	y = _FontHeight*13 -1;
	TextOut(hDC, x, y, "Video Setting  ビデオ環境の設定", 31);
	y += _FontHeight*5;
	TextOut(hDC, x, y, "Sound Setting  サウンド環境の設定", 33);

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static LRESULT CALLBACK
option_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	BOOL		bOk;
	Sint32		screenType;
	Sint32		psgQuality;
	MSG			msg;

	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(STARTUP_GetHwnd(), FALSE);//スタートアップウィンドウを無効化してモーダルに。
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
			OPTION_Deinit();
			return 0;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case BUTTON_HELP: //v2.77追加
				MessageBox(hWnd,
					"For the important setting of Ootake, please use the four buttons on    \n"
					"the left.    \n"
					"In lower side of the setting screen, the setting of Video and Sound    \n"
					"can be set. When you press the [Set]button(bottom-right of window),    \n"
					"it will reflect the setting.    \n"
					"In addition, the menu appears when you click on the game screen    \n"
					"during game play. It can be a variety of settings. Please select   \n"
					"[Info -> Manual-English] menu(rightmost of menu), and the    \n"
					"explanation of each menu is displayed.    \n"
					"\n"
					"In Japanese language\n"
					"左の４つのボタンで、Ootakeの動作に必要となる基本的な（重要な）   \n"
					"設定を行えます。\n"
					"設定画面下側の、ビデオとサウンドの設定は、一番右下の[Set]ボタン   \n"
					"を押すと、設定が反映されます。   \n"
					"これらの設定は、ゲームプレイ中に画面をクリックすると現れるメニュー   \n"
					"からも設定することができます。各メニューの説明は、メニュー一番右の   \n"
					"[Info -> Manual-Japanese]を選択してください。   ",
					"About Ootake Setting", MB_OK);
				break;

			case BUTTON_CDDATIMING: //v2.77追加
				MessageBox(hWnd,
					"The timing value of the CD sound (CD-DA and ADPCM) is set    \n"
					"automatically to match the sound to the picture.    \n"
					"The set completion is about 60 seconds. Please close other heavy    \n"
					"applications for an accurate measurement now.    \n"
					"\n"
					"In Japanese language\n"
					"お使いのパソコン環境において、絵とCD音源（CD-DA,ADPCM）のズレ   \n"
					"が起こらないように、タイミング値を自動設定します。\n"
					"設定完了まで約60秒間掛かります。その間、Ootakeを処理落ちさせない   \n"
					"ために、他の重いアプリケーションは閉じてください。   ",
					"About \"CD-DA Play Timing\" Setting", MB_OK);
				psgQuality = 0;
				PSGSETTING_Init(_hInstance, &psgQuality); //PsgSettingウィンドウを表示
				while (psgQuality == 0) //ダイアログが結果を返すまでループ
				{ 
					GetMessage(&msg ,NULL, 0, 0);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if (psgQuality >= 1) //キャンセルではなく、３つのボタンのいずれかが押されていた場合
				{
					APP_SetPsgQuality(psgQuality);
					if (psgQuality == 1) //処理速度優先を選んだ場合
					{
						if (APP_GetRemoveDisplayLagFrame() > 1)
							APP_SetRemoveDisplayLagFrame(1); //処理を軽くするため遅延対策処理を最低限に。v2.89
					} else if (psgQuality == 4) //最高設定を選んだ場合
					{
						if (APP_GetRemoveDisplayLagFrame() >= 1) //遅延対策処理がオフじゃなかった場合
							APP_SetRemoveDisplayLagFrame(3); //処理を軽くするため遅延対策処理も最高設定に。v2.89
					}
					screenType = SendMessage(GetDlgItem(hWnd, COMBOBOX_SCREENTYPE), CB_GETCURSEL, 0, 0) + 1;
					switch (screenType) //ウィンドウモードかフルスクリーンモードかの設定を変更している場合、ここで設定を確定する。
					{
					case 1:	APP_SetStartFullScreen(FALSE); break;
					case 2:
						APP_SetMagnification(2); //x2倍に設定
						APP_SetStartFullScreen(FALSE);
						break;
					case 3:
						APP_SetMagnification(3); //x3倍に設定
						APP_SetStartFullScreen(FALSE);
						break;
					case 4:
						APP_SetMagnification(4); //x4倍に設定
						APP_SetStartFullScreen(FALSE);
						break;
					case 5:	APP_SetStartFullScreen(TRUE); break;
					case 6:
						APP_SetFullScreen640(); //640x480に設定
						APP_SetStartFullScreen(TRUE);
						break;
					}
					APP_UpdateScreenMode(FALSE); //ゲームウィンドウを初期化＆表示
					SetForegroundWindow(WINMAIN_GetHwnd()); //ゲームウィンドウを手前に表示
					if (APP_GetFullScreen())
						Sleep(3000); //Windowsの画面切り替えの処理がもたついている間に計測を始めてしまうと処理落ちで誤計測になってしまうので３秒待つ。
					else
						Sleep(1000); //Windowモードの場合は１秒。
					bOk = APP_SetCddaTimingAuto(TRUE); //タイミングを自動設定
					APP_SaveConfig();//設定を保存
					if (bOk)
					{
						MessageBox(WINMAIN_GetHwnd(),
							"While playing a game, the menu (click on the game screen)    \n"
							"[Audio -> Adjust CD-DA (for Sync) -> Adjust CD-DA Auto Set]    \n"
							"can be used to set this setting.    \n"
							"If there is a gap of CD-DA, please use it.    \n"
							"\n"
							"In Japanese language\n"
							"この設定は、ゲーム中メニュー（ゲーム画面をクリック）の   \n"
							"「Audio -> Adjust CD-DA (for Sync) -> Adjust CD-DA Auto Set」   \n"
							"からも設定することができます。音ズレがあった場合、再設定してみて   \n"
							"ください。   \n"
							"※ゲームによっては、実機でも音ズレしているものがあります。   \n"
							"　Ootake側が原因のゲームな場合、お知らせくだされば、可能な限り   \n"
							"　ですが、修正いたします。   ",
							"About \"CD-DA Play Timing\" Setting", MB_OK);
					}
					if (APP_GetFullScreen()) //フルスクリーンモードだった場合、デスクトップ表示へ戻す。
					{
						APP_SetFullScreen(FALSE);
						APP_UpdateScreenMode(FALSE);
						APP_SetFullScreen(TRUE);
					}
					WINMAIN_SetMiniSize(); //ゲームウィンドウを最小化
					SetForegroundWindow(hWnd); //設定ウィンドウを手前に表示
				}
				break;
		
			case BUTTON_FUNCKEY: //v2.77追加
				bOk = MessageBox(hWnd,
					"Witn Ootake, you can set the \"Function Button\" on your joypad.    \n"
					"[ Set the button you press with your left hand index finger(e.g.     \n"
					"  L2 button at the PS-Pad) to the \"Function Button\". It is    \n"
					"  very useful because it can be various settings at hand. ]    \n"
					"Function Button + [I]  button -> Switching of Auto-fire(Turbo)    \n"
					"Function Button + [II] button -> Switching of Auto-fire(Turbo)    \n"
					"Function Button + [RUN] button -> Switching the Game Soft.    \n"
					"Function Button + [SELECT] button -> Shooting of Screenshots    \n"
					"Function Button + Up,Down,Left,Right -> Audio Volume control    \n"
					"+ Operate while Pressing the Function Button    \n"
					"Press \"Yes\" button, and Setup-Screen appears.    \n\n"
					"In Japanese language\n"
					"Ootakeは、ジョイパッド上に\"ファンクションボタン\"を設定できます。   \n"
					"[ 左手人差し指で押すボタン（例えばPS型ならL2ボタン）をファンク   \n"
					"  ションボタンに設定すると、手元で色々設定できるので便利です。]   \n"
					"ファンクションボタン＋ Iボタン or IIボタン…「連射設定の切替」   \n"
					"ファンクションボタン＋ RUNボタン…「ゲームの切替」   \n"
					"ファンクションボタン＋ SELECTボタン…「スクリーンショット保存」   \n"
					"ファンクションボタン＋ 十字キー…「音量の調節」   \n"
					"※上記の操作は、ファンクションボタンを押しながら操作します。   \n"
					"「はい」を押すと設定画面が出ますので、ボタンを押してください。   \n",
					"About \"Function Button\" Setting", MB_YESNO);
				if (bOk == IDYES)
				{
					APP_SetShortcutButton(_hWnd, _hInstance, 6);
					APP_SaveConfig();//設定を保存
				}
				break;
		
			case BUTTON_PADCONFIG: //v2.77追加
				bOk = MessageBox(hWnd,
					"In real machine \"PC Engine(TG16)\", three types of game pad (    \n"
					"[2 button pad],[3 button pad],and [6 button pad]) exists.    \n"
					"In Ootake, setting of the button is set by three separate    \n"
					"settings. And, settings of three types are saved.    \n"
					"Press \"Yes\" button, and Setup-Screen appears. In the order of    \n"
					"[2 button pad],[3 button pad],and [6 button pad], please set it.    \n"
					"+In real machine, [RUN] or [SELECT] either is selected at    \n"
					" [III]button of [3 button pad]. In Ootake, set [IIIr]button and    \n"
					" [IIIs]button, and they can be used at the same time.    \n\n"
					"In Japanese language\n"
					"PCエンジン（実機）には、「２ボタン」「３ボタン」「６ボタン」の   \n"
					"３種類のゲームパッドが存在します。   \n"
					"Ootakeでは、ボタン設定を３種類別々に設定し、別々に保存します。   \n"
					"「はい」を押すと設定画面が出ますので、「２ボタン」「３ボタン」   \n"
					"「６ボタン」の順に３種類ぶん、ボタンを押して設定してください。   \n"
					"※「３ボタンパッド」は、実機では３ボタン目が「RUN,SELECTどちら   \n"
					"　かを切り替えて使用」でしたが、Ootakeでは、RUNとSELECTを別々   \n"
					"　にそれぞれを「IIIrボタン,IIIsボタンとして設定」します。   \n",
					"About \"PCE-Pad Controller\" Setting", MB_YESNO);
				if (bOk == IDYES)
				{
					//２ボタン，３ボタン，６ボタンパッドをコントローラ１～５まで全て初期化。v2.87
					//（v2.79より前のバージョンでジョイパッドを複数繋いだり外したりした場合、コントローラ２～５までに余分な設定が残り誤動作の恐れあり。それを解消するためにもここで初期化）
					APP_InitPadConfiguration();
					//２ボタンパッドを設定
					JOYPAD_UseThreeButton(FALSE);
					APP_SetInputConfiguration();
					APP_ConfigurePad(_hWnd, _hInstance, 1);
					//３ボタンパッドを設定
					JOYPAD_UseThreeButton(TRUE);
					APP_SetInputConfiguration();
					APP_ConfigurePad(_hWnd, _hInstance, 1);
					//６ボタンパッドを設定
					JOYPAD_UseSixButton(TRUE);
					APP_SetInputConfiguration();
					APP_ConfigurePad(_hWnd, _hInstance, 1);
					//２ボタンパッドがつながっている状態に戻す
					APP_ReturnCaption(); //６ボタンパッド設定完了のメッセージを消す。
					JOYPAD_UseThreeButton(FALSE);
					JOYPAD_UseSixButton(FALSE);
					APP_SetInputConfiguration();
					APP_SaveConfig();//設定を保存
					MessageBox(hWnd,
						"Pad configuration is complete.    \n"
						"In the game for 2-5 players, connect multiple game pads to your    \n"
						"PC. Or, use the keyboard.    \n"
						"For the setting of 2-5 players, click the    \n"
						"[Input -> Configure -> Configure Pad #2-#5] menu.    \n"
						"(The menu appears when you click on the game screen during    \n"
						"game play.)    \n\n"
						"In Japanese language\n"
						"パッド設定が完了しました。   \n"
						"なお、２人～５人プレイ対応のゲームでは、パソコンにゲームパッドを   \n"
						"複数つなげるか、キーボードの利用で、実機同様のプレイができます。   \n"
						"２～５プレイヤーのコントローラー設定は、ゲーム中メニュー（ゲーム   \n"
						"画面をクリックすると出ます）の右から２つ目にある   \n"
						"「Input -> Configure -> Configure Pad #2-#5」メニューから行って   \n"
						"ください。   ",
						"About \"PCE-Pad Controller\" Setting", MB_OK);
				}
				break;

			case BUTTON_RESUME: //v2.77追加
				bOk = MessageBox(hWnd,
					"Ootake has \"Resume Mode\" that performing a state save    \n"
					"automatically. When the game is started in the next time,    \n"
					"you can play from the continuation of the last end. In addition,    \n"
					"the settings(Sound Volume and Auto-fire, etc.) are loaded in    \n"
					"every game.    \n"
					"For using \"Resume Mode\", click the [File -> Set Resume] menu,    \n"
					"and \"Resume Mode\" switch of the playing game become ON.    \n"
					"(The menu appears when you click on the game screen during    \n"
					"game play.)    \n"
					"If you want to automatically turn on the \"Resume Mode\" in all    \n"
					"of the games, press [Yes]button(at bottom of this window).    \n"
					"If you want to manually turn on(off) to each game, press    \n"
					"[No]button(at bottom of this window).    \n"
					"\n"
					"In Japanese language\n"
					"Ootakeは、ゲーム終了時に自動でステートセーブを行う「レジューム機能」   \n"
					"があります。次回のゲーム開始時に、前回終了した続きからプレイでき、   \n"
					"サウンド音量や連射等の設定も、ゲーム毎に読み込まれます。   \n"
					"「レジューム機能」は、レジューム機能を使いたいゲームをプレイ中に、   \n"
					"メニュー（ゲーム画面をクリック）から、[File -> Set Resume]メニューを   \n"
					"選択してチェックを入れることでオンにできます。   \n"
					"全てのゲームで「レジューム機能」を自動的にオンにしたい場合は、   \n"
					"下の「はい」を押してください。上記のように手動でゲーム毎にオンオフ   \n"
					"したい場合は、「いいえ」を押してください。   ",
					"About \"Resume(Auto Save) Mode\" Setting", MB_YESNO);
				if (bOk == IDYES)
					APP_SetAutoResumeMode(TRUE);
				else
					APP_SetAutoResumeMode(FALSE);
				APP_SaveConfig();//設定を保存
				break;
		
			case COMBOBOX_SCREENTYPE:
			case COMBOBOX_DRAWMODE:
			case COMBOBOX_SOUNDBUFFER:
				if (HIWORD(wParam) == CBN_CLOSEUP)
					SetFocus(GetDlgItem(hWnd, BUTTON_SET));//OKボタンにフォーカス
				break;

			case BUTTON_LIGHT:
				SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 5, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 5, 0);
				SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 7, 0);
				break;

			case BUTTON_DEFAULT:
				SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 1, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 5, 0);
				break;

			case BUTTON_SET:
				//戻り値を設定
				_ScreenType = SendMessage(GetDlgItem(hWnd, COMBOBOX_SCREENTYPE), CB_GETCURSEL, 0, 0) + 1;
				_DrawMode = SendMessage(GetDlgItem(hWnd, COMBOBOX_DRAWMODE), CB_GETCURSEL, 0, 0) + 1;
				_SoundBuffer = SendMessage(GetDlgItem(hWnd, COMBOBOX_SOUNDBUFFER), CB_GETCURSEL, 0, 0) + 1;
				OPTION_Deinit();
				return 0;
		}
		break;

	case WM_CLOSE:
		OPTION_Deinit();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
option_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	HWND		hWndTmp;
	char		pCaption[64];
	
	strcpy_s(pCaption, 64, "\"Ootake v");
	strcat_s(pCaption, 64, APP_GetSoftVersionChar());
	strcat_s(pCaption, 64, "\" Setting");
	
	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = option_wnd_proc;
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
		pCaption,
		WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		STARTUP_GetHwnd(),
		NULL,
		_hInstance,
		NULL
	);

	if (hWnd == NULL)
		return FALSE;

	_hWnd = hWnd;

	//CD-DA Timingボタンを作成
	x = _FontWidth*2;
	y = _FontHeight +_FontHeight/2 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "Set CD-DA Play Timing",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_CDDATIMING, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//Helpボタンを作成
	x += _FontWidth*40;
	hWndTmp = CreateWindow(
		"BUTTON", "?",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*4, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_HELP, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	//FunctionButtonボタンを作成
	x = _FontWidth*2;
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "Set Function-Button",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_FUNCKEY, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	//PadConfigボタンを作成
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "Set Button of PCE-Pad Controller",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_PADCONFIG, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	//Resumeボタンを作成
	y += _FontHeight*2;
	hWndTmp = CreateWindow(
		"BUTTON", "Set Resume (Auto Save) Mode",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*38, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_RESUME, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	//フルスクリーンorウィンドウモード切替用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*14;
	_hWndC[COMBOBOX_SCREENTYPE] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*9,
		_hWnd, (HMENU)COMBOBOX_SCREENTYPE, _hInstance, NULL
	);
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start Window mode");
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start Window (x2)");
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start Window (x3)");
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start Window (x4)");
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start FullScreen mode");
	SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_ADDSTRING, 0, (LPARAM)"Start FullScreen (640x480 60Hz)");
	if (APP_GetStartFullScreen())
		SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 4, 0);
	else
		SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 0, 0);

	//TV or RGB切替用コンボボックスを作成
	x = _FontWidth*2;
	y += _FontHeight*2;
	_hWndC[COMBOBOX_DRAWMODE] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*14,
		_hWnd, (HMENU)COMBOBOX_DRAWMODE, _hInstance, NULL
	);
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Special Scanlined (Sharp&Gentle)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"TV Scanlined (Smooth, Fast)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Horizontal Scanlined");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Horizontal Scanlined 2 (Sharp)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Horizontal Scanlined 3 (More)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Horizontal Scanlined F (Fast)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Non-Scanlined (Fast)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Non-Scanlined 2 (Sharp, Fast)");
	SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_ADDSTRING, 0, (LPARAM)"Non-Scanlined TV (Smooth, Fast)");
	switch (APP_GetScanLineType())
	{
		case 0: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 6, 0); break;
		case 1: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 0, 0); break;
		case 2: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 2, 0); break;
		case 3: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 5, 0); break;
		case 4: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 1, 0); break;
		case 5: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 3, 0); break;
		case 6: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 4, 0); break;
		case 7: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 7, 0); break;
		case 8: SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 8, 0); break;
	}

	//サウンドバッファ用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*19;
	_hWndC[COMBOBOX_SOUNDBUFFER] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*18,
		_hWnd, (HMENU)COMBOBOX_SOUNDBUFFER, _hInstance, NULL
	);
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1024 (Not Delay)");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1152");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1280");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1408");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1536");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1664 (Default)");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 1792");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 2048");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 2176");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 2304");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 2560");
	SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_ADDSTRING, 0, (LPARAM)"Buffer 3072 (Beauty,Delay)");
	switch (APP_GetSoundBufferSize())
	{
		case 1024: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  0, 0); break;
		case 1152: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  1, 0); break;
		case 1280: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  2, 0); break;
		case 1408: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  3, 0); break;
		case 1536: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  4, 0); break;
		case 1792: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  6, 0); break;
		case 2048: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  7, 0); break;
		case 2176: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  8, 0); break;
		case 2304: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL,  9, 0); break;
		case 2560: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 10, 0); break;
		case 3072: SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 11, 0); break;
		default: //1664
			SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 5, 0); break;
	}

	//Lightボタンを作成
	x = _FontWidth*2;
	y = _FontHeight*21;
	hWndTmp = CreateWindow(
		"BUTTON", "Light(Fast)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*13, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_LIGHT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//Defaultボタンを作成
	x += _FontWidth*14;
	hWndTmp = CreateWindow(
		"BUTTON", "Default(HQ)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*14, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_DEFAULT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	//Setボタンを作成
	x = _FontWidth*36;
	y = _FontHeight*21;
	hWndTmp = CreateWindow(
		"BUTTON", "Set",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*10, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_SET, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	ImmAssociateContext(_hWnd, 0); //IMEを無効にする

	return TRUE;
}


BOOL
OPTION_Init(
	HINSTANCE	hInstance,
	Sint32*		screenType,
	Sint32*		drawMode,
	Sint32*		soundBuffer)
{
	if (_hInstance != NULL)
		OPTION_Deinit();

	_hInstance = hInstance;

	_pScreenType = screenType;
	_ScreenType = -1;//キャンセル
	_pDrawMode = drawMode;
	_DrawMode = -1;//キャンセル
	_pSoundBuffer = soundBuffer;
	_SoundBuffer = -1;//キャンセル

	return option_main();
}


void
OPTION_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//スタートアップウィンドウにフォーカスを戻し前面に。
		EnableWindow(STARTUP_GetHwnd(), TRUE);
		SetForegroundWindow(STARTUP_GetHwnd());
		
		//戻り値を設定
		*_pScreenType = _ScreenType;
		*_pDrawMode = _DrawMode;
		*_pSoundBuffer = _SoundBuffer; //この瞬間にStartup.c に制御が戻る。
	}
}


HWND
OPTION_GetHwnd()
{
	return _hWnd;
}

