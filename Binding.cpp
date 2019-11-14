#include "stdafx.h"
#include "Binding.h"

#define DEBUG_VERBOSE_FUNCTION

#define MODULE_SIZE_ADD_WIDTH 16
#define MODULE_SIZE_ADD_HEIGHT 3
#define MODULE_SIZE_SUB_WIDTH 16
#define MODULE_SIZE_SUB_HEIGHT 3
#define MODULE_SIZE_MUL_WIDTH 16
#define MODULE_SIZE_MUL_HEIGHT 13
#define MODULE_SIZE_REG_WIDTH 16
#define MODULE_SIZE_REG_HEIGHT 2

//#define THRESHOLD_CLOCK_PERIOD 6
#define THRESHOLD_CLOCK_PERIOD 17
CBinding::CBinding()
{
	m_nNodeAdd = 0;
	m_nNodeSub = 0;
	m_nNodeMul = 0;
	m_aNodeAdd = NULL;
	m_aNodeSub = NULL;
	m_aNodeMul = NULL;
	m_aBinding = NULL;
	m_aBindingBest = NULL;
	m_aTranslateBindingFUIndexToModuleIndexAdd = NULL;
	m_aTranslateBindingFUIndexToModuleIndexSub = NULL;
	m_aTranslateBindingFUIndexToModuleIndexMul = NULL;

	m_listCommunicationRequirementPro = NULL;
	m_listCommunicationRequirementFinal = NULL;

	m_aPriority = NULL;
	m_aInterFUConnection = NULL;

	m_poolInterconnectionTarget = NULL;
	m_nCountPoolInterconnectionTarget = 0;
	m_nCountAllocatedInterconnectionTarget = 0;
	m_poolRegGroupContainer = NULL;
	m_nCountPoolRegGroupContainer = 0;
	m_nCountAllocatedRegGroupContainer = 0;
	m_poolRegGroup = NULL;
	m_nCountPoolRegGroup = 0;
	m_nCountAllocatedRegGroup = 0;
	m_poolRegInstance = NULL;
	m_nCountPoolRegInstance = 0;
	m_nCountAllocatedRegInstance = 0;
	m_poolRegToRegComm = NULL;
	m_nCountPoolRegToRegComm = 0;
	m_nCountAllocatedRegToRegComm = 0;
	m_poolRegToFUComm = NULL;
	m_nCountPoolRegToFUComm = 0;
	m_nCountAllocatedRegToFUComm = 0;

	m_nRandomSeed = (unsigned)time(NULL);

}

CBinding::~CBinding()
{
	if( m_aNodeAdd ) GlobalFree( m_aNodeAdd );
	if( m_aNodeSub ) GlobalFree( m_aNodeSub );
	if( m_aNodeMul ) GlobalFree( m_aNodeMul );
	if( m_aBinding ) GlobalFree( m_aBinding );
	if( m_aBindingBest ) GlobalFree( m_aBindingBest );
	if( m_aTranslateBindingFUIndexToModuleIndexAdd ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexAdd );
	if( m_aTranslateBindingFUIndexToModuleIndexSub ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexSub );
	if( m_aTranslateBindingFUIndexToModuleIndexMul ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexMul );

	if( m_aPriority ) GlobalFree( m_aPriority );

	ClearCommunicationRequirementPro();
	ClearCommunicationRequirementFinal();
	ClearCommPipelineRegisters();
	if( m_aInterFUConnection ) GlobalFree( m_aInterFUConnection );

}

void CBinding::ClearCommPipelineRegisters(void)
{
	if( m_poolInterconnectionTarget ) GlobalFree( m_poolInterconnectionTarget );
	if( m_poolRegGroupContainer ) GlobalFree( m_poolRegGroupContainer );
	if( m_poolRegGroup ) GlobalFree( m_poolRegGroup );
	if( m_poolRegInstance ) GlobalFree( m_poolRegInstance );
	if( m_poolRegToRegComm ) GlobalFree( m_poolRegToRegComm );
	m_poolInterconnectionTarget = NULL;
	m_nCountPoolInterconnectionTarget = 0;
	m_nCountAllocatedInterconnectionTarget = 0;
	m_poolRegGroupContainer = NULL;
	m_nCountPoolRegGroupContainer = 0;
	m_nCountAllocatedRegGroupContainer = 0;
	m_poolRegInstance = NULL;
	m_nCountPoolRegInstance = 0;
	m_nCountAllocatedRegInstance = 0;
	m_poolRegToRegComm = NULL;
	m_nCountPoolRegToRegComm = 0;
	m_nCountAllocatedRegToRegComm = 0;
}

void CBinding::SetFUConfiguration(int nAdd, int nSub, int nMul)
{
	m_nAdd = nAdd;
	m_nSub = nSub;
	m_nMul = nMul;
}


int CBinding::RestoreBest()
{
	int N = m_pDFG->N;
	int n;
	for( n=0 ; n<N ; n++ ) m_aBinding[n] = m_aBindingBest[n];
	return 0;
}

int CBinding::SaveBest()
{
	int N = m_pDFG->N;
	int n;
	for( n=0 ; n<N ; n++ ) m_aBindingBest[n] = m_aBinding[n];

	///////////////////////////
	WriteBestBinding();
	WriteBestFloorplanPro();
	WriteBestFloorplanFinal();
	///////////////////////////

	return 0;
}

int CBinding::GenerateNeighbor()
{
	int N = m_pDFG->N;
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
		// ���Z��o�C�f�B���O���قȂ�ʂ̃m�[�h��I�сA�o�C���f�B���O������
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
			}while( k == m_nNodeBinding1 );
			//
			m_aBinding[n1] = k;
			m_nNode2 = -1;
		}else{
			// ���͋ߖT�𐶐���@��nModeNeighbor={0,1}�Ȃ̂ŁA�����ɗ��邱�Ƃ͂Ȃ�
		}
	}

	/*int n;
	FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	for( n=0 ; n<N ; n++ ){
		switch( *node[n].T ){
		case 'A':
			fprintf( fp, "%s Add%d\n", node[n].N, m_aBinding[n] );
			break;
		case 'S':
			fprintf( fp, "%s Sub%d\n", node[n].N, m_aBinding[n] );
			break;
		default:
			fprintf( fp, "%s Mul%d\n", node[n].N, m_aBinding[n] );
			break;
		}
	}
	for( n=0 ; n<N ; n++ ){
		fprintf( fp, "%2s ", node[n].N );
	}
	fprintf( fp, "\n" );
	for( n=0 ; n<N ; n++ ){
		fprintf( fp, "%2d ", m_aBinding[n] );
	}
	fprintf( fp, "\n" );
	fclose( fp );*/
	return 0;
}

int CBinding::RestoreNeighbor()
{
	NODE *node = m_pDFG->m_node;
	if( m_nNode2 >= 0 ){
		// ������߂�
		m_aBinding[m_nNode1] = m_nNodeBinding1;
		m_aBinding[m_nNode2] = m_nNodeBinding2;
	}else{
		// �ړ���߂�
		m_aBinding[m_nNode1] = m_nNodeBinding1;
	}
	return 0;
}

double CBinding::Evaluate(void)
{
	int nScore = GetScore();
	return (double)nScore;
}


/*
#define MODULE_SIZE_ADD_WIDTH 16
#define MODULE_SIZE_ADD_HEIGHT 3
#define MODULE_SIZE_SUB_WIDTH 16
#define MODULE_SIZE_SUB_HEIGHT 3
#define MODULE_SIZE_MUL_WIDTH 16
#define MODULE_SIZE_MUL_HEIGHT 13
#define MODULE_SIZE_REG_WIDTH 16
#define MODULE_SIZE_REG_HEIGHT 2
*/

//#define DEBUG_VERBOSE_SCORE
int CBinding::GetScore()
{
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScore --------------------\n" );
#endif
	///////////////////////////////////////////////////////////////////////////////////
	// (1)���o�C���f�B���O�Ɋ�Â����W���[��(���Z��A���W�X�^��)�����t���A�v�����ɓo�^
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase1 ---\n" );
#endif
	GetScorePhase1();

	///////////////////////////////////////////////////////////////////////////////////
	// (2)���t���A�v�����œK��
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase2 ---\n" );
#endif
	GetScorePhase2();

	/*///////////////////////////////////////////
	TRACE( "Score=%d\n", m_FloorplanPro.GetScore() );
	m_FloorplanPro.RestoreBest();
	m_nScore = m_FloorplanPro.GetScore();
	TRACE( "BestScore=%d\n", m_nScore );
	///////////////////////////////////////////*/

	/*///////////////////////////////////////////
	m_FloorplanPro.RestoreBest();
	TRACE( "BestScore=%d\n", m_FloorplanPro.GetScore() );
	WriteBestFloorplanPro();
	///////////////////////////////////////////*/

	///////////////////////////////////////////////////////////////////////////////////
	// (3)���t���A�v�����Ɋ�Â��A���Z�ԂɒʐM�N���b�N�T�C�N����ݒ�
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase3 ---\n" );
#endif
	GetScorePhase3();

	///////////////////////////////////////////////////////////////////////////////////
	// (4)�X�P�W���[�����O�A���W�X�^�A���P�[�V�����y�у��W�X�^�o�C���f�B���O
	// ���s�N���b�N�T�C�N����������
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase4 ---\n" );
#endif
	GetScorePhase4();

	///////////////////////////////////////////////////////////////////////////////////
	// (5)���W�X�^�A���P�[�V�������ʂɊ�Â����W���[����{�t���A�v�����ɓo�^
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase5 ---\n" );
#endif
	GetScorePhase5();

	///////////////////////////////////////////////////////////////////////////////////
	// (6)�{�t���A�v�����œK��
	// �N���b�N����������
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase6 ---\n" );
#endif
	GetScorePhase6();

	///////////////////////////////////////////////////////////////////////////////////
	// (7)�������s����=���s�N���b�N�T�C�N�����~�N���b�N�������X�R�A�Ƃ��ĕԂ�
#ifdef DEBUG_VERBOSE_SCORE
	TRACE( "CBinding::GetScorePhase7 ---\n" );
#endif
	GetScorePhase7();

	return m_nScore;
}

