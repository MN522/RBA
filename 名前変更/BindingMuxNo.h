#pragma once
#include "Solution.h"
#include "ListScheduleDFG.h"
#include "Floorplan.h"
#include "SimulatedAnnealing.h"

#define ARRAYINDEXEDLIST

typedef struct InterConnectionRec{
	// 演算器間のデータ通信接続要求
	int n;
	int nIndexFU;
	int nTypeFU;
#ifdef ARRAYINDEXEDLIST
	int nIndexInterconnectionTargetTop;
#else
	struct InterConnectionTargetRec *listTarget;
#endif
} INTERCONNECTION;

typedef struct InterConnectionTargetRec{
	// 演算器間データ通信接続先
	int n;
	int nIndexTargetFU;
	int nTypeFU;
	int nPipelineStages;
#ifdef ARRAYINDEXEDLIST
	int nIndexInterconnectionTargetNext;
	int nIndexRegGroupContainerTop;
#else
	struct InterConnectionTargetRec *next;
	struct RegGroupContainerRec *listRegGroupContainer;
#endif
} INTERCONNECTIONTARGET;

typedef struct RegGroupContainerRec{
	int n;
#ifdef ARRAYINDEXEDLIST
	int nIndexRegGroupTop;
	int nIndexRegGroupContainerNext;
#else
	struct RegGroupRec *listRegGroup;
	struct RegGroupContainerRec *next;
#endif
} REGGROUPCONTAINER;

typedef struct RegGroupRec{
	int n;
#ifdef ARRAYINDEXEDLIST
	int nIndexRegInstanceTop;
	int nIndexRegGroupNext;
#else
	struct RegInstanceRec *listRegInstance;
	struct RegGroupRec *next;
#endif
} REGGROUP;

typedef struct RegInstanceRec{
	int n;
	int *aBoundToReg;
#ifdef ARRAYINDEXEDLIST
	int nIndexRegToRegCommTop;
	int nIndexRegToFUCommTop;
	int nIndexRegInstanceNext;
#else
	struct RegToRegCommRec *listRegToRegComm;
	struct RegToFUCommRec *listRegToFUComm;
	struct RegInstanceRec *next;
#endif
} REGINSTANCE;

typedef struct RegToRegCommRec{
	int n;
#ifdef ARRAYINDEXEDLIST
	int nIndexRegInstance;
	int nIndexRegToRegCommNext;
#else
	struct RegInstanceRec *t;
	struct RegToRegCommRec *next;
#endif
} REGTOREGCOMM;

typedef struct RegToFUCommRec{
	int n;
#ifdef ARRAYINDEXEDLIST
	int nIndexFU;
	int nIndexRegToFUCommNext;
#else
	int nIndexFU;
	struct RegToRegFURec *next;
#endif
} REGTOFUCOMM;

typedef struct FPNodeRec{
	int nIndexAdd;	// 該当する演算器またはレジスタ群のインデックスのみ0以上の値、該当なければ-1
	int nIndexSub;
	int nIndexMul;
	int nIndexReg;
	struct FPEdgeRec *ilist,*olist;
} FPNODE;

typedef struct FPEdgeRec{
	struct FPNodeRec *f,*t;
	struct FPEdgeRec *ilist,*olist;
} FPEdge;

