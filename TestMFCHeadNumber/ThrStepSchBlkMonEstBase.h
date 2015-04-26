//*****************************************************************
// 文件名 :						ThrStepSchBlkMonEstBase.h
// 版本	 :						1.0
// 目的及主要功能 :				三步收索块运动估计算法求光流基类
// 创建日期 :					2014.01.07
// 修改日期 :					无	
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
	//构造函数
	ThrStepSchBlkMonEstBase();
	//析构函数
	~ThrStepSchBlkMonEstBase();
private:
	//大17点
	int array9x9[9][9];
	//小8点
	int array5x5[5][5];
	//正方形8点
	int array3x3Z1[3][3];
	//菱形8点
	int array3x3L1[3][3];
	//去掉的最小区域面积
	static const int MinArea=400;
	//为了算帧差保存的数据
	IplImage* imgA;
	//为了算帧差保存的数据
	IplImage* imgB;	
	//方向熵
	double dirEntropy;
	//幅值最大值
	double maxAmpl;
	//混合高斯定义
	GaussMix *m_gaussMix;
	//图像帧号
	int pictureNub;
	//FILE *saveAreaMaxAver;//保存2个摄像头每个区域最大和平均值
	//void initFile(FILE *saveAreaMaxAver);
	//角度
	vector<double> g_angle;
	//幅度                                                                                                                                                                                                                                                                                                                                      
	vector<double> g_amplitude;

	//Roi区域定义
	bool *m_Mask;
	//实验画幅度直方图
	//幅度直方图统计
	IplImage* histimg_Amplitude;
	//角度直方图统计
	IplImage* histimg_Angle;
	//幅度直方图列个数
	static const  int AmplitudeBinNumbs=40;
	//角度直方图列个数
	static const int AngleBinNumbs=60;
	//存储之前角度直方图数据
	int bins_angle_pre[500];
	//存储之前幅度直方图数据
	int bins_amplitude_pre[500];
	//保存每幅图像的幅值，测试用
	FILE *m_FileAmpli;
	
public:
	//根据两点求返回距离
	double distants(double x1,double y1,double x2,double y2);
	//求返回角度 360度
	double Angle(double x1,double y1,double x2,double y2);
	//点（m,n）是否是外围正方形
	bool is_outRect(int m,int n);
	//16*16区域，y是高度，x是宽度
	bool is_MotionArea(IplImage *src,int x,int y);
	//三步搜索块运动估计
	/*****************************************************************
		函数名字:			ThrStepSchBlk

		输入:
			img_Cur 表示当前图像
			img_FrameSubBin 表示帧差图像，暂时没用到
			img_TargetShow 用于显示的目标图像
		返回值:
			void
		功能描述:      三步搜索块运动估计,计算光流特征。
			 
	*****************************************************************/
	void ThrStepSchBlk(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/);
	//加载roi函数
	void LoadRoi(bool *mask);
	//在新图像上画直方图
	void drawHistogram(IplImage *dst,vector<double> vectordata,int binsNums,int bins[],int min,int max);
	//计算直方图
	void CalcHistogram(vector<double> VectorScr,int BinNum,int bins[],int max,int min);
	//计算方向熵
	double CalcEntropy();
	//得到幅度直方图图像
	IplImage *GetAmplHist();
	//初始化文件指针
	void initFile(FILE *fileampli);
	//得到幅度容器
	vector<double> Getg_amplitude();
};