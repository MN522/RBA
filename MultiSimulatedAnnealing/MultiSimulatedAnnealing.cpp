// MultiSimulatedAnnealing.cpp: CMultiSimulatedAnnealing クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "MultiSimulatedAnnealing.h"
#include "SAstatistics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SAVE_STATISTICS
#define SAVE_PARALLEL_STATISTICS

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
	int n = wvsprintf(buf, fmt, va);
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

CMultiSimulatedAnnealing::CMultiSimulatedAnnealing()
{
	m_bMaximizeScore = FALSE;
#if 0
	unsigned long nRandomSeed = (unsigned)time(NULL);
//	nRandomSeed = 1394690204;
	TRACE( "nRandomSeed = %d\n", nRandomSeed );
	dmesg( "SA.log", "nRandomSeed = %d\n", nRandomSeed );
//////////////////////////////////////////////////////////////////////
	FILE *fp;
	fopen_s( &fp, "Result.txt", "a" );
	fprintf( fp, "nRandomSeed = %d;\n", nRandomSeed );
	fclose(fp);
/////////////////////////////////////////////////////////////////////*/
	srand(nRandomSeed);		/*乱数の初期化*/
#endif
	m_hWnd = NULL;
}

CMultiSimulatedAnnealing::~CMultiSimulatedAnnealing()
{
}

void CMultiSimulatedAnnealing::SetParameters(double Tstart, double Tend, double Trate, int N, BOOL bMaximizeScore)
{
	m_Tstart = Tstart;
	m_Tend = Tend;
	m_Trate = Trate;
	m_N = N;
	m_bMaximizeScore = bMaximizeScore;
}

#define NUM_MULTIPLE_SOLUTION	4
void CMultiSimulatedAnnealing::Initialize(void)
{
	m_pSolution->Initialize();
	m_nCountHistory = 0;
	m_nIndexHistoryWrite = 0;
	m_nCountAccepted = 0;
}

