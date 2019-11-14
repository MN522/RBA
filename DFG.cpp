// DFG.cpp: CDFG クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "TestRangeChart.h"
#include "DFG.h"
//#define _CRT_SECURE_NO_WARNINGS

static char SEP[]=" \t";

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CDFG::CDFG()
{
	m_node = NULL;
	m_edge = NULL;
	m_proc = NULL;
	m_regis = NULL;
	N = 0;
	E = 0;
	P = 0;
	R = 0;
}

CDFG::~CDFG()
{
	releasedata();
}

/*
#データサンプル
T 16
S 21
P  1 A  1  1  53 bp bp
P  4 M  2  1  331 bp bp
C  1 bp ds  0  1  3
C  4 ds bp  1  1  3
R  1 bp 1 8
R  2 ds 2 4
N  1 A  0  0
E  1  1 10 0
B  1  1  2 0 T	; 条件分岐: ID, src, dest, delay, T or F
J  1  1 10	; 条件判定ノード -> 合流ノード
*/
int CDFG::readdata( TCHAR *file )
{
	char c;
	char *ptr;
	int j,k,n;
	int len;
	FILE *fp;
	char buf[256];
	char *cRefNode0;
	char *pTokenContext;
	
	if( _wfopen_s(&fp,file,_T("r")) ){
		fprintf( stderr, "Can't open file %ls\n", file );
		return( FALSE );
	}
	///////////////////////////////////////////
	releasedata();
	///////////////////////////////////////////

	N = 0;
	E = 0;
	P = 0;
	R = 0;
	for(;;){
		fgets( buf, 255, fp );
		if( feof(fp) ) break;
		switch( buf[0] ){
		case 'T':
		case 't':
			ptr = strtok_s( buf, SEP, &pTokenContext );	/* 'T' */
			ptr = strtok_s( NULL, SEP, &pTokenContext );	/* Tr */
			sscanf_s( ptr, "%lf", &Tr );
			break;
		case 'N':
		case 'n':
			N++;
			break;
		case 'E':
		case 'e':
			E++;
			break;
		case 'P':
		case 'p':
//			sscanf( buf, "%[^\n]", buf );
			P++;
			break;
		case 'R':
		case 'r':
			R++;
			break;
		}
	}
	fclose(fp);
	m_node = (NODE *)GlobalAlloc( GPTR, N*sizeof(NODE) );
	m_edge = (EDGE *)GlobalAlloc( GPTR, E*sizeof(EDGE) );
	m_proc = (PROCTYPE *)GlobalAlloc( GPTR, P*sizeof(PROCTYPE) );
	m_regis = (REGTYPE *)GlobalAlloc( GPTR, R*sizeof(REGTYPE) );
	NODE *node = m_node;
	EDGE *edge = m_edge;
	PROCTYPE *proc = m_proc;
	REGTYPE *regis = m_regis;

	char **edge_f = (char **)GlobalAlloc( GPTR, E*sizeof(char *) );
	char **edge_t = (char **)GlobalAlloc( GPTR, E*sizeof(char *) );
	_wfopen_s(&fp,file,_T("r"));
	N = 0;
	E = 0;
	P = 0;
	R = 0;
	cRefNode0 = NULL;
	for(;;){
		fgets( buf, 255, fp );
		if( feof(fp) ) break;
		len = strlen(buf);
		if( buf[len-1] == '\n' ) buf[len-1] = '\0';
//		sscanf_s( buf, "%[^\n]", buf );
		/*printf( "%s\n", buf );*/
		ptr = strtok_s( buf, SEP, &pTokenContext );
		if( ptr == NULL ) continue;
		c = *ptr;
		switch( *ptr ){
		case 'N':
		case 'n':
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			node[N].N = _strdup(ptr);
			node[N].n = N;
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			node[N].T = _strdup(ptr);
			/*ptr = strtok( NULL, SEP );
			node[N].LB = atoi(ptr);
			ptr = strtok( NULL, SEP );
			node[N].UB = atoi(ptr);*/
			N++;
			break;
		case 'E':
		case 'e':
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			edge[E].N = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			edge_f[E] = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			edge_t[E] = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			edge[E].d = atoi(ptr);
			E++;
			break;
		case 'P':
		case 'p':
//P  1 A  1  1  53 bp bp
//P  4 M  2  1  331 bp bp
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			proc[P].N = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			proc[P].T = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			proc[P].C = atoi(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			proc[P].L = atoi(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			proc[P].A = atoi(ptr);
			P++;
			break;
		case 'S':
		case 's':
			ptr = strtok_s( NULL, SEP, &pTokenContext );
			cRefNode0 = _strdup(ptr);
			break;
//R  1 bp 1 8
		case 'R':
		case 'r':
			ptr = strtok_s( NULL, SEP, &pTokenContext );	// N
			regis[R].N = _strdup(ptr);
			ptr = strtok_s( NULL, SEP, &pTokenContext );	// format
			ptr = strtok_s( NULL, SEP, &pTokenContext );	// num
			ptr = strtok_s( NULL, SEP, &pTokenContext );	// area
			regis[R].A = atoi(ptr);
			R++;
			break;
		}
	}
	fclose( fp );

	for( n=0 ; n<N ; n++ ){
		if( !strcmp( node[n].N, cRefNode0 ) ){
//			RefNode = &(node[n]);
			nRefNode = n;
			break;
		}
	}

	/* 各ノードに演算器種類を割り当て */
	for( n=0 ; n<N ; n++ ){
		for( j=0 ; j<P ; j++ ){
			if( !strcmp(node[n].T,proc[j].T) ){
				node[n].proc = j;
				node[n].C = proc[j].C;
				node[n].L = proc[j].L;
				break;
			}
		}
	}

	// 枝の始点と終点を正しく設定する
	for( k=0 ; k<E ; k++ ){
		c = 0;
		n = 0;
		do{
			if( !strcmp( node[n].N, edge_f[k] ) ){
				edge[k].f = &(node[n]);
				c++;
			}
			if( !strcmp( node[n].N, edge_t[k] ) ){
				edge[k].t = &(node[n]);
				c++;
			}
			n++;
		}while( c < 2 && n<N );
	}

	for( k=0 ; k<E ; k++ ){
		free( edge_f[k] );
		free( edge_t[k] );
	}
	GlobalFree( edge_f );
	GlobalFree( edge_t );
	free( cRefNode0 );

	ConstructNetlist();

	return TRUE;
}

void CDFG::releasedata( void )
{
	int k;
	if( m_node ){
		for( k=0 ; k<N ; k++ ){////////////////////////
			free( m_node[k].N );
			free( m_node[k].T );
		}
		GlobalFree( m_node );
		m_node = NULL;
	}
	if( m_edge ){
		for( k=0 ; k<E ; k++ ){
			free( m_edge[k].N );
		}
		GlobalFree( m_edge );
		m_edge = NULL;
	}
	if( m_proc ){
		for( k=0 ; k<P ; k++ ){
			free( m_proc[k].N );
			free( m_proc[k].T );
		}
		GlobalFree( m_proc );
		m_proc = NULL;
	}
	if( m_regis ){
		for( k=0 ; k<R ; k++ ){
			free( m_regis[k].N );
		}
		GlobalFree( m_regis );
		m_regis = NULL;
	}

}

void CDFG::ConstructNetlist(void)
{
	int e,n;
	NODE *node = m_node;
	EDGE *edge = m_edge;
	NODE *nptr;

	for( n=0 ; n<N ; n++ ){
		node[n].olist = NULL;
		node[n].ilist = NULL;
	}
	for( e=0 ; e<E ; e++ ){
		edge[e].ilist = NULL;
		edge[e].olist = NULL;
	}

	for( e=0 ; e<E ; e++ ){
		nptr = edge[e].f;
		edge[e].olist = nptr->olist;
		nptr->olist = &(edge[e]);
		nptr = edge[e].t;
		edge[e].ilist = nptr->ilist;
		nptr->ilist = &(edge[e]);
	}

#ifdef DEBUG1
	FILE *fp = fopen( "netlist.txt", "w" );
	EDGE *eptr;
	for( n=0 ; n<N ; n++ ){
		eptr = node[n].ilist;
		if( eptr == NULL ) continue;
		fprintf( fp, "In(%2s): ", node[n].N );
		while( eptr != NULL ){
			fprintf( fp, "(%s,%s) ", eptr->f->N, eptr->t->N );
			eptr = eptr->ilist;
		}
		fprintf( fp, "\n" );
	}
	for( n=0 ; n<N ; n++ ){
		eptr = node[n].olist;
		if( eptr == NULL ) continue;
		fprintf( fp, "Out(%2s): ", node[n].N );
		while( eptr != NULL ){
			fprintf( fp, "(%s,%s) ", eptr->f->N, eptr->t->N );
			eptr = eptr->olist;
		}
		fprintf( fp, "\n" );
	}
	fclose( fp );
#endif
}

void CDFG::Duplicate(CDFG *pDFG)
{
	NODE *node0 = pDFG->m_node;
	EDGE *edge0 = pDFG->m_edge;
	EDGE *eptr,**epptr;
	int e;
	int n;

	N = pDFG->N;
	E = pDFG->E;
	m_node = (NODE *)GlobalAlloc( GPTR, N*sizeof(NODE) );
	m_edge = (EDGE *)GlobalAlloc( GPTR, E*sizeof(EDGE) );

	for( n=0 ; n<N ; n++ ){
		m_node[n].n = node0[n].n;
		m_node[n].C = node0[n].C;
		m_node[n].L = node0[n].L;
		m_node[n].T = _strdup( node0[n].T );
		m_node[n].N = _strdup( node0[n].N );
	}
	for( e=0 ; e<E ; e++ ){
		m_edge[e].N = _strdup( edge0[e].N );
		//
		n = edge0[e].f->n;
		m_edge[e].f = &(m_node[n]);
		//
		n = edge0[e].t->n;
		m_edge[e].t = &(m_node[n]);
	}
	ConstructNetlist();
}