// (1)���o�C���f�B���O�Ɋ�Â����W���[��(���Z��A���W�X�^��)�����t���A�v�����ɓo�^
void CBinding::GetScorePhase1(void)
{
	int nIndexFUType;
	int nIndexNode;
	int nModuleWidth,nModuleHeight;
	int k;
	int n;
	int nFU,nFUactual;
	int nNodeCount;
	int *aNodeFU;
	int *aFUUsage;
	int nCountModuleFU;
	int *aTranslateBindingFUIndexToModuleIndex;

	m_FloorplanPro.Clear();

	// ���Z��ނ��ƂɌ��݂̃o�C���f�B���O�Ɋ�Â��Ď��ۂɗ��p���鉉�Z�퐔�𒲂ׂ�
	nCountModuleFU = 0;
	for( nIndexFUType=0 ; nIndexFUType<=2 ; nIndexFUType++ ){
		TRACE( "CBinding::GetScore: nIndexFUType=%d -----------------\n", nIndexFUType );
		switch( nIndexFUType ){
		case 0:	// ���Z(add)
			nNodeCount = m_nNodeAdd;	// ���Z�m�[�h�̐�
			aNodeFU = m_aNodeAdd;	// ���Z�m�[�h�̃m�[�h�ԍ����L�^����z��
			nFU = m_nAdd;		// ���p�\�ȉ��Z�퐔
			aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexAdd;
			break;
		case 1:	// ���Z(sub)
			nNodeCount = m_nNodeSub;
			aNodeFU = m_aNodeSub;
			nFU = m_nSub;
			aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexSub;
			break;
		default:	// ��Z(mul)
			nNodeCount = m_nNodeMul;
			aNodeFU = m_aNodeMul;
			nFU = m_nMul;
			aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexMul;
			break;
		}
		aFUUsage = (int *)GlobalAlloc( GPTR, nFU*sizeof(int) );
		for( k=0 ; k<nFU ; k++ ) aFUUsage[k] = 0;
		for( nIndexNode=0 ; nIndexNode<nNodeCount ; nIndexNode++ ){
			n = aNodeFU[nIndexNode];	// ���݂̉��Z���(nIndexFUType)�̃m�[�h�̂����AnIndexNode�Ԗڂ̃m�[�h�̔ԍ����擾����n�ɑ��
			aFUUsage[m_aBinding[n]] = 1;	// �m�[�hn���o�C���f�B���O����Ă��鉉�Z��ԍ�(m_aBinding[n])�̉��Z��ɗ��p�ς݂̈������
											//TRACE( "nIndexNode=%d, N=%s, ==> %d\n", nIndexNode, m_DFG.m_node[n].N, m_aBinding[n] );
		}
		// ���p�\��nFU�̉��Z��̂����A���ۂ�1�ȏ�̃m�[�h���o�C���f�B���O����Ďg�p����鉉�Z��̐� ==> nFUactual
		nFUactual = 0;
		for( k=0 ; k<nFU ; k++ ){
			aTranslateBindingFUIndexToModuleIndex[k] = -1;
			if( aFUUsage[k] ){
				aTranslateBindingFUIndexToModuleIndex[k] = nFUactual;	// k�Ԗڂ̉��Z��ւ̃o�C���f�B���O�́A���ۂɎg�p���鉉�Z���nFUactual�Ԗڂ̉��Z��ւ̃o�C���f�B���O
				nFUactual++;
			}
			TRACE( "aTranslateBindingFUIndexToModuleIndex[%d]=%d\n", k, aTranslateBindingFUIndexToModuleIndex[k] ); 
		}
		GlobalFree( aFUUsage );
		TRACE( "nFUactual=%d\n", nFUactual );

		switch( nIndexFUType ){
		case 0:	// ���Z(add)
			m_nProCountModuleAdd = nFUactual;
			m_nProIndexModuleBaseAdd = nCountModuleFU;
			break;
		case 1:	// ���Z(sub)
			m_nProCountModuleSub = nFUactual;
			m_nProIndexModuleBaseSub = nCountModuleFU;
			break;
		default:	// ��Z(mul)
			m_nProCountModuleMul = nFUactual;
			m_nProIndexModuleBaseMul = nCountModuleFU;
			break;
		}
		nCountModuleFU += nFUactual;	// ���Z�A���Z�A��Z�̍��v���Z�퐔
	}

	// ���Z����t���A�v�����ɓo�^
	for( nIndexFUType=0 ; nIndexFUType<=2 ; nIndexFUType++ ){
		switch( nIndexFUType ){
		case 0:	// ���Z(add)
			nFUactual = m_nProCountModuleAdd;		// ���p�\�ȉ��Z�퐔
			nModuleWidth = MODULE_SIZE_ADD_WIDTH;
			nModuleHeight = MODULE_SIZE_ADD_HEIGHT;
			break;
		case 1:	// ���Z(sub)
			nFUactual = m_nProCountModuleSub;
			nModuleWidth = MODULE_SIZE_SUB_WIDTH;
			nModuleHeight = MODULE_SIZE_SUB_HEIGHT;
			break;
		default:	// ��Z(mul)
			nFUactual = m_nProCountModuleMul;
			nModuleWidth = MODULE_SIZE_MUL_WIDTH;
			nModuleHeight = MODULE_SIZE_MUL_HEIGHT;
			break;
		}
		// ���Z����t���A�v�����ɓo�^
		for( k=0 ; k<nFUactual ; k++ ){
			m_FloorplanPro.AddModule( nModuleWidth, nModuleHeight );
		}
	}
	// �e���Z��ɏo�̓f�[�^�p�̃��W�X�^��1���o�^
	m_nProCountReg = nCountModuleFU;
	m_nProIndexBaseReg = nCountModuleFU;
	for( k=0 ; k<m_nProCountReg ; k++ ){
		m_FloorplanPro.AddModule( MODULE_SIZE_REG_WIDTH, MODULE_SIZE_REG_HEIGHT );
	}
	//TRACE( "m_nProIndexBaseReg=%d, m_nProCountReg=%d\n", m_nProIndexBaseReg, m_nProCountReg );

	// ���݂̃o�C���f�B���O�Ɋ�Â��ĉ��t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
	ClearCommunicationRequirementPro();
	AnalyzeInterModuleCommunicationRequirementsPro();
	/////////////////////////
	m_listCommunicationRequirement = m_listCommunicationRequirementPro;
	ShowCommunicationRequirement();
	////////////////////////*/

	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "Modules: A=%d, S=%d, M=%d, R=%d\n", m_nProCountModuleAdd, m_nProCountModuleSub, m_nProCountModuleMul, m_nProCountReg );
	fprintf( fp, "\n" );
	fprintf( fp, "Binding: m_listCommunicationRequirementPro=%0X\n", m_listCommunicationRequirementPro );
	fclose( fp );*/

	m_FloorplanPro.SetCommunicationRequirement(m_listCommunicationRequirementPro);


}

// (2)���t���A�v�����œK��
void CBinding::GetScorePhase2(void)
{
	//m_SAforFloorplanPro.SetWindowHandle( m_hWnd );
	//m_SAforFloorplanPro.RegisterSolution( &m_FloorplanPro );
	//m_SAforFloorplanPro.Initialize( m_fTempStart, m_fTempEnd, m_fRate, m_nIterate, FALSE/*MaximizeScore*/ );
	m_SAforFloorplanPro.Initialize();//CSimulatedAnnealing�^

	int nResultCode = m_SAforFloorplanPro.Execute();
	if( nResultCode != 1 ){
		CString str;
		switch( nResultCode ){
		case 2:
			str = L"Can't open .csv file";
			break;
		default:
			str.Format( L"Unknow error in Execute() of simulated annealing\nResultCode = %d", nResultCode );
			break;
		}
		AfxMessageBox( str, MB_ICONSTOP|MB_OK );
		return;
	}
}

// (3)���t���A�v�����Ɋ�Â��A���Z�ԂɒʐM�N���b�N�T�C�N����ݒ�
void CBinding::GetScorePhase3(void)
{
	//AnalyzeInterModuleCommunicationRequirementsPro();
	//m_listCommunicationRequirement = m_listCommunicationRequirementPro;
	//ShowCommunicationRequirement();
	AddCommPipelineRegisters();
}

// (4)�X�P�W���[�����O�A���W�X�^�A���P�[�V�����y�у��W�X�^�o�C���f�B���O
void CBinding::GetScorePhase4(void)
{
	ListScheduleWithComminicationDelay();
	AddRegistersFinal();
}

// (5)���W�X�^�A���P�[�V�������ʂɊ�Â����W���[����{�t���A�v�����ɓo�^
void CBinding::GetScorePhase5(void)
{
	int nIndexFUType;
	int nModuleWidth,nModuleHeight;
	int k;
	int nFUactual;
	int nIndexRegInstance;

	m_FloorplanFinal.Clear();

	// ���Z����t���A�v�����ɓo�^
	for( nIndexFUType=0 ; nIndexFUType<=2 ; nIndexFUType++ ){
		switch( nIndexFUType ){
		case 0:	// ���Z(add)
			nFUactual = m_nProCountModuleAdd;		// ���p�\�ȉ��Z�퐔
			nModuleWidth = MODULE_SIZE_ADD_WIDTH;
			nModuleHeight = MODULE_SIZE_ADD_HEIGHT;
			break;
		case 1:	// ���Z(sub)
			nFUactual = m_nProCountModuleSub;
			nModuleWidth = MODULE_SIZE_SUB_WIDTH;
			nModuleHeight = MODULE_SIZE_SUB_HEIGHT;
			break;
		default:	// ��Z(mul)
			nFUactual = m_nProCountModuleMul;
			nModuleWidth = MODULE_SIZE_MUL_WIDTH;
			nModuleHeight = MODULE_SIZE_MUL_HEIGHT;
			break;
		}
		// ���Z����t���A�v�����ɓo�^
		for( k=0 ; k<nFUactual ; k++ ){
			m_FloorplanFinal.AddModule( nModuleWidth, nModuleHeight );
		}
	}
	// ���W�X�^��o�^
	for( nIndexRegInstance=0 ; nIndexRegInstance<m_nCountAllocatedRegInstance ; nIndexRegInstance++ ){
		m_FloorplanFinal.AddModule( MODULE_SIZE_REG_WIDTH, MODULE_SIZE_REG_HEIGHT );
	}

	// ���݂̃o�C���f�B���O�Ɋ�Â��Ė{�t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
	ClearCommunicationRequirementFinal();
	AnalyzeInterModuleCommunicationRequirementsFinal();
	/////////////////////////////////
	m_listCommunicationRequirement = m_listCommunicationRequirementFinal;
	ShowCommunicationRequirement();
	/////////////////////////////////*/

	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "Modules: A=%d, S=%d, M=%d, R=%d\n", m_nProCountModuleAdd, m_nProCountModuleSub, m_nProCountModuleMul, m_nProCountReg );
	fprintf( fp, "\n" );
	fprintf( fp, "Binding: m_listCommunicationRequirementPro=%0X\n", m_listCommunicationRequirementPro );
	fclose( fp );*/

	m_FloorplanFinal.SetCommunicationRequirement(m_listCommunicationRequirementFinal);

}

// (6)�{�t���A�v�����œK�� ==> �N���b�N����������
void CBinding::GetScorePhase6(void)
{
	m_SAforFloorplanFinal.Initialize();

	int nResultCode = m_SAforFloorplanFinal.Execute();
	if( nResultCode != 1 ){
		CString str;
		switch( nResultCode ){
		case 2:
			str = L"Can't open .csv file";
			break;
		default:
			str.Format( L"Unknow error in Execute() of simulated annealing\nResultCode = %d", nResultCode );
			break;
		}
		AfxMessageBox( str, MB_ICONSTOP|MB_OK );
		return;
	}
}

// (7)�������s����=���s�N���b�N�T�C�N�����~�N���b�N�������X�R�A�Ƃ��ĕԂ�
void CBinding::GetScorePhase7(void)
{
	m_FloorplanFinal.RestoreBest();
	m_nClockPeriod = m_FloorplanFinal.GetScore();

	m_nScore = m_nClockPeriod * m_nClockCycles;
	TRACE( "%d nClockPeriod=%d, m_nClockCycles=%d, m_nScore=%d\n", m_nID, m_nClockPeriod, m_nClockCycles, m_nScore );

//	WriteBestFloorplanFinal( 10+m_nID );

}

void CBinding::Setup(void)
{
	m_FloorplanPro.SetDFG( m_pDFG );
	m_FloorplanFinal.SetDFG( m_pDFG );
	m_SAforFloorplanPro.RegisterSolution( &m_FloorplanPro );
	m_SAforFloorplanFinal.RegisterSolution( &m_FloorplanFinal );
}

int CBinding::Initialize()
{
	int n;
	int m;
	int nFUCount;
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;

	m_aPriority = (int *)GlobalAlloc( GPTR, N*sizeof(int) );

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

	m_aBinding = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	m_aBindingBest = (int *)GlobalAlloc( GPTR, N*sizeof(int) );

	if( m_aTranslateBindingFUIndexToModuleIndexAdd ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexAdd );
	if( m_aTranslateBindingFUIndexToModuleIndexSub ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexSub );
	if( m_aTranslateBindingFUIndexToModuleIndexMul ) GlobalFree( m_aTranslateBindingFUIndexToModuleIndexMul );
	if( m_nAdd ){
		m_aTranslateBindingFUIndexToModuleIndexAdd = (int *)GlobalAlloc( GPTR, m_nAdd*sizeof(int) );
	}
	if( m_nSub ){
		m_aTranslateBindingFUIndexToModuleIndexSub = (int *)GlobalAlloc( GPTR, m_nSub*sizeof(int) );
	}
	if( m_nMul ){
		m_aTranslateBindingFUIndexToModuleIndexMul = (int *)GlobalAlloc( GPTR, m_nMul*sizeof(int) );
	}

	// �����o�C���f�B���O
//	srand((unsigned int) time(0)); /* ���ݎ������擾���ė����V�[�h������������B */
	for( n=0 ; n<N ; n++ ){
		switch( *node[n].T ){
		case 'A': nFUCount = m_nAdd; break;
		case 'S': nFUCount = m_nSub; break;
		default: nFUCount = m_nMul; break;
		}
		//m = (int)((rand() / ((double) RAND_MAX + 1.0)) * nNodeCount); /* [0, nNodeCount-1] �͈̔͂̒l�̂����ꂩ���Ԃ�B */
		m = (int)((rand()* nFUCount / (1.0+ RAND_MAX)));	// 0��n1��nNodeCount-1�̗��������߂�
		m_aBinding[n] = m;

	}

	return 0;
}

#define DEBUG_VERBOSE
// ���݂̃o�C���f�B���O�Ɋ�Â��ĉ��t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
void CBinding::AnalyzeInterModuleCommunicationRequirementsPro(void)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	EDGE *eptr;
	int nT;
	int nF;
	int nFUTypeF,nFUTypeT;
	int nIndexModuleT;
	int nIndexModuleF;
	int nIndexRegT;
	int nIndexRegF;

	///////////////////////////////////
	m_listCommunicationRequirement = m_listCommunicationRequirementPro;
	///////////////////////////////////

#ifdef DEBUG_VERBOSE_FUNCTION
	TRACE( "%d CBinding::AnalyzeInterModuleCommunicationRequirementsPro ==========\n", m_nID );
