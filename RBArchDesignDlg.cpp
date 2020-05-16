
// RBArchDesignDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "RBArchDesign.h"
#include "RBArchDesignDlg.h"
#include "afxdialogex.h"
#include <locale.h>
#include <sys/timeb.h>
#include <time.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////
#ifdef _DEBUG
#ifdef ATLTRACE 
#undef ATLTRACE
#undef ATLTRACE2

#define ATLTRACE CustomTrace
#define ATLTRACE2 ATLTRACE
#endif // ATLTRACE
#endif // _DEBUG
/*//////////////////////////////////////////////////
void CustomTrace(const wchar_t* format, ...)
{
const int TraceBufferSize = 1024;
wchar_t buffer[TraceBufferSize];

va_list argptr; va_start(argptr, format);
vswprintf_s(buffer, format, argptr);
va_end(argptr);

::OutputDebugString(buffer);
}

void CustomTrace(int dwCategory, int line, const wchar_t* format, ...)
{
va_list argptr; va_start(argptr, format);
CustomTrace(format, argptr);
va_end(argptr);
}
/*//////////////////////////////////////////////////
void CustomTrace(const char* format, ...)
{
	const int TraceBufferSize = 1024;
	char buffer[TraceBufferSize];

	va_list argptr; va_start(argptr, format);
	vsprintf_s(buffer, format, argptr);
	va_end(argptr);

	wchar_t ws[TraceBufferSize];
	size_t nwslen;
	_tsetlocale(LC_ALL,_T(""));
	//	mbstowcs( ws, buffer, TraceBufferSize );
	//	mbstowcs_s( &nwslen, ws, strlen(buffer), buffer, TraceBufferSize );
	mbstowcs_s( &nwslen, ws, strlen(buffer)+1, buffer, _TRUNCATE);

	::OutputDebugString(ws);
}

void CustomTrace(int dwCategory, int line, const char* format, ...)
{
	va_list argptr; va_start(argptr, format);
	CustomTrace(format, argptr);
	va_end(argptr);
}
//////////////////////////////////////////////////*/


UINT ThreadExecute(LPVOID pParam)
{
	CRBArchDesignDlg* pObject = (CRBArchDesignDlg *)pParam;
	pObject->Execute();
	return 0;
}

UINT ThreadTestFinal(LPVOID pParam)
{
	CRBArchDesignDlg* pObject = (CRBArchDesignDlg *)pParam;
	pObject->TestFinal();
	return 0;
}


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRBArchDesignDlg ダイアログ



CRBArchDesignDlg::CRBArchDesignDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RBARCHDESIGN_DIALOG, pParent)
	, m_strTempCurrent(_T(""))
	, m_strScoreMin(_T(""))
	, m_strCPUTime(_T(""))
	, m_bMultiThread(FALSE)
	, m_strCPUTimeEstimate(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRBArchDesignDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILENAME, m_strFilename);
	DDX_Text(pDX, IDC_EDIT_TEMP_START, m_dTstartBinding);
	DDX_Text(pDX, IDC_EDIT_TEMP_END, m_dTendBinding);
	DDX_Text(pDX, IDC_EDIT_TEMP_RATE, m_dRateBinding);
	DDX_Text(pDX, IDC_EDIT_ITERATE, m_nIterateBinding);
	DDX_Text(pDX, IDC_EDIT_TEMP_START2, m_dTstartFloorplanPro);
	DDX_Text(pDX, IDC_EDIT_TEMP_END2, m_dTendFloorplanPro);
	DDX_Text(pDX, IDC_EDIT_TEMP_RATE2, m_dRateFloorplanPro);
	DDX_Text(pDX, IDC_EDIT_ITERATE2, m_nIterateFloorplanPro);
	DDX_Text(pDX, IDC_EDIT_TEMP_START3, m_dTstartFloorplanFinal);
	DDX_Text(pDX, IDC_EDIT_TEMP_END3, m_dTendFloorplanFinal);
	DDX_Text(pDX, IDC_EDIT_TEMP_RATE3, m_dRateFloorplanFinal);
	DDX_Text(pDX, IDC_EDIT_ITERATE3, m_nIterateFloorplanFinal);
	DDX_Text(pDX, IDC_EDIT_TEMP_CURRENT, m_strTempCurrent);
	DDX_Text(pDX, IDC_EDIT_MIN_SCORE, m_strScoreMin);
	DDX_Text(pDX, IDC_EDIT_MIN_SCORE_GLOBAL, m_strScoreMinGlobal);
	DDX_Text(pDX, IDC_EDIT_CPU_TIME, m_strCPUTime);
	DDX_Text(pDX, IDC_EDIT_TIME_START, m_strTimeStart);
	DDX_Text(pDX, IDC_EDIT_TIME_FINISH, m_strTimeFinish);
	DDX_Text(pDX, IDC_EDIT_TIME_CURRENT, m_strTimeCurrent);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
	DDX_Text(pDX, IDC_EDIT_FU_0, m_nAdd);
	DDX_Text(pDX, IDC_EDIT_FU_1, m_nSub);
	DDX_Text(pDX, IDC_EDIT_FU_2, m_nMul);
	DDX_Check(pDX, IDC_CHECK_MULTI_THREAD, m_bMultiThread);
	DDX_Text(pDX, IDC_EDIT_CPU_TIME_ESTIMATE, m_strCPUTimeEstimate);
}

