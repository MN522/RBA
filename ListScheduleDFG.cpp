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

// ALAP�X�P�W���[�����s���A�m�[�h�̗D��x�����߂�
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

	// Bellman-Ford�@��p�����Œ��o�H���̉��Ƃ��āAALAP�X�P�W���[�������߂�
	for( m=0 ; m<N ; m++ ){
		complete = TRUE;
		for( n=0 ; n<N ; n++ ){
			TC = node[n].C;
			if( *node[n].T == 'Z' ) TC = 1;	// �D�揇�ʂ����邽�߂ɁAJoin�m�[�h�ɏd��1��^����
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
	// �����Ƀ\�[�g(Priority�͕��ŋ��߂��Ă���̂ŁA��΂̍~���ɂ��邽��)
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
	Init( 10 );	// 10�͂�����x�傫�Ȓl
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
		// �D�揇�ʂ̍����m�[�h���珇�ɒ��ׂ�
		n = m_aPriority[k];
		if( node[n].fix ) continue;
		// �m�[�hn�̉��Z��ނɉ����ĉ��Z�험�p�󋵔z��(aAllocatedToFU)�A���Z�퐔(nMaxFU)�A���Z����(nLatency)�A�p�C�v���C���Ԋu(nLen)���擾
		IdentifyOperation( n, &aAllocatedToFU, &nMaxFU, &nLatency, &nLen );
		// �m�[�hn�̎��s�J�n�ő����������߂�
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

// ���Z�m�[�hn�̎��s������nTime�Ŋm�肷��
// �ő���s����m_nEndTime���X�V����
void CListScheduleDFG::FixNode(int n, int nTime)
{
	m_node[n].Time = nTime;
	m_node[n].fix = 1;
	if( m_node[n].Time+m_node[n].C > m_nEndTime ){
		m_nEndTime = m_node[n].Time+m_node[n].C;
	}
	TRACE( "**Node %s [Priority=%d] fixed at t=%d [%d:%d]\n", m_node[n].N, m_node[n].nPriority, m_node[n].Time, m_node[n].LB, m_node[n].UB );
}

// �e�����Ɏg�p���鉉�Z�퐔���L�^����z��m_aAllocatedToAdd�Ȃǂ��m�ۂ���
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

// �X�P�W���[�����O�ɂ�肠���鉉�Z�m�[�h�͎���m_nDeadLine�܂łɎ��s��������Ƒz�肵��
// �z��m_aAllocatedToAdd�Ȃǂ�p�ӂ��Ă���
// ��������m_nDeadLine�𒴂��ăX�P�W���[������K�v���������ꍇ��
// �z��m_aAllocatedToAdd�Ȃǂ����v�f���̑���(nDeadLine>m_nDeadLine)�z��ɍ�蒼��
// �v�f��nDeadLine�̔z����m��(GlobalAlloc)�A���̔z�񂩂�v�f��]�ʂ��A���̔z������(GlobalFree)����
// �Ȃ��A���̏�����p�ɂɎ��s���邱�Ƃ͌����������̂�m_nDeadLine����������x�傫��nDeadLine���w�肷�邱��
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
		// ���Z���nType�̉��Z�̎��s����nLen�����߂�
		for( n=0 ; n<N ; n++ ){
			if( *node[n].T != cType[nType] ) continue;
			nLen = node[n].C;
			break;
		}
		// ���Z���nType�̉��Z�̍ő哯�����s�������߂� => nMaxFU[nType]
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
	// "Time:"�̏�̉��� 
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
	// "Time:"����щ��Z��̖��O
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
	// "Time:"�̉��̉��� 
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

	// ���Z�튄�蓖�Ă��m�肷��
	for( n=0 ; n<N ; n++ ) aPindex[n] = -1;
	for( t=0 ; t<nDeadLine ; t++ ){
		for( nType=0 ; nType<MAX_TYPE_COUNT ; nType++ ){
			for( n=0 ; n<N ; n++ ){
				if( *node[n].T == cType[nType] ) break;
			}
			if( n >= N ) continue;	// �w�肵���^�C�v�̉��Z��DFG�Ŏg�p����Ă��Ȃ�
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

// �����ˑ��ɂ�鋤�L���l�����ē������s�m�[�h�������߁A���Z�퐔����ȓ��Ŋ��蓖�Ă��邩���ׂ�
int CListScheduleDFG::CanAllocateToFU(int t, int *aAllocatedToFU, int nLen, int nMaxFU )
{
	int t0;
	int nResult = 1;

	// ����t�̉��Z�틤�L�󋵂����߂�
	// ���蓖�čς݂̉��Z
	for( t0=0 ; t0<nLen ; t0++ ){
		if( t+t0 >= m_nEndTime ) continue;	// �܂��������蓖�Ă��Ă��Ȃ������Ȃ̂ŁA���Ȃ炸�w�艉�Z�퐔�ȓ��Ŋ��蓖�ĉ\

		if( aAllocatedToFU[t+t0]>=nMaxFU ){
			nResult = 0;	// �w�艉�Z�퐔�ł͊��蓖�ĕs��
			break;
		}
	}
	return nResult;
}


// ����t�Ɏ��s����Ă���m�[�h�����Z��Ɋ��蓖�Ă�
void CListScheduleDFG::AllocateToFU( int n, int t, int nLen, int *aAllocatedToFU )
{
	int t0;
	if( t+nLen >= m_nDeadLine ){
		int nMaxFU0, nLatency0, nLen0;
		InitRealloc( t+nLen+10 );	// +10�͏\���傫���l�ŁA�p�ɂ�InitRealloc���Ă΂�Ȃ��悤�ɂ���
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