#endif
	for( nT=0 ; nT<N ; nT++ ){//N�̓m�[�h�̌�
		nIndexModuleT = GetModuleIndexBoundToNode( nT );
		nIndexRegT = m_nProIndexBaseReg+nIndexModuleT;
		switch( *node[nT].T ){
		case 'A': nFUTypeT = 0; break;
		case 'S': nFUTypeT = 1; break;
		default: nFUTypeT = 2; break;
		}
		for( eptr=node[nT].ilist ; eptr ; eptr=eptr->ilist ){
			nF = eptr->f->n;
#ifdef DEBUG_VERBOSE
			TRACE( "Node %s -> %s: ", eptr->f->N, node[nT].N );
#endif
			nIndexModuleF = GetModuleIndexBoundToNode( nF );
			nIndexRegF = m_nProIndexBaseReg+nIndexModuleF;
			switch( *node[nF].T ){
			case 'A': nFUTypeF = 0; break;
			case 'S': nFUTypeF = 1; break;
			default: nFUTypeF = 2; break;
			}
#ifdef DEBUG_VERBOSE
			TRACE( ": Node %s(%c) bound to %d, ", eptr->f->N, *node[nF].T, m_aBinding[nF] );
			TRACE( "Node %s(%c) bound to %d, ", node[nT].N, *node[nT].T, m_aBinding[nT] );
			TRACE( "nIndexModuleF=%d, nIndexModuleT=%d, nIndexRegF=%d, nIndexRegT=%d\n", nIndexModuleF, nIndexModuleT, nIndexRegF, nIndexRegT );
#endif
			/*FILE *fp;
			fopen_s( &fp, "status.txt", "a" );
			fprintf( fp, "nIndexModuleF=%d, nIndexModuleT=%d, nIndexRegF=%d, nIndexRegT=%d\n", nIndexModuleF, nIndexModuleT, nIndexRegF, nIndexRegT );
			fclose( fp );*/
#if 0
			// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			// �m�[�hnF����Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			// �m�[�hnF�����Z/���Z�A�m�[�hnT����Z�̏ꍇ:	�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			// �m�[�hnF����Z�A�m�[�hnT����Z:				�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			// �@�ȉ��̂悤�ɐ���
			// �m�[�hnF����Z�̏ꍇ: �m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			// �m�[�hnT����Z�̏ꍇ: �m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			if( nFUTypeF == 2 ){	// �m�[�hnF����Z
				AddCommunicationRequirementFUToReg( nIndexModuleF, nFUTypeF, nIndexRegF );
			}else if( nFUTypeT == 2 ){	// �m�[�hnT����Z
				AddCommunicationRequirementRegToFU( nIndexRegF, nIndexModuleT, nFUTypeT );
			}else{	// �m�[�hnF�����Z/���Z�A�m�[�hn�����Z/���Z�̏ꍇ
				AddCommunicationRequirementRegFUReg( nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegT );
			}
#endif
			// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			// �m�[�hnF�����Z/���Z�A�m�[�hnT����Z�̏ꍇ:	�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			// �m�[�hnF����Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			//											�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			// �m�[�hnF����Z�A�m�[�hnT����Z:				�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			//											�m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			if( nFUTypeF < 2 && nFUTypeT < 2 ){	// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z
				AddCommunicationRequirementRegFUReg( nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegT );
			}else if( nFUTypeF < 2 && nFUTypeT == 2 ){	// �m�[�hnF�����Z/���Z�A�m�[�hnT����Z
				AddCommunicationRequirementRegToFU( nIndexRegF, nIndexModuleT, nFUTypeT );
			}else if( nFUTypeF == 2 && nFUTypeT < 2 ){	// �m�[�hnF����Z�A�m�[�hnT�����Z/���Z
				AddCommunicationRequirementFUToReg( nIndexModuleF, nFUTypeF, nIndexRegF );
				AddCommunicationRequirementRegFUReg( nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegT );
			}else{	// �m�[�hnF����Z�A�m�[�hnT����Z
				AddCommunicationRequirementRegToFU( nIndexRegF, nIndexModuleT, nFUTypeT );
				AddCommunicationRequirementFUToReg( nIndexModuleF, nFUTypeF, nIndexRegF );
			}
		}
	}
	///////////////////////////////////
	m_listCommunicationRequirementPro = m_listCommunicationRequirement;
	///////////////////////////////////

}

void CBinding::AddCommunicationRequirementRegFUReg(int nIndexRegF, int nIndexModuleFU, int nFUtype, int nIndexRegT)
{
	COMREQUIREMENT *crptr;
	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		if( crptr->nIndexModuleSource==nIndexRegF && crptr->nIndexModuleVia==nIndexModuleFU && crptr->nIndexModuleDest==nIndexRegT ) break;
	}
	if( crptr ) return;
	crptr = (COMREQUIREMENT *)GlobalAlloc( GPTR, sizeof(COMREQUIREMENT) );
	crptr->nIndexModuleSource = nIndexRegF;
	crptr->nIndexModuleDest = nIndexRegT;
	crptr->nIndexModuleVia = nIndexModuleFU;
	crptr->nDelayModuleSource = 0;
	crptr->nDelayModuleDest = 0;
	switch( nFUtype ){
	case 0:
		crptr->nDelayModuleVia = DURATION_ADD;
		break;
	case 1:
		crptr->nDelayModuleVia = DURATION_SUB;
		break;
	default:
		crptr->nDelayModuleVia = 0;	// ���W�X�^�Ԃ̒ʐM���Ԃ̕]���ɂ����āA���W�X�^�̊Ԃɏ�Z�킪����P�[�X�͍l���Ȃ�(��Z��̓p�C�v���C��������Ă���)
		break;
	}
	crptr->next = m_listCommunicationRequirement;
	m_listCommunicationRequirement = crptr;
	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "nIndexModuleSource=%d, nIndexModuleVia=%d, nIndexModuleDest=%d\n", nIndexRegF, nIndexModuleFU, nIndexRegT );
	fclose( fp );*/
}

void CBinding::AddCommunicationRequirementFUToReg(int nIndexModuleFU, int nFUtype, int nIndexRegT)
{
	COMREQUIREMENT *crptr;
	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		if( crptr->nIndexModuleSource==nIndexModuleFU && crptr->nIndexModuleVia<0 && crptr->nIndexModuleDest==nIndexRegT ) break;
	}
	if( crptr ) return;
	crptr = (COMREQUIREMENT *)GlobalAlloc( GPTR, sizeof(COMREQUIREMENT) );
	crptr->nIndexModuleSource = nIndexModuleFU;
	crptr->nIndexModuleDest = nIndexRegT;
	crptr->nIndexModuleVia = -1;
	crptr->nDelayModuleVia = 0;
	crptr->nDelayModuleDest = 0;
	switch( nFUtype ){
	case 0:
		crptr->nDelayModuleSource = DURATION_ADD;	// ���̃P�[�X�͎��ۂɂ͂��肦�Ȃ�
		break;
	case 1:
		crptr->nDelayModuleSource = DURATION_SUB;	// ���̃P�[�X�͎��ۂɂ͂��肦�Ȃ�
		break;
	default:
		crptr->nDelayModuleSource = DURATION_MUL2;	// ��Z��̍ŏI�i�x������
		break;
	}
	crptr->next = m_listCommunicationRequirement;
	m_listCommunicationRequirement = crptr;
	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "nIndexModuleSource=%d, nIndexModuleDest=%dR\n", nIndexModuleFU, nIndexRegT );
	fclose( fp );*/
}

void CBinding::AddCommunicationRequirementRegToFU(int nIndexRegF, int nIndexModuleFU, int nFUtype)
{
	COMREQUIREMENT *crptr;
	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		if( crptr->nIndexModuleSource==nIndexRegF && crptr->nIndexModuleVia<0 && crptr->nIndexModuleDest==nIndexModuleFU ) break;
	}
	if( crptr ) return;
	crptr = (COMREQUIREMENT *)GlobalAlloc( GPTR, sizeof(COMREQUIREMENT) );
	crptr->nIndexModuleSource = nIndexRegF;
	crptr->nIndexModuleDest = nIndexModuleFU;
	crptr->nIndexModuleVia = -1;
	crptr->nDelayModuleVia = 0;
	crptr->nDelayModuleSource = 0;
	switch( nFUtype ){
	case 0:
		crptr->nDelayModuleDest = DURATION_ADD;	// ���̃P�[�X�͎��ۂɂ͂��肦�Ȃ�
		break;
	case 1:
		crptr->nDelayModuleDest = DURATION_SUB;	// ���̃P�[�X�͎��ۂɂ͂��肦�Ȃ�
		break;
	default:
		crptr->nDelayModuleDest = DURATION_MUL1;	// ��Z��̑�1�i�x������
		break;
	}
	crptr->next = m_listCommunicationRequirement;
	m_listCommunicationRequirement = crptr;
	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "nIndexModuleSource=%dR, nIndexModuleDest=%d\n", nIndexRegF, nIndexModuleFU );
	fclose( fp );*/
}

void CBinding::AddCommunicationRequirementRegToReg(int nIndexRegF, int nIndexRegT)
{
	COMREQUIREMENT *crptr;
	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		if( crptr->nIndexModuleSource==nIndexRegF && crptr->nIndexModuleVia<0 && crptr->nIndexModuleDest==nIndexRegT ) break;
	}
	if( crptr ) return;
	crptr = (COMREQUIREMENT *)GlobalAlloc( GPTR, sizeof(COMREQUIREMENT) );
	crptr->nIndexModuleSource = nIndexRegF;
	crptr->nIndexModuleDest = nIndexRegT;
	crptr->nIndexModuleVia = -1;
	crptr->nDelayModuleVia = 0;
	crptr->nDelayModuleSource = 0;
	crptr->nDelayModuleDest = 0;
	crptr->next = m_listCommunicationRequirement;
	m_listCommunicationRequirement = crptr;
	/*FILE *fp;
	fopen_s( &fp, "status.txt", "a" );
	fprintf( fp, "nIndexModuleSource=%dR, nIndexModuleDest=%d\n", nIndexRegF, nIndexModuleFU );
	fclose( fp );*/
}

void CBinding::ClearCommunicationRequirementPro(void)	//Score1 ���݂̃o�C���f�B���O�Ɋ�Â��ĉ��t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
{
	COMREQUIREMENT *crptr;//OMREQUIREMENT int�^��Source Dest Via�Ǝ����ւ�*next������ 
	while(m_listCommunicationRequirementPro){//COMREQUIREMENT�^
		crptr = m_listCommunicationRequirementPro;
		m_listCommunicationRequirementPro = m_listCommunicationRequirementPro->next;
		GlobalFree( crptr );
	}
}

void CBinding::ClearCommunicationRequirementFinal(void)
{
	COMREQUIREMENT *crptr;
	while(m_listCommunicationRequirementFinal){
		crptr = m_listCommunicationRequirementFinal;
		m_listCommunicationRequirementFinal = m_listCommunicationRequirementFinal->next;
		GlobalFree( crptr );
	}
}

void CBinding::ShowCommunicationRequirement(void)// ���݂̃o�C���f�B���O�Ɋ�Â��ĉ��t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
{
	TRACE( "CFloorplan::ShowCommunicationRequirement ===========\n" );
	COMREQUIREMENT *crptr;
	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		if( crptr->nIndexModuleVia >= 0 ){
			TRACE( "Module %d ==> Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleVia, crptr->nIndexModuleDest );
		}else{
			TRACE( "Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleDest );
		}
		TRACE( " :" );
		if( crptr->nDelayModuleSource > 0 ) TRACE( " DelayModuleSource=%d", crptr->nDelayModuleSource );
		if( crptr->nDelayModuleVia > 0 ) TRACE( " nDelayModuleVia=%d", crptr->nDelayModuleVia );
		if( crptr->nDelayModuleDest > 0 ) TRACE( " nDelayModuleDest=%d", crptr->nDelayModuleDest );
		TRACE( "\n" );
	}
}

int CBinding::GetModuleIndexBoundToNode(int n)
{
	NODE *node = m_pDFG->m_node;
	int nIndexModuleBase;
	int *aTranslateBindingFUIndexToModuleIndex;

	switch( *node[n].T ){
	case 'A':
		nIndexModuleBase = m_nProIndexModuleBaseAdd;
		aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexAdd;
		break;
	case 'S':
		nIndexModuleBase = m_nProIndexModuleBaseSub;
		aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexSub;
		break;
	default:
		nIndexModuleBase = m_nProIndexModuleBaseMul;
		aTranslateBindingFUIndexToModuleIndex = m_aTranslateBindingFUIndexToModuleIndexMul;
		break;
	}
	return nIndexModuleBase+aTranslateBindingFUIndexToModuleIndex[m_aBinding[n]];
	//int *m_aBinding; n�Ԗڃm�[�h�����蓖�Ă�ꂽFu�̌ŗL�l�ƃx�[�X�ƂȂ�nIndexModuleBase�𑫂�
}

#define FILENAME_PRO_BEST_BINDING "BestBindingPro.txt"
#define FILENAME_PRO_GLOBAL_BEST_BINDING "GlobalBestBindingPro.txt"
void CBinding::WriteBestBinding(BOOL bGlobalBest)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	int n;
	FILE *fp;

	if( bGlobalBest ){
		fopen_s( &fp, FILENAME_PRO_GLOBAL_BEST_BINDING, "w" );
	}else{
		fopen_s( &fp, FILENAME_PRO_BEST_BINDING, "w" );
	}
	for( n=0 ; n<N ; n++ ){
		switch( *node[n].T ){
		case 'A':
			fprintf( fp, "%s Add%d\n", node[n].N, m_aBindingBest[n] );
			break;
		case 'S':
			fprintf( fp, "%s Sub%d\n", node[n].N, m_aBindingBest[n] );
			break;
		default:
			fprintf( fp, "%s Mul%d\n", node[n].N, m_aBindingBest[n] );
			break;
		}
	}
	fclose( fp );
}

#define FILENAME_FINAL_BEST_SCHEDULE "BestScheduleFinal.txt"
#define FILENAME_FINAL_GLOBAL_BEST_SCHEDULE "GlobalBestScheduleFinal.txt"
void CBinding::WriteBestSchedule(BOOL bGlobalBest)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	int n;
	FILE *fp;

	if( bGlobalBest ){
		fopen_s( &fp, FILENAME_FINAL_GLOBAL_BEST_SCHEDULE, "w" );
	}else{
		fopen_s( &fp, FILENAME_FINAL_BEST_SCHEDULE, "w" );
	}
	for( n=0 ; n<N ; n++ ){
		fprintf( fp, "%s %d\n", node[n].N, node[n].Time );
	}
	fclose( fp );
}
/*
1 Add2
2 Add0
3 Add1
4 Mul1
*/
#define FILENAME_PRO_BEST_BINDING_FOR_TEST "BestBindingPro1.txt"
#define SEP " \t"
void CBinding::LoadBinding(void)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	int n;
	char buf[256];
	FILE *fp;
	char *pTokenContext;
	char *ptr;
	char *name;

	if( fopen_s( &fp, FILENAME_PRO_BEST_BINDING_FOR_TEST, "r" ) ){
		return;
	}
	for(;;){
		fgets( buf, 255, fp );
		if( feof(fp) ) break;

		name = strtok_s( buf, SEP, &pTokenContext );	/* n */
		for( n=0 ; n<N ; n++ ){
			if( !strcmp(name,node[n].N) ) break;
		}
		if( n >= N ) continue;	// Something is wrong if this condition is true.
		//
		ptr = strtok_s( NULL, SEP, &pTokenContext );	/* name */
		while( *ptr < '0' || *ptr > '9' ) ptr++;
		m_aBinding[n] = atoi( ptr );
	}
	fclose( fp );

	for( n=0 ; n<N ; n++ ){
		TRACE( "Node %s: FU %d\n", node[n].N, m_aBinding[n] );
	}
}

