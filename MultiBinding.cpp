#include "stdafx.h"
#include "MultiBinding.h"

UINT ThreadSolution(LPVOID pParam)
{
	THREAD_TOKEN *pThreadToken = (THREAD_TOKEN *)pParam;

	TRACE( "Thread %d initiated\n", pThreadToken->nID );
	// ����������
	srand( ((CBinding *)(pThreadToken->pObject))->m_nRandomSeed );

	while(1){
		WaitForSingleObject( pThreadToken->hEvent, INFINITE );	// �J�n�w����҂�
		SetEvent( pThreadToken->hEvent2 );						// �J�n��ʒm
//		TRACE( "Thread %d started\n", pThreadToken->nID );

		if( pThreadToken->nCommand == 1 ){
			pThreadToken->dScore = ((CBinding *)(pThreadToken->pObject))->Evaluate();
		}
//		TRACE( "Thread %d finished\n", pThreadToken->nID );

		WaitForSingleObject( pThreadToken->hEvent, INFINITE );	// �I���ʒm����҂�
		SetEvent( pThreadToken->hEvent2 );						// �I����ʒm
		if( pThreadToken->nCommand == 0 ) break;
	}
	return 0;
}


CMultiBinding::CMultiBinding()
{
	m_aBindingRoot = NULL;
	m_aBinding = NULL;
	m_aBindingBest = NULL;
	m_aBindingGlobalBest = NULL;

	m_aNode1 = NULL;
	m_aNodeBinding1 = NULL;
	m_aNode2 = NULL;
	m_aNodeBinding2 = NULL;
}


CMultiBinding::~CMultiBinding()
{
	if( m_aBinding ){
		GlobalFree( m_aBinding );
		GlobalFree( m_aBindingRoot );
		GlobalFree( m_aBindingBest );
		GlobalFree( m_aBindingGlobalBest );
	}
	if( m_aNode1 ){
		GlobalFree( m_aNode1 );
		GlobalFree( m_aNodeBinding1 );
		GlobalFree( m_aNode2 );
		GlobalFree( m_aNodeBinding2 );
	}
}

void CMultiBinding::SetFUConfiguration(int nAdd, int nSub, int nMul)
{
	m_nAdd = nAdd;
	m_nSub = nSub;
	m_nMul = nMul;
}

void CMultiBinding::SetParamSAforFloorplanPro(double dTstart, double dTend, double dRate, int nIterate)
{
	m_dTstartFloorplanPro = dTstart;
	m_dTendFloorplanPro = dTend;
	m_dRateFloorplanPro = dRate;
	m_nIterateFloorplanPro = nIterate;
}

void CMultiBinding::SetParamSAforFloorplanFinal(double dTstart, double dTend, double dRate, int nIterate)
{
	m_dTstartFloorplanFinal = dTstart;
	m_dTendFloorplanFinal = dTend;
	m_dRateFloorplanFinal = dRate;
	m_nIterateFloorplanFinal = nIterate;
}

int CMultiBinding::Initialize(void)
{
	int k;
	m_pThread = (CWinThread **)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(CWinThread *) );
	for( k=0 ; k<m_nNumSolutions ; k++ ){
		m_aThreadToken[k].nCommand = 1;
		m_pThread[k] = ::AfxBeginThread( ThreadSolution, (LPVOID)(&(m_aThreadToken[k])) );
		m_pThread[k]->m_bAutoDelete = FALSE;
		//	DWORD rthread = pSchedulingThread->ResumeThread();		// �T�X�y���h����
	}

	return 1;
}

int CMultiBinding::SaveGlobalBest(int nIndex)
{ 
	int n;
	for( n=0 ; n<N ; n++ ) m_aBindingGlobalBest[n] = m_aBindingCore[nIndex]->m_aBinding[n];
	m_nClockCyclesGlobalBest = m_aBindingCore[nIndex]->m_nClockCycles;
	m_nClockPeriodGlobalBest = m_aBindingCore[nIndex]->m_nClockPeriod;

	///////////////////////////
	m_aBindingCore[nIndex]->WriteBestBinding(TRUE);
	m_aBindingCore[nIndex]->WriteBestSchedule(TRUE);
	m_aBindingCore[nIndex]->WriteBestFloorplanPro(TRUE);
	m_aBindingCore[nIndex]->WriteBestFloorplanFinal(TRUE);
	///////////////////////////
	return 1;
}