BEGIN_MESSAGE_MAP(CRBArchDesignDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_EXECUTE, &CRBArchDesignDlg::OnBnClickedButtonExecute)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_FILE, &CRBArchDesignDlg::OnBnClickedButtonChooseFile)
	ON_MESSAGE(WM_NEW_TEMPERATURE, &CRBArchDesignDlg::OnNewTemperature)
	ON_MESSAGE(WM_EXECUTION_DONE, &CRBArchDesignDlg::OnExecutionDone)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CRBArchDesignDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_TEST_FINAL, &CRBArchDesignDlg::OnBnClickedButtonTestFinal)
	ON_BN_CLICKED(IDC_CHECK_MULTI_THREAD, &CRBArchDesignDlg::OnBnClickedCheckMultiThread)
END_MESSAGE_MAP()


// CRBArchDesignDlg メッセージ ハンドラー

BOOL CRBArchDesignDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	//GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(FALSE);
	//GetDlgItem(IDC_BUTTON_TEST_FINAL)->EnableWindow(FALSE);

	// TODO: 初期化をここに追加します。
	///////////////////
	m_dTstartBinding = 100; m_dTendBinding = 1; m_dRateBinding = 0.99; m_nIterateBinding = 1000;
	m_dTstartBinding = 100; m_dTendBinding = 1; m_dRateBinding = 0.99; m_nIterateBinding = 100;
	m_dTstartBinding = 10; m_dTendBinding = 0.1; m_dRateBinding = 0.99; m_nIterateBinding = 100;
	m_dTstartBinding = 10; m_dTendBinding = 0.1; m_dRateBinding = 0.95; m_nIterateBinding = 100;
	m_dTstartBinding = 100; m_dTendBinding = 1; m_dRateBinding = 0.9; m_nIterateBinding = 10;
	/*//////////////////
	m_dTstartBinding = 100; m_dTendBinding = 1; m_dRateBinding = 0.95; m_nIterateBinding = 100;
	//////////////////*/
	///////////////////
	m_dTstartFloorplanPro = 100; m_dTendFloorplanPro = 1; m_dRateFloorplanPro = 0.99; m_nIterateFloorplanPro = 1000;
	m_dTstartFloorplanPro = 100; m_dTendFloorplanPro = 1; m_dRateFloorplanPro = 0.95; m_nIterateFloorplanPro = 100;
	m_dTstartFloorplanPro = 10; m_dTendFloorplanPro = 0.1; m_dRateFloorplanPro = 0.95; m_nIterateFloorplanPro = 100;
	m_dTstartFloorplanPro = 10; m_dTendFloorplanPro = 0.1; m_dRateFloorplanPro = 0.9; m_nIterateFloorplanPro = 100;
	/*//////////////////
	m_dTstartFloorplanPro = 100; m_dTendFloorplanPro = 1; m_dRateFloorplanPro = 0.95; m_nIterateFloorplanPro = 1000;
	//////////////////*/
	///////////////////
	m_dTstartFloorplanFinal = 100; m_dTendFloorplanFinal = 1; m_dRateFloorplanFinal = 0.99; m_nIterateFloorplanFinal = 1000;
	m_dTstartFloorplanFinal = 10; m_dTendFloorplanFinal = 0.1; m_dRateFloorplanFinal = 0.99; m_nIterateFloorplanFinal = 10000;
	m_dTstartFloorplanFinal = 10; m_dTendFloorplanFinal = 0.1; m_dRateFloorplanFinal = 0.9; m_nIterateFloorplanFinal = 100;
	///////////////////

	m_bMultiThread = TRUE;

	m_nAdd = 3;
	m_nMul = 0;
	m_nSub = 0;

	time( &m_timeStart );
	struct tm strtim;
	localtime_s( &strtim, &m_timeStart );
	m_strTimeStart.Format( _T("%d/%d %d:%02d:%02d"), strtim.tm_mon+1, strtim.tm_mday, strtim.tm_hour, strtim.tm_min, strtim.tm_sec );

	UpdateData(FALSE);

	m_staticStatus.SetWindowTextW( _T("Ready.") );

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CRBArchDesignDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CRBArchDesignDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CRBArchDesignDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRBArchDesignDlg::OnBnClickedButtonChooseFile()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CFileDialog FileDlg(TRUE, _T(".*") );
	TCHAR types[128],*p;
	CString str;
	//	char cPathName[1024];

	UpdateData(TRUE);
	p = types;
	str = "Data Files (*.txt)";
	lstrcpy( p, str );
	p += (str.GetLength()+1);
	str = "*.txt";
