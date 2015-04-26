#include "StdAfx.h"
#include "GaussMixModified.h"
#include <math.h>


GaussMix::GaussMix(void)
{
	//-------------------------------------------------------------------------------------------------
	//�����������ã�����ʱ�޸�
	m_dStdThreshold = 12;//7.5	// ������OpenCV�еĶ��壬�жϷ��ϵ�ǰ��ģ�͵ķ���ı��� /* lambda=2.5 is 99% */
	//m_dStdThreshold = .045;//������ֵ������log(x/255+1)��������ж�ǰ��
	m_dBackgroundThreshold = 0.75;		//�ж�Ϊǰ���Ľ��ձ��� ��鼸����˹ģ�ͣ�ԽС���Խ�٣�Ϊ0��Ϊ����˹/* threshold sum of weights for background test */
	m_dUpdataRateWeight = 0.05;	//0.05			//����Ȩ�صı���
	m_dUpdataRateWeight2 = 1 - m_dUpdataRateWeight;
	m_dUpdataRateMean = 0.03;//0.03
	m_dUpdataRateSigma = 0.05;//0.03
	m_dUpdataRateSigma2 = 1 - m_dUpdataRateSigma;
	m_nGaussians = 5;					//��˹ģ�͵Ĳ���Ϊ5
	m_dWeightInit = 0.05;				//��ʼȨ��Ϊ0.05
	m_nSigmaInit = 2 * m_dStdThreshold;					//��ʼ����Ϊ30
	m_nRefreshPercent = 16;				//ÿ֡���±���Ϊȫ��ͼ���1/30

	// zcx���������δ�С
	m_nRectWith=4;
	m_nRectHeight=4;
	//-------------------------------------------------------------------------------------------------
	m_UpdateLock=false;
	m_AccessLock=false;
	//-------------------------------------------------------------------------------------------------
	//�������ÿ�
	m_pfBkMean.clear();
	m_pfBkWeight.clear();
	m_pfBkSigma.clear();

	m_BkgImage=NULL;
	m_UpdateFrameNumber=0;
	m_GrayOffset=0;
	//-------------------------------------------------------------------------------------------------
}

GaussMix::GaussMix(double StdThreshold,	double BackgroundThreshold, double UpdataRateWeight,
				   double UpdataRateMean, double UpdataRateSigma, double WeightInit, int Gaussians, int RefreshPercent)
{
	m_dStdThreshold = StdThreshold;//7.5	// ������OpenCV�еĶ��壬�жϷ��ϵ�ǰ��ģ�͵ķ���ı��� /* lambda=2.5 is 99% */
	m_dBackgroundThreshold = BackgroundThreshold;		//�ж�Ϊǰ���Ľ��ձ��� /* threshold sum of weights for background test */
	m_dUpdataRateWeight = UpdataRateWeight;				//����Ȩ�صı���
	m_dUpdataRateWeight2 = 1.0 - m_dUpdataRateWeight;
	m_dUpdataRateMean = UpdataRateMean;//0.03
	m_dUpdataRateSigma = 0.05;//0.03
	m_dUpdataRateSigma2 = 1 - m_dUpdataRateSigma;
	m_nGaussians = Gaussians;					//��˹ģ�͵Ĳ���Ϊ5
	m_dWeightInit = WeightInit;				//��ʼȨ��Ϊ0.05
	m_nSigmaInit = 2 * m_dStdThreshold;					//��ʼ����Ϊ30
	m_nRefreshPercent = RefreshPercent;				//ÿ֡���±���Ϊȫ��ͼ���1/30

	//-------------------------------------------------------------------------------------------------
	m_UpdateLock=false;
	m_AccessLock=false;
	//-------------------------------------------------------------------------------------------------
	//�������ÿ�
	m_pfBkMean.clear();
	m_pfBkWeight.clear();
	m_pfBkSigma.clear();
	//-------------------------------------------------------------------------------------------------
	m_BkgImage=NULL;
	m_UpdateFrameNumber=0;
	m_GrayOffset=0;
}


GaussMix::~GaussMix(void)
{
	int t_nGaussians = m_pfBkMean.size();
	int i;
	for ( i = 0; i < t_nGaussians; i++ )
	{
		double * t_pfData = m_pfBkMean.back();
		delete[] t_pfData;
		m_pfBkMean.pop_back();

		t_pfData = m_pfBkWeight.back();
		delete[] t_pfData;
		m_pfBkWeight.pop_back();

		t_pfData = m_pfBkSigma.back();
		delete[] t_pfData;
		m_pfBkSigma.pop_back();
	}

	if(m_BkgImage!=NULL)
		cvReleaseImage(&m_BkgImage);
}

