#include "StdAfx.h"
#include "SequencePair.h"

CSequencePair::CSequencePair(void)
{
	m_Module = NULL;
	m_nModule = 0;
	m_Unit = NULL;
	m_nUnit = 0;
	m_GammaPlus = NULL;
	m_GammaMinus = NULL;
	m_GammaMinusOrder = NULL;
	m_BestGammaPlus = NULL;
	m_BestGammaMinus = NULL;
	m_BestModuleRotation = NULL;
	bShowFlag = 0;
	m_nCount = 0;

	m_node = NULL;
	m_elist = NULL;
	mEdgeStack = NULL;
	mNodeStack = NULL;
}

CSequencePair::~CSequencePair(void)
{
	Clear();
}

void CSequencePair::Clear(void)
{
	if( m_Module != NULL ){
		GlobalFree( m_Module );
		m_Module = NULL;
	}
	if( m_Unit != NULL ){
		GlobalFree( m_Unit );
		m_Unit = NULL;
	}
	if( m_GammaPlus != NULL ){
		GlobalFree( m_GammaPlus );
		GlobalFree( m_GammaMinus );
		GlobalFree( m_GammaMinusOrder );
		GlobalFree( m_BestGammaPlus );
		GlobalFree( m_BestGammaMinus );
		GlobalFree( m_BestModuleRotation );
		m_GammaPlus = NULL;
		m_GammaMinus = NULL;
		m_GammaMinusOrder = NULL;
		m_BestGammaPlus = NULL;
		m_BestGammaMinus = NULL;
		m_BestModuleRotation = NULL;
	}
	if( m_node != NULL ){
		GlobalFree( m_node );
		GlobalFree( m_elist );
		GlobalFree( mEdgeStack );
		GlobalFree( mNodeStack );
		m_node = NULL;
		m_elist = NULL;
		mEdgeStack = NULL;
		mNodeStack = NULL;
	}
	m_nModule = 0;
	m_nUnit = 0;
}

int CSequencePair::AddUnitType(int nWidth, int nHeight)
{
	UNIT *tmpUnit = m_Unit;
	m_Unit  = (UNIT *)GlobalAlloc( GPTR, (m_nUnit+1)*sizeof(UNIT) );
	if( tmpUnit != NULL ){
		int i;
		for( i=0 ; i<m_nUnit ; i++ ){
			m_Unit[i] = tmpUnit[i];
		}
		GlobalFree( tmpUnit );
	}
	m_Unit[m_nModule].nWidth = nWidth;
	m_Unit[m_nModule].nHeight = nHeight;
	m_Unit[m_nModule].nUnitName = m_nUnit;
	return m_nUnit++;
}



int CSequencePair::AddModule(int nUnitType)
{
	int i,m;
	for( i=0 ; i<m_nUnit ; i++ ){
		if( m_Unit[i].nUnitName == nUnitType ) break;
	}
	if( i>=m_nUnit ){
		return -1;
	}

	m = AddModule( m_Unit[i].nWidth, m_Unit[i].nHeight );
	if( m<0 ) return -1;
	m_Module[m].nUnitType = nUnitType;
	return m;
}

int CSequencePair::AddModule(int nWidth, int nHeight)
{
	int i;
	MODULE *tmpModule = m_Module;
	m_Module  = (MODULE *)GlobalAlloc( GPTR, (m_nModule+1)*sizeof(MODULE) );
	if( tmpModule != NULL ){
		for( i=0 ; i<m_nModule ; i++ ){
			m_Module[i] = tmpModule[i];
		}
		GlobalFree( tmpModule );
	}
	m_Module[m_nModule].nWidth = nWidth;
	m_Module[m_nModule].nHeight = nHeight;
	m_Module[m_nModule].nOrientation = 0;
	m_Module[m_nModule].nUnitType = 0;
	return m_nModule++;
}

