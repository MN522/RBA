// SimulatedAnnealing.cpp: CSimulatedAnnealing クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "SimulatedAnnealing.h"
#include "SAstatistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SAVE_STATISTICS

/*///////////////////////////////////////////////////////////
#define DEBUG_FILEOUT
///////////////////////////////////////////////////////////*/
#ifdef DEBUG_FILEOUT
#include <time.h>
#include "stdio.h"
void dmesg( char const *file, char const *fmt, ... )
{
	va_list va;
	va_start(va, fmt);
	char buf[1000];
	int n = vsprintf(buf, fmt, va);
//	int n = wvsprintf(buf, fmt, va);
//	lstrcat( buf, "\n" );
	FILE *fp = fopen( file, "a" );
/*	time_t ltime;
	time( &ltime );
	fprintf( fp, "%s ", ctime( &ltime ) );*/
	fprintf( fp, buf );

	fclose( fp );
}
#else
#define dmesg 1?(void)0:
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CSimulatedAnnealing::CSimulatedAnnealing()
{
	m_bStatistics = FALSE;
	m_bMaximizeScore = FALSE;
#if 0
	unsigned long nRandomSeed = (unsigned)time(NULL);
//	nRandomSeed = 1136795343;
//	nRandomSeed = 1249195978;
//	nRandomSeed = 1279034870;
	TRACE( "nRandomSeed = %d\n", nRandomSeed );
	dmesg( "SA.log", "nRandomSeed = %d\n", nRandomSeed );
	srand(nRandomSeed);		/*乱数の初期化*/
	/*//////////////////////////////////////////
	FILE *fp;
	fopen_s(&fp,"result.txt","w");
	fprintf( fp, "nRandomSeed = %d\n", nRandomSeed );
	fclose(fp);
	//////////////////////////////////////////*/
#endif
	m_hWnd = NULL;
}

CSimulatedAnnealing::~CSimulatedAnnealing()
{

}

void CSimulatedAnnealing::SetParameters(double Tstart, double Tend, double Trate, int N, BOOL bMaximizeScore)
{
	m_Tstart = Tstart;
	m_Tend = Tend;
	m_Trate = Trate;
	m_N = N;
	m_bMaximizeScore = bMaximizeScore;
}

void CSimulatedAnnealing::Initialize(void)
{
	m_pSolution->Initialize();
}

#define PROB_FACTOR 1000
int CSimulatedAnnealing::Execute()
{
	double t;
	int n;
	int nBestScore = -1;
	int nPrevScore = -1;
	int nScore;
	int nProb;
	int bUpdate;
#ifdef SAVE_STATISTICS
	CSAstatistics sas;
	CString strStatFilename( L"statistics.csv" );
	if( m_bStatistics ) if( sas.SetOutFilename( strStatFilename ) != 1 ) return 2;
#endif

/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CSimulatedAnnealing::Execute m_Tstart=%f, m_Tend=%f\n", m_Tstart, m_Tend );
fclose(fp);}*/
	// 初期解のスコアを求め、これを nPrevScore とする
	nPrevScore = m_pSolution->GetScore();

	for( t=m_Tstart ; t>=m_Tend ; t*=m_Trate ){
//		t = m_Tend;
		/*if( m_nID == 0 ){FILE *fp;
		fopen_s(&fp,"status.txt","a");
		fprintf(fp, "%d t=%.3f\n", m_nID, t );
		fclose(fp);}*/
		if( m_nID == 0 ) TRACE( "%d t=%f =====================================================\n", m_nID, t );
#ifdef DEBUG_FILEOUT
		char buf[64];
		sprintf( buf, "t=%g =====================================================\n", t );
		dmesg( "SA.log", buf );
#endif
/*////////////////////////////////////////////////////////////////////////////////
		if( t < 50 ) break;/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////*/
#ifdef SAVE_STATISTICS
		if( m_bStatistics ) sas.SetTemerature( t );
		if( m_bStatistics ) sas.AddScore( nPrevScore );
#endif
		if( m_hWnd ) ::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)nBestScore );
		for( n=0 ; n<m_N ; n++ ){
/*{FILE *fp=fopen("result.txt","a");
if( n%10 == 0 ){
fprintf(fp, " t=%f, n=%d\n", t, n );
}
fclose(fp);}*/
			m_pSolution->GenerateNeighbor();
/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CSimulatedAnnealing::Execute GenerateNeighbor done\n" );
fclose(fp);}*/
			nScore = m_pSolution->GetScore();
