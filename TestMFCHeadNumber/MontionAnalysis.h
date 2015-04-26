//*****************************************************************
// 文件名 :						MontionAnalysis.h
// 版本	 :						1.0
// 目的及主要功能 :				派生之ThrStepSchBlkMonEstBase三步收索
//                              块运动估计算法,求数人数及打架斗殴判断
// 创建日期 :					2014.01.07
// 修改日期 :					无	
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
	//车辆视频检测人数
	/*****************************************************************
		函数名字:			BusVideDetectPersonNumb

		输入:
			img_Cur 表示当前图像
			img_FrameSubBin 表示帧差图像，暂时没用到
			img_TargetShow 用于显示的目标图像
		返回值:
			void
		功能描述:      车辆视频检测人数。
			 
	*****************************************************************/
	void BusVideDetectPersonNumb(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/);
	//打架斗殴检测
	/*****************************************************************
		函数名字:	FightingDetect

		输入:
			img_Cur 表示当前图像
			img_FrameSubBin 表示帧差图像，暂时没用到
			img_TargetShow 用于显示的目标图像
		返回值:
			void
		功能描述:      打架斗殴检测
			 
	*****************************************************************/
	void FightingDetect(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/);
	//加载roi区域
	void LoadRoi(bool *mask,int choose);
};