int CSequencePair::Initialize()
{
	//InitGamma();
	AnyGamma();//簡単化のためガンマ値固定.一時的
	if( m_node != NULL ){
		GlobalFree( m_node );
		GlobalFree( m_elist );
		GlobalFree( mEdgeStack );
		GlobalFree( mNodeStack );
	}
	m_node = (LPNODE *)GlobalAlloc( GPTR, m_nModule*sizeof(LPNODE) );
	m_elist = (LPEDGE *)GlobalAlloc( GPTR, m_nModule*(m_nModule-1)/2*sizeof(LPEDGE) );
	mEdgeStack = (LPEDGE **)GlobalAlloc( GPTR, m_nModule*sizeof(LPEDGE *) );
	mNodeStack = (LPNODE **)GlobalAlloc( GPTR, m_nModule*sizeof(LPNODE *) );

	return 1;
}
int CSequencePair::LookPalameter() {

	TRACE("m_GammaPlus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaPlus[i]);
	}
	TRACE("] ");
	TRACE("m_GammaMinus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaMinus[i]);
	}
	TRACE("] ");
	//set
	m_GammaPlus[0] = 7, m_GammaPlus[1] = 3, m_GammaPlus[2] = 4, m_GammaPlus[3] = 8, m_GammaPlus[4] = 2,
		m_GammaPlus[5] = 0, m_GammaPlus[6] = 9, m_GammaPlus[7] = 1, m_GammaPlus[8] = 6, m_GammaPlus[9] = 5;
	m_GammaMinus[0] = 9, m_GammaMinus[1] = 2, m_GammaMinus[2] = 7, m_GammaMinus[3] = 6, m_GammaMinus[4] = 1,
		m_GammaMinus[5] = 4, m_GammaMinus[6] = 5, m_GammaMinus[7] = 0, m_GammaMinus[8] = 8, m_GammaMinus[9] = 2;
	TRACE("m_GammaPlus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaPlus[i]);
	}
	TRACE("] ");
	TRACE("m_GammaMinus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaMinus[i]);
	}
	TRACE("] ");
	return 1;
}

int CSequencePair::GetScore()//Areaを求めるGetScoreは使われていない
{
	m_nScore = GetArea();
	return m_nScore;
}

int CSequencePair::GenerateNeighbor()
{
	switch( rand()%6 ){
	case 0:
	case 1:
		m_nModeGenerateNeighbor = 0;
		SwapGammaPlus();//左右を入れ替える
		break;
	case 2:
		m_nModeGenerateNeighbor = 1;
		SwapGammas();//上下左右を入れ替える
		break;
	case 3:
	case 4:
	case 5:
	default:
		m_nModeGenerateNeighbor = 2;
		RotateModule();
		break;
	}
	return 1;
}

int CSequencePair::RestoreNeighbor()
{
	switch( m_nModeGenerateNeighbor ){
	case 0:
		RestoreGammaPlus();
		break;
	case 1:
		RestoreGammaPlus();
		RestoreGammaMinus();
		break;
	default:
		RestoreModuleRotate();
		break;
	}
	return 1;
}

int CSequencePair::InitGamma()//Moduleの数だけシークエンスペアを生成
{
//	TRACE( "CSequencePair::InitGamma: m_nModule=%d\n", m_nModule );
	if( m_GammaPlus != NULL ){
		GlobalFree( m_GammaPlus );
		GlobalFree( m_GammaMinus );
		GlobalFree( m_GammaMinusOrder );
		GlobalFree( m_BestGammaPlus );
		GlobalFree( m_BestGammaMinus );
		GlobalFree( m_BestModuleRotation );
	}
	m_GammaPlus  = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );
	m_GammaMinus = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );
	m_GammaMinusOrder = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );
	m_BestGammaPlus  = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );
	m_BestGammaMinus = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );
	m_BestModuleRotation = (int *)GlobalAlloc( GPTR, m_nModule*sizeof(int) );

	int i,n,k;
	for( i=0 ; i<m_nModule ; i++ ){
		m_GammaPlus[i] = i;
		m_GammaMinus[i] = i;
	}

	for( n=m_nModule ; n>1 ; n-- ){
		k = (int)((rand()* n / (1.0+ RAND_MAX)));	// [0,n-1]の乱数
		i = m_GammaPlus[n-1];
		m_GammaPlus[n-1] = m_GammaPlus[k];
		m_GammaPlus[k] = i;
		k = (int)((rand()* n / (1.0+ RAND_MAX)));	// [0,n-1]の乱数
		i = m_GammaMinus[n-1];
		m_GammaMinus[n-1] = m_GammaMinus[k];
		m_GammaMinus[k] = i;
	}
	return 1;
}