#define FILENAME_PRO_BEST_FLOORPLAN "BestFloorplanPro.out"
#define FILENAME_PRO_GLOBAL_BEST_FLOORPLAN "GlobalBestFloorplanPro.out"
void CBinding::WriteBestFloorplanPro(BOOL bGlobalBest)
{
	int nModule = m_FloorplanPro.GetModuleNum();
	int m;
	int k;
	int nIndex;
	int x, y, nOrientation;
	int w, h;
	FILE *fp;
	if( bGlobalBest ){
		fopen_s( &fp, FILENAME_PRO_GLOBAL_BEST_FLOORPLAN, "w" );
	}else{
		fopen_s( &fp, FILENAME_PRO_BEST_FLOORPLAN, "w" );
	}

	fprintf( fp, "*Score\n" );
	fprintf( fp, "%d\n", m_nScore );
	fprintf( fp, "\n" );
	fprintf( fp, "*Module_data\n" );
	m = 0;
	// ���Z�탂�W���[��
	for( k=0 ; k<m_nAdd ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleAdd ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexAdd[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleAdd ) continue;	// m_nAdd�̉��Z��̂����Ak�Ԗڂ̉��Z��͉��Z���o�C���f�B���O���ꂸ���g�p
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Add%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nSub ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleSub ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexSub[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleSub ) continue;
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Sub%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nMul ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleMul ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexMul[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleMul ) continue;
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Mul%d %d %d\n", k, w, h );
		m++;
	}
	// ���W�X�^���W���[��
	for( k=0 ; k<m_nAdd ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleAdd ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexAdd[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleAdd ) continue;
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "RegA%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nSub ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleSub ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexSub[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleSub ) continue;
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "RegS%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nMul ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleMul ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexMul[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleMul ) continue;
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "RegM%d %d %d\n", k, w, h );
		m++;
	}

	fprintf( fp, "\n" );
	fprintf( fp, "*Coordinate\n" );
	for( m=0 ; m<nModule ; m++ ){
		m_FloorplanPro.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "%d %d %d\n", x, y, nOrientation );
	}
	fclose(fp);
}

#define FILENAME_FINAL_BEST_FLOORPLAN "BestFloorplanFinal.out"
#define FILENAME_FINAL_GLOBAL_BEST_FLOORPLAN "GlobalBestFloorplanFinal.out"
void CBinding::WriteBestFloorplanFinal(BOOL bGlobalBest)
{
	int nModule = m_FloorplanFinal.GetModuleNum();
	int m;
	int k;
	int nIndex;
	int x, y, nOrientation;
	int w, h;
	FILE *fp;
	if( bGlobalBest ){
		if( bGlobalBest > 1 ){
			char filename[64];
			sprintf_s( filename, "FloorplanFinal%d.out", bGlobalBest );
			fopen_s( &fp, filename, "w" );
		}else{
			fopen_s( &fp, FILENAME_FINAL_GLOBAL_BEST_FLOORPLAN, "w" );
		}
	}else{
		fopen_s( &fp, FILENAME_FINAL_BEST_FLOORPLAN, "w" );
	}

	fprintf( fp, "*Score\n" );
	fprintf( fp, "%d\n", m_nScore );
	fprintf( fp, "\n" );
	fprintf( fp, "*Module_data\n" );
	m = 0;
	// ���Z�탂�W���[��
	for( k=0 ; k<m_nAdd ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleAdd ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexAdd[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleAdd ) continue;	// m_nAdd�̉��Z��̂����Ak�Ԗڂ̉��Z��͉��Z���o�C���f�B���O���ꂸ���g�p
		m_FloorplanFinal.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Add%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nSub ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleSub ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexSub[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleSub ) continue;
		m_FloorplanFinal.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Sub%d %d %d\n", k, w, h );
		m++;
	}
	for( k=0 ; k<m_nMul ; k++ ){
		for( nIndex=0 ; nIndex<m_nProCountModuleMul ; nIndex++ ){
			if( m_aTranslateBindingFUIndexToModuleIndexMul[k] == nIndex ) break;
		}
		if( nIndex>=m_nProCountModuleMul ) continue;
		m_FloorplanFinal.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Mul%d %d %d\n", k, w, h );
		m++;
	}
	// ���W�X�^���W���[��
	k = 0;
	for( ; m<nModule ; m++ ){
		m_FloorplanFinal.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "Reg%d %d %d\n", k++, w, h );
	}

	fprintf( fp, "\n" );
	fprintf( fp, "*Coordinate\n" );
	for( m=0 ; m<nModule ; m++ ){
		m_FloorplanFinal.GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
		fprintf( fp, "%d %d %d\n", x, y, nOrientation );
	}
	fclose(fp);
}

// ���̊֐����ĂԑO�� CBinding::Initialize �����s���Ă�������
// m_aBinding, m_aNodeAdd, m_aTranslateBindingFUIndexToModuleIndexAdd, �Ȃǂ̔z��� CBinding::Initialize �Ŋm�ۂ����
// �܂��Am_nNodeAdd, m_aNodeAdd �Ȃǂ͐������ݒ肳���
#define FILENAME_PRO_BEST_FLOORPLAN_FOR_TEST "BestFloorplanPro1.out"
void CBinding::LoadFloorplanPro(void)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	int k;
	char c;
	char buf[256];
	FILE *fp;
	char *pTokenContext;
	char *ptr;
	int nMode;
	int nIndex;
	int nCountModuleFU;

	if( fopen_s( &fp, FILENAME_PRO_BEST_FLOORPLAN_FOR_TEST, "r" ) ){
		return;
	}

	// ���Z���ނ��ƂɎ��ۂɉ��Z���o�C���f�B���O����Ďg�p����Ă��鉉�Z�퐔�𒲂ׂ�
	int *aAddUsage = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	int *aSubUsage = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	int *aMulUsage = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
	for( nIndex=0 ; nIndex<N ; nIndex++ ){
		aAddUsage[nIndex] = 0;
		aSubUsage[nIndex] = 0;
		aMulUsage[nIndex] = 0;
	}
	nMode = 0;
	for(;;){
		fgets( buf, 255, fp );
		if( feof(fp) ) break;

		if( buf[0] == '\n' ){
			nMode = 0;
			continue;
		}
		if( !strncmp(buf, "*Module_data", 12) ){
			nMode = 1;
			continue;
		}
		if( nMode == 1 ){
			ptr = buf;	/* Module name */
			c = *ptr;	// ���W���[��(���Z����)�̐擪����
			while( *ptr < '0' || *ptr > '9' ) ptr++;
			nIndex = atoi( ptr );
			switch( c ){
			case 'a':
			case 'A':
				aAddUsage[nIndex] = 1;
				break;
			case 's':
			case 'S':
				aSubUsage[nIndex] = 1;
				break;
			case 'm':
			case 'M':
				aMulUsage[nIndex] = 1;
				break;
			}
		}
	}
	fclose(fp);

	m_nProCountModuleAdd = 0;
	m_nProCountModuleSub = 0;
	m_nProCountModuleMul = 0;
	for( nIndex=0 ; nIndex<N ; nIndex++ ){
		if( aAddUsage[nIndex] ) m_nProCountModuleAdd++;
		if( aSubUsage[nIndex] ) m_nProCountModuleSub++;
		if( aMulUsage[nIndex] ) m_nProCountModuleMul++;
	}

	nIndex = 0;
	for( k=0 ; k<m_nAdd ; k++ ){
		if( aAddUsage[k] ){
			m_aTranslateBindingFUIndexToModuleIndexAdd[k] = nIndex++;	// �p�ӂ���Ă���m_nAdd�̉��Z���k�Ԗڂ͎g�p����Ă���A���ۂɂ�nIndex�Ԗڂ̉��Z��
		}else{
			m_aTranslateBindingFUIndexToModuleIndexAdd[k] = -1;	// k�Ԗڂ̉��Z��͉��Z���o�C���f�B���O����Ă��炸�g�p���Ȃ�
		}
	}
	nIndex = 0;
	for( k=0 ; k<m_nSub ; k++ ){
		if( aSubUsage[k] ){
			m_aTranslateBindingFUIndexToModuleIndexSub[k] = nIndex++;
		}else{
			m_aTranslateBindingFUIndexToModuleIndexSub[k] = -1;
		}
	}
	nIndex = 0;
	for( k=0 ; k<m_nMul ; k++ ){
		if( aMulUsage[k] ){
			m_aTranslateBindingFUIndexToModuleIndexMul[k] = nIndex++;
		}else{
			m_aTranslateBindingFUIndexToModuleIndexMul[k] = -1;
		}
	}
	nCountModuleFU = 0;
	m_nProIndexModuleBaseAdd = nCountModuleFU;
	nCountModuleFU += m_nProCountModuleAdd;
	m_nProIndexModuleBaseSub = nCountModuleFU;
	nCountModuleFU += m_nProCountModuleSub;
	m_nProIndexModuleBaseMul = nCountModuleFU;
	nCountModuleFU += m_nProCountModuleMul;
	// �e���Z��ɏo�̓f�[�^�p�̃��W�X�^��1���o�^
	m_nProCountReg = nCountModuleFU;
	m_nProIndexBaseReg = nCountModuleFU;

	// �t�@�C���ɋL�^���ꂽ���Ɋ�Â��ăt���A�v�����Ƀ��W���[���o�^
	int x,y,nOrientation;
	int w,h;
	fopen_s( &fp, FILENAME_PRO_BEST_FLOORPLAN_FOR_TEST, "r" );
	nMode = 0;
	for(;;){
		fgets( buf, 255, fp );
		if( feof(fp) ) break;

		if( buf[0] == '\n' ){
			nMode = 0;
			continue;
		}
		if( !strncmp(buf, "*Module_data", 12) ){
			nMode = 1;
			continue;
		}
		if( !strncmp(buf, "*Coordinate", 11) ){
			nMode = 2;
			nIndex = 0;
			continue;
		}
		if( nMode == 0 ){
			continue;
		}else if( nMode == 1 ){
			// ���W���[���̃T�C�Y�Ɋւ�����
			// �f�[�^�t�@�C���ɂ́A���Z��A���Z��A��Z��̏��ɋL�^����Ă���Ɖ��肷��
			ptr = strtok_s( buf, SEP, &pTokenContext );	/* Module name */
			while( *ptr < '0' || *ptr > '9' ) ptr++;
			nIndex = atoi( ptr );
			//
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			w = atoi( ptr );
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			h = atoi( ptr );
			m_FloorplanPro.AddModule( w, h );
		}else if( nMode == 2 ){
			// ���W���[���̍��W(x,y)����щ�](nOrientation)��ݒ�
			// �܂��o�^�ς݂̃��W���[���T�C�Y(w,h)���擾
			m_FloorplanPro.GetModuleStatus( nIndex, &x, &y, &w, &h, &nOrientation );
			ptr = strtok_s( buf, SEP, &pTokenContext );	/* Module name */
			x = atoi( ptr );
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			y = atoi( ptr );
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			nOrientation = atoi( ptr );
			m_FloorplanPro.SetModuleStatus( nIndex, x, y, w, h, nOrientation );
			nIndex++;
		}
		//
	}
	fclose(fp);
}

/*
*Square
625.000000

*Module_data
Add0 16 3
Add1 16 3
Add2 16 3
Mul0 16 13
Mul1 16 13
RegA0 16 2
RegA1 16 2
RegA2 16 2
RegM0 16 2
RegM1 16 2

*Tm
1027.000000

*Coordinate
0 5 2
0 12 2
0 0 0
0 15 2
16 0 1
0 10 0
0 8 0
0 3 2
16 16 1
18 16 2
*/

void CBinding::TestAddRegisters(void)
{
	Initialize();
	LoadBinding();
	GetScorePhase1();

	GetScorePhase2();

	// (3)���t���A�v�����Ɋ�Â��A���Z�ԂɒʐM�N���b�N�T�C�N����ݒ�
	GetScorePhase3();
	// (4)�X�P�W���[�����O�A���W�X�^�A���P�[�V�����y�у��W�X�^�o�C���f�B���O ==> ���s�N���b�N�T�C�N����������
	GetScorePhase4();
	// (5)���W�X�^�A���P�[�V�������ʂɊ�Â����W���[����{�t���A�v�����ɓo�^
	GetScorePhase5();

	GetScorePhase6();
//	GetScorePhase7();

	m_FloorplanFinal.RestoreBest();
	m_nClockPeriod = m_FloorplanFinal.GetScore();
	m_nScore = m_nClockPeriod * m_nClockCycles;
	TRACE( "%d nClockPeriod=%d, m_nClockCycles=%d, m_nScore=%d\n", m_nID, m_nClockPeriod, m_nClockCycles, m_nScore );

	WriteBestFloorplanFinal();


}