void CMultiBinding::GlobalInit(int nNumthreads)
{
	TCHAR name[64];
	int k;

	unsigned long nRandomSeed = (unsigned)time(NULL);
	m_nNumSolutions = nNumthreads;
	m_aBindingCore = (CBinding **)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(CBinding *) );
	m_pDFGCore = (CListScheduleDFG **)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(CListScheduleDFG *) );
	for( k=0 ; k<m_nNumSolutions ; k++ ){
		m_pDFGCore[k] = new CListScheduleDFG;
		m_aBindingCore[k] = new CBinding;
		m_aBindingCore[k]->m_nID = k;
		m_aBindingCore[k]->SetRandomeSeed( nRandomSeed+k );
//		m_aBindingCore[k]->Initialize(m_nModule, m_Module);
		m_aBindingCore[k]->m_SAforFloorplanPro.SetParameters(m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro, FALSE/*minimize*/);
		m_aBindingCore[k]->m_SAforFloorplanPro.SetID( 1+k*10 );
		m_aBindingCore[k]->m_SAforFloorplanFinal.SetParameters(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal, FALSE/*minimize*/);
		m_aBindingCore[k]->m_SAforFloorplanFinal.SetID( 2+k*10 );
		m_aBindingCore[k]->m_FloorplanPro.SetMode( 0 );
		m_aBindingCore[k]->m_FloorplanFinal.SetMode( 1 );
		m_pDFGCore[k]->Duplicate( m_pDFG );
		m_aBindingCore[k]->SetDFG( m_pDFGCore[k] );
		m_aBindingCore[k]->SetFUConfiguration( m_nAdd, m_nSub, m_nMul );
		m_aBindingCore[k]->Setup();
		m_aBindingCore[k]->Initialize();
	}
	m_hEvent = (HANDLE *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(HANDLE) );
	m_hEvent2 = (HANDLE *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(HANDLE) );
	for( k=0 ; k<m_nNumSolutions ; k++ ){
		_stprintf_s( name, sizeof(name)/sizeof(name[0]), _T("MyEvent%d"), k );
		m_hEvent[k] = CreateEvent( NULL, FALSE, FALSE, name );
		_stprintf_s( name, sizeof(name)/sizeof(name[0]), _T("MyEvent2%d"), k );
		m_hEvent2[k] = CreateEvent( NULL, FALSE, FALSE, name );
	}
	m_aThreadToken = (THREAD_TOKEN *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(THREAD_TOKEN) );
	for( k=0 ; k<m_nNumSolutions ; k++ ){
		m_aThreadToken[k].nID = k;
		m_aThreadToken[k].pObject = m_aBindingCore[k];
		m_aThreadToken[k].hEvent = m_hEvent[k];
		m_aThreadToken[k].hEvent2 = m_hEvent2[k];
	}
	/*	m_aStrutsParentRecord = (int **)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int *) );
	for( k=0 ; k<m_nNumSolutions ; k++ ) m_aStrutsParentRecord[k] = (int *)GlobalAlloc( GPTR, E*sizeof(int) );*/

	N = m_pDFG->N;
	m_aBindingRoot = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	m_aBinding = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	m_aBindingBest = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	m_aBindingGlobalBest = (int *)GlobalAlloc( GPTR, N*sizeof(int) );