int CSequencePair::AnyGamma()//Moduleの数だけシークエンスペアを生成.簡単化のためガンマを任意に固定・自作
{
	//	TRACE( "CSequencePair::InitGamma: m_nModule=%d\n", m_nModule );
	if (m_GammaPlus != NULL) {
		GlobalFree(m_GammaPlus);
		GlobalFree(m_GammaMinus);
		GlobalFree(m_GammaMinusOrder);
		GlobalFree(m_BestGammaPlus);
		GlobalFree(m_BestGammaMinus);
		GlobalFree(m_BestModuleRotation);
	}
	m_GammaPlus = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	m_GammaMinus = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	m_GammaMinusOrder = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	m_BestGammaPlus = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	m_BestGammaMinus = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	m_BestModuleRotation = (int *)GlobalAlloc(GPTR, m_nModule * sizeof(int));
	
	int i, n, k;
	for (i = 0; i<5; i++) {
		m_GammaPlus[i] = i;
		m_GammaMinus[i] = i;
	}
	m_GammaPlus[5] = 9, m_GammaPlus[6] = 8, m_GammaPlus[7] = 7, m_GammaPlus[8] = 6, m_GammaPlus[9] = 5;
	m_GammaMinus[5] = 9, m_GammaMinus[6] = 8, m_GammaMinus[7] = 7, m_GammaMinus[8] = 6, m_GammaMinus[9] = 5;
	return 1;
}
void CSequencePair::CheckGamma()//Gammaの中身見る関数。自作
{
	TRACE("m_GammaPlus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaPlus[i]);	
	}
	TRACE("m_GammaMinus=[");
	for (int i = 0; i < m_nModule; i++) {
		TRACE("%d,", m_GammaMinus[i]);
	}
}
void CSequencePair::ComputeLongestPath()
{
	int n;
	LPEDGE *eptr;
	LPNODE *nptrF,*nptrT;
	int nLevel;

	for( n=0 ; n<m_nModule ; n++ ) m_node[n].nLength = 0;
	for( n=0 ; n<m_nModule ; n++ ){
		m_node[n].nFixed = 0;
		if( m_node[n].nCountNotFixedIn == 0 ) m_node[n].nFixed = 1;
	}
	for( n=0 ; n<m_nModule ; n++ ){
		if( ! m_node[n].nFixed ) continue;
		nptrF = &(m_node[n]);
		eptr = nptrF->olist;
		nLevel = 0;
		while(1){
			for( ; eptr ; eptr=eptr->olist ){
				nptrT = eptr->t;
				if( nptrT->nCountNotFixedIn == 0 ) continue;
				if( nptrT->nLength < nptrF->nLength+nptrF->w ){
					nptrT->nLength = nptrF->nLength+nptrF->w;
				}
				if( --(nptrT->nCountNotFixedIn) == 0 ){
					mEdgeStack[nLevel] = eptr;
					mNodeStack[nLevel] = nptrF;
					nLevel++;
					nptrF = nptrT;
					eptr = nptrF->olist;
					goto next;
				}
			}
			if( --nLevel < 0 ) break;
			eptr = mEdgeStack[nLevel]->olist;
			nptrF = mNodeStack[nLevel];
		next:;
		}
	}

}

