#pragma once

typedef struct MyThreadToken {
	int nID;
	void *pObject;
	int nCommand;
	HANDLE hEvent;
	HANDLE hEvent2;
	double dScore;
} THREAD_TOKEN;

class CMultiSolution
{
public:
	CMultiSolution(void);
	~CMultiSolution(void);

public:
	virtual int Initialize(void){ return 1; }
	virtual int GenerateNeighbors( double dAcceptRatio, int aParent[] ){ return 1; }
	virtual int RestoreNeighbor( int nIndex ){ return 1; }
	virtual int GetScore( double aScore[] ){ return 0; }
	virtual int SaveBest( int nIndex ){ return 1; }
	virtual int RestoreBest( void ){ return 1; }
	virtual int SaveGlobalBest( int nIndex ){ return 1; }
	virtual int GetNumSolutions( void ){ return 0; }
	virtual void AcceptNeighbor( int nIndex ){};
};

