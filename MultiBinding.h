#pragma once
#include "MultiSolution.h"
#include "ListScheduleDFG.h"
#include "Binding.h"

class CMultiBinding :
	public CMultiSolution
{
public:
	CMultiBinding();
	~CMultiBinding();

public:
	int GenerateNeighbor();
	int GenerateNeighbors( double dAcceptRatio, int aParent[] );
	int RestoreNeighbor( int nIndex );
	int GetScore( double aScore[] );
	int SaveBest( int nIndex );
	//	int RestoreBest();
	int SaveGlobalBest( int nIndex );
	int GetNumSolutions( void ){ return m_nNumSolutions; }
	void AcceptNeighbor( int nIndex );

	THREAD_TOKEN *m_aThreadToken;
	HANDLE *m_hEvent;
	HANDLE *m_hEvent2;
	CListScheduleDFG **m_pDFGCore;
	CBinding **m_aBindingCore;
	CWinThread **m_pThread;
	int m_nNumSolutions;
	void GlobalInit(int nNumthreads);
//	double Evaluate(void);

	void SetDFG(CListScheduleDFG *pDFG){ m_pDFG = pDFG; }
	CListScheduleDFG *m_pDFG;

	int Initialize(void);
	void SetFUConfiguration(int nAdd, int nSub, int nMul);
	void SetParamSAforFloorplanPro(double dTstart, double dTend, double dRate, int nIterate);
	void SetParamSAforFloorplanFinal(double dTstart, double dTend, double dRate, int nIterate);
	double m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro;
	double m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal;
	int m_nIterateFloorplanPro;
	int m_nIterateFloorplanFinal;

	int N;
	int m_nAdd;
	int m_nSub;
	int m_nMul;
	int m_nNodeAdd;
	int m_nNodeSub;
	int m_nNodeMul;
	int *m_aNodeAdd;
	int *m_aNodeSub;
	int *m_aNodeMul;

	int *m_aBindingRoot;
	int *m_aBinding;
	int *m_aBindingBest;
	int *m_aBindingGlobalBest;

	int m_nClockCyclesGlobalBest;
	int m_nClockPeriodGlobalBest;

	int m_nNode1;
	int m_nNodeBinding1;
	int m_nNode2;
	int m_nNodeBinding2;
	int *m_aNode1;
	int *m_aNodeBinding1;
	int *m_aNode2;
	int *m_aNodeBinding2;

};