#define CHUNK_POOL_ALLOCATION 100
INTERCONNECTIONTARGET *CBinding::GetElementInterconnectionTarget(void)
{
	INTERCONNECTIONTARGET *p;
	int k;
	if( m_nCountAllocatedInterconnectionTarget >= m_nCountPoolInterconnectionTarget ){
		p = (INTERCONNECTIONTARGET *)GlobalAlloc( GPTR, (m_nCountPoolInterconnectionTarget+CHUNK_POOL_ALLOCATION)*sizeof(INTERCONNECTIONTARGET) );
		for( k=0 ; k<m_nCountPoolInterconnectionTarget ; k++ ) p[k] = m_poolInterconnectionTarget[k];
		m_nCountPoolInterconnectionTarget += CHUNK_POOL_ALLOCATION;
		for(  ; k<m_nCountPoolInterconnectionTarget ; k++ ){
			p[k].n = k;
		}
		if( m_poolInterconnectionTarget ) GlobalFree( m_poolInterconnectionTarget );
		m_poolInterconnectionTarget = p;
	}
	p = &(m_poolInterconnectionTarget[m_nCountAllocatedInterconnectionTarget]);
	p->n = m_nCountAllocatedInterconnectionTarget;
	p->nIndexRegGroupContainerTop = -1;
	m_nCountAllocatedInterconnectionTarget++;
	return p;
}

REGGROUPCONTAINER *CBinding::GetElementRegGroupContainer(void)
{
	REGGROUPCONTAINER *p;
	int k;
	if( m_nCountAllocatedRegGroupContainer >= m_nCountPoolRegGroupContainer ){
		p = (REGGROUPCONTAINER *)GlobalAlloc( GPTR, (m_nCountPoolRegGroupContainer+CHUNK_POOL_ALLOCATION)*sizeof(REGGROUPCONTAINER) );
		for( k=0 ; k<m_nCountPoolRegGroupContainer ; k++ ) p[k] = m_poolRegGroupContainer[k];
		m_nCountPoolRegGroupContainer += CHUNK_POOL_ALLOCATION;
		for(  ; k<m_nCountPoolRegGroupContainer ; k++ ){
			p[k].n = k;
		}
		if( m_poolRegGroupContainer ) GlobalFree( m_poolRegGroupContainer );
		m_poolRegGroupContainer = p;
	}
	p = &(m_poolRegGroupContainer[m_nCountAllocatedRegGroupContainer]);
	p->n = m_nCountAllocatedRegGroupContainer;
	p->nIndexRegGroupTop = -1;
	m_nCountAllocatedRegGroupContainer++;
	return p;
}

REGGROUP *CBinding::GetElementRegGroup(void)
{
	REGGROUP *p;
	int k;
	if( m_nCountAllocatedRegGroup >= m_nCountPoolRegGroup ){
		p = (REGGROUP *)GlobalAlloc( GPTR, (m_nCountPoolRegGroup+CHUNK_POOL_ALLOCATION)*sizeof(REGGROUPCONTAINER) );
		for( k=0 ; k<m_nCountPoolRegGroup ; k++ ) p[k] = m_poolRegGroup[k];
		m_nCountPoolRegGroup += CHUNK_POOL_ALLOCATION;
		for(  ; k<m_nCountPoolRegGroup ; k++ ){
			p[k].n = k;
		}
		if( m_poolRegGroup ) GlobalFree( m_poolRegGroup );
		m_poolRegGroup = p;
	}
	p = &(m_poolRegGroup[m_nCountAllocatedRegGroup]);
	p->n = m_nCountAllocatedRegGroup;
	m_nCountAllocatedRegGroup++;
	return p;
}

REGINSTANCE *CBinding::GetElementRegInstance(void)
{
	REGINSTANCE *p;
	int k;
	int t;
	if( m_nCountAllocatedRegInstance >= m_nCountPoolRegInstance ){
		p = (REGINSTANCE *)GlobalAlloc( GPTR, (m_nCountPoolRegInstance+CHUNK_POOL_ALLOCATION)*sizeof(REGINSTANCE) );
		for( k=0 ; k<m_nCountPoolRegInstance ; k++ ) p[k] = m_poolRegInstance[k];
		m_nCountPoolRegInstance += CHUNK_POOL_ALLOCATION;
		for(  ; k<m_nCountPoolRegInstance ; k++ ){
			p[k].n = k;
			p[k].aBoundToReg = NULL;
		}
		if( m_poolRegInstance ) GlobalFree( m_poolRegInstance );
		m_poolRegInstance = p;
	}
	p = &(m_poolRegInstance[m_nCountAllocatedRegInstance]);
	p->n = m_nCountAllocatedRegInstance;
	p->aBoundToReg = (int *)GlobalAlloc( GPTR, m_nClockCycles*sizeof(int) );
	for( t=0 ; t<m_nClockCycles ; t++ ) p->aBoundToReg[t] = -1;
	p->nIndexRegToRegCommTop = -1;
	p->nIndexRegToFUCommTop = -1;
	m_nCountAllocatedRegInstance++;
	return p;
}

REGTOREGCOMM *CBinding::GetElementRegToRegComm(void)
{
	REGTOREGCOMM *p;
	int k;
	if( m_nCountAllocatedRegToRegComm >= m_nCountPoolRegToRegComm ){
		p = (REGTOREGCOMM *)GlobalAlloc( GPTR, (m_nCountPoolRegToRegComm+CHUNK_POOL_ALLOCATION)*sizeof(REGTOREGCOMM) );
		for( k=0 ; k<m_nCountPoolRegToRegComm ; k++ ) p[k] = m_poolRegToRegComm[k];
		m_nCountPoolRegToRegComm += CHUNK_POOL_ALLOCATION;
		if( m_poolRegToRegComm ) GlobalFree( m_poolRegToRegComm );
		m_poolRegToRegComm = p;
	}
	p = &(m_poolRegToRegComm[m_nCountAllocatedRegToRegComm]);
	p->n = m_nCountAllocatedRegToRegComm;
	m_nCountAllocatedRegToRegComm++;
	return p;
}

REGTOFUCOMM *CBinding::GetElementRegToFUComm(void)
{
	REGTOFUCOMM *p;
	int k;
	if( m_nCountAllocatedRegToFUComm >= m_nCountPoolRegToFUComm ){
		p = (REGTOFUCOMM *)GlobalAlloc( GPTR, (m_nCountPoolRegToFUComm+CHUNK_POOL_ALLOCATION)*sizeof(REGTOFUCOMM) );
		for( k=0 ; k<m_nCountPoolRegToFUComm ; k++ ) p[k] = m_poolRegToFUComm[k];
		m_nCountPoolRegToFUComm += CHUNK_POOL_ALLOCATION;
		if( m_poolRegToFUComm ) GlobalFree( m_poolRegToFUComm );
		m_poolRegToFUComm = p;
	}
	p = &(m_poolRegToFUComm[m_nCountAllocatedRegToFUComm]);
	p->n = m_nCountAllocatedRegToFUComm;
	m_nCountAllocatedRegToFUComm++;
	return p;
}

void CBinding::ClearRegistersFinal(void)
{
	if( m_poolRegGroupContainer ) GlobalFree( m_poolRegGroupContainer );
	m_poolRegGroupContainer = NULL;
	m_nCountPoolRegGroupContainer = 0;
	m_nCountAllocatedRegGroupContainer = 0;
	//
	if( m_poolRegGroup ) GlobalFree( m_poolRegGroup );
	m_poolRegGroup = NULL;
	m_nCountPoolRegGroup = 0;
	m_nCountAllocatedRegGroup = 0;
	//
	if( m_poolRegInstance ){
		for( int k=0 ; k<m_nCountAllocatedRegInstance ; k++ ) GlobalFree( m_poolRegInstance[k].aBoundToReg );
		GlobalFree( m_poolRegInstance );
	}
	m_poolRegInstance = NULL;
	m_nCountPoolRegInstance = 0;
	m_nCountAllocatedRegInstance = 0;
	//
	if( m_poolRegToRegComm ) GlobalFree( m_poolRegToRegComm );
	m_poolRegToRegComm = NULL;
	m_nCountPoolRegToRegComm = 0;
	m_nCountAllocatedRegToRegComm = 0;
	//
	if( m_poolRegToFUComm ) GlobalFree( m_poolRegToFUComm );
	m_poolRegToFUComm = NULL;
	m_nCountPoolRegToFUComm = 0;
	m_nCountAllocatedRegToFUComm = 0;

}

