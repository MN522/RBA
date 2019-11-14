#pragma once
#include "Binding.h"
#include "MultiBinding.h"
#include "SimulatedAnnealing.h"
#include "MultiSimulatedAnnealing.h"
#include "ListScheduleDFG.h"

class CDesign
{
public:
	CDesign();
	~CDesign();

public:
	void Initialize(void);
	void SetParamSAforBinding(double dTstart, double dTend, double dRate, int nIterate);
	void SetParamSAforFloorplanPro(double dTstart, double dTend, double dRate, int nIterate);
	void SetParamSAforFloorplanFinal(double dTstart, double dTend, double dRate, int nIterate);
	CListScheduleDFG *GetDFG(void){ return &m_DFG; }
	void SetFUConfiguration(int nAdd, int nSub, int nMul);
	void Optimize(void);

	CBinding m_Binding;
	CSimulatedAnnealing m_SA;
	CMultiBinding m_MultiBinding;
	CMultiSimulatedAnnealing m_MultiSA;
	CListScheduleDFG m_DFG;

	void SetMultiThread(BOOL bMultiThread){ m_bMultiThread = bMultiThread; }
	BOOL m_bMultiThread;

	double m_dTstartBinding, m_dTendBinding, m_dRateBinding;
	double m_dTstartFloorplanPro, m_dTendFloorplanPro, m_dRateFloorplanPro;
	double m_dTstartFloorplanFinal, m_dTendFloorplanFinal, m_dRateFloorplanFinal;
	int m_nIterateBinding;
	int m_nIterateFloorplanPro;
	int m_nIterateFloorplanFinal;
};

