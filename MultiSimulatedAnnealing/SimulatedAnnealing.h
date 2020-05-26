// SimulatedAnnealing.h: CSimulatedAnnealing クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMULATEDANNEALING_H__BE89992E_30EF_4218_9FED_3616823C33F0__INCLUDED_)
#define AFX_SIMULATEDANNEALING_H__BE89992E_30EF_4218_9FED_3616823C33F0__INCLUDED_

#include "Solution.h"	// ClassView によって追加されました。
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSimulatedAnnealing  
{
public:
	CSimulatedAnnealing();
	virtual ~CSimulatedAnnealing();

public:
	int Execute();
	void RegisterSolution( CSolution *pSolution ){
		m_pSolution = pSolution;
	}
	void SetParameters( double Tstart, double Tend, double Trate, int N, BOOL bMaxmizeScore=FALSE );
	void Initialize(void);
	void SetWindowHandle( HWND hWnd ){ m_hWnd = hWnd; }
	void EnableStatistics(void){ m_bStatistics = TRUE; }
	void SetID(int nID){ m_nID = nID; }
	//自作
	void KARI();
private:
	CSolution *m_pSolution;
	double m_Tstart;
	double m_Tend;
	double m_Trate;
	int m_N;
	BOOL m_bMaximizeScore;
	BOOL m_bStatistics;
	int m_nID;

	HWND m_hWnd;

};

#endif // !defined(AFX_SIMULATEDANNEALING_H__BE89992E_30EF_4218_9FED_3616823C33F0__INCLUDED_)
