#include "StdAfx.h"
#include "ListScheduleDFG.h"
static char SEP[]=" \t";

CListScheduleDFG::CListScheduleDFG(void)
{
	m_aPriority = NULL;
	m_nDeadLine = 0;
}

CListScheduleDFG::~CListScheduleDFG(void)
{
	if( m_aPriority ){
		GlobalFree( m_aPriority );
	}
}

// ALAPスケジュールを行い、ノードの優先度を求める
int CListScheduleDFG::CalcNodePriority(void)
{
	int n,m;
	int temp;
	char complete;
	EDGE *eptr;
	NODE *node = m_node;
	NODE *nptr;
	int *w;
	int TC;
	int t;

	w = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	for( n=0 ; n<N ; n++ ){
		if( node[n].olist == NULL ){
			node[n].nPriority = -node[n].C;
		}else{
			node[n].nPriority = INTINF;
		}
		w[n] = node[n].nPriority;
	}

	// Bellman-Ford法を用いた最長経路問題の解として、ALAPスケジュールを求める
	for( m=0 ; m<N ; m++ ){
		complete = TRUE;
		for( n=0 ; n<N ; n++ ){
			TC = node[n].C;
			if( *node[n].T == 'Z' ) TC = 1;	// 優先順位をつけるために、Joinノードに重み1を与える
			for( eptr=node[n].olist; eptr != NULL; eptr=eptr->olist ){
				nptr = eptr->t;
				t = nptr->nPriority;
//				temp = t - TC + eptr->d*Tr;
				temp = t - TC;
				if( temp < w[n] ){
					w[n] = temp;
					complete = FALSE;
				}
			}
		}
		if( complete ) break;
		for( n=0 ; n<N ; n++ ){
			node[n].nPriority = w[n];
		}
	}

	if( !complete ){
		fprintf( stderr, "ALAP scheduling does not converge.\n" );
		exit(1);
	}
	if( m_aPriority ) GlobalFree( m_aPriority );
	m_aPriority = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	for( n=0 ; n<N ; n++ ){
		m_aPriority[n] = n;
		w[n] = node[n].nPriority;
	}
	// 昇順にソート(Priorityは負で求められているので、絶対の降順にするため)
	for( n=0 ; n<N-1 ; n++ ){
		for( m=n+1 ; m<N ; m++ ){
			if( w[n] > w[m] ){
				int t;
				t = w[n]; w[n] = w[m]; w[m] = t;
				t = m_aPriority[n]; m_aPriority[n] = m_aPriority[m]; m_aPriority[m] = t;
			}
		}
	}

	GlobalFree( w );
	////////////////////////////////////////////////////////////////////////////
	for( n=0 ; n<N ; n++ ){
		TRACE( "Node %s: Priority=%d\n", node[n].N, node[n].nPriority );
	}
	///////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////
	for( n=0 ; n<N ; n++ ){
		TRACE( "Priority=%d: Node %s\n", node[m_aPriority[n]].nPriority, node[m_aPriority[n]].N );
	}
	///////////////////////////////////////////////////////////////////////////*/
	return 1;
}

void CListScheduleDFG::ListScheduleInit(void)
{
	CalcNodePriority();
	Init( 10 );	// 10はある程度大きな値
}

