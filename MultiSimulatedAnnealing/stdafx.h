// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 一部の CString コンストラクターは明示的です。
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // ダイアログ内の MFC コントロールのサポートを削除します。

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Windows ヘッダーから使用されていない部分を除外します。
#endif

#include <afx.h>
#include <afxwin.h>         // MFC のコアおよび標準コンポーネント



// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#define WM_NEW_TEMPERATURE	(WM_USER+1)
#define WM_EXECUTION_DONE	(WM_USER+2)

///////////////////////////////////////////////////
#ifdef _DEBUG
#ifdef ATLTRACE 
#undef ATLTRACE
#undef ATLTRACE2

#define ATLTRACE CustomTrace
#define ATLTRACE2 ATLTRACE
#endif // ATLTRACE
#endif // _DEBUG
extern void CustomTrace(const char* format, ...);
extern void CustomTrace(int dwCategory, int line, const char* format, ...);