int GaussMix::Init( IplImage* m_pFrame )	//��ʼ������
{
	//m_pBkImg=m_pFrame;
	int i, j;
	//---------------------------------------------------------------------
	//��ȡͼ���������
	m_nImageWidth = m_pFrame->width;
	m_nImageHeight = m_pFrame->height;
	m_nDataSize = m_pFrame->imageSize;
	m_nPixNumber = m_nImageWidth * m_nImageHeight;
	m_nChannels = m_pFrame->nChannels;
	m_nLineSize = m_pFrame->widthStep;
	m_nLineExpand = m_nLineSize - m_nImageWidth * m_nChannels;
	//---------------------------------------------------------------------
	

	//---------------------------------------------------------------------
	//��վɿռ�
	int t_nGaussians = m_pfBkMean.size();
	for ( i = 0; i < t_nGaussians; i++ )
	{
		double * t_pfData = m_pfBkMean.back();
		delete[] t_pfData;
		m_pfBkMean.pop_back();

		t_pfData = m_pfBkWeight.back();
		delete[] t_pfData;
		m_pfBkWeight.pop_back();

		t_pfData = m_pfBkSigma.back();
		delete[] t_pfData;
		m_pfBkSigma.pop_back();
	}
	//---------------------------------------------------------------------
	
	
	//---------------------------------------------------------------------
	//���ٿռ�
	for ( i = 0; i < m_nGaussians; i++ )
	{
		double * t_pfBkMean;
		t_pfBkMean = new double [m_nDataSize];
		m_pfBkMean.push_back( t_pfBkMean );

		double * t_pfBkWeight;
		t_pfBkWeight = new double [m_nPixNumber];		//Ȩ�أ�ÿ����һ��
		m_pfBkWeight.push_back( t_pfBkWeight );

		double * t_pfBkSigma;
		t_pfBkSigma = new double [m_nDataSize];
		m_pfBkSigma.push_back( t_pfBkSigma );
	}
	//---------------------------------------------------------------------


	//---------------------------------------------------------------------
	//��ʼ������
	for ( i = 0; i < m_nDataSize; i++ )
	{
		m_pfBkMean[0][i] = (unsigned char)m_pFrame->imageData[i];		//��ʼ������0
		m_pfBkSigma[0][i] = m_nSigmaInit;

		for ( j = 1; j < m_nGaussians; j++ )		//���γ�ʼ��������������
		{
			m_pfBkMean[j][i] = -60 * j;
			m_pfBkSigma[j][i] = m_nSigmaInit;
		}
	}

	for ( i = 0; i < m_nPixNumber; i++ )
	{
		m_pfBkWeight[0][i] = 1;

		for ( j = 1; j < m_nGaussians; j++ )		//���γ�ʼ����������
		{
			m_pfBkWeight[j][i] = 0;
		}
	}

	//---------------------------------------------------------------------
	m_BkgImage=cvCreateImage(cvSize(m_pFrame->width,m_pFrame->height),m_pFrame->depth,m_pFrame->nChannels);
	m_UpdateFrameNumber=0;
	m_GrayOffset=0;
	return 1;
}
void GaussMix::SetParas(double StdThreshold,	double BackgroundThreshold, double UpdataRateWeight,	
						double UpdataRateMean, double UpdataRateSigma, double WeightInit, int Gaussians,  int RefreshPercent)
{

	int t_nGaussians = m_pfBkMean.size();
	int i=0;
	for ( i = 0; i < t_nGaussians; i++ )
	{
		double * t_pfData = m_pfBkMean.back();
		delete[] t_pfData;
		m_pfBkMean.pop_back();

		t_pfData = m_pfBkWeight.back();
		delete[] t_pfData;
		m_pfBkWeight.pop_back();

		t_pfData = m_pfBkSigma.back();
		delete[] t_pfData;
		m_pfBkSigma.pop_back();
	}

	m_dStdThreshold = StdThreshold;//7.5	// ������OpenCV�еĶ��壬�жϷ��ϵ�ǰ��ģ�͵ķ���ı��� /* lambda=2.5 is 99% */
	m_dBackgroundThreshold = BackgroundThreshold;		//�ж�Ϊǰ���Ľ��ձ��� /* threshold sum of weights for background test */
	m_dUpdataRateWeight = UpdataRateWeight;				//����Ȩ�صı���
	m_dUpdataRateWeight2 = 1.0 - m_dUpdataRateWeight;
	m_dUpdataRateMean = UpdataRateMean;//0.03
	m_dUpdataRateSigma = 0.05;//0.03
	m_dUpdataRateSigma2 = 1 - m_dUpdataRateSigma;
	m_nGaussians = Gaussians;					//��˹ģ�͵Ĳ���Ϊ5
	m_dWeightInit = WeightInit;				//��ʼȨ��Ϊ0.05
	m_nSigmaInit = 2 * m_dStdThreshold;					//��ʼ����Ϊ30
	m_nRefreshPercent = RefreshPercent;				//ÿ֡���±���Ϊȫ��ͼ���1/30

	//-------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------
	//�������ÿ�
	m_pfBkMean.clear();
	m_pfBkWeight.clear();
	m_pfBkSigma.clear();

	m_UpdateFrameNumber=0;
	m_GrayOffset=0;
	return;
}
//int GaussMix::UpdateRGB( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber )
//{
//	Update1( m_pFrame, m_pBkImg, m_dCurrentFrameNumber );
//	return 1;
//}
//
//int GaussMix::UpdateHSV( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber )
//{
//	Update1( m_pFrame, m_pBkImg, m_dCurrentFrameNumber );
//	return 1;
//}

