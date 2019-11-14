
// RBArchDesignDlg.h : ヘッダー ファイル
//

#pragma once
#include "afxwin.h"
#include "Design.h"

#define WM_NEW_TEMPERATURE	(WM_USER+1)
#define WM_EXECUTION_DONE	(WM_USER+2)


// CRBArchDesignDlg ダイアログ
class CRBArchDesignDlg : public CDialogEx
{
// コンストラクション
public:
	CRBArchDesignDlg(CWnd* pParent = NULL);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RBARCHDESIGN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	CString m_strFilename;
/*	CEdit	m_editTempStart;
	CEdit	m_editTempEnd;
	CEdit	m_editTempRate;
	CEdit	m_editIterate;
*/
	double m_dTstartBinding, m_dTendBinding, m_dRateBinding;
	double m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro;
	double m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal;
	int m_nIterateBinding;
	int m_nIterateFloorplanPro;
	int m_nIterateFloorplanFinal;

	time_t m_timeCurrent;
	unsigned short m_millitmCurrent;
	CWinThread *m_pExecutingThread;

	CDesign m_design;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonExecute();
	afx_msg void OnBnClickedButtonChooseFile();
	afx_msg LRESULT OnNewTemperature(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExecutionDone(WPARAM wParam, LPARAM lParam);
	void Execute(void);
	void TestFinal(void);

	CString m_strTimeStart;
	CString m_strTimeFinish;
	CString m_strTimeCurrent;
	time_t m_timeStart;

	CString m_strTempCurrent;
	CString m_strScoreMin;
	CString m_strScoreMinGlobal;
	CString m_strCPUTime;
	CString m_strCPUTimeEstimate;
	CStatic m_staticStatus;
	int m_nAdd;
	int m_nSub;
	int m_nMul;
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonTestFinal();
	BOOL m_bMultiThread;
	afx_msg void OnBnClickedCheckMultiThread();
};
