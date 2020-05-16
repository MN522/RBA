#pragma once
#include "SequencePair.h"
#include "ListScheduleDFG.h"
#include <vector>

#define DURATION_ADD 10
#define DURATION_SUB 10
#define DURATION_MUL1 10
#define DURATION_MUL2 10
#define DELAY_WIRE_PER_LENGTH 1


typedef struct MuxRec {
	int nIn1Module = -1;		//入力するModule番号(レジスタor演算器)
	int nIn2Module = -1;		//-1のときはつながっていない
	int nOutModule = -1;		//出力するModule番号(レジスタor演算器)
	int id;
	struct MuxRec *In1Mux=NULL;	// 入力するMUXのアドレス
	struct MuxRec *In2Mux=NULL;	//
	struct MuxRec *outMux = NULL;//出力するModule(MUX)アドレス

} MuxNode;

typedef struct CountInModulRec {
	int DestModuleName;		//対象とするDestModule
	int  nCountModule ;		//入力するModule数(レジスタor演算器)
	std::vector<int> aInModule;//入力するModule番号を収めた配列

} InModule;

typedef struct CommunicationRequirementRec {
	int nIndexModuleSource;
	int nIndexModuleDest;
	int nIndexModuleVia;
	int nDelayModuleSource;
	int nDelayModuleDest;
	int nDelayModuleVia;
	struct CommunicationRequirementRec *next;
	int  *nIndexmux;
	int   nDelaymux;
} COMREQUIREMENT;

class CFloorplan :
	public CSequencePair
{
public:
	CFloorplan();
	~CFloorplan();

public:
	int GetScore();
	int GetScorePro();
	int GetScoreFinal();

	int m_nModeProFinal;
	void SetMode(int nMode){ m_nModeProFinal = nMode; }
	void GetCoordinateModuleTerminal(int nIndexModule, int nInOut, int *pnX, int *pnY);
	int CalcWireDelay(int nX1, int nY1, int nX2, int nY2);
	void SetCommunicationRequirement(COMREQUIREMENT *listCommunicationRequirement){m_listCommunicationRequirement = listCommunicationRequirement;}

	void SetDFG( CListScheduleDFG *pDFG ){ m_pDFG = pDFG; }

	CListScheduleDFG *m_pDFG;
	COMREQUIREMENT *m_listCommunicationRequirement;
};