//	m_aModeGenerateNeighbor = (int *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int) );
	m_aNode1 = (int *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int) );
	m_aNodeBinding1 = (int *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int) );
	m_aNode2 = (int *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int) );
	m_aNodeBinding2 = (int *)GlobalAlloc( GPTR, m_nNumSolutions*sizeof(int) );

	int n;
	int nFUCount;
	NODE *node = m_pDFG->m_node;

	m_nNodeAdd = 0;
	m_nNodeSub = 0;
	m_nNodeMul = 0;
	for( n=0 ; n<N ; n++ ){
		switch( *node[n].T ){
		case 'A': m_nNodeAdd++; break;
		case 'S': m_nNodeSub++; break;
		default: m_nNodeMul++; break;
		}
	}
	if( m_nNodeAdd ){
		m_aNodeAdd = (int *)GlobalAlloc( GPTR, m_nNodeAdd*sizeof(int) );
	}
	if( m_nNodeSub ){
		m_aNodeSub = (int *)GlobalAlloc( GPTR, m_nNodeSub*sizeof(int) );
	}
	if( m_nNodeMul ){
		m_aNodeMul = (int *)GlobalAlloc( GPTR, m_nNodeMul*sizeof(int) );
	}

	m_nNodeAdd = 0;
	m_nNodeSub = 0;
	m_nNodeMul = 0;
	for( n=0 ; n<N ; n++ ){
		switch( *node[n].T ){
		case 'A': m_aNodeAdd[m_nNodeAdd++] = n; break;
		case 'S': m_aNodeSub[m_nNodeSub++] = n; break;
		default: m_aNodeMul[m_nNodeMul++] = n; break;
		}
	}

}

// aParent[n]: -1:���݉�(root), >=0: ��������ߖT��
int CMultiBinding::GenerateNeighbors( double dAcceptRatio, int aParent[] )
{
	TRACE( "GenerateNeighbors ---------------\n" );
	int nIndex;
	int n;
	int nIndexParent;
	int nIndexCurrent;
	int nPattern = 0;

	//for( nIndex=0 ; nIndex<m_nNumSolutions ; nIndex++ ) aParent[nIndex] = -1;
	// (�Ⴆ��AcceptRatio�Ɋ�Â���)�V�K�ɐ�������ߖT���̍\���p�^�[�������߂�
	// �\���p�^�[���͐e�̃C���f�b�N�X�Ƃ��ĕ\������
	/*	if( dAcceptRatio > 0.9 ){
	aParent[0] = -1;
	aParent[1] = 0;
	aParent[2] = 1;
	aParent[3] = 2;
	nPattern = 1;
	}else if( dAcceptRatio > 0.66 ){
	aParent[0] = -1;
	aParent[1] = 0;
	aParent[2] = 1;
	aParent[3] = 1;
	nPattern = 2;
	}else if( dAcceptRatio > 0.33 ){
	aParent[0] = -1;
	aParent[1] = -1;
	aParent[2] = -1;
	aParent[3] = 2;
	nPattern = 3;
	}else{
	aParent[0] = -1;
	aParent[1] = -1;
	aParent[2] = -1;
	aParent[3] = -1;
	nPattern = 0;
	}*/
	if( dAcceptRatio > 0.68 ){
		aParent[0] = -1;
		aParent[1] = 0;
		aParent[2] = 1;
		aParent[3] = 2;
		nPattern = 1;
	}else if( dAcceptRatio > 0.5 ){
		aParent[0] = -1;
		aParent[1] = 0;
		aParent[2] = 1;
		aParent[3] = -1;
		nPattern = 2;
	}else if( dAcceptRatio > 0.32 ){
		aParent[0] = -1;
		aParent[1] = 0;
		aParent[2] = -1;
		aParent[3] = -1;
		nPattern = 3;
	}else{
		aParent[0] = -1;
		aParent[1] = -1;
		aParent[2] = -1;
		aParent[3] = -1;
		nPattern = 0;
	}
	if( m_nNumSolutions == 8 ){
		if( dAcceptRatio > 0.68 ){
			aParent[0] = -1;
			aParent[1] = 0;
			aParent[2] = 1;
			aParent[3] = 2;
			aParent[4] = 3;
			aParent[5] = 4;
			aParent[6] = 5;
			aParent[7] = 6;
			nPattern = 1;
		}else if( dAcceptRatio > 0.5 ){
			aParent[0] = -1;
			aParent[1] = 0;
			aParent[2] = 1;
			aParent[3] = 2;
			aParent[4] = -1;
			aParent[5] = 4;
			aParent[6] = 5;
			aParent[7] = -1;
			nPattern = 2;
		}else{
			for( nIndex=0 ; nIndex<m_nNumSolutions ; nIndex++ ) aParent[nIndex] = -1;
			nPattern = 0;
		}
	}

	// ���[�g���̏�Ԃ�ۑ�
	for( n=0 ; n<N ; n++ ) m_aBindingRoot[n] = m_aBinding[n];

	// �\���p�^�[���ɏ]���ċߖT���𐶐�����
	nIndexCurrent = -1;	// ���݁Am_GammaPlus�Ȃǂɂ�Index�Ԗڂ̋ߖT�����L�^����Ă���B�l-1��root�����L�^����Ă��邱�Ƃ��Ӗ�����B
	for( nIndex=0 ; nIndex<m_nNumSolutions ; nIndex++ ){
		// �e(aParent[nIndex])�̃C���f�b�N�X�𓾂�
		nIndexParent = aParent[nIndex];
		// �e(aParent[nIndex])�̏�Ԃ𓾂�
		if( nIndexParent != nIndexCurrent ){
			if( nIndexParent < 0 ){
				for( n=0 ; n<N ; n++ ) m_aBinding[n] = m_aBindingRoot[n];
			}else{
				for( n=0 ; n<N ; n++ ) m_aBinding[n] = m_aBindingCore[nIndexParent]->m_aBinding[n];
			}
		}
		// �ߖT�𐶐�
		GenerateNeighbor();
		// ���������ߖT����]���p�ɋL�^
		for( n=0 ; n<N ; n++ ) m_aBindingCore[nIndex]->m_aBinding[n] = m_aBinding[n];
		nIndexCurrent = nIndex;
		// �������p���邽�߂̏����L�^
		//m_aModeGenerateNeighbor[nIndex] = m_nModeGenerateNeighbor;
		m_aNode1[nIndex] = m_nNode1;
		m_aNodeBinding1[nIndex] = m_nNodeBinding1;
		m_aNode2[nIndex] = m_nNode2;
		m_aNodeBinding2[nIndex] = m_nNodeBinding2;
	}

	TRACE( "GenerateNeighbors DONE ---------------\n" );
	return nPattern;
}

