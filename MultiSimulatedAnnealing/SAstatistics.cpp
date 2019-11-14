// SAstatistics.cpp: CSAstatistics クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAstatistics.h"

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CSAstatistics::CSAstatistics()
{

}

CSAstatistics::~CSAstatistics()
{

}

int CSAstatistics::SetOutFilename(CString strFilename)
{
	if( _wfopen_s( &m_fp, strFilename, L"w") ) return 0;
	return 1;
}

void CSAstatistics::SetTemerature(double fTemp)
{
	m_fTemp = fTemp;
	m_nTotalScore = 0;
	m_N = 0;
}

void CSAstatistics::AddScore(int nScore)
{
	m_nTotalScore += nScore;
	if( m_N == 0 ){
		m_nMaxScore = m_nMinScore = nScore;
	}else{
		if( m_nMaxScore < nScore ) m_nMaxScore = nScore;
		if( m_nMinScore > nScore ) m_nMinScore = nScore;
	}
	m_N++;
}

int CSAstatistics::WriteResult(int N)
{
	if( m_N > 0 ){
		m_nPrevTotalScore = m_nTotalScore/m_N;
	}
	fprintf( m_fp, "%f,%d,%d,%d\n", m_fTemp, m_nPrevTotalScore, m_nMaxScore, m_nMinScore );
	return 1;
}

int CSAstatistics::Finish()
{
	return fclose( m_fp );
}
