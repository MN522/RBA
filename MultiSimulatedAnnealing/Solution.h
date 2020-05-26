// Solution.h: CSolution クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOLUTION_H__D60D0D32_5FFB_4211_80E4_1A854B5E9203__INCLUDED_)
#define AFX_SOLUTION_H__D60D0D32_5FFB_4211_80E4_1A854B5E9203__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSolution  
{
public:
	CSolution();
	virtual ~CSolution();

public:
	virtual int Initialize( void ){ return 1; }
	virtual int GenerateNeighbor( void ){ return 1; }
	virtual int RestoreNeighbor( void ){ return 1; }
	virtual int GetScore( void ){ return 0; }
	virtual int SaveBest( void ){ return 1; }
	virtual int RestoreBest( void ){ return 1; }
	virtual int LookPalameter(void) { return 1; }
};

#endif // !defined(AFX_SOLUTION_H__D60D0D32_5FFB_4211_80E4_1A854B5E9203__INCLUDED_)
