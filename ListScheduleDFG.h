#pragma once
#include "DFG.h"

class CListScheduleDFG :
	public CDFG
{
public:
	CListScheduleDFG(void);
	~CListScheduleDFG(void);

	int CalcNodePriority(void);
	int GetEndTime( void ){ return m_nEndTime; }
	void SetFUConfig(int nMaxAdd, int nMaxSub, int nMaxMul);
	CString GetResult( void ){ return m_strResult; }

public:
	int m_nDeadLine;
	int m_nEndTime;
	int ListSchedule(void);
	void ListScheduleInit(void);
	void FixNode(int n, int nTime);
	int *m_aPriority;	// リストスケジューリングのためのノード優先順に基づいてノードの順序を記録する配列
	CString m_strResult;

	int m_nMaxAdd;		// 加算器(add)の指定個数
	int m_nMaxSub;
	int m_nMaxMul;
	int *m_aAllocatedToAdd;	// 各時刻における加算器(add)利用数を記録する配列
	int *m_aAllocatedToSub;
	int *m_aAllocatedToMul;

	void Init( int nDeadLine );
	void InitRealloc( int nDeadLine );
	void ShowSchedule(void);
	void ShowScheduleVertical(void);
	void ShowScheduleAssignment(int *aPindex, int *aRename, int *pnDeadLine);
	int IdentifyOperation(int n, int **aAllocatedToFU, int *nMaxFU, int *nLatency, int *nLen);
	int CanAllocateToFU( int t, int *aAllocatedToFU, int nLen, int nMaxFU );
	void AllocateToFU(int n, int t, int nLen, int *aAllocatedToFU);
	void FixInputNode(void);
	void ShowAssignments(void);
};