int CSequencePair::Pack()
{
	int kp,km;
	int nModuleA,nModuleB;
	int nCount;
	LPEDGE *eptr;

	// モジュールmがΓ-上で先頭から何番目にあるか求める
	for( km=0 ; km<m_nModule ; km++ ) m_GammaMinusOrder[m_GammaMinus[km]] = km;

	// 水平方向パッキング
	for( kp=0 ; kp<m_nModule ; kp++ ){
		m_node[kp].nCountNotFixedIn = 0;
		m_node[kp].olist = NULL;
		switch( m_Module[kp].nOrientation ){
		case 0:
		case 2:
			m_node[kp].w = m_Module[kp].nWidth;
			break;
		default:
			m_node[kp].w = m_Module[kp].nHeight;
			break;
		}
	}

	nCount = 0;
	for( kp=0 ; kp<m_nModule-1 ; kp++ ){
		// Γ+上でkp番目にあるモジュールm_GammaPlus[kp]
		nModuleA = m_GammaPlus[kp];
		for( km=kp+1 ; km<m_nModule ; km++ ){
			nModuleB = m_GammaPlus[km];
			// Γ+上で前にあるnModuleA、後ろにあるモジュールnModuleB
			if( m_GammaMinusOrder[nModuleA] < m_GammaMinusOrder[nModuleB] ){
				// Γ-上で前にnModule1、後ろにnModuleB ==> nModuleAは左、nModuleBは右 ==> 枝(nModuleA,nModuleB)を張る
				eptr = m_elist+nCount;
				eptr->olist = m_node[nModuleA].olist;
				m_node[nModuleA].olist = eptr;
				eptr->t = &(m_node[nModuleB]);
				eptr->t->nCountNotFixedIn++;
				nCount++;
			}
		}
	}
	//TRACE( "nCount=%d\n", nCount );
	/*////////////////////////////////////////////////
	if( nCount > m_nModule*(m_nModule-1)/2 ){
	FILE *fp;
	fopen_s( &fp, "report.txt", "a" );
	fprintf( fp, "# modules =%d, Max edges =%d, # edges = %d\n", m_nModule, m_nModule*(m_nModule-1)/2, nCount );
	fclose(fp);
	}
	////////////////////////////////////////////////*/
	// 最長経路を求める
	ComputeLongestPath();
	for( kp=0 ; kp<m_nModule ; kp++ ) m_Module[kp].x = m_node[kp].nLength;//Moduleのx座標をを求める

	// 垂直方向パッキング
	for( kp=0 ; kp<m_nModule ; kp++ ){
		m_node[kp].nCountNotFixedIn = 0;
		m_node[kp].olist = NULL;
		switch( m_Module[kp].nOrientation ){
		case 0:
		case 2:
			m_node[kp].w = m_Module[kp].nHeight;
			break;
		default:
			m_node[kp].w = m_Module[kp].nWidth;
			break;
		}
	}
	nCount = 0;
	for( kp=0 ; kp<m_nModule-1 ; kp++ ){
		// Γ+上でkp番目にあるモジュールm_GammaPlus[kp]
		nModuleA = m_GammaPlus[kp];
		for( km=kp+1 ; km<m_nModule ; km++ ){
			nModuleB = m_GammaPlus[km];
			// Γ+上で前にあるnModuleA、後ろにあるモジュールnModuleB
			if( m_GammaMinusOrder[nModuleA] > m_GammaMinusOrder[nModuleB] ){
				// Γ-上で前にnModuleB、後ろにnModuleA ==> nModuleAは上、nModuleBは下 ==> 枝(nModuleB,nModuleA)を張る
				eptr = m_elist+nCount;
				eptr->olist = m_node[nModuleB].olist;
				m_node[nModuleB].olist = eptr;
				eptr->t = &(m_node[nModuleA]);
				eptr->t->nCountNotFixedIn++;
				nCount++;
			}
		}
	}
	//TRACE( "nCount=%d\n", nCount );
	// 最長経路を求める
	ComputeLongestPath();
	for( kp=0 ; kp<m_nModule ; kp++ ) m_Module[kp].y = m_node[kp].nLength;//Moduleのy座標をを求める

	return 1;
}

