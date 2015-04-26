//zcx  �޸Ľ�����
//20150303
#pragma once

#include "opencv2/highgui//highgui.hpp"
#include "opencv2/core/core.hpp"
#include <vector>
using namespace std;
//#ifdef MONTIONANALYSIS_EXPORTS
//#define MOTIONANAL_API __declspec(dllexport)
//#else
//#define MOTIONANAL_API __declspec(dllimport)
//#endif
class /*MOTIONANAL_API*/  GaussMix
{
public:
	GaussMix(void);
	GaussMix(double StdThreshold,	double BackgroundThreshold, double UpdataRateWeight,	
		double UpdataRateMean, double UpdataRateSigma, double WeightInit, int Gaussians,  int RefreshPercent);
	~GaussMix(void);


//���к���
public:
	int Init( IplImage* m_pFrame/*, IplImage* m_pBkImg, IplImage* m_pFrImg */);		//��ʼ������
	void SetParas(double StdThreshold,	double BackgroundThreshold, double UpdataRateWeight,	
		double UpdataRateMean, double UpdataRateSigma, double WeightInit, int Gaussians,  int RefreshPercent);
	//int UpdateRGB( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber );		//���±���
	//int UpdateYUV( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber );
	//int UpdateHSV( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber );
	//int UpdateGray( IplImage* m_pFrame, IplImage* m_pBkImg, IplImage* m_pFrImg, int m_dCurrentFrameNumber );
public:
	int Update( IplImage* m_pFrame,int choose_way=2);
	//��ʾ���������
	int Update1( IplImage* m_pFrame/*, IplImage* m_pBkImg, int m_dCurrentFrameNumber*/ );	//���ɱ��������������ָ��·�����״����
	//��ʾ�������и���
	int Update2( IplImage* m_pFrame);
	//������������int t_nSetOffsetFlag=1, int t_nChooseBkLevelFlag=0
	//t_nSetOffsetFlag��ʾ�Ƿ�����ƫ������t_nChooseBkLevelFlag�������ǰ��ʱҪ��Ҫֻ���ǵ�һ���˹
	//Ĭ�����������ƫ����������ȫ����˹����
	int CountFrImage1( IplImage* m_pFrame, IplImage* m_pFrImg, int t_nSetOffsetFlag=1, int t_nChooseBkLevelFlag=0);	//����ǰ��ͼ��
//��˹ģ�ͻ��������趨
private:
	double m_dStdThreshold;				//�жϷ��ϵ�ǰ��ģ�͵ķ���ı��� /* lambda=2.5 is 99% */
	double m_dBackgroundThreshold;		//�ж�Ϊǰ���Ľ��ձ��� /* threshold sum of weights for background test */
	double m_dUpdataRateWeight;				//����Ȩ�صı���
	double m_dUpdataRateWeight2;	//Ϊ������ٶȣ������ظ������������
	double m_dUpdataRateMean;
	double m_dUpdataRateSigma;
	double m_dUpdataRateSigma2;
	double m_dWeightInit;				//��ʼȨ��
	int m_nGaussians;					//��˹ģ�͵Ĳ���
	double m_nSigmaInit;					//��ʼ����
	int m_nRefreshPercent;				//ÿ֡���µı���

	//zcx ���ƽ�ά����
	int m_nRectWith;//����С���ο�
	int m_nRectHeight;//����С���θ�

//˽�к���
public:	

	IplImage *m_BkgImage;//����ͼ
//˽��������
private:
	int m_nImageWidth;			//ͼ����
	int m_nImageHeight;			//ͼ��߶�
	int m_nPixNumber;			//ͼ��������Ŀ
	int m_nDataSize;			//ͼ�����ݴ�С
	int m_nLineExpand;			//ͼ��ÿ�к������
	int m_nLineSize;			//ͼ��ÿ�е��ֽ���
	int m_nChannels;			//ͨ����

	bool m_AccessLock;
	bool m_UpdateLock;

	vector <double *> m_pfBkMean;		//����ͼ���ֵ
	
	vector <double *> m_pfBkWeight;		//����ͼ��Ȩֵ

	vector <double *> m_pfBkSigma;		//����ͼ�񷽲�

	unsigned int m_UpdateFrameNumber;
	int m_GrayOffset;
};