int CListScheduleDFG::ListSchedule(void)
{
	NODE *node = m_node;
	NODE *pNodeFrom;
	EDGE *eptr;
	int k;
	int n;
	int nTime;
	int nMaxFU;
	int nLen;
	int nLatency;
	int *aAllocatedToFU;

/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "  CListScheduleDFG::ListSchedule ReleaseAssignedToFU done\n" );
fclose(fp);}*/
	for( k=0 ; k<N ; k++ ) node[k].fix = 0;
	FixInputNode();
	m_nEndTime = -1;

	for( k=0 ; k<N ; k++ ){
		// 優先順位の高いノードから順に調べる
		n = m_aPriority[k];
		if( node[n].fix ) continue;
		// ノードnの演算種類に応じて演算器利用状況配列(aAllocatedToFU)、演算器数(nMaxFU)、演算時間(nLatency)、パイプライン間隔(nLen)を取得
		IdentifyOperation( n, &aAllocatedToFU, &nMaxFU, &nLatency, &nLen );
		// ノードnの実行開始最早時刻を求める
		node[n].LB = -1;
		for( eptr=node[n].ilist ; eptr ; eptr=eptr->ilist ){
			pNodeFrom = eptr->f;
			if( node[n].LB < pNodeFrom->Time + pNodeFrom->C ){
				node[n].LB = pNodeFrom->Time + pNodeFrom->C;
			}
		}
		if( *node[n].T == 'Z' ){
			nTime = node[n].LB;
		}else{
			for( nTime=node[n].LB ; ; nTime++ ){
				if( CanAllocateToFU( nTime, aAllocatedToFU, nLen, nMaxFU ) ){
					break;
				}
			}
			AllocateToFU( n, nTime, nLen, aAllocatedToFU );
		}
		FixNode( n, nTime );
	}
//	ShowSchedule();
//	ShowScheduleVertical();

/*	{FILE *fp=fopen("result.txt","a");
	fprintf(fp, "EndTime=%d\n", m_nEndTime );
	fprintf(fp, "###############################################################\n" );
	fclose(fp);}*/
	return 1;
}

// 演算ノードnの実行時刻をnTimeで確定する
// 最大実行時刻m_nEndTimeを更新する
void CListScheduleDFG::FixNode(int n, int nTime)
{
	m_node[n].Time = nTime;
	m_node[n].fix = 1;
	if( m_node[n].Time+m_node[n].C > m_nEndTime ){
		m_nEndTime = m_node[n].Time+m_node[n].C;
	}
	TRACE( "**Node %s [Priority=%d] fixed at t=%d [%d:%d]\n", m_node[n].N, m_node[n].nPriority, m_node[n].Time, m_node[n].LB, m_node[n].UB );
}

// 各時刻に使用する演算器数を記録する配列m_aAllocatedToAddなどを確保する
void CListScheduleDFG::Init( int nDeadLine )
{
/*{FILE *fp=fopen("status.txt","a");
fprintf(fp, "CListScheduleDFG::Init start: N=%d, nDeadLine=%d\n", N, nDeadLine );
fclose(fp);}*/
	int k;
	m_nDeadLine = nDeadLine;
	m_aAllocatedToAdd = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	m_aAllocatedToSub = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	m_aAllocatedToMul = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	for( k=0 ; k<nDeadLine ; k++ ){
		m_aAllocatedToAdd[k] = 0;
		m_aAllocatedToSub[k] = 0;
		m_aAllocatedToMul[k] = 0;
	}
	m_nDeadLine = nDeadLine;

}

// スケジューリングによりあらゆる演算ノードは時刻m_nDeadLineまでに実行完了すると想定して
// 配列m_aAllocatedToAddなどを用意している
// もし時刻m_nDeadLineを超えてスケジュールする必要が生じた場合に
// 配列m_aAllocatedToAddなどをより要素数の多い(nDeadLine>m_nDeadLine)配列に作り直す
// 要素数nDeadLineの配列を確保(GlobalAlloc)、元の配列から要素を転写し、元の配列を解放(GlobalFree)する
// なお、この処理を頻繁に実行することは効率が悪いのでm_nDeadLineよりもある程度大きなnDeadLineを指定すること
void CListScheduleDFG::InitRealloc( int nDeadLine )
{
	if( m_nDeadLine >= nDeadLine ) return;

	int k;
	int *aAllocatedToAdd = m_aAllocatedToAdd;
	int *aAllocatedToSub = m_aAllocatedToSub;
	int *aAllocatedToMul = m_aAllocatedToMul;
	m_aAllocatedToAdd = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	m_aAllocatedToSub = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	m_aAllocatedToMul = (int *)GlobalAlloc( GPTR, nDeadLine*sizeof(int) );
	for( k=0 ; k<m_nDeadLine ; k++ ){
		m_aAllocatedToAdd[k] = aAllocatedToAdd[k];
		m_aAllocatedToSub[k] = aAllocatedToSub[k];
		m_aAllocatedToMul[k] = aAllocatedToMul[k];
	}
	for( ; k<nDeadLine ; k++ ){
		m_aAllocatedToAdd[k] = 0;
		m_aAllocatedToSub[k] = 0;
		m_aAllocatedToMul[k] = 0;
	}
	GlobalFree( aAllocatedToAdd );
	GlobalFree( aAllocatedToSub );
	GlobalFree( aAllocatedToMul );
	m_nDeadLine = nDeadLine;
}

