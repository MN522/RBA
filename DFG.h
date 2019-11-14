#include "TypeDef.h"
#pragma once

class CDFG  
{
public:
	CDFG();
	virtual ~CDFG();

public:
	int readdata( TCHAR *file );
	void releasedata( void );
	void ConstructNetlist(void);
	int GetNodeNum( void ) { return N; }

	void SetN( int N0 ) { N = N0; }
	double GetTr( void ) { return Tr; }
	void SetTr( double Tr0 ) { Tr = Tr0; }

	void Duplicate(CDFG *pDFG);

// Data member
	int N,E,P,R,B;
	NODE *m_node;
	EDGE *m_edge;
	PROCTYPE *m_proc;
	REGTYPE *m_regis;

	int nRefNode;
	double Tr;

};