#define THRESHOLD_CLOCK_PERIOD 17
void CBinding::AddCommPipelineRegisters(void)
{
#ifdef DEBUG_VERBOSE_FUNCTION
	TRACE( "%d CBinding::AddCommPipelineRegisters ------------------\n", m_nID );
#endif
	COMREQUIREMENT *crptr;
	int nXs,nYs,nXd,nYd,nXv1,nYv1,nXv2,nYv2;
	int nDelay;
	int nPipelineStages;
	int nIndexFU;
	int nIndexFUsrc,nIndexFUdest;
	int k;
	int nProCountFU = m_nProCountModuleAdd+m_nProCountModuleSub+m_nProCountModuleMul;

	if( m_aInterFUConnection ) GlobalFree( m_aInterFUConnection );
	m_aInterFUConnection = (INTERCONNECTION *)GlobalAlloc( GPTR, nProCountFU*sizeof(INTERCONNECTION) );
	/////////////////////////////////////////////////
	ClearRegistersFinal();


	INTERCONNECTIONTARGET *ictptr;
	REGGROUPCONTAINER *rgcptr;
	REGGROUP *rgptr,*rgptr0;
	int *rgcnext;

	nIndexFU = 0;
	for( k=0 ; k<m_nProCountModuleAdd ; k++ ){
		m_aInterFUConnection[nIndexFU].n = nIndexFU;
		m_aInterFUConnection[nIndexFU].nTypeFU = 0;	// Add
		m_aInterFUConnection[nIndexFU].nIndexFU = k;
		m_aInterFUConnection[nIndexFU].nIndexInterconnectionTargetTop = -1;
		nIndexFU++;
	}
	for( k=0 ; k<m_nProCountModuleSub ; k++ ){
		m_aInterFUConnection[nIndexFU].n = nIndexFU;
		m_aInterFUConnection[nIndexFU].nTypeFU = 1;	// Sub
		m_aInterFUConnection[nIndexFU].nIndexFU = k;
		m_aInterFUConnection[nIndexFU].nIndexInterconnectionTargetTop = -1;
		nIndexFU++;
	}
	for( k=0 ; k<m_nProCountModuleMul ; k++ ){
		m_aInterFUConnection[nIndexFU].n = nIndexFU;
		m_aInterFUConnection[nIndexFU].nTypeFU = 2;	// Mul
		m_aInterFUConnection[nIndexFU].nIndexFU = k;
		m_aInterFUConnection[nIndexFU].nIndexInterconnectionTargetTop = -1;
		nIndexFU++;
	}

#ifdef DEBUG_VERBOSE
	TRACE( "--- Communication Delay\n" );
#endif
	for( crptr=m_listCommunicationRequirementPro ; crptr ; crptr=crptr->next ){
		// ���t���A�v�����Ɋ�Â��āA�ʐM�v��(�ʐM���A�ʐM�扉�Z��)�̒x�����Ԃ����߂�
#ifdef DEBUG_VERBOSE
		if( crptr->nIndexModuleVia >= 0 ){
			TRACE( "Module %d ==> Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleVia, crptr->nIndexModuleDest );
		}else{
			TRACE( "Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleDest );
		}
		TRACE( " : DelayModuleSource=%d, nDelayModuleVia=%d, nDelayModuleDest=%d", crptr->nDelayModuleSource, crptr->nDelayModuleVia, crptr->nDelayModuleDest );
#endif
		m_FloorplanPro.GetCoordinateModuleTerminal(crptr->nIndexModuleSource, 1/*InOut*/, &nXs, &nYs);//crptr->nIndexModulSource�̏o�͒[�q�̍��W�����߂�
		m_FloorplanPro.GetCoordinateModuleTerminal(crptr->nIndexModuleDest, 0/*InOut*/, &nXd, &nYd);//crptr->nIndexModuleDest�̓��͒[�q�̍��W�����߂�*pnX/y���㏑��
		if( crptr->nIndexModuleVia >= 0 ){
			m_FloorplanPro.GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 0/*InOut*/, &nXv1, &nYv1);
			m_FloorplanPro.GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 1/*InOut*/, &nXv2, &nYv2);
			nDelay = m_FloorplanPro.CalcWireDelay(nXs, nYs, nXv1, nYv1);
			nDelay += m_FloorplanPro.CalcWireDelay(nXv2, nYv2, nXd, nYd);//�}���n�b�^�����������߂�
#ifdef DEBUG_VERBOSE
			TRACE( " (%d,%d)->(%d,%d)[](%d,%d)->(%d,%d)", nXs, nYs, nXv1, nYv1, nXv2, nYv2, nXd, nYd );
#endif
		}else{
			nDelay = m_FloorplanPro.CalcWireDelay(nXs, nYs, nXd, nYd);
#ifdef DEBUG_VERBOSE
			TRACE( " (%d,%d)->(%d,%d)", nXs, nYs, nXd, nYd );
#endif
		}
		nDelay += crptr->nDelayModuleSource;
		nDelay += crptr->nDelayModuleVia;
		nDelay += crptr->nDelayModuleDest;
		//
#ifdef DEBUG_VERBOSE
		TRACE( ":: Delay=%d", nDelay );
		TRACE( "\n" );
#endif
		//
		// �x�����ԂɊ�Â��ĒʐM�����Z��̏o�͂ɐڑ����郌�W�X�^�i�������肷��
		if( nDelay <= THRESHOLD_CLOCK_PERIOD ){
			nPipelineStages = 1;
		}else{
			nPipelineStages = 2;	// ���ɒǉ����郌�W�X�^��2�i�Ƃ���(���Z��o�͂��󂯎�郌�W�X�^��1�i�ڂƍl����)
		}
		if( crptr->nIndexModuleVia >= 0 ){//crptr=COMREQUIREMENT
//			TRACE( "Module %d ==> Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleVia, crptr->nIndexModuleDest );
			// crptr->nIndexModuleSource�̓��W�X�^, crptr->nIndexModuleVia�͒��Ԃ̉��Z��/���Z��, crptr->nIndexModuleDest�̓��W�X�^(���ԉ��Z��̌��ʂ��󂯎�郌�W�X�^)
			// crptr->nIndexModuleSource�Ɍ��ʂ�ۑ�������Z��/���Z�킩��A���Ԃ̉��Z��/���Z��܂ł̊ԂɃ��W�X�^��ǉ�
			nIndexFUsrc = crptr->nIndexModuleSource - m_nProIndexBaseReg;	// ���W�X�^�ԍ����烌�W�X�^�ԍ���������ƁA���Z��ԍ���������
			nIndexFUdest = crptr->nIndexModuleVia;
		}else if( crptr->nIndexModuleSource >= m_nProIndexBaseReg ){
//			TRACE( "Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleDest );
			// crptr->nIndexModuleSource�̓��W�X�^, crptr->nIndexModuleDest�͏�Z��
			nIndexFUsrc = crptr->nIndexModuleSource - m_nProIndexBaseReg;	// ���W�X�^�ԍ����烌�W�X�^�ԍ���������ƁA���Z��ԍ���������
			nIndexFUdest = crptr->nIndexModuleDest;
		}else{
			// crptr->nIndexModuleSource�͏�Z��, crptr->nIndexModuleDest�͏�Z��̏o�͂��󂯎�郌�W�X�^
//			TRACE( "Module %d ==> Module %d", crptr->nIndexModuleSource, crptr->nIndexModuleDest );
			continue;
			// ���Ƀ��W�X�^�ǉ��K�v�Ȃ�
		}
#ifdef DEBUG_VERBOSE
		TRACE( " (FU %d ==> FU %d) %d Regs\n", nIndexFUsrc, nIndexFUdest, nPipelineStages );
#endif
		// �ʐM�����Z�킩��ʐM��扉�Z��֌������ʐM�H��ǉ�
		ictptr = GetElementInterconnectionTarget();//ictptr=INTERCONNECTIONTARGET
		ictptr->nPipelineStages = nPipelineStages;
		ictptr->nIndexTargetFU = nIndexFUdest;
		ictptr->nIndexInterconnectionTargetNext = m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop;
		m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop = ictptr->n;
	}

#ifdef DEBUG_VERBOSE
	TRACE( "--- Insert Register Groups\n" );
#endif
	int *aRegGroupForInput= (int *)GlobalAlloc( GPTR, nProCountFU*sizeof(int) );	// ���Z��ɒ��ړ��̓f�[�^��^���郌�W�X�^�O���[�v�̔ԍ�
	for( nIndexFUdest=0 ; nIndexFUdest<nProCountFU ; nIndexFUdest++ ) aRegGroupForInput[nIndexFUdest] = -1;

	int nIndexInterconnectionTarget;
	int nIndexRegGroupContainer;
	for( nIndexFUsrc=0 ; nIndexFUsrc<nProCountFU ; nIndexFUsrc++ ){
		// ���Z��nIndexFUsrc�̌��ʂ𒼐ڕۑ����鉉�Z��o�̓��W�X�^�̃��W�X�^�O���[�v��p��
		rgptr0 = GetElementRegGroup();
		rgptr0->nIndexRegInstanceTop = -1;
		// �ʐM�����Z�킩��ʐM�扉�Z��֌������ʐM�H�ɁA�i��nPipelineStages�ɉ����ă��W�X�^�O���[�v��ǉ�
		nIndexInterconnectionTarget = m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop;
		nIndexFUdest = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexTargetFU;
		for( ; nIndexInterconnectionTarget>=0 ; nIndexInterconnectionTarget=m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexInterconnectionTargetNext ){
			nPipelineStages = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nPipelineStages;
			rgcnext = &(m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexRegGroupContainerTop);
			rgcptr = GetElementRegGroupContainer();
			rgcptr->nIndexRegGroupTop = rgptr0->n;
			rgcptr->nIndexRegGroupContainerNext = -1;
			*rgcnext = rgcptr->n;
			rgcnext = &(rgcptr->nIndexRegGroupContainerNext);
			for( k=1 ; k<nPipelineStages ; k++ ){
				rgcptr = GetElementRegGroupContainer();
				if( k == nPipelineStages-1 ){
					// �ʐM��̒��O�̃��W�X�^�O���[�v
					if( aRegGroupForInput[nIndexFUdest] >= 0 ){
						rgptr = &(m_poolRegGroup[aRegGroupForInput[nIndexFUdest]]);
					}else{
						rgptr = GetElementRegGroup();
						aRegGroupForInput[nIndexFUdest] = rgptr->n;
					}
				}else{
					rgptr = GetElementRegGroup();
				}
				rgptr->nIndexRegInstanceTop = -1;
				rgcptr->nIndexRegGroupTop = rgptr->n;
				rgcptr->nIndexRegGroupContainerNext = -1;
				*rgcnext = rgcptr->n;
				rgcnext = &(rgcptr->nIndexRegGroupContainerNext);
			}
		}
	}
#ifdef DEBUG_VERBOSE
	// �m�F
	for( nIndexFUsrc=0 ; nIndexFUsrc<nProCountFU ; nIndexFUsrc++ ){
		nIndexInterconnectionTarget = m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop;
		while( nIndexInterconnectionTarget>=0 ){
			TRACE( "FU %d", nIndexFUsrc );
			nIndexRegGroupContainer = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexRegGroupContainerTop;
			while( nIndexRegGroupContainer>=0 ){
				TRACE( " -> RGC(%d)[RGindex=%d]", m_poolRegGroupContainer[nIndexRegGroupContainer].n, m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupTop );
				nIndexRegGroupContainer = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupContainerNext;
			}
			nIndexFUdest = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexTargetFU;
			TRACE( " -> FU %d\n", nIndexFUdest );
			//
			nIndexInterconnectionTarget = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexInterconnectionTargetNext;
		}
		
	}
#endif
	GlobalFree( aRegGroupForInput );

}

void CBinding::ListScheduleWithComminicationDelay(void)
{
	int N = m_pDFG->N;//
	NODE *node = m_pDFG->m_node;
	EDGE *eptr;
	int k,n;
	int nIndexModuleF,nIndexModuleT;
	int nF,nT;
	int nProCountFU = m_nProCountModuleAdd+m_nProCountModuleSub+m_nProCountModuleMul;
	int nIndexInterconnectionTarget;
	
	// �ʐM�N���b�N�T�C�N�����e�}�ɉ�����
	for( nT=0 ; nT<N ; nT++ ){
		nIndexModuleT = GetModuleIndexBoundToNode( nT );//�Ԓl:nIndexModuleBase+aTranslateBindingFUIndexToModuleIndex[m_aBinding[n]]
		for( eptr=node[nT].ilist ; eptr ; eptr=eptr->ilist ){
			nF = eptr->f->n;//�Ώۃm�[�hnT�̓��̓G�b�W�̃t�����m�[�h�̔z���̃C���f�b�N��nF�Ɋi�[
			nIndexModuleF = GetModuleIndexBoundToNode( nF );
			//
			nIndexInterconnectionTarget = m_aInterFUConnection[nIndexModuleF].nIndexInterconnectionTargetTop;
			while( nIndexInterconnectionTarget>=0 ){
				if( nIndexModuleT == m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexTargetFU ) break;
				nIndexInterconnectionTarget = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexInterconnectionTargetNext;
			}
			if( nIndexInterconnectionTarget < 0 ) continue;	// ���̏󋵂ɂ͂Ȃ�Ȃ��͂�
			eptr->com = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nPipelineStages-1;
		}
	}

	// ���Z�m�[�h�̗D��x�����߂�
	int m,t,t0;
	BOOL bUpdate;

	for( n=0 ; n<N ; n++ ){
		if( node[n].olist == NULL ){
			node[n].nPriority = -node[n].C;
		}else{
			node[n].nPriority = INTINF;
		}
	}

	// Bellman-Ford�@��p�����Œ��o�H���̉��Ƃ��āAALAP�X�P�W���[�������߂�
	for( m=0 ; m<N ; m++ ){
		bUpdate = TRUE;
		for( n=0 ; n<N ; n++ ){
			for( eptr=node[n].olist ; eptr ; eptr=eptr->olist ){
				t = eptr->t->nPriority - node[n].C - eptr->com;
				if( node[n].nPriority > t ){
					node[n].nPriority = t;
					bUpdate = FALSE;
				}
			}
		}
		if( bUpdate ) break;
	}

	if( !bUpdate ){
		fprintf( stderr, "ALAP scheduling does not converge.\n" );
		exit(1);
	}
	int *w = (int *)GlobalAlloc( GPTR, N*sizeof(int) );
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

	/*///////////////////////////////
	// �D�揇�ʊm�F
	for( k=0 ; k<N ; k++ ){
		n = m_aPriority[k];
		TRACE( "Node %s, Priority=%d\n", node[n].N, node[n].nPriority );
	}
	TRACE( "-----------\n" );
	///////////////////////////////*/


	//
#define SCHEDULING_CHUNK 100
	NODE *pNodeFrom;
	int nIndexModule;
	int nTime;
	int nLen;
	int nLatency;
	int nAllocatedTime = SCHEDULING_CHUNK;
	int **aAllocatedToFU = (int **)GlobalAlloc( GPTR, nProCountFU*sizeof(int *) );
	for( nIndexModule=0 ; nIndexModule<nProCountFU ; nIndexModule++ ){
		aAllocatedToFU[nIndexModule] = (int *)GlobalAlloc( GPTR, SCHEDULING_CHUNK*sizeof(int) );
		for( t=0 ; t<nAllocatedTime ; t++ ) aAllocatedToFU[nIndexModule][t] = -1;
	}

	for( n=0 ; n<N ; n++ ) node[n].fix = 0;
	for( k=0 ; k<N ; k++ ){
		// �D�揇�ʂ̍����m�[�h���珇�ɒ��ׂ�
		n = m_aPriority[k];
		if( node[n].fix ) continue;
		// �m�[�hn�̉��Z��ނɉ����ĉ��Z�험�p�󋵔z��(aAllocatedToFU)�A���Z�퐔(nMaxFU)�A���Z����(nLatency)�A�p�C�v���C���Ԋu(nLen)���擾
//		IdentifyOperation( n, &aAllocatedToFU, &nMaxFU, &nLatency, &nLen );
		nLatency = node[n].C;
		nLen = node[n].L;
		nIndexModule = GetModuleIndexBoundToNode( n );
		// �m�[�hn�̎��s�J�n�ő����������߂�
		node[n].LB = 0;
		for( eptr=node[n].ilist ; eptr ; eptr=eptr->ilist ){
			pNodeFrom = eptr->f;
			if( node[n].LB < pNodeFrom->Time + pNodeFrom->C + eptr->com ){
				node[n].LB = pNodeFrom->Time + pNodeFrom->C + eptr->com;
			}
		}
		if( *node[n].T == 'Z' ){
			nTime = node[n].LB;
		}else{
			for( nTime=node[n].LB ; ; nTime++ ){
				for( t0=0 ; t0<nLen ; t0++ ){
					//
					if( nTime+t0 >= nAllocatedTime ){
						int **aAllocatedToFUnew = (int **)GlobalAlloc( GPTR, nProCountFU*sizeof(int *) );
						for( nIndexModule=0 ; nIndexModule<nProCountFU ; nIndexModule++ ){
							aAllocatedToFUnew[nIndexModule] = (int *)GlobalAlloc( GPTR, nAllocatedTime+SCHEDULING_CHUNK*sizeof(int) );
							for( t=0 ; t<nAllocatedTime ; t++ ) aAllocatedToFUnew[nIndexModule][t] = aAllocatedToFU[nIndexModule][t];
							for( ; t<nAllocatedTime+SCHEDULING_CHUNK ; t++ ) aAllocatedToFUnew[nIndexModule][t] = -1;
							GlobalFree( aAllocatedToFU[nIndexModule] );
							aAllocatedToFU[nIndexModule] = aAllocatedToFUnew[nIndexModule];
						}
						nAllocatedTime += SCHEDULING_CHUNK;
						GlobalFree( aAllocatedToFUnew );
					}
					//
					if( aAllocatedToFU[nIndexModule][nTime+t0] >= 0 ) break;
				}
				if( t0>=nLen ) break;
			}
			for( t0=0 ; t0<nLen ; t0++ ){
				aAllocatedToFU[nIndexModule][nTime+t0] = n;
			}
		}
		node[n].Time = nTime;
	}

	m_nClockCycles = 0;
	for( n=0 ; n<N ; n++ ){
		if( m_nClockCycles < node[n].Time+node[n].C ){
			m_nClockCycles = node[n].Time+node[n].C;
		}
	}
	/*///////////////////////////////
	char filename[64];
	sprintf_s( filename, "schedule%d.txt", m_nID );
	FILE *fp;
	fopen_s( &fp, filename, "w" );
	fprintf( fp, "m_nClockCycles=%d\n", m_nClockCycles );
	for( int n=0 ; n<N ; n++ ){
		fprintf( fp, "Node %s, Time=%d, C=%d\n", node[n].N, node[n].Time, node[n].C );
	}
	fclose(fp);
	///////////////////////////////*/

	/*///////////////////////////////
	// �X�P�W���[���m�F
	TRACE( "   :" );
	for( nIndexModule=0 ; nIndexModule<nProCountFU ; nIndexModule++ ){
		if( nIndexModule < m_nProIndexModuleBaseSub ){
			TRACE( "A%d|", nIndexModule-m_nProIndexModuleBaseAdd );
		}else if( nIndexModule < m_nProIndexModuleBaseMul ){
			TRACE( "S%d|", nIndexModule-m_nProIndexModuleBaseSub );
		}else{
			TRACE( "M%d|", nIndexModule-m_nProIndexModuleBaseMul );
		}
	}
	TRACE( "\n" );
	for( t=0 ; t<m_nClockCycles ; t++ ){
		TRACE( "%3d:", t );
		for( nIndexModule=0 ; nIndexModule<nProCountFU ; nIndexModule++ ){
			if( aAllocatedToFU[nIndexModule][t] >= 0 ){
				TRACE( "%2s,", node[aAllocatedToFU[nIndexModule][t]].N );
			}else{
				TRACE( "  ," );
			}
		}
		TRACE( "\n" );
	}
	///////////////////////////////*/

	for( nIndexModule=0 ; nIndexModule<nProCountFU ; nIndexModule++ ) GlobalFree( aAllocatedToFU[nIndexModule] );
	GlobalFree( aAllocatedToFU );

#ifdef DEBUG_VERBOSE_FUNCTION
	TRACE( "%d CBinding::ListScheduleWithComminicationDelay obtained m_nClockCycles=%d ------\n", m_nID, m_nClockCycles );
#endif
}

