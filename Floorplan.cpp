#include "stdafx.h"
#include "Floorplan.h"

CFloorplan::CFloorplan()
{
	m_listCommunicationRequirement = NULL;
}

CFloorplan::~CFloorplan()
{
}

// ���W���[���̕��̒����ɓ��o�͒[�q������ƍl���āA���̍��W�����߂� nInout=0: ���͒[�q�AnInOut=1: �o�͒[�q
void CFloorplan::GetCoordinateModuleTerminal(int nIndexModule, int nInOut, int *pnX, int *pnY)
{
	// ���W���[���͉�]�p�x�ɂ�炸�������̍��W��m_Module[nIndexModule].x�Am_Module[nIndexModule].y�ɐݒ肳��Ă���
	switch( m_Module[nIndexModule].nOrientation ){
	case 0:	// ����]
		*pnX = m_Module[nIndexModule].x+m_Module[nIndexModule].nWidth/2;
		if( nInOut==0 ){
			*pnY = m_Module[nIndexModule].y+m_Module[nIndexModule].nHeight;
		}else{
			*pnY = m_Module[nIndexModule].y;
		}
		break;
	case 1:	// �����v����90�x��]
		if( nInOut==0 ){
			*pnX = m_Module[nIndexModule].x;
		}else{
			*pnX = m_Module[nIndexModule].x+m_Module[nIndexModule].nHeight;
		}
		*pnY = m_Module[nIndexModule].y+m_Module[nIndexModule].nWidth/2;
		break;
	case 2:	// �����v����180�x��]
		*pnX = m_Module[nIndexModule].x+m_Module[nIndexModule].nWidth/2;
		if( nInOut==0 ){
			*pnY = m_Module[nIndexModule].y;
		}else{
			*pnY = m_Module[nIndexModule].y+m_Module[nIndexModule].nHeight;
		}
		break;
	default:	// �����v����270�x��]
		if( nInOut==0 ){
			*pnX = m_Module[nIndexModule].x+m_Module[nIndexModule].nHeight;
		}else{
			*pnX = m_Module[nIndexModule].x;
		}
		*pnY = m_Module[nIndexModule].y+m_Module[nIndexModule].nWidth/2;
		break;
	}
}

int CFloorplan::GetScore()
{
	int nRetVal;
	if( m_nModeProFinal==0 ){
		nRetVal = GetScorePro();
	}else{
		nRetVal = GetScoreFinal();
	}
	return nRetVal;
}

int CFloorplan::GetScorePro()
{
	////////////////////////////////
	Pack();
	////////////////////////////////

	int nReturnValue;
	int nScore;
	int nScoreMax;
	int nScoreTotal;
	COMREQUIREMENT *crptr;
	int nXs,nYs,nXd,nYd,nXv1,nYv1,nXv2,nYv2;
	nScoreMax = 0;
	nScoreTotal = 0;

	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		GetCoordinateModuleTerminal(crptr->nIndexModuleSource, 1/*InOut*/, &nXs, &nYs);
		GetCoordinateModuleTerminal(crptr->nIndexModuleDest, 0/*InOut*/, &nXd, &nYd);
		if( crptr->nIndexModuleVia >= 0 ){
			GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 0/*InOut*/, &nXv1, &nYv1);
			GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 1/*InOut*/, &nXv2, &nYv2);
			nScore = CalcWireDelay(nXs, nYs, nXv1, nYv1);
			nScore += CalcWireDelay(nXv2, nYv2, nXd, nYd);
		}else{
			nScore = CalcWireDelay(nXs, nYs, nXd, nYd);
		}
		nScore += crptr->nDelayModuleSource;
		nScore += crptr->nDelayModuleVia;
		nScore += crptr->nDelayModuleDest;
		//
		if( nScoreMax < nScore ) nScoreMax = nScore;
		nScoreTotal += nScore;
	}
	nReturnValue = nScoreMax*10+nScoreTotal/20;
	nReturnValue = nScoreMax;
