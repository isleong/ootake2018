/******************************************************************************
Ootake

 [PsgSetting.h]
	���������̃N�I���e�B(�Č��𑜓x)��ݒ肷��t�H�[��

Copyright(C)2006-2017 Kitao Nakamura.
	�����ŁE��p�ł����J�Ȃ���Ƃ��͕K���\�[�X�R�[�h��Y�t���Ă��������B
	���̍ۂɎ���ł��܂��܂���̂ŁA�ЂƂ��Ƃ��m�点����������ƍK���ł��B
	���I�ȗ��p�͋ւ��܂��B
	���Ƃ́uGNU General Public License(��ʌ��O���p�����_��)�v�ɏ����܂��B

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
