#include "stdafx.h"
#include "MontionAnalysis.h"
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif
MontionAnalysis::MontionAnalysis():ThrStepSchBlkMonEstBase()
{

}
MontionAnalysis::~MontionAnalysis()
{

}
//������Ƶ�������
void MontionAnalysis::BusVideDetectPersonNumb(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/)
{
	ThrStepSchBlk(img_Cur,img_FrameSubBin,img_TargetShow);
}
//��ܶ�Ź���
void MontionAnalysis::FightingDetect(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/)
{
	ThrStepSchBlk(img_Cur,img_FrameSubBin,img_TargetShow);
}
//����roi����
void MontionAnalysis::LoadRoi(bool *mask,int choose)
{
	ThrStepSchBlkMonEstBase::LoadRoi(mask);
}