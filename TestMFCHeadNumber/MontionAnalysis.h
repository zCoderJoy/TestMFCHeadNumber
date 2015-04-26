//*****************************************************************
// �ļ��� :						MontionAnalysis.h
// �汾	 :						1.0
// Ŀ�ļ���Ҫ���� :				����֮ThrStepSchBlkMonEstBase��������
//                              ���˶������㷨,������������ܶ�Ź�ж�
// �������� :					2014.01.07
// �޸����� :					��	
//*****************************************************************/
#pragma once
#include "ThrStepSchBlkMonEstBase.h"
//#ifdef MONTIONANALYSIS_EXPORTS
//#define MOTIONANAL_API __declspec(dllexport)
//#else
//#define MOTIONANAL_API __declspec(dllimport)
//#endif
class /*MOTIONANAL_API*/ MontionAnalysis :public ThrStepSchBlkMonEstBase
{
public:
	MontionAnalysis();
	~MontionAnalysis();

public:
	//������Ƶ�������
	/*****************************************************************
		��������:			BusVideDetectPersonNumb

		����:
			img_Cur ��ʾ��ǰͼ��
			img_FrameSubBin ��ʾ֡��ͼ����ʱû�õ�
			img_TargetShow ������ʾ��Ŀ��ͼ��
		����ֵ:
			void
		��������:      ������Ƶ���������
			 
	*****************************************************************/
	void BusVideDetectPersonNumb(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/);
	//��ܶ�Ź���
	/*****************************************************************
		��������:	FightingDetect

		����:
			img_Cur ��ʾ��ǰͼ��
			img_FrameSubBin ��ʾ֡��ͼ����ʱû�õ�
			img_TargetShow ������ʾ��Ŀ��ͼ��
		����ֵ:
			void
		��������:      ��ܶ�Ź���
			 
	*****************************************************************/
	void FightingDetect(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/);
	//����roi����
	void LoadRoi(bool *mask,int choose);
};