void CBinding::AddRegistersFinal(void)
{
	int N = m_pDFG->N;
	NODE *node = m_pDFG->m_node;
	EDGE *eptr;
	int nLifetimeStart,nLifetimeEnd;
	int nF,nT;
	int nIndexModuleF,nIndexModuleT;
	int nIndexInterconnectionTarget;
	int nIndexRegGroupContainer;
	int nIndexRegGroupContainerNext;
	int nIndexRegGroup;
	int nIndexRegInstance;
	int nIndexRegInstancePrev;
	int nIndexRegToRegComm;
	int nIndexRegToFUComm;
	REGINSTANCE *riptr;
	REGTOREGCOMM *rtrcptr;
	REGTOFUCOMM *rtfcptr;
	int *nRiptr;
	int k;
	int t,t0;
	int TL;

	for( k=0 ; k<N ; k++ ){
		// �D�揇�ʂ̍����m�[�h���珇�ɒ��ׂ�
		nF = m_aPriority[k];
		nIndexModuleF = GetModuleIndexBoundToNode( nF );
		nLifetimeStart = node[nF].Time+node[nF].C;//���s�J�n���ԁ{���s����
		for( eptr=node[nF].olist ; eptr ; eptr=eptr->olist ){
			nT = eptr->t->n;//�G�b�W��To�m�[�h�̃C���f�b�N�X
			nIndexModuleT = GetModuleIndexBoundToNode( nT );
			nLifetimeEnd = node[nT].Time; //nLifetimeStart-nLifetimeEnd���G�b�W(���Z����)�̃��C�t�^�C��
			if( nLifetimeEnd > m_nClockCycles ){
				char filename[64];
				sprintf_s( filename, "error%d.txt", m_nID );
				FILE *fp;
				fopen_s( &fp, filename, "w" );
				fprintf( fp, "nLifetimeEnd=%d, m_nClockCycles=%d\n", nLifetimeEnd, m_nClockCycles );
				for( int n=0 ; n<N ; n++ ){
					fprintf( fp, "Node %s, Time=%d, C=%d\n", node[n].N, node[n].Time, node[n].C );
				}
				fclose(fp);
				TRACE( "error\n" );
				exit(1);
			}
#ifdef DEBUG_VERBOSE
			TRACE( "%s(%d) -> %s(%d) Lifetime=[%d,%d] -----------------\n", node[nF].N, nIndexModuleF, node[nT].N, nIndexModuleT, nLifetimeStart, nLifetimeEnd );
#endif
			//
			nIndexInterconnectionTarget = m_aInterFUConnection[nIndexModuleF].nIndexInterconnectionTargetTop;
			while( nIndexInterconnectionTarget >= 0 ){
				if( m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexTargetFU == nIndexModuleT ) break;
				nIndexInterconnectionTarget = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexInterconnectionTargetNext;
			}
			if( nIndexInterconnectionTarget < 0 ) continue;	// ���̏����ɂȂ邱�Ƃ͂��肦�Ȃ�
			t = nLifetimeStart;
			nIndexRegInstancePrev = -1; 
			nIndexRegGroupContainer = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexRegGroupContainerTop;
			while( nIndexRegGroupContainer >= 0 ){
				nIndexRegGroupContainerNext = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupContainerNext;
				if( nIndexRegGroupContainerNext >= 0 ){
					TL = t;//�Ō�̃��W�X�^�O���[�v�łȂ��Ȃ�f�[�^���C�t�^�C���̃X�^�[�g���L�^
				}else{
					// �Ō�̃��W�X�^�O���[�v�Ȃ�΁A�f�[�^���C�t�^�C���̍Ō�܂ŋL�^
					TL = nLifetimeEnd;
				}
				nIndexRegGroup = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupTop;
				// ���W�X�^�O���[�vnIndexRegGroup�ɉ��Z�m�[�hnF�̌��ʂ�����t�ɂ����ĕێ��ł�����̂����邩�T��
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroup].nIndexRegInstanceTop;
				while( nIndexRegInstance >= 0 ){
					for( t0=t ; t0<=TL ; t0++ ){
						if( ! (m_poolRegInstance[nIndexRegInstance].aBoundToReg[t0] == nF || m_poolRegInstance[nIndexRegInstance].aBoundToReg[t0] < 0) ) break;
					}
					if( t0 > TL ) break;	// nF�̌��ʂ�ێ��ł��郌�W�X�^����������
					nIndexRegInstance = m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext;
				}
				if( nIndexRegInstance < 0 ){
					// �ێ��Ɏg���郌�W�X�^�����݂��Ȃ�
					// �V�K�Ƀ��W�X�^���擾���A���W�X�^�O���[�v�̖����ɒǉ�
					nRiptr = &(m_poolRegGroup[nIndexRegGroup].nIndexRegInstanceTop);
					while( *nRiptr >= 0 ){
						nIndexRegInstance = *nRiptr;
						nRiptr = &(m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext);
					}
					riptr = GetElementRegInstance();
					nIndexRegInstance = riptr->n;
					m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext = -1;
					*nRiptr = nIndexRegInstance;
				}
				for( t0=t ; t0<=TL ; t0++ )	m_poolRegInstance[nIndexRegInstance].aBoundToReg[t0] = nF;
#ifdef DEBUG_VERBOSE
				TRACE( " bound to RG%d, Reg%d [%d,%d]", nIndexRegGroup, nIndexRegInstance, t, TL );
#endif
				// ���O�̃��W�X�^����ʐM�v�����L�^
				if( nIndexRegInstancePrev >= 0 ){
					nIndexRegToRegComm = m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToRegCommTop;
					while( nIndexRegToRegComm >= 0 ){
						if( m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegInstance == nIndexRegInstance ) break;	// ���łɓ����ʐM�v�����L�^�ς�
						nIndexRegToRegComm = m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegToRegCommNext;
					}
					if( nIndexRegToRegComm < 0 ){
						// �V�K�ɒʐM�v�����L�^
						rtrcptr = GetElementRegToRegComm();
						rtrcptr->nIndexRegInstance = nIndexRegInstance;
						rtrcptr->nIndexRegToRegCommNext = m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToRegCommTop;
						m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToRegCommTop = rtrcptr->n;
					}
				}
				//
				nIndexRegInstancePrev = nIndexRegInstance; 
				t = TL+1;

				nIndexRegGroupContainer = nIndexRegGroupContainerNext;
			}
#ifdef DEBUG_VERBOSE
			TRACE( "\n" );
#endif
			// �ʐM�扉�Z��̒��O�̃��W�X�^(nIndexRegInstancePrev)����A���̉��Z��ւ̒ʐM�v�����L�^
			if( nIndexRegInstancePrev >= 0 ){
				nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToFUCommTop;
				while( nIndexRegToFUComm >= 0 ){
					if( m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU == nIndexModuleT ) break;	// ���łɓ����ʐM�v�����L�^�ς�
					nIndexRegToFUComm = m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext;
				}
				if( nIndexRegToFUComm < 0 ){
					// �V�K�ɒʐM�v�����L�^
					rtfcptr = GetElementRegToFUComm();
					rtfcptr->nIndexFU = nIndexModuleT;
					rtfcptr->nIndexRegToFUCommNext = m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToFUCommTop;
					m_poolRegInstance[nIndexRegInstancePrev].nIndexRegToFUCommTop = rtfcptr->n;
				}
			}
		}
	}

#ifdef DEBUG_VERBOSE
	// ���ʊm�F
	TRACE( "m_nCountAllocatedRegGroup=%d, m_nCountAllocatedRegInstance=%d\n", m_nCountAllocatedRegGroup, m_nCountAllocatedRegInstance );
	TRACE( " T :" );
	for( nIndexRegGroup=0 ; nIndexRegGroup<m_nCountAllocatedRegGroup ; nIndexRegGroup++ ){
		nIndexRegInstance = m_poolRegGroup[nIndexRegGroup].nIndexRegInstanceTop;
		for( ; nIndexRegInstance >= 0 ; nIndexRegInstance = m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
			TRACE( "%2d:%2d ", nIndexRegGroup, nIndexRegInstance );
		}
	}
	TRACE( "\n" );
	for( t=0 ; t<m_nClockCycles ; t++ ){
		TRACE( "%3d:", t );
		for( nIndexRegGroup=0 ; nIndexRegGroup<m_nCountAllocatedRegGroup ; nIndexRegGroup++ ){
			nIndexRegInstance = m_poolRegGroup[nIndexRegGroup].nIndexRegInstanceTop;
			for( ; nIndexRegInstance >= 0 ; nIndexRegInstance = m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
				TRACE( "  %2s ,", m_poolRegInstance[nIndexRegInstance].aBoundToReg[t]>=0 ? node[m_poolRegInstance[nIndexRegInstance].aBoundToReg[t]].N : "  " );
			}
		}
		TRACE( "\n" );
	}
	for( nIndexRegInstance=0 ; nIndexRegInstance<m_nCountAllocatedRegInstance ; nIndexRegInstance++ ){
		nIndexRegToRegComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToRegCommTop;
		if( nIndexRegToRegComm < 0 ) continue;
		TRACE( "Reg %d ==> ", nIndexRegInstance );
		for( ; nIndexRegToRegComm >= 0 ; nIndexRegToRegComm = m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegToRegCommNext ){
			TRACE( "Reg %d, ", m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegInstance );
		}
		TRACE( "\n" );
	}
	for( nIndexRegInstance=0 ; nIndexRegInstance<m_nCountAllocatedRegInstance ; nIndexRegInstance++ ){
		nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToFUCommTop;
		if( nIndexRegToFUComm < 0 ) continue;
		TRACE( "Reg %d ==> ", nIndexRegInstance );
		for( ; nIndexRegToFUComm >= 0 ; nIndexRegToFUComm = m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext ){
			TRACE( "FU %d, ", m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU );
		}
		TRACE( "\n" );
	}
#endif
}