#define PROB_FACTOR 1000
int CMultiSimulatedAnnealing::Execute()
{
	double t;
	int n;
	double dGlobalBestScore = -1.0;
	double dBestScore = -1.0;
	double dPrevScore = -1.0;
	double dScore;
	int nProb;
	int bUpdate;
	int nIndex;
	int nIndexLastAccepted;
	int nIndexLastRejected;
	int nIndexParent;
	double dPrevScore0;
	double *aScore;
	int *aParent;
	double dAcceptRatio;
	int nNumSolutions;
	int nNumSolutionsValid;
#ifdef SAVE_STATISTICS
	CSAstatistics sas;
	CString strStatFilename( L"statistics.csv" );
	if( sas.SetOutFilename( strStatFilename ) != 1 ) return 2;
#endif
	int nTotalSolutionsValid;
	int nTotalSolutionsAccepted;
	int nTotalSolutionsAccepted2;
	int aCountPattern[4];
	int aCountSolutionsValid[4];
	int nPattern;
#ifdef SAVE_PARALLEL_STATISTICS
	FILE *fp;
	fopen_s( &fp, "accept.csv", "w" );
	fprintf( fp, "t,accept,accept2,solutions,rate\n" );
	FILE *fp3;
	fopen_s( &fp3, "pattern.csv", "w" );
	fprintf( fp3, "t,p0,p1,p2,p3,c0,c1,c2,c3\n" );
#endif
	double aScoreArray[2];

	nNumSolutions = m_pSolution->GetNumSolutions();
	aParent = (int *)GlobalAlloc( GPTR, nNumSolutions*sizeof(int) );
	aScore = (double *)GlobalAlloc( GPTR, nNumSolutions*sizeof(double) );
	/////////////////////////////////////////////
	int *aUsed = (int *)GlobalAlloc( GPTR, nNumSolutions*sizeof(int) );
	///////////////////////////////////////////////////////
	// 初期解
	dPrevScore0 = -1;
	dAcceptRatio = 1.0;
	///////////////////////////////////////////////////////

	for( t=m_Tstart ; t>=m_Tend ; t*=m_Trate ){
		/*/////////////////////////////////////////////////////
		FILE *fp2;
		fopen_s( &fp2, "progress.txt", "a" );
		fprintf( fp2, "t=%g =====================================================\n", t );
		fclose(fp2);
		/////////////////////////////////////////////////////*/
		nTotalSolutionsValid = 0;
		nTotalSolutionsAccepted = 0;
		nTotalSolutionsAccepted2 = 0;
		for( int m=0 ; m<4 ; m++ ){
			aCountPattern[m] = 0;
			aCountSolutionsValid[m] = 0;
		}
//		t = m_Tend;
		TRACE( "t=%f =====================================================\n", t );
#ifdef DEBUG_FILEOUT
		char buf[64];
		sprintf( buf, "t=%g =====================================================\n", t );
		dmesg( "SA.log", buf );
#endif
/*////////////////////////////////////////////////////////////////////////////////
		if( t < 50 ) break;/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////*/
#ifdef SAVE_STATISTICS
		sas.SetTemerature( t );
#endif
		if( m_hWnd ){
			aScoreArray[0] = dBestScore;
			aScoreArray[1] = dGlobalBestScore;
			::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)aScoreArray );
//			::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)(int)dBestScore );
		}
		for( n=0 ; n<m_N ; n += nNumSolutionsValid ){
			// 近傍解生成パターンを決定
			nPattern = m_pSolution->GenerateNeighbors( dAcceptRatio, aParent );
			m_pSolution->GetScore( aScore );
			aCountPattern[nPattern]++;
			//////////////////////////////////////////////////////
			for( int k=0 ; k<nNumSolutions ; k++ ) aUsed[k] = -1;
			TRACE( "Accept ratio=%f\n", dAcceptRatio );
			TRACE( "Parent: " );
			for( int k=0 ; k<nNumSolutions ; k++ ){
				TRACE( "%d ", aParent[k] );
			}
			TRACE( "\n" );
			TRACE( "Score: " );
			for( int k=0 ; k<nNumSolutions ; k++ ){
				TRACE( "%f ", aScore[k] );
			}
			TRACE( "\n" );
			/////////////////////////////////////////////////////*/
			nNumSolutionsValid = 0;
			// スコアを分析して近傍解の受理/棄却を決める
			TRACE( " PrevScore0=%.1f\n", dPrevScore0 );
			nIndex = -1;
			nIndexParent = -1;
			while(1){
				// 親がnIndexParentの解を探す
				for( nIndex++ ; nIndex<nNumSolutions ; nIndex++ ){
					if( aParent[nIndex] == nIndexParent ) break;
				}
				if( nIndex >= nNumSolutions ){
					// 全ての解を調べた
					break;
				}
				nNumSolutionsValid++;
				nTotalSolutionsValid++;
				aCountSolutionsValid[nPattern]++;
				if( nIndexParent < 0 ){
					dPrevScore = dPrevScore0;	// root解のスコア
				}else{
					dPrevScore = aScore[nIndexParent];
				}
				dScore = aScore[nIndex];
				bUpdate = 0;
				TRACE( " %d: PrevScore=%.1f(%d), Score=%.1f", nIndex, dPrevScore, aParent[nIndex], dScore );
				if( dPrevScore < 0 || 
					! m_bMaximizeScore && dScore < dPrevScore ||
					m_bMaximizeScore && dScore > dPrevScore ){
					// より良い解が得られた
					// この解を採用
					bUpdate = 1;
					nProb = -1;
					nTotalSolutionsAccepted++;
					/*/////////////////////////////////////////////////////
					fopen_s( &fp2, "progress.txt", "a" );
					fprintf( fp2, "%d: PrevScore=%f -> Score=%f\n", n, dPrevScore, dScore );
					fclose(fp2);
					/////////////////////////////////////////////////////*/
				}else{
					// より悪い解が得られた --> nPrevScore-nScore <= 0
					// 確率計算
					nProb = exp( (dPrevScore-dScore)/t )*PROB_FACTOR;
					int nRand = rand()%PROB_FACTOR;
	//				TRACE( "nPrevScore=%d nScore=%d nProb=%d, nRand=%d PROB_FACTOR=%d\n", nPrevScore, nScore, nProb, nRand, PROB_FACTOR );
					if( nRand < nProb ){
						bUpdate = 1;
						nTotalSolutionsAccepted2++;
					}
					/*/////////////////////////////////////////////////////
					fopen_s( &fp2, "progress.txt", "a" );
					fprintf( fp2, "%d: PrevScore=%f :: Score=%f nProb=%d, nRand=%d %s\n", n, dPrevScore, dScore, nProb, nRand, bUpdate?"Accept":"Reject" );
					fclose(fp2);
					/////////////////////////////////////////////////////*/
				}
				TRACE( " %s\n", bUpdate?"accept":"reject" );
				aUsed[nIndex] = bUpdate;	///////////////////////////////////
				if( bUpdate ){
					// 変更を採用
#ifdef SAVE_STATISTICS
					sas.AddScore( (int)dScore );
#endif
//					TRACE( L"Score %g -> %g (n=%d, nProb=%d)\n", dPrevScore, dScore, n, nProb );
					StoreJudgeResult( 1 );	// accepted
					nIndexLastAccepted = nIndex;
					// 最善解を記録
					if( dBestScore < 0 || 
						! m_bMaximizeScore && dScore < dBestScore ||
						m_bMaximizeScore && dScore > dBestScore ){
//						TRACE( "Best Score %g -> %g (n=%d, nProb=%d)\n", dBestScore, dScore, n, nProb );
						m_pSolution->SaveBest( nIndex );
						dBestScore = dScore;
					}
					nIndexParent = nIndex;	// 次は、この解を親に持つ解を探す
				}else{
					// 変更を棄却
					StoreJudgeResult( 0 );	// rejected
					nIndexLastRejected = nIndex;
					// 次は、同じ親を持つ兄弟解を探す
				}
			}
			if( bUpdate ){
				// 解nIndexが受理された
				TRACE( "Accept solution %d\n", nIndexLastAccepted );
				m_pSolution->AcceptNeighbor( nIndexLastAccepted );
				dPrevScore0 = aScore[nIndexLastAccepted];
			}else{
				// 解が棄却され、解nIndexの親に戻して次のrootとする
				TRACE( "Restore solution to %d(%d)(nIndex=%d) \n", nIndexParent, aParent[nIndexLastRejected], nIndexLastRejected );
				m_pSolution->RestoreNeighbor( nIndexLastRejected );
				if( aParent[nIndexLastRejected] >= 0 ) dPrevScore0 = aScore[aParent[nIndexLastRejected]];
			}
			dAcceptRatio = GetAcceptRatio();
			TRACE( "Solution: " );
			for( int k=0 ; k<nNumSolutions ; k++ ){
				TRACE( "%d(%c) ", k, aUsed[k]==1?'o':aUsed[k]==0?'x':'-' );
			}
			TRACE( "\n" );
			/*/////////////////////////////////////////////////
			FILE *fp;
			fopen_s( &fp, "glpk64/edges.txt", "a" );
			for( int k=0 ; k<nNumSolutions ; k++ ){
				fprintf( fp, "%d(%c) ", k, aUsed[k]==1?'o':aUsed[k]==0?'x':'-' );
			}
			fprintf( fp, "\n" );
			fclose( fp );
			/////////////////////////////////////////////////*/
			for( nIndex=0 ; nIndex<nNumSolutions ; nIndex++ ){
				dScore = aScore[nIndex];
				if( dGlobalBestScore < 0 || 
					! m_bMaximizeScore && dScore < dGlobalBestScore ||
					m_bMaximizeScore && dScore > dGlobalBestScore ){
					dGlobalBestScore = dScore;
					m_pSolution->SaveGlobalBest( nIndex );
				}
			}
		}