void CListScheduleDFG::ShowSchedule(void)
{
	int t;
	int nDeadLine;
	int n;
	int k;
	int nMaxFU, nMaxFU0;
	NODE *node = m_node;
	int nShown;
	int nType;
	char cType[] = {'A', 'S'};
	int nLen;
//	double *aUsage;
	int *aPindex = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	int *aRename = (int *)GlobalAlloc( GPTR, N*sizeof(int) );

	TRACE( "-----------------------------------------\n" );
	ShowScheduleAssignment(aPindex, aRename, &nDeadLine);
				////////////////////////////////////

	TRACE( "Time: " );
	for( t=0 ; t<nDeadLine ; t++ ){
		TRACE( "  %2d   ", t );
	}
	TRACE( "\n" );
	TRACE( "----+" );
	for( t=0 ; t<nDeadLine ; t++ ){
		TRACE( "-------" );
	}
	TRACE( "\n" );
	for( nType=0 ; nType<2 ; nType++ ){
		for( n=0 ; n<N ; n++ ){
			if( *node[n].T != cType[nType] ) continue;
			nLen = node[n].C;
			break;
		}
		nMaxFU = 0;
		for( t=0 ; t<nDeadLine ; t++ ){
			nMaxFU0 = 0;
			for( n=0 ; n<N ; n++ ){
				if( *node[n].T != cType[nType] ) continue;
				if( node[n].Time <= t && t < node[n].Time+nLen ) nMaxFU0++;
//					if( node[n].LBL <= t && t < node[n].LBL+nLen ) nMaxFU0++;
			}
			if( nMaxFU < nMaxFU0 ) nMaxFU = nMaxFU0;
		}
		for( k=0 ; k<nMaxFU ; k++ ){
			TRACE( "%c%d:  ", cType[nType], k );
			for( t=0 ; t<nDeadLine ; t++ ){
				nShown = 0;
				nMaxFU0 = 0;
				for( n=0 ; n<N ; n++ ){
					if( *node[n].T != cType[nType] ) continue;
					if( node[n].Time <= t && t < node[n].Time+nLen ){
//						if( node[n].LBL <= t && t < node[n].LBL+nLen ){
						if( nMaxFU0 == k ){
							TRACE( "%3s(%d) ", node[n].N, node[n].pindex );
							nShown = 1;
							break;
						}
						nMaxFU0++;
					}
				}
				if( !nShown ) TRACE( "%7s", "" );
			}
			TRACE( "\n" );
		}
	}
	GlobalFree( aRename );
	GlobalFree( aPindex );
}

#define MAX_TYPE_COUNT	3
void CListScheduleDFG::ShowScheduleVertical(void)
{
	int aMaxFU[MAX_TYPE_COUNT];
	int aLen[MAX_TYPE_COUNT];
	int t;
	int nDeadLine;
	int n;
	int k;
	int nMaxFU, nMaxFU0;
	NODE *node = m_node;
	int nShown;
	int nType;
	char cType[] = {'A', 'S', 'M'};
	int nLen;
	CString str;
//	double *aUsage;
	int *aPindex = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	int *aRename = (int *)GlobalAlloc( GPTR, N*sizeof(int) );

	m_strResult = "";
//	ShowScheduleAssignment(aPindex, aRename, &nDeadLine);
	nDeadLine = 10;
				////////////////////////////////////

	for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
		// 演算種類nTypeの演算の実行時間nLenを求める
		for( n=0 ; n<N ; n++ ){
			if( *node[n].T != cType[nType] ) continue;
			nLen = node[n].C;
			break;
		}
		// 演算種類nTypeの演算の最大同時実行数を求める => nMaxFU[nType]
		nMaxFU = 0;
		for( t=0 ; t<m_nEndTime ; t++ ){
			nMaxFU0 = 0;
			for( n=0 ; n<N ; n++ ){
				if( *node[n].T != cType[nType] ) continue;
				if( node[n].Time <= t && t < node[n].Time+nLen ) nMaxFU0++;
			}
			if( nMaxFU < nMaxFU0 ) nMaxFU = nMaxFU0;
		}
		aMaxFU[nType] = nMaxFU;
		aLen[nType] = nLen;
	}


