// SAstatistics.h: CSAstatistics クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SASTATISTICS_H__8292207A_29A0_43C7_A7B4_5E1237AFFEAC__INCLUDED_)
#define AFX_SASTATISTICS_H__8292207A_29A0_43C7_A7B4_5E1237AFFEAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSAstatistics  
{
public:
	CSAstatistics();
	virtual ~CSAstatistics();

public:
	int Finish();
	int WriteResult( int N );
	void AddScore( int nScore );
	void SetTemerature( double fTemp );
	int SetOutFilename( CString strFilename );

private:
	int m_nTotalScore;
	int m_nPrevTotalScore;
	int m_nMaxScore;
	int m_nMinScore;
	int m_N;
	double m_fTemp;
	FILE *m_fp;

};

#endif // !defined(AFX_SASTATISTICS_H__8292207A_29A0_43C7_A7B4_5E1237AFFEAC__INCLUDED_)