//	str = "dfg_*.txt";
	lstrcpy( p, str );
	p += (str.GetLength()+1);
	str = "All Files (*.*)";
	lstrcpy( p, str );
	p += (str.GetLength()+1);
	str = "*.*";
	lstrcpy( p, str );
	p += (str.GetLength()+1);
	*p = 0x0;

	FileDlg.m_ofn.lpstrFilter = types;

	if( FileDlg.DoModal() != IDOK ) return;
	m_strFilename = FileDlg.GetPathName();
	//	TRACE( "m_strFilename=%s\n", m_strFilename );
	UpdateData(FALSE);
	struct _stat buf;
	//	WideCharToMultiByte( CP_THREAD_ACP, 0, m_strFilename.GetBuffer(), -1, cPathName, 1024, NULL, NULL );
	int result = _tstat( (LPCTSTR)m_strFilename.GetBuffer(), &buf );
	/* ファイル状態情報が取得できたかのチェック */
	if( result != 0 ){
		AfxMessageBox( _T("データファイルが正しく読み込めません"), MB_ICONSTOP );
		return;
	}

	UpdateData( FALSE );
}

void CRBArchDesignDlg::OnBnClickedButtonExecute()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);

	struct timeb timebuffer;
	ftime( &timebuffer );
	m_timeCurrent = timebuffer.time;
	m_millitmCurrent = timebuffer.millitm;

	//開始時刻をダイアログに表示
	time(&m_timeStart);
	struct tm strtim;
	localtime_s(&strtim, &m_timeStart);
	m_strTimeStart.Format(_T("%d/%d %d:%02d:%02d"), strtim.tm_mon + 1, strtim.tm_mday, strtim.tm_hour, strtim.tm_min, strtim.tm_sec);
	UpdateData(FALSE);

	//////////////////////////////////////////////////////////////////////////////////////////////
	m_pExecutingThread = ::AfxBeginThread( ThreadExecute, (LPVOID)this );//マルチスレッド作成
	m_pExecutingThread->m_bAutoDelete = FALSE;
	//	DWORD rthread = pSchedulingThread->ResumeThread();		// サスペンド解除
	m_staticStatus.SetWindowText( L"Executing..." );
	/*//////////////////////////////////////////////////////////////////////////////////////////////
	Execute();
	OnExecutionDone(0, 0);
	//////////////////////////////////////////////////////////////////////////////////////////////*/
}

void CRBArchDesignDlg::Execute(void)
{
	unsigned long nRandomSeed = (unsigned)time(NULL);
	//nRandomSeed = 0;
	srand(nRandomSeed);		/*乱数の初期化*/
	FILE *fp;
	fopen_s(&fp,"result.txt","w");
	fprintf( fp, "nRandomSeed = %d\n", nRandomSeed );
	fclose(fp);

	m_design.SetMultiThread( m_bMultiThread );
	m_design.GetDFG()->readdata( m_strFilename.GetBuffer( m_strFilename.GetLength()+1 ) );
	m_design.SetFUConfiguration( m_nAdd, m_nSub, m_nMul );
	if( m_bMultiThread ){
		m_design.m_MultiSA.SetWindowHandle( m_hWnd );
	}else{
		m_design.m_SA.SetWindowHandle( m_hWnd );
		m_design.m_Binding.Setup();
	}
	m_design.SetParamSAforBinding(m_dTstartBinding, m_dTendBinding, m_dRateBinding, m_nIterateBinding);
	m_design.SetParamSAforFloorplanPro(m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro);
	m_design.SetParamSAforFloorplanFinal(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal);
	m_design.Initialize();
	m_design.Optimize();

//	int nScore = m_design.m_Binding.GetScore();
//	TRACE( "Score=%d\n", nScore );
//	m_design.m_Binding.RestoreBest();
//	TRACE( "BestScore=%d\n", m_design.m_Binding.GetScore() );
	::PostMessage(m_hWnd, WM_EXECUTION_DONE, (WPARAM)0, (LPARAM)0 );
}