int CSequencePair::GetArea()
{
	int nWidth=0, nHeight=0;
	int k;
	////////////////////////////////
	Pack();
	////////////////////////////////
	for( k=0 ; k<m_nModule ; k++ ){
		switch( m_Module[k].nOrientation ){
		case 0:
		case 2:
			if( nWidth < m_Module[k].x + m_Module[k].nWidth ){
				nWidth = m_Module[k].x + m_Module[k].nWidth;
			}
			if( nHeight < m_Module[k].y + m_Module[k].nHeight ){
				nHeight = m_Module[k].y + m_Module[k].nHeight;
			}
			break;
		default:
			if( nWidth < m_Module[k].x + m_Module[k].nHeight ){
				nWidth = m_Module[k].x + m_Module[k].nHeight;
			}
			if( nHeight < m_Module[k].y + m_Module[k].nWidth ){
				nHeight = m_Module[k].y + m_Module[k].nWidth;
			}
			break;
		}
	}
/*	if( nWidth*nHeight ==8400 ){
		TRACE( "m_nCount=%d, nWidth=%d, nHeight=%d\n", m_nCount, nWidth, nHeight );
		int kp;
		for( kp=0 ; kp<m_nModule ; kp++ ){
			TRACE( "%c:%d,%d,%d ", 'a'+kp, m_Module[kp].x, m_Module[kp].y, m_Module[kp].nOrientation );
		}
		TRACE( "\n" );
	}
	m_nCount++;*/
	return nWidth*nHeight;
}

int CSequencePair::SwapGammaPlus()
{
	int m1 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));	// [0,m_nModule-1]の乱数を求める
	int m2 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	while( m2 == m1 ){
		m1 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	}
	//	TRACE( "CSequencePair::SwapGammaPlus: %d <=> %d\n", m1, m2 );
	int k = m_GammaPlus[m1];
	m_GammaPlus[m1] = m_GammaPlus[m2];
	m_GammaPlus[m2] = k;
	m_nGammaPlusIndex1 = m1;
	m_nGammaPlusIndex2 = m2;
	return 1;
}

int CSequencePair::SwapGammaMinus()
{
	int m1 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	int m2 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	while( m2 == m1 ){
		m1 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	}
	//	TRACE( "CSequencePair::SwapGammaMinus: %d <=> %d\n", m1, m2 );
	int k = m_GammaMinus[m1];
	m_GammaMinus[m1] = m_GammaMinus[m2];
	m_GammaMinus[m2] = k;
	m_nGammaMinusIndex1 = m1;
	m_nGammaMinusIndex2 = m2;
	return 1;
}

int CSequencePair::SwapGammas()
{
	SwapGammaPlus();
	SwapGammaMinus();
	return 1;
}

/*	int m_nModeGenerateNeighbor;
	int m_nGammaPlusIndex1;
	int m_nGammaPlusIndex2;
	int m_nGammaMinusIndex1;
	int m_nGammaMinusIndex2;
	int m_nModuleRotate;*/
int CSequencePair::RotateModule()
{
	int m1 = (int)((rand()* m_nModule / (1.0+ RAND_MAX)));
	int nOrientation = (int)((rand()* 4 / (1.0+ RAND_MAX)));
	m_nModuleOrientation = m_Module[m1].nOrientation;
	while( nOrientation == m_Module[m1].nOrientation ){
		nOrientation = (int)((rand()* 4 / (1.0+ RAND_MAX)));
	}
	m_Module[m1].nOrientation = nOrientation;
	m_nGammaPlusIndex1 = m1;
	return 1;
}

