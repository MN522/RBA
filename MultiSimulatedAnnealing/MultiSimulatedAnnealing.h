// SimulatedAnnealing.h: CSimulatedAnnealing クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#include "MultiSolution.h"	// ClassView によって追加されました。
#pragma once

#define ACCEPT_HISTRY_LENGTH 100

class CMultiSimulatedAnnealing  
{
public:
	CMultiSimulatedAnnealing();
	virtual ~CMultiSimulatedAnnealing();

private:
	CMultiSolution *m_pSolution;
	double m_Tstart;
	double m_Tend;
	double m_Trate;
	int m_N;
	BOOL m_bMaximizeScore;

	HWND m_hWnd;
	int m_nID;

	int m_aHistoryAccepted[ACCEPT_HISTRY_LENGTH];
	int m_nCountAccepted;
	int m_nCountHistory;
	int m_nIndexHistoryWrite;

public:
	int Execute();
	void RegisterSolution( CMultiSolution *pSolution ){
		m_pSolution = pSolution;
	}
	void SetParameters( double Tstart, double Tend, double Trate, int N, BOOL bMaxmizeScore=FALSE );
	void Initialize();
	void SetWindowHandle( HWND hWnd ){ m_hWnd = hWnd; }
	void StoreJudgeResult(int bAccept);
	double GetAcceptRatio(void);
//	int JudgeNeighbor(double dPrevScore, double dScore, double t);
	void SetID(int nID){ m_nID = nID; }


};