int CMultiBinding::GenerateNeighbor()
{
	NODE *node = m_pDFG->m_node;
	int n1,n2;
	int k;
	int nFU;
	int nNodeCount;
	int *aNodeFU;
	int boolGenerateDone = 0;

	///////////////////
	// �S�Ẳ��Z���ނŉ��Z�퐔��1���̏ꍇ�A�o�C���f�B���O�͈�ӂł���A�ߖT�o�C���f�B���O�͂��肦�Ȃ�
	// ���Ȃ��Ƃ�1�̉��Z��ނŁA���Z�킪2�ȏ゠��
	///////////////////

	int nModeNeighbor;
	switch( rand()%6 ){
	case 0:
	case 1:
	case 2:
		nModeNeighbor = 0;	//		SwapNodes();
		break;
	case 3:
	case 4:
	case 5:
	default:
		nModeNeighbor = 1;	//		MoveNode();
		break;
	}

	while(1){
		// �m�[�h��1�I��
		n1 = (int)((rand()* N / (1.0+ RAND_MAX)));	// 0��n1��N-1�̗���n1�����߂�
		switch( *node[n1].T ){
		case 'A':
			nNodeCount = m_nNodeAdd;
			aNodeFU = m_aNodeAdd;
			nFU = m_nAdd;
			break;
		case 'S':
			nNodeCount = m_nNodeSub;
			aNodeFU = m_aNodeSub;
			nFU = m_nSub;
			break;
		default:
			nNodeCount = m_nNodeMul;
			aNodeFU = m_aNodeMul;
			nFU = m_nMul;
			break;
		}
		if( nFU > 1 ) break;
	}
	// ������n1�́A���Z�킪2�ȏ゠�鉉�Z��ނ̃m�[�h
	m_nNode1 = n1;
	m_nNodeBinding1 = m_aBinding[n1];

	if( nModeNeighbor == 0 ){
		// ���Z�프�f�B���O���قȂ�ʂ̃m�[�h��I�сA�o�C���f�B���O������
		// �m�[�hn1�ƈقȂ鉉�Z��ɔ��f�B���O����Ă���m�[�hn2�����߂�
		do{
			k =  (int)(rand() * nNodeCount / (1.0 + RAND_MAX));
			n2 = aNodeFU[k];
		}while( n2 == n1 );
		m_nNode2 = n2;
		m_nNodeBinding2 = m_aBinding[n2];
		// ����
		m_aBinding[n1] = m_nNodeBinding2;
		m_aBinding[n2] = m_nNodeBinding1;
		boolGenerateDone = 1;
	}
	if( ! boolGenerateDone ){
		if( nModeNeighbor == 1 ){
			// �m�[�hn1�̃o�C���f�B���O���ړ�
			do{
				k =  (int)(rand() * nFU / (1.0 + RAND_MAX));
			}while( k == m_nNodeBinding1 );//k�����݂̃o�C���f�B���O(m_nNodeBinding1�j�ƈႤ�l�ɂȂ�܂�k�������_������
			//
			m_aBinding[n1] = k;//�m�[�hn1�̃o�C���f�B���O���Z��͉��Z��k�ɂȂ���
			m_nNode2 = -1;
		}else{
			// ���͋ߖT�𐶐���@��nModeNeighbor={0,1}�Ȃ̂ŁA�����ɗ��邱�Ƃ͂Ȃ�
		}
	}
	return 0;
}