/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CSimulatedAnnealing::Execute GetScore t=%f, n=%d done\n", t, n );
fclose(fp);}*/
			bUpdate = 0;
//			if( nPrevScore < 0 || 
//			! m_bMaximizeScore && nScore < nPrevScore ||
			if( ! m_bMaximizeScore && nScore < nPrevScore ||
				m_bMaximizeScore && nScore > nPrevScore ){
				// より良い解が得られた
				// この解を採用
				bUpdate = 1;
				nProb = -1;
				/*if( m_nID == 0 ){FILE *fp;
				fopen_s(&fp,"status.txt","a");
				fprintf(fp, "%d n=%d P=%d S=%d B=%d\n", m_nID, n, nPrevScore, nScore, nBestScore );
				fclose(fp);}*/
			}else{
				// より悪い解が得られた --> nPrevScore-nScore <= 0
				// 確率計算
				nProb = exp( (nPrevScore-nScore)/t )*PROB_FACTOR;
				int nRand = rand()%PROB_FACTOR;
//				TRACE( "nPrevScore=%d nScore=%d nProb=%d, nRand=%d PROB_FACTOR=%d\n", nPrevScore, nScore, nProb, nRand, PROB_FACTOR );
				if( nRand < nProb ){
					bUpdate = 1;
					/*if( m_nID == 0 ){FILE *fp;
					fopen_s(&fp,"status.txt","a");
					fprintf(fp, "%d n=%d P=%d S=%d B=%d nProb=%d, nRand=%d \n", m_nID, n, nPrevScore, nScore, nBestScore, nProb, nRand );
					fclose(fp);}*/
				}
			}
			if( bUpdate ){
				// 変更を採用
#ifdef SAVE_STATISTICS
				if( m_bStatistics ) sas.AddScore( nScore );
#endif
//				TRACE( "Score %d -> %d (n=%d, nProb=%d)\n", nPrevScore, nScore, n, nProb );
				nPrevScore = nScore;
				if( nBestScore < 0 || 
					! m_bMaximizeScore && nScore < nBestScore ||
					m_bMaximizeScore && nScore > nBestScore ){
//					TRACE( "Best Score %d -> %d (Area=%d) (n=%d)\n", nBestScore, nScore, m_pSequencePair->GetArea(), n );
//					TRACE( "Best Score %d -> %d (n=%d, nProb=%d)\n", nBestScore, nScore, n, nProb );
						/*{FILE *fp;
						fopen_s(&fp,"status.txt","a");
						//fprintf(fp, "%d T=%.3f n=%d P=%d S=%d B=%d A\n", m_nID, t, n, nPrevScore, nScore, nBestScore );
						fprintf(fp, "%d n=%d P=%d S=%d B=%d A\n", m_nID, n, nPrevScore, nScore, nBestScore );
						fclose(fp);}
						if( nScore < 0 ) exit(1);/////////////////////////////
						*/
						m_pSolution->SaveBest();
/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CSimulatedAnnealing::Execute SaveBest done\n" );
fclose(fp);}*/
					nBestScore = nScore;
				}
			}else{
				// 変更を棄却
				m_pSolution->RestoreNeighbor();
/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CSimulatedAnnealing::Execute RestoreNeighbor done\n" );
fclose(fp);}*/
			}
		}
#ifdef SAVE_STATISTICS
		if( m_bStatistics ) sas.WriteResult( m_N );
#endif
/*////////////////////////////////////////////////////////////////////////////////
		if( t < 8 ) break;/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////*/
		//if( m_nID == 0 ) break;
	}
	if( m_hWnd ) ::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)nBestScore );
	if( m_nID == 0 ) TRACE( "%d done =====================================================\n", m_nID, t );
#ifdef SAVE_STATISTICS
	if( m_bStatistics ) sas.Finish();
#endif
	return 1;
}