//int GaussMix::UpdateYUV( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber )
//{
//	int t_nOffset = Update1( m_pFrame, m_pBkImg, m_dCurrentFrameNumber );
//	CountFrImage1( m_pFrame, m_pFrImg, t_nOffset );
//	return 1;
//}

int GaussMix::Update( IplImage* m_pFrame,int choose_way)
{
	if (choose_way==1)//������±���
	{
		Update1(m_pFrame);
	}
	else
		if(choose_way==2)
		{
			Update2(m_pFrame);
		}
	return 0;
}

int GaussMix::Update1( IplImage* m_pFrame/*, IplImage* m_pBkImg, int m_dCurrentFrameNumber*/ )
{
	if(m_BkgImage==NULL)
		return -1;
	if (m_pFrame==NULL)//zcx
	{
		return -1;
	}
	//---------------------------------------------------------------------
	//������ʼ����ֹ��ַ
	if (!m_pFrame)
	{
		return -1;
	}
	//zcx������γ�ʼֵ
	int t_UpdateNumb=m_UpdateFrameNumber%(m_nRectHeight*m_nRectWith);
	int t_m=(t_UpdateNumb)/m_nRectWith;
	int t_n=(t_UpdateNumb)%m_nRectWith;
	//int t_nNumberOfLine = m_nImageHeight / m_nRefreshPercent;	//��ǰ֡һ����������Ҫ����
	//if ( m_nImageHeight % m_nRefreshPercent > m_dCurrentFrameNumber % m_nRefreshPercent )	//�����������������粻�����������������һ�У�
	//{
	//	t_nNumberOfLine++;
	//}

	/*int t_nRange = m_nLineSize * ( m_nRefreshPercent - 1 );
	int t_nRangeWeight = m_nImageWidth * ( m_nRefreshPercent - 1 );*/

	int i, j, k, l;
	int t_nMatchLevel;		//ƥ���ģ�ͱ��
	double t_dThreshold;		//�������ֵ
	double t_dAddupSigma;		//����ɫͨ������ĺ�
	double t_dDelta;			//��ǰͨ���Ĳ�

	int t_nPossition;		//��ǰ��ĵ�ַ��
	int t_nPossitionWeight;	//Ȩ�صĵ�ַ��

	//t_nPossition = ( t_m * m_nImageWidth + t_n )*m_nChannels;	//ָ��
	//t_nPossitionWeight = (  t_m * m_nImageWidth + t_n );	//ָ���һ������
	//t_nPossition = ( m_dCurrentFrameNumber % m_nRefreshPercent ) * m_nLineSize;	//ָ���һ������
	//t_nPossitionWeight = ( m_dCurrentFrameNumber % m_nRefreshPercent ) * m_nImageWidth;	//ָ���һ������

	int m_nStatDelta[512] = {0};	//����ͳ�ƻҶȲ�ֲ�������
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	//zcx
	//for (int t_m=0;t_m<m_nRectHeight;++t_m)
		//for (int t_n=0;t_n<m_nRectWith;++t_n)
		//{
			//��ʼ����
			for ( i = t_m; i < m_nImageHeight; i+=m_nRectHeight )		//һ��ɨ��
			{
				for ( j = t_n; j < m_nImageWidth; j+=m_nRectWith )	//ÿ��ɨ��j������
				{
					t_nPossition = ( i * m_nImageWidth + j )*m_nChannels;	//ָ��
					t_nPossitionWeight = ( i * m_nImageWidth + j );	//ָ��
					t_dDelta = (unsigned char)m_pFrame->imageData[t_nPossition] - m_pfBkMean[0][t_nPossition];	//����ͳ�����ȸı�
					m_nStatDelta[ (int)t_dDelta + 255 ] ++;	//ͳ�Ʋ�ֵ

					//---------------------------------------------------------
					//���ҵ�ƥ���ģ��
					t_nMatchLevel = -1;		//���û���κ�ģ����ƥ�䣬����Ϊ-1

					for ( k = 0; k < m_nGaussians; k++ )	//ɨ�����ģ��
					{
						t_dAddupSigma = 0;		//�������0
						t_dThreshold = 0;		//������ֵ��0

						for ( l = 0; l < m_nChannels; l++ )		//�����ͨ����ֵ�ͷ����
						{
							t_dDelta = m_pfBkMean[k][t_nPossition + l] - (unsigned char)m_pFrame->imageData[t_nPossition + l];	//�����

							t_dAddupSigma += t_dDelta * t_dDelta;				//ͬ���ط����ۼ�
							t_dThreshold += m_pfBkSigma[k][t_nPossition + l];	//ͬ���ط�����ֵ�ۼ�
						}

						if ( t_dAddupSigma< t_dThreshold * m_dStdThreshold )	//�������С�ڷֲ�
						{
							t_nMatchLevel = k;
							break;
						}
					}
					//---------------------------------------------------------

					//---------------------------------------------------------
					//�ҵ�ƥ��ģ�ͺ󣬶Ը�ģ�ͽ����޸�
					if ( t_nMatchLevel != -1 )		//���ĳһģ��ƥ�䣬�����
					{
						//----------------------------------------------
						//���²���
						for ( k = 0; k < m_nGaussians; k++ )
						{
							m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k][t_nPossitionWeight] * ( m_dUpdataRateWeight2 );	//����Ȩ��
						}

						m_pfBkWeight[t_nMatchLevel][t_nPossitionWeight] += m_dUpdataRateWeight;			//����Ȩ��

						for ( l = 0; l < m_nChannels; l++ )	//���¾�ֵ�ͷ���
						{
							t_dDelta = (unsigned char)m_pFrame->imageData[t_nPossition + l] - m_pfBkMean[t_nMatchLevel][t_nPossition + l];
							m_pfBkMean[t_nMatchLevel][t_nPossition + l] = m_pfBkMean[t_nMatchLevel][t_nPossition + l] + t_dDelta * m_dUpdataRateMean;	//���¾�ֵ
							m_pfBkSigma[t_nMatchLevel][t_nPossition + l] = t_dDelta * t_dDelta * m_dUpdataRateSigma + m_pfBkSigma[t_nMatchLevel][t_nPossition + l] * ( m_dUpdataRateSigma2 );

							//					m_pBkImg->imageData[t_nPossition + l] = (int)m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
						}
						//----------------------------------------------

						//----------------------------------------------
						//����˳��
						for ( k = t_nMatchLevel; k > 0; k-- )	//���¶��϶Ա��ж�
						{
							if ( m_pfBkWeight[k][t_nPossitionWeight] > m_pfBkWeight[k - 1][t_nPossitionWeight] )	//Ȩ��˳���Ƿ����仯����������仯�����¸���ģ��
							{
								//����Ȩֵ
								double t_dWeight;
								t_dWeight = m_pfBkWeight[k][t_nPossitionWeight];	
								m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k -1][t_nPossitionWeight];
								m_pfBkWeight[k - 1][t_nPossitionWeight] = t_dWeight;

								//���¸�������ֵ
								for ( l = 0; l < m_nChannels; l++ )
								{
									double t_dForChange;
									t_dForChange = m_pfBkMean[k][t_nPossition + l];//������ֵ
									m_pfBkMean[k][t_nPossition + l] = m_pfBkMean[k -1][t_nPossition + l];
									m_pfBkMean[k -1][t_nPossition + l] = t_dForChange;

									t_dForChange = m_pfBkSigma[k][t_nPossition + l];//��������
									m_pfBkSigma[k][t_nPossition + l] = m_pfBkSigma[k -1][t_nPossition + l];
									m_pfBkSigma[k -1][t_nPossition + l] = t_dForChange;
								}
							}
							else	//�����˳�ѭ��
							{
								break;
							}
						}
					}
					else	//���û���κ�ģ��ƥ�䣬������µ�ģ��
					{
						m_pfBkWeight[m_nGaussians - 1][t_nPossitionWeight] = m_dUpdataRateWeight;

						double t_dWeightAddup = 0;

						for ( k = 0; k < m_nGaussians; k++ )
						{
							t_dWeightAddup += m_pfBkWeight[k][t_nPossitionWeight];
						}

						for ( k = 0; k < m_nGaussians; k++ )
						{
							m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k][t_nPossitionWeight] / t_dWeightAddup;//��һ��
						}

						for ( l = 0; l < m_nChannels; l++ )
						{
							m_pfBkMean[m_nGaussians - 1][t_nPossition + l] = (unsigned char)m_pFrame->imageData[t_nPossition + l];
							m_pfBkSigma[m_nGaussians - 1][t_nPossition + l] = m_nSigmaInit;

							//					m_pBkImg->imageData[t_nPossition + l] = (unsigned char)m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
						}

					}
					//---------------------------------------------------------

					//---------------------------------------------------------
					//���ɱ���ͼ��
					//for ( l = 0; l < m_nChannels; l++ )
					//{
					//	/*m_pBkImg*/m_BkgImage->imageData[t_nPossition + l] = (int)/*(unsigned char)*/m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
					//}
					//---------------------------------------------------------

					//---------------------------------------------------------
					//���²���
					//t_nPossitionWeight ++;
					//t_nPossition += m_nChannels;
					//---------------------------------------------------------
				}
				//m_UpdateLock=false;

				//t_nPossition = ( i * m_nRectWith + t_n ) * m_nLineSize;	//ָ��
				//t_nPossitionWeight = (  t_m * m_nRectWith + t_n ) * m_nImageWidth;	//ָ���һ������
			}
		//}
	
	//---------------------------------------------------------------------
	i=0;
	while(m_UpdateLock)
	{
		//Sleep(1);
		i++;
		if(i>1000)
			return -1;
	}

	i=0;
	while(m_AccessLock)
	{
		//Sleep(1);
		i++;
		if(i>1000)
			return -2;
	}
	m_UpdateLock=true;
	for ( i = t_m; i < m_nImageHeight; i+=m_nRectHeight )		//һ��ɨ��
	{
		for ( j = t_n; j < m_nImageWidth; j+=m_nRectWith )	//ÿ��ɨ��j������
		{
			t_nPossition = ( i * m_nImageWidth + j )*m_nChannels;	//ָ��
			t_nPossitionWeight = ( i * m_nImageWidth + j );	//ָ��
			for ( l = 0; l < m_nChannels; l++ )
			{
				/*m_pBkImg*/m_BkgImage->imageData[t_nPossition + l] = (int)/*(unsigned char)*/m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
			}
		}
	}

	m_UpdateLock=false;
	//---------------------------------------------------------------------
	//ͳ������ƫ��
	int t_nAddUp = m_nStatDelta[100] + m_nStatDelta[101];
	///int t_nAddUp = 0;
	//int sum=0;
	int t_nMaxGrayLevel = 0;	//����ͳ��ֵ
	int t_nMaxGrayPos = 0;		//���ͳ��ֵ��������

	for ( i =102; i < 501; i++ )//�����ֵ
	{
	t_nAddUp += m_nStatDelta[i];

	if ( t_nAddUp > t_nMaxGrayLevel )
	{
	t_nMaxGrayPos = i;
	t_nMaxGrayLevel = t_nAddUp;
	}

	t_nAddUp -= m_nStatDelta[i-2];
	}
	/*for ( i = 200; i < 401; i++ )
	{
		t_nAddUp += m_nStatDelta[i]*i;
		sum+=m_nStatDelta[i];
		
	}
	t_nMaxGrayPos=t_nAddUp/sum;*/
	m_UpdateFrameNumber++;
	m_GrayOffset= t_nMaxGrayPos - 255;
	return ( t_nMaxGrayPos - 255 );
}
//Update2��ʾ���и���
int GaussMix::Update2( IplImage* m_pFrame)
{
	if(m_BkgImage==NULL)
		return -1;
	if (m_pFrame==NULL)//zcx
	{
		return -1;
	}
	//---------------------------------------------------------------------
	//������ʼ����ֹ��ַ
	int t_nNumberOfLine = m_nImageHeight / m_nRefreshPercent;	//��ǰ֡һ����������Ҫ����
	if ( m_nImageHeight % m_nRefreshPercent > m_UpdateFrameNumber % m_nRefreshPercent )	//�����������������粻�����������������һ�У�
	{
		t_nNumberOfLine++;
	}

	int t_nRange = m_nLineSize * ( m_nRefreshPercent - 1 );
	int t_nRangeWeight = m_nImageWidth * ( m_nRefreshPercent - 1 );

	int i, j, k, l;
	int t_nMatchLevel;		//ƥ���ģ�ͱ��
	double t_dThreshold;		//�������ֵ
	double t_dAddupSigma;		//����ɫͨ������ĺ�
	double t_dDelta;			//��ǰͨ���Ĳ�

	int t_nPossition;		//��ǰ��ĵ�ַ��
	int t_nPossitionWeight;	//Ȩ�صĵ�ַ��

	t_nPossition = ( m_UpdateFrameNumber % m_nRefreshPercent ) * m_nLineSize;	//ָ���һ������
	t_nPossitionWeight = ( m_UpdateFrameNumber % m_nRefreshPercent ) * m_nImageWidth;	//ָ���һ������

	int m_nStatDelta[512] = {0};	//����ͳ�ƻҶȲ�ֲ�������
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	//��ʼ����
	for ( i = 0; i < t_nNumberOfLine; i++ )		//һ��ɨ�� t_nNumberOfLine ��
	{
		for ( j = 0; j < m_nImageWidth; j++ )	//ÿ��ɨ��j������
		{
			t_dDelta = (unsigned char)m_pFrame->imageData[t_nPossition] - m_pfBkMean[0][t_nPossition];	//����ͳ�����ȸı�
			m_nStatDelta[ (int)t_dDelta + 255 ] ++;	//ͳ�Ʋ�ֵ

			//---------------------------------------------------------
			//���ҵ�ƥ���ģ��
			t_nMatchLevel = -1;		//���û���κ�ģ����ƥ�䣬����Ϊ-1

			for ( k = 0; k < m_nGaussians; k++ )	//ɨ�����ģ��
			{
				t_dAddupSigma = 0;		//�������0
				t_dThreshold = 0;		//������ֵ��0

				for ( l = 0; l < m_nChannels; l++ )		//�����ͨ����ֵ�ͷ����
				{
					t_dDelta = m_pfBkMean[k][t_nPossition + l] - (unsigned char)m_pFrame->imageData[t_nPossition + l];	//�����

					t_dAddupSigma += t_dDelta * t_dDelta;				//ͬ���ط����ۼ�
					t_dThreshold += m_pfBkSigma[k][t_nPossition + l];	//ͬ���ط�����ֵ�ۼ�
				}

				if ( t_dAddupSigma< t_dThreshold * m_dStdThreshold )	//�������С�ڷֲ�
				{
					t_nMatchLevel = k;
					break;
				}
			}
			//---------------------------------------------------------

			//---------------------------------------------------------
			//�ҵ�ƥ��ģ�ͺ󣬶Ը�ģ�ͽ����޸�
			if ( t_nMatchLevel != -1 )		//���ĳһģ��ƥ�䣬�����
			{
				//----------------------------------------------
				//���²���
				for ( k = 0; k < m_nGaussians; k++ )
				{
					m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k][t_nPossitionWeight] * ( m_dUpdataRateWeight2 );	//����Ȩ��
				}

				m_pfBkWeight[t_nMatchLevel][t_nPossitionWeight] += m_dUpdataRateWeight;			//����Ȩ��

				for ( l = 0; l < m_nChannels; l++ )	//���¾�ֵ�ͷ���
				{
					t_dDelta = (unsigned char)m_pFrame->imageData[t_nPossition + l] - m_pfBkMean[t_nMatchLevel][t_nPossition + l];
					m_pfBkMean[t_nMatchLevel][t_nPossition + l] = m_pfBkMean[t_nMatchLevel][t_nPossition + l] + t_dDelta * m_dUpdataRateMean;	//���¾�ֵ
					m_pfBkSigma[t_nMatchLevel][t_nPossition + l] = t_dDelta * t_dDelta * m_dUpdataRateSigma + m_pfBkSigma[t_nMatchLevel][t_nPossition + l] * ( m_dUpdataRateSigma2 );

					//					m_pBkImg->imageData[t_nPossition + l] = (int)m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
				}
				//----------------------------------------------

				//----------------------------------------------
				//����˳��
				for ( k = t_nMatchLevel; k > 0; k-- )	//���¶��϶Ա��ж�
				{
					if ( m_pfBkWeight[k][t_nPossitionWeight] > m_pfBkWeight[k - 1][t_nPossitionWeight] )	//Ȩ��˳���Ƿ����仯����������仯�����¸���ģ��
					{
						//����Ȩֵ
						double t_dWeight;
						t_dWeight = m_pfBkWeight[k][t_nPossitionWeight];	
						m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k -1][t_nPossitionWeight];
						m_pfBkWeight[k - 1][t_nPossitionWeight] = t_dWeight;

						//���¸�������ֵ
						for ( l = 0; l < m_nChannels; l++ )
						{
							double t_dForChange;
							t_dForChange = m_pfBkMean[k][t_nPossition + l];//������ֵ
							m_pfBkMean[k][t_nPossition + l] = m_pfBkMean[k -1][t_nPossition + l];
							m_pfBkMean[k -1][t_nPossition + l] = t_dForChange;

							t_dForChange = m_pfBkSigma[k][t_nPossition + l];//��������
							m_pfBkSigma[k][t_nPossition + l] = m_pfBkSigma[k -1][t_nPossition + l];
							m_pfBkSigma[k -1][t_nPossition + l] = t_dForChange;
						}
					}
					else	//�����˳�ѭ��
					{
						break;
					}
				}
			}
			else	//���û���κ�ģ��ƥ�䣬������µ�ģ��
			{
				m_pfBkWeight[m_nGaussians - 1][t_nPossitionWeight] = m_dUpdataRateWeight;

				double t_dWeightAddup = 0;

				for ( k = 0; k < m_nGaussians; k++ )
				{
					t_dWeightAddup += m_pfBkWeight[k][t_nPossitionWeight];
				}

				for ( k = 0; k < m_nGaussians; k++ )
				{
					m_pfBkWeight[k][t_nPossitionWeight] = m_pfBkWeight[k][t_nPossitionWeight] / t_dWeightAddup;//��һ��
				}

				for ( l = 0; l < m_nChannels; l++ )
				{
					m_pfBkMean[m_nGaussians - 1][t_nPossition + l] = (unsigned char)m_pFrame->imageData[t_nPossition + l];
					m_pfBkSigma[m_nGaussians - 1][t_nPossition + l] = m_nSigmaInit;
				}
			}
			//---------------------------------------------------------


			//---------------------------------------------------------
			//���²���
			t_nPossitionWeight ++;
			t_nPossition += m_nChannels;
			//---------------------------------------------------------
		}

		t_nPossitionWeight += t_nRangeWeight;
		t_nPossition += t_nRange;		//ÿ��ָ�����׵�һ���ֽ�
	}

	i=0;
	while(m_UpdateLock)
	{
		//Sleep(1);
		i++;
		if(i>1000)
			return -1;
	}

	i=0;
	while(m_AccessLock)
	{
		//Sleep(1);
		i++;
		if(i>1000)
			return -2;
	}

	m_UpdateLock=true;
	t_nPossition = ( m_UpdateFrameNumber % m_nRefreshPercent ) * m_nLineSize;	//ָ���һ������
	t_nPossitionWeight = ( m_UpdateFrameNumber % m_nRefreshPercent ) * m_nImageWidth;	//ָ���һ������
	//---------------------------------------------------------------------
	for ( i = 0; i < t_nNumberOfLine; i++ )		//һ��ɨ�� t_nNumberOfLine ��
	{
		for ( j = 0; j < m_nImageWidth; j++ )	//ÿ��ɨ��j������
		{
			//���ɱ���ͼ��
			for ( l = 0; l < m_nChannels; l++ )
			{
				m_BkgImage->imageData[t_nPossition + l] = (unsigned char)m_pfBkMean[0][t_nPossition + l];	//����ѭ�������㱳��ͼ��
			}
			t_nPossition += m_nChannels;
		}
		t_nPossition += t_nRange;		
	}
	m_UpdateLock=false;

	//---------------------------------------------------------------------
	//ͳ������ƫ��
	int t_nAddUp = m_nStatDelta[100] + m_nStatDelta[101];
	///int t_nAddUp = 0;
	//int sum=0;
	int t_nMaxGrayLevel = 0;	//����ͳ��ֵ
	int t_nMaxGrayPos = 0;		//���ͳ��ֵ��������

	for ( i =102; i < 501; i++ )//�����ֵ
	{
		t_nAddUp += m_nStatDelta[i];

		if ( t_nAddUp > t_nMaxGrayLevel )
		{
			t_nMaxGrayPos = i;
			t_nMaxGrayLevel = t_nAddUp;
		}

		t_nAddUp -= m_nStatDelta[i-2];
	}
	m_UpdateFrameNumber++;
	m_GrayOffset= t_nMaxGrayPos - 255;
	return m_GrayOffset;
}
//t_nChooseBkLevelFlag �����Ƿ�ֻѡ��һ��
int GaussMix::CountFrImage1( IplImage* m_pFrame, IplImage* m_pFrImg, int t_nSetOffsetFlag, int t_nChooseBkLevelFlag)
{
	//---------------------------------------------------------------------
	//��ʼ����ַ���
	int t_nPossition = 0;		//��ǰ��ĵ�ַ�ţ�ָ���һ������
	int t_nPossitionWeight = 0;	//Ȩ�صĵ�ַ�ţ�ָ���һ������
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	//�������
	int i, j, k, l;
	double t_dAddupSigma;		//�����
	double t_dAddupWeight;
	double t_dThreshold;		//������ֵ
	double t_dDelta;			//�ҶȲ�
	int t_nChsBkLevel;
	//---------------------------------------------------------------------
	//i=0;
	//while(m_UpdateLock)
	//{
	//	//Sleep(1);
	//	i++;
	//	if(i>1000)
	//		return -1;
	//}

	//i=0;
	//while(m_AccessLock)
	//{
	//	//Sleep(1);
	//	i++;
	//	if(i>1000)
	//		return -2;
	//}

	//m_AccessLock=true;
	//ѡ�񼸲���м���ǰ��
	if (t_nChooseBkLevelFlag)
	{
		t_nChsBkLevel=1;
	}
	else
		t_nChsBkLevel=m_nGaussians;
	//---------------------------------------------------------------------
	//��ʼ����ǰ��������ǰ��ʱ���������ؽ��м���
	for ( i = 0; i < m_nImageHeight; i++ )
	{
		for ( j = 0; j < m_nImageWidth; j++ )
		{
			t_dAddupWeight = 0;		//Ȩ����0

			for ( k = 0; k < t_nChsBkLevel; k++ )
			{
				t_dAddupSigma = 0;		//�������0
				t_dThreshold = 0;		//������ֵ��0
				if (t_nSetOffsetFlag)
				{
					t_dDelta=fabs((unsigned char)m_pFrame->imageData[t_nPossition] - m_pfBkMean[k][t_nPossition] - m_GrayOffset/*t_nOffset*/) ;	//�����
				}
				else
				{
					t_dDelta=fabs((unsigned char)m_pFrame->imageData[t_nPossition] - m_pfBkMean[k][t_nPossition]);
				}
				
				t_dAddupSigma += t_dDelta * t_dDelta;				//�ۼӷ���
				t_dThreshold += m_pfBkSigma[k][t_nPossition];	//�ۼ���ֵ
				//t_dAddupSigma+=t_dDelta/255;
				//for ( l = 1; l < m_nChannels; l++ )
				//{
				//	t_dDelta=fabs((unsigned char)m_pFrame->imageData[t_nPossition] - m_pfBkMean[k][t_nPossition] - t_nOffset) ;	//�����
				//	t_dAddupSigma+=t_dDelta/255;				//�ۼӷ���
				//	t_dThreshold += m_pfBkSigma[k][t_nPossition + l];	//�ۼ���ֵ
				//}
				for ( l = 1; l < m_nChannels; l++ )
				{
					t_dDelta = m_pfBkMean[k][t_nPossition + l] - (unsigned char)m_pFrame->imageData[t_nPossition + l];	//�����

					t_dAddupSigma += t_dDelta * t_dDelta;				//�ۼӷ���
					t_dThreshold += m_pfBkSigma[k][t_nPossition + l];	//�ۼ���ֵ
				}

				if ( t_dThreshold < 10 )//zcxע��
				{
					t_dThreshold = 10;
				}
				//if ( log(t_dAddupSigma +1)< 0.36 )	//������ϸ÷ֲ�������Ϊ����������ѭ��
				//{
				//	m_pFrImg->imageData[t_nPossitionWeight] = 0;
				//	break;
				//}

				if ( t_dAddupSigma < t_dThreshold * m_dStdThreshold*1.1 )	//������ϸ÷ֲ�������Ϊ����������ѭ��
				{
					m_pFrImg->imageData[t_nPossitionWeight] = 0;
					break;
				}

				t_dAddupWeight += m_pfBkWeight[k][t_nPossitionWeight];

				if ( t_dAddupWeight > m_dBackgroundThreshold )
				{
 					m_pFrImg->imageData[t_nPossitionWeight] = (char)255;
					break;
				}
			}

			t_nPossitionWeight++;
			t_nPossition += m_nChannels;
		}
	}
	//---------------------------------------------------------------------

	return 0;
}