#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "###############################################################\n" );
fprintf(fp, "Time: " );
fclose(fp);}
#endif
	/////////////////////////////////////////////////////
	// "Time:"の上の横線 
	TRACE( "----+-" );
	m_strResult += "----+-";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "----+" );
fclose(fp);}
#endif
	for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
		for( k=0 ; k<aMaxFU[nType] ; k++ ){
			TRACE( "-------" );
			m_strResult += "-------";
	}
		}
	TRACE( "\n" );
	m_strResult += "\r\n";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "\n" );
fclose(fp);}
#endif

	/////////////////////////////////////////////////////
	// "Time:"および演算器の名前
	TRACE( "Time: " );
	m_strResult += "Time: ";
	for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
		for( k=0 ; k<aMaxFU[nType] ; k++ ){
			TRACE( "  %c%d   ", cType[nType], k );
			str.Format( L"  %c%d   ", cType[nType], k );
			m_strResult += str;
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, " %c%d   ", cType[nType], k );
fclose(fp);}
#endif
		}
	}
	TRACE( "\n" );
	m_strResult += "\r\n";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "\n" );
fclose(fp);}
#endif

	/////////////////////////////////////////////////////
	// "Time:"の下の横線 
	TRACE( "----+-" );
	m_strResult += "----+-";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "----+" );
fclose(fp);}
#endif
	for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
		for( k=0 ; k<aMaxFU[nType] ; k++ ){
			TRACE( "-------" );
			m_strResult += "-------";
		}
	}
	TRACE( "\n" );
	m_strResult += "\r\n";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "\n" );
fclose(fp);}
#endif
	for( t=0 ; t<m_nEndTime ; t++ ){
		TRACE( "%4d: ", t );
		str.Format( L"%4d: ", t );
		m_strResult += str;
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "%4d: ", t );
fclose(fp);}
#endif
		for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
				nLen = aLen[nType];
			for( k=0 ; k<aMaxFU[nType] ; k++ ){
				nShown = 0;
				nMaxFU0 = 0;
				for( n=0 ; n<N ; n++ ){
					if( *node[n].T != cType[nType] ) continue;
					if( node[n].Time <= t && t < node[n].Time+nLen ){
						if( nMaxFU0 == k ){
							TRACE( "%3s(%d) ", node[n].N, node[n].pindex );
							str.Format( L"%3S(%d) ", node[n].N, node[n].pindex );
							m_strResult += str;
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "%3s(%d) ", node[n].N, node[n].pindex );
fclose(fp);}
#endif
							nShown = 1;
							break;
						}
						nMaxFU0++;
					}
				}
				if( !nShown ){
					TRACE( "%7s", "" );
					str.Format( L"%7S", "" );
					m_strResult += str;
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "%7s", "" );
fclose(fp);}
#endif
				}
			}
		}
		TRACE( "\r\n" );
		m_strResult += "\r\n";
#ifdef NDEBUG
{FILE *fp;
fopen_s(&fp,"result.txt","a");
fprintf(fp, "\n" );
fclose(fp);}
#endif
	}
	GlobalFree( aRename );
	GlobalFree( aPindex );
}