#if 0
	if( nReturnValue < 0 ){
		int nModule = GetModuleNum();
		int m;
		int nIndex;
		int x, y, nOrientation;
		int w, h;
		FILE *fp;

		fopen_s( &fp, "status.txt", "a" );
		fprintf( fp, "nScoreMax=%d nScoreTotal=%d\n", nScoreMax, nScoreTotal );
		fprintf( fp, "m_listCommunicationRequirement=%0X\n", m_listCommunicationRequirement );
		for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
			GetCoordinateModuleTerminal(crptr->nIndexModuleSource, 1/*InOut*/, &nXs, &nYs);
			GetCoordinateModuleTerminal(crptr->nIndexModuleDest, 0/*InOut*/, &nXd, &nYd);
			if( crptr->nIndexModuleVia >= 0 ){
				GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 0/*InOut*/, &nXv1, &nYv1);
				GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 1/*InOut*/, &nXv2, &nYv2);
				nScore = CalcWireDelay(nXs, nYs, nXv1, nYv1);
				nScore += CalcWireDelay(nXv2, nYv2, nXd, nYd);
			}else{
				nScore = CalcWireDelay(nXs, nYs, nXd, nYd);
			}
			nScore += crptr->nDelayModuleSource;
			nScore += crptr->nDelayModuleVia;
			nScore += crptr->nDelayModuleDest;
			fprintf( fp, "nIndexModuleSource=%d(%d,%d) nIndexModuleDest=%d(%d,%d) nScore=%d\n", crptr->nIndexModuleSource, nXs, nYs, crptr->nIndexModuleDest, nXd, nYd, nScore );
		}
		fclose(fp);

		fopen_s( &fp, "illegalFloorplanPro.out", "w" );

		fprintf( fp, "*Score\n" );
		fprintf( fp, "%d\n", m_nScore );
		fprintf( fp, "\n" );
		fprintf( fp, "*Module_data\n" );
		// ���Z�탂�W���[��
		for( m=0 ; m<nModule ; m++ ){
			GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
			fprintf( fp, "%d %d %d\n", m, w, h );
		}

		fprintf( fp, "\n" );
		fprintf( fp, "*Coordinate\n" );
		for( m=0 ; m<nModule ; m++ ){
			GetModuleStatus( m, &x, &y, &w, &h, &nOrientation );
			fprintf( fp, "%d %d %d\n", x, y, nOrientation );
		}
		fclose(fp);
	}
#endif
	return nReturnValue;
}

int CFloorplan::GetScoreFinal()
{
	////////////////////////////////
	Pack();
	////////////////////////////////

	int nReturnValue;
	int nScore;
	int nScoreMax;
	int nScoreTotal;
	int nScoreTotalSquare;
	COMREQUIREMENT *crptr;
	int nXs,nYs,nXd,nYd,nXv1,nYv1,nXv2,nYv2;
	nScoreMax = 0;
	nScoreTotal = 0;
	nScoreTotalSquare = 0;

	for( crptr=m_listCommunicationRequirement ; crptr ; crptr=crptr->next ){
		GetCoordinateModuleTerminal(crptr->nIndexModuleSource, 1/*InOut*/, &nXs, &nYs);
		GetCoordinateModuleTerminal(crptr->nIndexModuleDest, 0/*InOut*/, &nXd, &nYd);
		if( crptr->nIndexModuleVia >= 0 ){
			GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 0/*InOut*/, &nXv1, &nYv1);
			GetCoordinateModuleTerminal(crptr->nIndexModuleVia, 1/*InOut*/, &nXv2, &nYv2);
			nScore = CalcWireDelay(nXs, nYs, nXv1, nYv1);
			nScore += CalcWireDelay(nXv2, nYv2, nXd, nYd);
		}else{
			nScore = CalcWireDelay(nXs, nYs, nXd, nYd);
		}
		nScore += crptr->nDelayModuleSource;
		nScore += crptr->nDelayModuleVia;
		nScore += crptr->nDelayModuleDest;
		//
		if( nScoreMax < nScore ) nScoreMax = nScore;
		nScoreTotal += nScore;
		nScoreTotalSquare += nScore*nScore;
	}
	nReturnValue = nScoreMax*10+nScoreTotal/20;
	nReturnValue = nScoreMax;
	//nReturnValue = nScoreTotalSquare;
	return nReturnValue;
}

int CFloorplan::CalcWireDelay(int nX1, int nY1, int nX2, int nY2)
{
	int nLengthTotal;
	int nLength;
	//
	// �}���n�b�^�����������߂�
	nLength = nX1-nX2;
	if( nLength<0 ) nLength = nX2-nX1;
	nLengthTotal = nLength;
	//
	nLength = nY1-nY2;
	if( nLength<0 ) nLength = nY2-nY1;
	nLengthTotal += nLength;
	//
	return nLengthTotal*DELAY_WIRE_PER_LENGTH;
}