int CMultiBinding::RestoreNeighbor( int nIndex )
{
	//	TRACE( "Neighbor restored\n" );
	int n;
	for( n=0 ; n<N ; n++ ) m_aBinding[n] = m_aBindingCore[nIndex]->m_aBinding[n];

	return 1;
}

void CMultiBinding::AcceptNeighbor( int nIndex )
{
	int n;
	for( n=0 ; n<N ; n++ ) m_aBinding[n] = m_aBindingCore[nIndex]->m_aBinding[n];
}

int CMultiBinding::SaveBest( int nIndex )
{
	int n;
	for( n=0 ; n<N ; n++ ) m_aBindingBest[n] = m_aBindingCore[nIndex]->m_aBinding[n];

	m_aBindingCore[nIndex]->SaveBest();
	return 1;
}

int CMultiBinding::GetScore( double aScore[] )
{
	//	return m_RangeChart.DoRangeChart();
	//	m_nScore = (int)m_pDFG->Evaluate();
	//	dmesg( "SA.log", "Score = %d <<<<<<<<<\n", m_nScore );
	int k;
	for( k=0 ; k<m_nNumSolutions ; k++ ) SetEvent( m_hEvent[k] );			// �J�n�w��
	if( m_nNumSolutions == 1 ){
		WaitForSingleObject( m_hEvent2[0], INFINITE );	// ���[�J�̊J�n��҂�
	}else{
		WaitForMultipleObjects( m_nNumSolutions, m_hEvent2, TRUE, INFINITE );	// ���[�J�̊J�n��҂�
	}
	for( k=0 ; k<m_nNumSolutions ; k++ ) SetEvent( m_hEvent[k] );			// ���[�J�ɏI���񍐂�����
	if( m_nNumSolutions == 1 ){
		WaitForSingleObject( m_hEvent2[0], INFINITE );	// ���[�J�̏I����҂�
	}else{
		WaitForMultipleObjects( m_nNumSolutions, m_hEvent2, TRUE, INFINITE );	// ���[�J�̏I����҂�
	}
	for( k=0 ; k<m_nNumSolutions ; k++ ) aScore[k] = m_aThreadToken[k].dScore;
	return 1;
}