LRESULT CRBArchDesignDlg::OnNewTemperature(WPARAM wParam, LPARAM lParam)
{
	double *TempCurrent = (double *)wParam;
	int nScoreMin;
	int nScoreMinGlobal;
	CString str;

	if( m_bMultiThread ){
		double *aScoreArray = (double *)lParam;
		nScoreMin = (int)aScoreArray[0];
		nScoreMinGlobal = (int)aScoreArray[1];
		m_strScoreMin.Format( L"%d", nScoreMin );
		m_strScoreMinGlobal.Format( L"%d", nScoreMinGlobal );
	}else{
		nScoreMin = (int)lParam;
		m_strScoreMin.Format( L"%d", nScoreMin );
	}
	m_strTempCurrent.Format( L"%.3f", *TempCurrent );

	struct timeb timebuffer;
	unsigned long nCPUtime;

	ftime( &timebuffer );
	nCPUtime = (timebuffer.time-m_timeCurrent)*1000+timebuffer.millitm-m_millitmCurrent;
	m_strCPUTime.Format( L"%.3f", nCPUtime/1000.0 );

	time_t etime;
	time_t diffTime;
	time_t currentTime;
	time( &currentTime );
	struct tm strtim_e;
	localtime_s( &strtim_e, &currentTime );
	m_strTimeCurrent.Format( _T("%d/%d %d:%02d:%02d"), strtim_e.tm_mon+1, strtim_e.tm_mday, strtim_e.tm_hour, strtim_e.tm_min, strtim_e.tm_sec );

	diffTime = currentTime-m_timeStart;
	diffTime = log(m_dTstartBinding/m_dTendBinding)/log(m_dTstartBinding/(*TempCurrent))*diffTime;
	m_strCPUTimeEstimate.Format( L"%d", diffTime );

	etime = m_timeStart+diffTime;
	localtime_s( &strtim_e, &etime );
	m_strTimeFinish.Format( _T("%d/%d %d:%02d:%02d"), strtim_e.tm_mon+1, strtim_e.tm_mday, strtim_e.tm_hour, strtim_e.tm_min, strtim_e.tm_sec );

	UpdateData(FALSE);
	return 0;
}

LRESULT CRBArchDesignDlg::OnExecutionDone(WPARAM wParam, LPARAM lParam)
{
	int t0 = (int)wParam;
	int t1 = (int)lParam;
	struct timeb timebuffer;
	unsigned long nCPUtime;

	ftime( &timebuffer );
	nCPUtime = (timebuffer.time-m_timeCurrent)*1000+timebuffer.millitm-m_millitmCurrent;

	if( m_pExecutingThread ){
		DWORD dwExitCode;
		MSG    msg;
		do{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			Sleep( 0 );
			GetExitCodeThread( m_pExecutingThread->m_hThread, &dwExitCode );
			//TRACE( "Thread: %d\n", dwExitCode );
		}while( dwExitCode == STILL_ACTIVE );
		if( m_pExecutingThread != NULL ){
			delete m_pExecutingThread;
		}
		m_pExecutingThread = NULL;
	}

	m_strCPUTime.Format( L"%g", nCPUtime/1000.0 );
	//	str.Format( L"%d", m_DFG.GetAddCount() );
	//	m_editAdd.SetWindowText( str );
	//	str.Format( L"%d", m_DFG.GetMulCount() );
	//	m_editMul.SetWindowText( str );
	//	str.Format( L"%d", m_DFG.GetRegCount() );
	//	m_editReg.SetWindowText( str );
	if( t1 == 0 ){
		if( m_bMultiThread ){
		}else{
			m_design.m_Binding.RestoreBest();
			int nScore = m_design.m_Binding.GetScore();
			m_strScoreMin.Format( L"%d", nScore );
		}
	}
	//	m_editScoreMin.SetWindowText( str );

	int nClockCycles;
	int nClockPeriod;
	if( m_bMultiThread ){
		nClockCycles = m_design.m_MultiBinding.m_nClockCyclesGlobalBest;
		nClockPeriod = m_design.m_MultiBinding.m_nClockPeriodGlobalBest;
	}else{
		nClockCycles = m_design.m_Binding.m_nClockCycles;
		nClockPeriod = m_design.m_Binding.m_nClockPeriod;
	}


//	WriteResult();
	UpdateData(TRUE);

	time_t currentTime;
	time( &currentTime );
	struct tm strtim_e;
	localtime_s( &strtim_e, &currentTime );
	m_strTimeFinish.Format( _T("%d/%d %d:%02d:%02d F"), strtim_e.tm_mon+1, strtim_e.tm_mday, strtim_e.tm_hour, strtim_e.tm_min, strtim_e.tm_sec );

	if( t1 == 0 ){
		char cFilename[64];
		FILE *fp;
		sprintf_s( cFilename, 64, "result%d%02d%02d%02d%02d%02d.txt", strtim_e.tm_year+1900, strtim_e.tm_mon+1, strtim_e.tm_mday, strtim_e.tm_hour, strtim_e.tm_min, strtim_e.tm_sec );
		if( ! fopen_s( &fp, cFilename, "w" ) ){
			_tsetlocale(LC_ALL, _T("japanese"));
			fwprintf_s( fp, L"Data File: %s\n", m_strFilename );
//			fprintf_s( fp, "Data File: %s\n", m_strFilename.GetBuffer( m_strFilename.GetLength()+1 ) );
			fprintf_s( fp, "Binding: Tstart=%f, Tend=%f, Trate=%f, Iterate=%d\n", m_dTstartBinding, m_dTendBinding, m_dRateBinding, m_nIterateBinding );
			fprintf_s( fp, "Pro Floorplan: Tstart=%f, Tend=%f, Trate=%f, Iterate=%d\n", m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro );
			fprintf_s( fp, "Final Floorplan: Tstart=%f, Tend=%f, Trate=%f, Iterate=%d\n", m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal );
			fprintf_s( fp, "Clock Cycles = %d\n", nClockCycles );
			fprintf_s( fp, "Clock Pediod = %d\n", nClockPeriod );
			fprintf_s( fp, "CPU time = %d seconds\n", (int)(currentTime-m_timeStart) );
			fclose( fp );
		}
	}
	m_staticStatus.SetWindowText( L"Done." );
	UpdateData(FALSE);

	return 0;
}

