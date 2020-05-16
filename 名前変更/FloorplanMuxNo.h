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
	int nIn1Module = -1;		//���͂���Module�ԍ�(���W�X�^or���Z��)
	int nIn2Module = -1;		//-1�̂Ƃ��͂Ȃ����Ă��Ȃ�
	int nOutModule = -1;		//�o�͂���Module�ԍ�(���W�X�^or���Z��)
	int id;
	struct MuxRec *In1Mux=NULL;	// ���͂���MUX�̃A�h���X
	struct MuxRec *In2Mux=NULL;	//
	struct MuxRec *outMux = NULL;//�o�͂���Module(MUX)�A�h���X

} MuxNode;

typedef struct CountInModulRec {
	int DestModuleName;		//�ΏۂƂ���DestModule
	int  nCountModule ;		//���͂���Module��(���W�X�^or���Z��)
	std::vector<int> aInModule;//���͂���Module�ԍ������߂��z��

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

