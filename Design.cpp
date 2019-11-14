#include "stdafx.h"
#include "Design.h"


CDesign::CDesign()
{
}

CDesign::~CDesign()
{
}

void CDesign::SetParamSAforBinding(double dTstart, double dTend, double dRate, int nIterate)
{
	m_dTstartBinding = dTstart;
	m_dTendBinding = dTend;
	m_dRateBinding = dRate;
	m_nIterateBinding = nIterate;
}

void CDesign::SetParamSAforFloorplanPro(double dTstart, double dTend, double dRate, int nIterate)
{
	m_dTstartFloorplanPro = dTstart;
	m_dTendFloorplanPro = dTend;
	m_dRateFloorplanPro = dRate;
	m_nIterateFloorplanPro = nIterate;
}


void CDesign::SetParamSAforFloorplanFinal(double dTstart, double dTend, double dRate, int nIterate)
{
	m_dTstartFloorplanFinal = dTstart;
	m_dTendFloorplanFinal = dTend;
	m_dRateFloorplanFinal = dRate;
	m_nIterateFloorplanFinal = nIterate;
}

void CDesign::SetFUConfiguration(int nAdd, int nSub, int nMul)
{
	m_Binding.SetFUConfiguration( nAdd, nSub, nMul );
	m_MultiBinding.SetFUConfiguration( nAdd, nSub, nMul );
}

#define NUM_MULTI_THREADS	1////4->1�ɕύX
void CDesign::Initialize(void)
{
	if( m_bMultiThread ){
		m_MultiBinding.SetDFG( &m_DFG );
		m_MultiSA.RegisterSolution( &m_MultiBinding );
		m_MultiSA.SetParameters(m_dTstartBinding, m_dTendBinding, m_dRateBinding, m_nIterateBinding, FALSE/*minimize*/);
		m_MultiBinding.SetParamSAforFloorplanPro(m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro);
		m_MultiBinding.SetParamSAforFloorplanFinal(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal);
		m_MultiSA.SetID( 0 );
		m_MultiBinding.GlobalInit(NUM_MULTI_THREADS);
	}else{
		m_Binding.SetDFG( &m_DFG );
		m_SA.RegisterSolution( &m_Binding );
		m_SA.SetParameters(m_dTstartBinding, m_dTendBinding, m_dRateBinding, m_nIterateBinding, FALSE/*minimize*/);
		m_SA.SetID( 0 );
		m_Binding.m_SAforFloorplanPro.SetParameters(m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro, m_nIterateFloorplanPro, FALSE/*minimize*/);
		m_Binding.m_SAforFloorplanPro.SetID( 1 );
		m_Binding.m_SAforFloorplanFinal.SetParameters(m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal, m_nIterateFloorplanFinal, FALSE/*minimize*/);
		m_Binding.m_SAforFloorplanFinal.SetID( 2 );
		m_Binding.m_FloorplanPro.SetMode( 0 );
		m_Binding.m_FloorplanFinal.SetMode( 1 );
	}
}

void CDesign::Optimize(void)
{
	if( m_bMultiThread ){
		m_MultiSA.Initialize();
		m_MultiSA.Execute();
	}else{
		m_SA.EnableStatistics();
		m_SA.Initialize();
		m_SA.Execute();
	}
}