void CRBArchDesignDlg::OnBnClickedButtonTest()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);

	m_design.SetMultiThread( FALSE );
	m_design.GetDFG()->readdata( m_strFilename.GetBuffer( m_strFilename.GetLength()+1 ) );
	m_design.SetFUConfiguration( m_nAdd, m_nSub, m_nMul );
	m_design.m_SA.SetWindowHandle( m_hWnd );
	m_design.SetParamSAforFloorplanPro(m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro);
	m_design.SetParamSAforFloorplanFinal(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal);
	m_design.Initialize();
	m_design.m_Binding.Setup();
	m_design.m_Binding.TestAddRegisters();
}


void CRBArchDesignDlg::OnBnClickedButtonTestFinal()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	struct timeb timebuffer;
	ftime( &timebuffer );
	m_timeCurrent = timebuffer.time;
	m_millitmCurrent = timebuffer.millitm;

	//////////////////////////////////////////////////////////////////////////////////////////////
	m_pExecutingThread = ::AfxBeginThread( ThreadTestFinal, (LPVOID)this );
	m_pExecutingThread->m_bAutoDelete = FALSE;
	//	DWORD rthread = pSchedulingThread->ResumeThread();		// サスペンド解除
	m_staticStatus.SetWindowText( L"Executing..." );
	/*//////////////////////////////////////////////////////////////////////////////////////////////
	TestFinal();
	OnExecutionDone(0, 1);
	//////////////////////////////////////////////////////////////////////////////////////////////*/
}

void CRBArchDesignDlg::TestFinal(void)
{
	UpdateData(TRUE);
	unsigned long nRandomSeed = (unsigned)time(NULL);
	srand(nRandomSeed);		/*乱数の初期化*/
	FILE *fp;
	fopen_s(&fp,"result.txt","w");
	fprintf( fp, "nRandomSeed = %d\n", nRandomSeed );
	fclose(fp);
	m_design.GetDFG()->readdata( m_strFilename.GetBuffer( m_strFilename.GetLength()+1 ) );
	m_design.SetFUConfiguration( m_nAdd, m_nSub, m_nMul );
	m_design.m_SA.SetWindowHandle( m_hWnd );
	m_design.SetParamSAforFloorplanFinal(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal);
	m_design.m_Binding.Setup();
	m_design.m_Binding.m_SAforFloorplanFinal.SetWindowHandle( m_hWnd );
	m_design.m_Binding.m_SAforFloorplanFinal.EnableStatistics();
	m_design.Initialize();
	m_design.m_Binding.TestAddRegisters();
	::PostMessage(m_hWnd, WM_EXECUTION_DONE, (WPARAM)0, (LPARAM)1 );
//	m_staticStatus.SetWindowText( L"Done." );
}


void CRBArchDesignDlg::OnBnClickedCheckMultiThread()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}