class CBinding :
	public CSolution
{
public:
	CBinding();
	~CBinding();

public:
	int RestoreBest();
	int SaveBest();
	int GenerateNeighbor();
	int RestoreNeighbor();
	int GetScore();
	int Initialize();

	double Evaluate(void);
	int m_nScore;

	void AnalyzeInterModuleCommunicationRequirementsPro(void);
	void AddCommunicationRequirementRegFUReg(int nIndexRegF, int nIndexModuleFU, int nFUtype, int nIndexRegT);
	void AddCommunicationRequirementFUToReg(int nIndexModuleFU, int nFUtype, int nIndexRegT);
	void AddCommunicationRequirementRegToFU(int nIndexRegF, int nIndexModuleFU, int nFUtype);
	void AddCommunicationRequirementRegToReg(int nIndexRegF, int nIndexRegT);
	void ClearCommunicationRequirementPro(void);
	void ShowCommunicationRequirement(void);
	//void makeMuxTree(MuxNode *ptr1,int num);
	int GetModuleIndexBoundToNode(int n);

	INTERCONNECTION *m_aInterFUConnection;
	void ClearCommPipelineRegisters(void);
	void AddCommPipelineRegisters(void);
	INTERCONNECTIONTARGET *GetElementInterconnectionTarget(void);
	REGGROUPCONTAINER *GetElementRegGroupContainer(void);
	REGGROUP *GetElementRegGroup(void);
	REGINSTANCE *GetElementRegInstance(void);
	REGTOREGCOMM *GetElementRegToRegComm(void);
	REGTOFUCOMM *GetElementRegToFUComm(void);
	//
	INTERCONNECTIONTARGET *m_poolInterconnectionTarget;
	int m_nCountPoolInterconnectionTarget;
	int m_nCountAllocatedInterconnectionTarget;
	REGGROUPCONTAINER *m_poolRegGroupContainer;
	int m_nCountPoolRegGroupContainer;
	int m_nCountAllocatedRegGroupContainer;
	REGGROUP *m_poolRegGroup;
	int m_nCountPoolRegGroup;
	int m_nCountAllocatedRegGroup;
	REGINSTANCE *m_poolRegInstance;
	int m_nCountPoolRegInstance;
	int m_nCountAllocatedRegInstance;
	REGTOREGCOMM *m_poolRegToRegComm;
	int m_nCountPoolRegToRegComm;
	int m_nCountAllocatedRegToRegComm;
	REGTOFUCOMM *m_poolRegToFUComm;
	int m_nCountPoolRegToFUComm;
	int m_nCountAllocatedRegToFUComm;

	void SetFUConfiguration(int nAdd, int nSub, int nMul);
	void Setup(void);
	void WriteBestBinding(BOOL bGlobalBest=FALSE);
	void WriteBestSchedule(BOOL bGlobalBest=FALSE);
	void WriteBestFloorplanPro(BOOL bGlobalBest=FALSE);
	void WriteBestFloorplanFinal(BOOL bGlobalBest=FALSE);
	void LoadBinding(void);
	void LoadFloorplanPro(void);
	void GetScorePhase1(void);
	void GetScorePhase2(void);
	void GetScorePhase3(void);
	void GetScorePhase4(void);
	void GetScorePhase4_1(void);
	void GetScorePhase5(void);
	void GetScorePhase6(void);
	void GetScorePhase7(void);

	void ListScheduleWithComminicationDelay(void);
	void AddRegistersFinal(void);
	void ClearRegistersFinal(void);
	void ClearCommunicationRequirementFinal(void);
	void AnalyzeInterModuleCommunicationRequirementsFinal(void);

	void TestAddRegisters(void);

	void SetDFG(CListScheduleDFG *pDFG){ m_pDFG = pDFG; }
	void SetRandomeSeed(unsigned long nRandomSeed){ m_nRandomSeed = nRandomSeed; }
	CListScheduleDFG *m_pDFG;
	int m_nID;
	unsigned long m_nRandomSeed;
	CFloorplan m_FloorplanPro;
	CFloorplan m_FloorplanFinal;
	CSimulatedAnnealing m_SAforFloorplanPro;
	CSimulatedAnnealing m_SAforFloorplanFinal;

	int m_nAdd;
	int m_nSub;
	int m_nMul;
	int m_nNodeAdd;
	int m_nNodeSub;
	int m_nNodeMul;
	int *m_aNodeAdd;
	int *m_aNodeSub;
	int *m_aNodeMul;
	int *m_aBinding;
	int *m_aBindingBest;
	int **m_aBoundToAdd;	// 各時刻における演算器割り当てを記録する配列
	int **m_aBoundToSub;
	int **m_aBoundToMul;

	int m_nNode1;
	int m_nNodeBinding1;
	int m_nNode2;
	int m_nNodeBinding2;

	int *m_aTranslateBindingFUIndexToModuleIndexAdd;
	int *m_aTranslateBindingFUIndexToModuleIndexSub;
	int *m_aTranslateBindingFUIndexToModuleIndexMul;

	int m_nProCountModuleAdd;
	int m_nProCountModuleSub;
	int m_nProCountModuleMul;
	int m_nProCountReg;
	int m_nProIndexModuleBaseAdd;
	int m_nProIndexModuleBaseSub;
	int m_nProIndexModuleBaseMul;
	int m_nProIndexBaseReg;

	COMREQUIREMENT *m_listCommunicationRequirement;
	COMREQUIREMENT *m_listCommunicationRequirementPro;
	COMREQUIREMENT *m_listCommunicationRequirementFinal;
	COMREQUIREMENT *m_listCommunicationRequirementMux;

	int *m_aPriority;
	int m_nClockCycles;
	int m_nClockPeriod;

	int m_nFinalReg;
	//自作
	#include <vector>
	bool is1;
	void makeMuxTree(MuxNode *ptr1, std::vector<int> aIn);

};

