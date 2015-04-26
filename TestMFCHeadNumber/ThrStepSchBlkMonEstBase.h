//*****************************************************************
// �ļ��� :						ThrStepSchBlkMonEstBase.h
// �汾	 :						1.0
// Ŀ�ļ���Ҫ���� :				�����������˶������㷨���������
// �������� :					2014.01.07
// �޸����� :					��	
//*****************************************************************/
#pragma once
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <vector>
#include "GaussMixModified.h"
//#ifdef MONTIONANALYSIS_EXPORTS
//#define MOTIONANAL_API __declspec(dllexport)
//#else
//#define MOTIONANAL_API __declspec(dllimport)
//#endif
class /*MOTIONANAL_API*/ ThrStepSchBlkMonEstBase
{
public:
	//���캯��
	ThrStepSchBlkMonEstBase();
	//��������
	~ThrStepSchBlkMonEstBase();
private:
	//��17��
	int array9x9[9][9];
	//С8��
	int array5x5[5][5];
	//������8��
	int array3x3Z1[3][3];
	//����8��
	int array3x3L1[3][3];
	//ȥ������С�������
	static const int MinArea=400;
	//Ϊ����֡��������
	IplImage* imgA;
	//Ϊ����֡��������
	IplImage* imgB;	
	//������
	double dirEntropy;
	//��ֵ���ֵ
	double maxAmpl;
	//��ϸ�˹����
	GaussMix *m_gaussMix;
	//ͼ��֡��
	int pictureNub;
	//FILE *saveAreaMaxAver;//����2������ͷÿ����������ƽ��ֵ
	//void initFile(FILE *saveAreaMaxAver);
	//�Ƕ�
	vector<double> g_angle;
	//����                                                                                                                                                                                                                                                                                                                                      
	vector<double> g_amplitude;

	//Roi������
	bool *m_Mask;
	//ʵ�黭����ֱ��ͼ
	//����ֱ��ͼͳ��
	IplImage* histimg_Amplitude;
	//�Ƕ�ֱ��ͼͳ��
	IplImage* histimg_Angle;
	//����ֱ��ͼ�и���
	static const  int AmplitudeBinNumbs=40;
	//�Ƕ�ֱ��ͼ�и���
	static const int AngleBinNumbs=60;
	//�洢֮ǰ�Ƕ�ֱ��ͼ����
	int bins_angle_pre[500];
	//�洢֮ǰ����ֱ��ͼ����
	int bins_amplitude_pre[500];
	//����ÿ��ͼ��ķ�ֵ��������
	FILE *m_FileAmpli;
	
public:
	//���������󷵻ؾ���
	double distants(double x1,double y1,double x2,double y2);
	//�󷵻ؽǶ� 360��
	double Angle(double x1,double y1,double x2,double y2);
	//�㣨m,n���Ƿ�����Χ������
	bool is_outRect(int m,int n);
	//16*16����y�Ǹ߶ȣ�x�ǿ��
	bool is_MotionArea(IplImage *src,int x,int y);
	//�����������˶�����
	/*****************************************************************
		��������:			ThrStepSchBlk

		����:
			img_Cur ��ʾ��ǰͼ��
			img_FrameSubBin ��ʾ֡��ͼ����ʱû�õ�
			img_TargetShow ������ʾ��Ŀ��ͼ��
		����ֵ:
			void
		��������:      �����������˶�����,�������������
			 
	*****************************************************************/
	void ThrStepSchBlk(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/);
	//����roi����
	void LoadRoi(bool *mask);
	//����ͼ���ϻ�ֱ��ͼ
	void drawHistogram(IplImage *dst,vector<double> vectordata,int binsNums,int bins[],int min,int max);
	//����ֱ��ͼ
	void CalcHistogram(vector<double> VectorScr,int BinNum,int bins[],int max,int min);
	//���㷽����
	double CalcEntropy();
	//�õ�����ֱ��ͼͼ��
	IplImage *GetAmplHist();
	//��ʼ���ļ�ָ��
	void initFile(FILE *fileampli);
	//�õ���������
	vector<double> Getg_amplitude();
};