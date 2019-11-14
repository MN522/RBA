#pragma once
#include "Solution.h"

typedef struct LongestPathNodeRec{
	int n;
	int w;
	int nLength;
	int nCountNotFixedIn;
	int nFixed;
	struct LongestPathEdgeRec *olist;
} LPNODE;
typedef struct LongestPathEdgeRec{
	struct LongestPathNodeRec *t;
	struct LongestPathEdgeRec *olist;
} LPEDGE;

class CSequencePair : public CSolution
{
public:
	typedef struct moduleRec {
		int nWidth, nHeight;
		int nOrientation;
		int x, y;
		int nUnitType;
	} MODULE;
	typedef struct unitRec {
		int nUnitName;
		int nWidth, nHeight;
	} UNIT;

public:
	CSequencePair(void);
	~CSequencePair(void);

public:
	int RestoreBest();
	int SaveBest();
	int GenerateNeighbor();
	int RestoreNeighbor();
	int GetScore();
	int Initialize();

	void Clear(void);
	int GetModuleNum(void){ return m_nModule; }
	int GetModuleStatus( int m, int *x, int *y, int *w, int *h, int *nOrientation );
	void SetModuleStatus(int m, int x, int y, int w, int h, int nOrientation);
	int RotateModule();
	int SwapGammas();
	int SwapGammaPlus();
	int SwapGammaMinus();
	void RestoreGammaPlus();
	void RestoreGammaMinus();
	void RestoreModuleRotate();
	int GetArea();
	int Pack();
	void ComputeLongestPath();
	int InitGamma();
	int AddModule( int nWidth, int nHeight );
	int AddModule( int nUnitType );
	int AddUnitType( int nWidth, int nHeight );
	int bShowFlag;
	int RetrieveScore(){ return m_nScore; }
/*
	virtual int Initialize( void ){ return 1; }
	virtual int GenerateNeighbor( void ){ return 1; }
	virtual int RestoreNeighbor( void ){ return 1; }
	virtual int GetScore( void ){ return 0; }
	virtual int SaveBest( void ){ return 1; }
	virtual int RestoreBest( void ){ return 1; }
*/

public:
	MODULE *m_Module;
	UNIT *m_Unit;
	int m_nModule;
	int m_nUnit;
	int *m_GammaPlus;
	int *m_GammaMinus;
	int *m_GammaMinusOrder;
	int *m_BestGammaPlus;
	int *m_BestGammaMinus;
	int *m_BestModuleRotation;
	int m_nScore;
	int m_nCount;
	int m_nModeGenerateNeighbor;
	int m_nGammaPlusIndex1;
	int m_nGammaPlusIndex2;
	int m_nGammaMinusIndex1;
	int m_nGammaMinusIndex2;
	int m_nModuleOrientation;

	LPNODE *m_node;
	LPEDGE *m_elist;
	LPEDGE **mEdgeStack;
	LPNODE **mNodeStack;

};

