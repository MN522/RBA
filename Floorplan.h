#pragma once
#include "SequencePair.h"
#include "ListScheduleDFG.h"

#define DURATION_ADD 10
#define DURATION_SUB 10
#define DURATION_MUL1 10
#define DURATION_MUL2 10
#define DELAY_WIRE_PER_LENGTH 1

typedef struct CommunicationRequirementRec {
	int nIndexModuleSource;
	int nIndexModuleDest;
	int nIndexModuleVia;
	int nDelayModuleSource;
	int nDelayModuleDest;
	int nDelayModuleVia;
	struct CommunicationRequirementRec *next;
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