#ifdef SAVE_STATISTICS
		sas.WriteResult( m_N );
#endif
#ifdef SAVE_PARALLEL_STATISTICS
		//fprintf( fp, "t,accept,accept2,solutions,rate\n" );
		fprintf( fp, "%f,%d,%d,%d,%f\n", t, nTotalSolutionsAccepted, nTotalSolutionsAccepted2, nTotalSolutionsValid, (double)(nTotalSolutionsAccepted+nTotalSolutionsAccepted2)/nTotalSolutionsValid );
		//fprintf( fp3, "t,p0,p1,p2,p3,c0,c1,c2,c3\n" );
		fprintf( fp3, "%f,%d,%d,%d,%d,%f,%f,%f,%f\n", t, aCountPattern[0], aCountPattern[1], aCountPattern[2], aCountPattern[3], aCountPattern[0]?(double)aCountSolutionsValid[0]/aCountPattern[0]:0.0, aCountPattern[1]?(double)aCountSolutionsValid[1]/aCountPattern[1]:0.0, aCountPattern[2]?(double)aCountSolutionsValid[2]/aCountPattern[2]:0.0, aCountPattern[3]?(double)aCountSolutionsValid[3]/aCountPattern[3]:0.0 );
//		fprintf( fp3, "%f,%d,%d,%d,%d,%d,%d,%d,%d\n", t, aCountPattern[0], aCountPattern[1], aCountPattern[2], aCountPattern[3], aCountSolutionsValid[0], aCountSolutionsValid[1], aCountSolutionsValid[2], aCountSolutionsValid[3] );
#endif
/*////////////////////////////////////////////////////////////////////////////////
		if( t < 8 ) break;/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////*/
	}
	if( m_hWnd ){
		aScoreArray[0] = dBestScore;
		aScoreArray[1] = dGlobalBestScore;
		::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)aScoreArray );
		//			::PostMessage(m_hWnd, WM_NEW_TEMPERATURE, (WPARAM)&t, (LPARAM)(int)dBestScore );
	}
	TRACE( "done =====================================================\n", t );
#ifdef SAVE_STATISTICS
	sas.Finish();
#endif
#ifdef SAVE_PARALLEL_STATISTICS
	fclose( fp );
	fclose( fp3 );
#endif
	GlobalFree( aParent );
	GlobalFree( aScore );
	////////////////
	GlobalFree( aUsed );
	return 1;
}

void CMultiSimulatedAnnealing::StoreJudgeResult(int bAccept)
{
	if( ++m_nIndexHistoryWrite >= ACCEPT_HISTRY_LENGTH ){
		m_nIndexHistoryWrite = 0;
	}
	if( m_nCountHistory < ACCEPT_HISTRY_LENGTH ){
		m_nCountHistory++;
		if( bAccept ) m_nCountAccepted++;
	}else{
		if( m_aHistoryAccepted[m_nIndexHistoryWrite] ){
			if( ! bAccept ) m_nCountAccepted--;
		}else{
			if( bAccept ) m_nCountAccepted++;
		}
	}
	m_aHistoryAccepted[m_nIndexHistoryWrite] = bAccept;

}

double CMultiSimulatedAnnealing::GetAcceptRatio(void)
{
	return (double)m_nCountAccepted/m_nCountHistory;
}
