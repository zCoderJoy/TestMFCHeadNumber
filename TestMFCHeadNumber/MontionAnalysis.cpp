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
//车辆视频检测人数
void MontionAnalysis::BusVideDetectPersonNumb(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/)
{
	ThrStepSchBlk(img_Cur,img_FrameSubBin,img_TargetShow);
}
//打架斗殴检测
void MontionAnalysis::FightingDetect(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/)
{
	ThrStepSchBlk(img_Cur,img_FrameSubBin,img_TargetShow);
}
//加载roi区域
void MontionAnalysis::LoadRoi(bool *mask,int choose)
{
	ThrStepSchBlkMonEstBase::LoadRoi(mask);
}