// ���݂̃o�C���f�B���O�Ɋ�Â��Ė{�t���A�v�����p�Ƀ��W���[���ԒʐM�v���𒲂ׂ�
void CBinding::AnalyzeInterModuleCommunicationRequirementsFinal(void)
{
	int nFUTypeF,nFUTypeT;
	int nIndexModuleT;
	int nIndexModuleF;
	int nIndexRegT;
	int nIndexRegF;
	int nIndexFUsrc,nIndexFUdest;
	int nProCountFU = m_nProCountModuleAdd+m_nProCountModuleSub+m_nProCountModuleMul;
	int nIndexInterconnectionTarget;
	int nIndexInterconnectionTargetT;
	int nIndexRegGroupContainer;
	int nIndexRegInstance;
	int nIndexRegGroupContainerT;
	int nIndexRegGroup0,nIndexRegGroupLast,nIndexRegGroupT;
	int nIndexRegInstanceT;
	int nIndexRegToRegComm;
	int nIndexRegToFUComm;

	///////////////////////////////////
	m_listCommunicationRequirement = m_listCommunicationRequirementFinal;
	///////////////////////////////////

#ifdef DEBUG_VERBOSE_FUNCTION
	TRACE( "%d CBinding::AnalyzeInterModuleCommunicationRequirementsFinal ==========\n", m_nID );
#endif
	// ���Z���֗^���郌�W�X�^�Ԓx������
	for( nIndexFUsrc=0 ; nIndexFUsrc<nProCountFU ; nIndexFUsrc++ ){
		nIndexModuleF = nIndexFUsrc;
		if( nIndexFUsrc >= m_nProIndexModuleBaseMul ){
			nFUTypeF = 2;	// Mul
		}else if( nIndexFUsrc >= m_nProIndexModuleBaseSub ){
			nFUTypeF = 1;	// Sub
		}else{
			nFUTypeF = 0;	// Add
		}
		// �ʐM�����Z��nIndexFUsrc�̒���̃��W�X�^�O���[�v�𓾂� ==> nIndexRegGroup0
		nIndexInterconnectionTarget = m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop;
		if( nIndexInterconnectionTarget < 0 ) continue;	// nIndexFUsrc�́A�f�[�^�ʐM�悪�Ȃ����Z�� ==> nIndexFUsrc�����Ƃ���ʐM�v���͂Ȃ�
		nIndexRegGroupContainer = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexRegGroupContainerTop;
		if( nIndexRegGroupContainer >= 0 && m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupTop >= 0 ){
			nIndexRegGroup0 = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupTop;
		}
		//
		nIndexInterconnectionTarget = m_aInterFUConnection[nIndexFUsrc].nIndexInterconnectionTargetTop;
		for( ; nIndexInterconnectionTarget>=0 ; nIndexInterconnectionTarget=m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexInterconnectionTargetNext ){
			nIndexFUdest = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexTargetFU;
			nIndexModuleT = nIndexFUdest;
			if( nIndexFUdest >= m_nProIndexModuleBaseMul ){
				nFUTypeT = 2;	// Mul
			}else if( nIndexFUdest >= m_nProIndexModuleBaseSub ){
				nFUTypeT = 1;	// Sub
			}else{
				nFUTypeT = 0;	// Add
			}
#ifdef DEBUG_VERBOSE
			TRACE( "nIndexModuleF=%d, nIndexModuleT=%d ----\n", nIndexModuleF, nIndexModuleT );
#endif
			// �ʐM�扉�Z��nIndexFUdest�̒��O�̃��W�X�^�O���[�v�𓾂� ==> nIndexRegGroupLast
			nIndexRegGroupContainer = m_poolInterconnectionTarget[nIndexInterconnectionTarget].nIndexRegGroupContainerTop;
			while( nIndexRegGroupContainer>=0 ){
				if( m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupContainerNext < 0 ) break;
				nIndexRegGroupContainer = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupContainerNext;
			}
			if( nIndexRegGroupContainer < 0 ) continue;	// ���̏����͐������Ȃ��͂�
			nIndexRegGroupLast = m_poolRegGroupContainer[nIndexRegGroupContainer].nIndexRegGroupTop;
			//
			// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			// �m�[�hnF�����Z/���Z�A�m�[�hnT����Z�̏ꍇ:	�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			// �m�[�hnF����Z�A�m�[�hnT�����Z/���Z�̏ꍇ: �m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			//											�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̏o�̓f�[�^�p���W�X�^nIndexRegT�֒ʐM�v��(�r���ɉ��Z��/���Z��nIndexModuleT���o�R)
			// �m�[�hnF����Z�A�m�[�hnT����Z:				�m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF����m�[�hnT�̉��Z��(��Z��)nIndexModuleT�֒ʐM�v��
			//											�m�[�hnF�̉��Z��(��Z��)nIndexModuleF����m�[�hnF�̏o�̓f�[�^�p���W�X�^nIndexRegF�֒ʐM�v��
			if( nFUTypeF < 2 && nFUTypeT < 2 ){	// �m�[�hnF�����Z/���Z�A�m�[�hnT�����Z/���Z
				// nIndexRegGroupLast�̃��W�X�^����ʐM�扉�Z��nIndexFUdest�̒���̃��W�X�^�֒ʐM�v���ǉ�
				if( m_aInterFUConnection[nIndexFUdest].nIndexInterconnectionTargetTop < 0 ) continue;	// nIndexFUdest�̓f�[�^�ʐM��̂Ȃ����Z��̏ꍇ
				nIndexInterconnectionTargetT = m_aInterFUConnection[nIndexFUdest].nIndexInterconnectionTargetTop;
				nIndexRegGroupContainerT = m_poolInterconnectionTarget[nIndexInterconnectionTargetT].nIndexRegGroupContainerTop;
				if( nIndexRegGroupContainerT >= 0 && m_poolRegGroupContainer[nIndexRegGroupContainerT].nIndexRegGroupTop >= 0 ){
					nIndexRegInstance = m_poolRegGroup[nIndexRegGroupLast].nIndexRegInstanceTop;
					for( ; nIndexRegInstance>=0 ; nIndexRegInstance=m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
						// ���W�X�^nIndexRegInstance���牉�Z��nIndexModuleT�ɒʐM�v�������邩���ׂ�
						nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToFUCommTop;
						for( ; nIndexRegToFUComm>=0 ; nIndexRegToFUComm=m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext ){
							if( m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU == nIndexModuleT ) break;	// �ʐM�v������
						}
						if( nIndexRegToFUComm < 0 ) continue;	// ���W�X�^���牉�Z��ւ̒ʐM�v������
						nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
						nIndexRegGroupT = m_poolRegGroupContainer[nIndexRegGroupContainerT].nIndexRegGroupTop;
						nIndexRegInstanceT = m_poolRegGroup[nIndexRegGroupT].nIndexRegInstanceTop;
						for( ; nIndexRegInstanceT>=0 ; nIndexRegInstanceT=m_poolRegInstance[nIndexRegInstanceT].nIndexRegInstanceNext ){
							nIndexRegT = m_nProIndexBaseReg + nIndexRegInstanceT;
							AddCommunicationRequirementRegFUReg( nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegT );	////
#ifdef DEBUG_VERBOSE
							TRACE( "Reg %d(%d) => FU %d (type=%d) => Reg %d(%d)\n", nIndexRegInstance, nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegInstanceT, nIndexRegT );
#endif
						}
					}
				}
			}else if( nFUTypeF < 2 && nFUTypeT == 2 ){	// �m�[�hnF�����Z/���Z�A�m�[�hnT����Z
				// nIndexRegGroupLast�̃��W�X�^����ʐM�扉�Z��nIndexFUdest(��Z��)�֒ʐM�v���ǉ�
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroupLast].nIndexRegInstanceTop;
				for( ; nIndexRegInstance>=0; nIndexRegInstance=m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
					// ���W�X�^nIndexRegInstance���牉�Z��nIndexModuleT�ɒʐM�v�������邩���ׂ�
					nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToFUCommTop;
					for( ; nIndexRegToFUComm>=0 ; nIndexRegToFUComm=m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext ){
						if( m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU == nIndexModuleT ) break;	// �ʐM�v������
					}
					if( nIndexRegToFUComm < 0 ) continue;	// ���W�X�^���牉�Z��ւ̒ʐM�v������
					nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
					AddCommunicationRequirementRegToFU( nIndexRegF, nIndexModuleT, nFUTypeT );	////
#ifdef DEBUG_VERBOSE
					TRACE( "Reg %d(%d) => FU %d (type=%d)\n", nIndexRegInstance, nIndexRegF, nIndexModuleT, nFUTypeT );
#endif
				}
			}else if( nFUTypeF == 2 && nFUTypeT < 2 ){	// �m�[�hnF����Z�A�m�[�hnT�����Z/���Z
				// �ʐM�����Z��nIndexFUsrc(��Z��)����nIndexFUsrc����̃��W�X�^nIndexRegGroup0�֒ʐM�v���ǉ�
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroup0].nIndexRegInstanceTop;
				for( ; nIndexRegInstance >= 0 ; nIndexRegInstance=m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
					nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
					AddCommunicationRequirementFUToReg( nIndexModuleF, nFUTypeF, nIndexRegF );	////
#ifdef DEBUG_VERBOSE
					TRACE( "FU %d (type=%d) => Reg %d(%d)\n", nIndexModuleF, nFUTypeF, nIndexRegInstance, nIndexRegF );
#endif
				}
				// nIndexRegGroupLast�̃��W�X�^����ʐM�扉�Z��nIndexFUdest(���Z��/���Z��)�̒���̃��W�X�^�֒ʐM�v���ǉ�
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroupLast].nIndexRegInstanceTop;
				for( ; nIndexRegInstance >= 0 ; nIndexRegInstance=m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
					// ���W�X�^nIndexRegInstance���牉�Z��nIndexModuleT�ɒʐM�v�������邩���ׂ�
					nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToFUCommTop;
					for( ; nIndexRegToFUComm>=0 ; nIndexRegToFUComm=m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext ){
						if( m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU == nIndexModuleT ) break;	// �ʐM�v������
					}
					if( nIndexRegToFUComm < 0 ) continue;	// ���W�X�^���牉�Z��ւ̒ʐM�v������
					nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
					if( m_aInterFUConnection[nIndexFUdest].nIndexInterconnectionTargetTop < 0 ) continue;	// nIndexFUdest�̓f�[�^�ʐM��̂Ȃ����Z��̏ꍇ
					nIndexInterconnectionTargetT = m_aInterFUConnection[nIndexFUdest].nIndexInterconnectionTargetTop;
					nIndexRegGroupContainerT = m_poolInterconnectionTarget[nIndexInterconnectionTargetT].nIndexRegGroupContainerTop;
					if( nIndexRegGroupContainerT >= 0 && m_poolRegGroupContainer[nIndexRegGroupContainerT].nIndexRegGroupTop >= 0 ){
						nIndexRegGroupT = m_poolRegGroupContainer[nIndexRegGroupContainerT].nIndexRegGroupTop;
						nIndexRegInstanceT = m_poolRegGroup[nIndexRegGroupT].nIndexRegInstanceTop;
						for( ; nIndexRegInstanceT>=0 ; nIndexRegInstanceT=m_poolRegInstance[nIndexRegInstanceT].nIndexRegInstanceNext ){
							nIndexRegT = m_nProIndexBaseReg + nIndexRegInstanceT;
							AddCommunicationRequirementRegFUReg( nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegT );	////
#ifdef DEBUG_VERBOSE
							TRACE( "Reg %d(%d) => FU %d (type=%d) => Reg %d(%d)\n", nIndexRegInstance, nIndexRegF, nIndexModuleT, nFUTypeT, nIndexRegInstanceT, nIndexRegT );
#endif
						}
					}
				}
			}else{	// �m�[�hnF����Z�A�m�[�hnT����Z
				// �ʐM�����Z��nIndexFUsrc(��Z��)����nIndexFUsrc����̃��W�X�^nIndexRegGroup0�֒ʐM�v���ǉ�
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroup0].nIndexRegInstanceTop;
				for( ; nIndexRegInstance >= 0 ; nIndexRegInstance = m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
					nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
					AddCommunicationRequirementFUToReg( nIndexModuleF, nFUTypeF, nIndexRegF );	////
#ifdef DEBUG_VERBOSE
					TRACE( "FU %d (type=%d) => Reg %d(%d)\n", nIndexModuleF, nFUTypeF, nIndexRegInstance, nIndexRegF );
#endif
				}
				// nIndexRegGroupLast�̃��W�X�^����ʐM�扉�Z��nIndexFUdest(��Z��)�֒ʐM�v���ǉ�
				nIndexRegInstance = m_poolRegGroup[nIndexRegGroupLast].nIndexRegInstanceTop;
				for( ; nIndexRegInstance >= 0 ; nIndexRegInstance = m_poolRegInstance[nIndexRegInstance].nIndexRegInstanceNext ){
					// ���W�X�^nIndexRegInstance���牉�Z��nIndexModuleT�ɒʐM�v�������邩���ׂ�
					nIndexRegToFUComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToFUCommTop;
					for( ; nIndexRegToFUComm>=0 ; nIndexRegToFUComm=m_poolRegToFUComm[nIndexRegToFUComm].nIndexRegToFUCommNext ){
						if( m_poolRegToFUComm[nIndexRegToFUComm].nIndexFU == nIndexModuleT ) break;	// �ʐM�v������
					}
					if( nIndexRegToFUComm < 0 ) continue;	// ���W�X�^���牉�Z��ւ̒ʐM�v������
					nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
					AddCommunicationRequirementRegToFU( nIndexRegF, nIndexModuleT, nFUTypeT );	////
#ifdef DEBUG_VERBOSE
					TRACE( "Reg %d(%d) => FU %d (type=%d)\n", nIndexRegInstance, nIndexRegF, nIndexModuleT, nFUTypeT );
#endif
				}
			}
		}
	}
	// �r���ɉ��Z���܂܂Ȃ����W�X�^�Ԓx������
	for( nIndexRegInstance=0 ; nIndexRegInstance<m_nCountAllocatedRegInstance ; nIndexRegInstance++ ){
		nIndexRegF = m_nProIndexBaseReg + nIndexRegInstance;
		nIndexRegToRegComm = m_poolRegInstance[nIndexRegInstance].nIndexRegToRegCommTop;
		for( ; nIndexRegToRegComm >= 0 ; nIndexRegToRegComm=m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegToRegCommNext ){
			nIndexRegT = m_nProIndexBaseReg + m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegInstance;
			AddCommunicationRequirementRegToReg( nIndexRegF, nIndexRegT );	////
#ifdef DEBUG_VERBOSE
			TRACE( "Reg %d(%d) => Reg %d(%d)\n", nIndexRegInstance, nIndexRegF, m_poolRegToRegComm[nIndexRegToRegComm].nIndexRegInstance, nIndexRegT );
#endif
		}
	}

	///////////////////////////////////
	m_listCommunicationRequirementFinal = m_listCommunicationRequirement;
	///////////////////////////////////
}