void CListScheduleDFG::ShowScheduleAssignment(int *aPindex, int *aRename, int *pnDeadLine)
{
	int t;
	int nDeadLine;
	int n;
//	int k;
	NODE *node = m_node;
//	NODE *nptr;
	int nType;
	char cType[] = {'A', 'S', 'M'};
	int nLen;
//	int *aAssignedToFU;

	nDeadLine = -1;
	for( n=0 ; n<N ; n++ ){
		nLen = node[n].C;
		if( nDeadLine < node[n].Time+nLen ){
			nDeadLine = node[n].Time+nLen;
		}
	}

	// 演算器割り当てを確定する
	for( n=0 ; n<N ; n++ ) aPindex[n] = -1;
	for( t=0 ; t<nDeadLine ; t++ ){
		for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
			for( n=0 ; n<N ; n++ ){
				if( *node[n].T == cType[nType] ) break;
			}
			if( n >= N ) continue;	// 指定したタイプの演算はDFGで使用されていない
		}
	}
	*pnDeadLine = nDeadLine;
}

int CListScheduleDFG::IdentifyOperation(int n, int **aAllocatedToFU, int *nMaxFU, int *nLatency, int *nLen)
{
	NODE *node = m_node;

	*nLen = node[n].L;
	*nLatency = node[n].C;
	switch( *node[n].T ){
	case 'A':
	case 'a':
		*nMaxFU = m_nMaxAdd;
		*aAllocatedToFU = m_aAllocatedToAdd;
		break;
	case 'S':
	case 's':
		*nMaxFU = m_nMaxSub;
		*aAllocatedToFU = m_aAllocatedToSub;
		break;
	case 'M':
	case 'm':
		*nMaxFU = m_nMaxMul;
		*aAllocatedToFU = m_aAllocatedToMul;
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

// 条件依存による共有を考慮して同時実行ノード数を求め、演算器数制約以内で割り当てられるか調べる
int CListScheduleDFG::CanAllocateToFU(int t, int *aAllocatedToFU, int nLen, int nMaxFU )
{
	int t0;
	int nResult = 1;

	// 時刻tの演算器共有状況を求める
	// 割り当て済みの演算
	for( t0=0 ; t0<nLen ; t0++ ){
		if( t+t0 >= m_nEndTime ) continue;	// まだ何も割り当てられていない時刻なので、かならず指定演算器数以内で割り当て可能

		if( aAllocatedToFU[t+t0]>=nMaxFU ){
			nResult = 0;	// 指定演算器数では割り当て不可
			break;
		}
	}
	return nResult;
}


// 時刻tに実行されているノードを演算器に割り当てる
void CListScheduleDFG::AllocateToFU( int n, int t, int nLen, int *aAllocatedToFU )
{
	int t0;
	if( t+nLen >= m_nDeadLine ){
		int nMaxFU0, nLatency0, nLen0;
		InitRealloc( t+nLen+10 );	// +10は十分大きい値で、頻繁にInitReallocが呼ばれないようにする
		IdentifyOperation( n, &aAllocatedToFU, &nMaxFU0, &nLatency0, &nLen0 );
	}
	for( t0=0 ; t0<nLen ; t0++ ) aAllocatedToFU[t+t0]++;
}

void CListScheduleDFG::FixInputNode(void)
{
	NODE *node = m_node;
	int n;
	for( n=0 ; n<N ; n++ ){
		if( *node[n].N == 'I' || *node[n].N == 'i' ){
			node[n].fix = 2;
		}
	}
}

void CListScheduleDFG::ShowAssignments(void)
{
	NODE *node = m_node;
	int n;
	TRACE( "=============\n" );
	for( n=0 ; n<N ; n++ ){
		TRACE( "Node %s: ", node[n].N );
		TRACE( "%d", node[n].Time );
		TRACE( "[%d,%d]", node[n].LB, node[n].UB );
		TRACE( "%s", node[n].fix>=2?"#":(node[n].fix?"*":"") );
		TRACE( "\n" );
	}
}

void CListScheduleDFG::SetFUConfig(int nMaxAdd, int nMaxSub, int nMaxMul)
{
	m_nMaxAdd = nMaxAdd;
	m_nMaxSub = nMaxSub;
	m_nMaxMul = nMaxMul;
}