int CSequencePair::GetModuleStatus(int m, int *x, int *y, int *w, int *h, int *nOrientation)
{
	*x = m_Module[m].x;
	*y = m_Module[m].y;
	*w = m_Module[m].nWidth;
	*h = m_Module[m].nHeight;
	*nOrientation = m_Module[m].nOrientation;
	return m;
}

void CSequencePair::SetModuleStatus(int m, int x, int y, int w, int h, int nOrientation)
{
	m_Module[m].x = x;
	m_Module[m].y = y;
	m_Module[m].nWidth = w;
	m_Module[m].nHeight = h;
	m_Module[m].nOrientation = nOrientation;
}

int CSequencePair::SaveBest()
{
/*///////////////////////////////////////////////////////////////////
	int kp,km;
	TRACE( "CSequencePair::SaveBestSequencePair: Gamma+: " );
	for( kp=0 ; kp<m_nModule ; kp++ ){
		TRACE( "%d ", m_GammaPlus[kp] );
	}
	TRACE( "\n" );
	TRACE( "CSequencePair::SaveBestSequencePair: Gamma-: " );
	for( km=0 ; km<m_nModule ; km++ ){
		TRACE( "%d ", m_GammaMinus[km] );
	}
	TRACE( "\n" );
////////////////////////////////////////////////////////////////////*/
	//TRACE( "CSequencePair::SaveBest\n" );
	int i;
	for( i=0 ; i<m_nModule ; i++ ){
		m_BestGammaPlus[i] = m_GammaPlus[i];
		m_BestGammaMinus[i] = m_GammaMinus[i];
		m_BestModuleRotation[i] = m_Module[i].nOrientation;
	}
	return 1;
}

int CSequencePair::RestoreBest()
{
	bShowFlag = 1;
	int i;
	for( i=0 ; i<m_nModule ; i++ ){
		m_GammaPlus[i] = m_BestGammaPlus[i];
		m_GammaMinus[i] = m_BestGammaMinus[i];
		m_Module[i].nOrientation = m_BestModuleRotation[i];
	}
/*///////////////////////////////////////////////////////////////////
	int kp,km;
	TRACE( "CSequencePair::PackBest: Gamma+: " );
	for( kp=0 ; kp<m_nModule ; kp++ ){
		TRACE( "%d ", m_GammaPlus[kp] );
	}
	TRACE( "\n" );
	TRACE( "CSequencePair::PackBest: Gamma-: " );
	for( km=0 ; km<m_nModule ; km++ ){
		TRACE( "%d ", m_GammaMinus[km] );
	}
	TRACE( "\n" );
////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////
#if !defined(_DEBUG)
	FILE *fp;
	fopen_s(&fp,"result.txt","a");
	for( i=0 ; i<m_nModule ; i++ ){
		fprintf( fp, "%2d ", m_GammaPlus[i] );
	}
	fprintf( fp, "\n" );
	for( i=0 ; i<m_nModule ; i++ ){
		fprintf( fp, "%2d ", m_GammaMinus[i] );
	}
	fprintf( fp, "\n" );
	fclose(fp);
#endif
////////////////////////////////////////////////////////////////////*/
	return 1;
}

void CSequencePair::RestoreGammaPlus()
{
	int k = m_GammaPlus[m_nGammaPlusIndex1];
	m_GammaPlus[m_nGammaPlusIndex1] = m_GammaPlus[m_nGammaPlusIndex2];
	m_GammaPlus[m_nGammaPlusIndex2] = k;
}

void CSequencePair::RestoreGammaMinus()
{
	int k = m_GammaMinus[m_nGammaMinusIndex1];
	m_GammaMinus[m_nGammaMinusIndex1] = m_GammaMinus[m_nGammaMinusIndex2];
	m_GammaMinus[m_nGammaMinusIndex2] = k;
}

void CSequencePair::RestoreModuleRotate()
{
	m_Module[m_nGammaPlusIndex1].nOrientation = m_nModuleOrientation;
}
