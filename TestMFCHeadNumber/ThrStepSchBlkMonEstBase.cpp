#include "stdafx.h"
#include "ThrStepSchBlkMonEstBase.h"

ThrStepSchBlkMonEstBase::ThrStepSchBlkMonEstBase()
{
	//9X9初始化
	array9x9[0][0]=1;array9x9[0][4]=1;array9x9[0][8]=1;array9x9[2][4]=1;
	array9x9[3][3]=1;array9x9[3][5]=1;array9x9[4][0]=1;array9x9[4][2]=1;
	array9x9[4][4]=1;array9x9[4][6]=1;array9x9[4][8]=1;array9x9[5][3]=1;
	array9x9[5][5]=1;array9x9[6][4]=1;array9x9[8][0]=1;array9x9[8][4]=1;
	array9x9[8][8]=1;
	//5x5正方形
	array5x5[0][0]=1;array5x5[0][2]=1;array5x5[0][4]=1;array5x5[2][0]=1;
	array5x5[2][4]=1;array5x5[4][0]=1;array5x5[4][2]=1;array5x5[4][4]=1;
	//3x3菱形
	array3x3L1[0][1]=1;array3x3L1[1][0]=1;array3x3L1[1][2]=1;array3x3L1[2][1]=1;
	//3x3正方形
	array3x3Z1[0][0]=1;array3x3Z1[1][0]=1;array3x3Z1[1][2]=1;array3x3Z1[2][1]=1;
	array3x3Z1[2][2]=1;array3x3Z1[0][1]=1;array3x3Z1[0][2]=1;array3x3Z1[2][0]=1;

	imgA=NULL;
	imgB=NULL;
	m_gaussMix=new GaussMix();
	m_Mask=NULL;
	histimg_Amplitude = cvCreateImage( cvSize(640,300), 8, 3 );//幅度直方图统计
	histimg_Angle = cvCreateImage( cvSize(640,300), 8, 3 );//角度直方图统计
	//	histimg_Angle_sub = cvCreateImage( cvSize(320,200), 8, 3 );//角度直方图统计
	//	histimg_Amplitude_sub = cvCreateImage( cvSize(320,200), 8, 3 );//角度直方图统计
	memset(bins_angle_pre,0,sizeof(bins_angle_pre)/*/sizeof(int)*/);//存储之前角度直方图数据
	memset(bins_amplitude_pre,0,sizeof(bins_amplitude_pre)/*/sizeof(int)*/);//存储之前幅度直方图数据
	cvZero( histimg_Amplitude );
	cvZero( histimg_Angle );
}
ThrStepSchBlkMonEstBase::~ThrStepSchBlkMonEstBase()
{
	delete m_gaussMix;
}
vector<double> ThrStepSchBlkMonEstBase::Getg_amplitude()//幅度
{
	return g_amplitude;
}
void ThrStepSchBlkMonEstBase::LoadRoi(bool *mask)
{
	m_Mask=mask;
}
double ThrStepSchBlkMonEstBase::distants(double x1,double y1,double x2,double y2)//根据两点求返回距离
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
double ThrStepSchBlkMonEstBase::Angle(double x1,double y1,double x2,double y2)//求返回角度 360度
{
	if (x1-x2==0&&y2-y1==0)
	{
		return 0;
	}
	double t_angle = atan2(y2-y1,x2-x1);
	if (t_angle>0)
	{
		return t_angle*180/3.141592654;
	}
	else
		return atan2(y2-y1,x2-x1)*180/3.141592654+360;


}
bool ThrStepSchBlkMonEstBase::is_outRect(int m,int n)//是否是外围正方形
{
	return (m==-4&&n==-4)||(m==-4&&n==0)||(m==-4&&n==4)||(m==0&&n==-4)||(m==0&&n==4)||(m==4&&n==-4)||(m==4&&n==0)||(m==4&&n==4);
}
bool ThrStepSchBlkMonEstBase::is_MotionArea(IplImage *src,int x,int y)//16*16区域，y是高度，x是宽度
{
	int sum=0;
	int t_nWidth = src->width;
	for (int i=-7;i<=8;i++)
	{
		for (int j=-7;j<=8;j++)
		{
			if (src->imageData[(y+i)*t_nWidth+j+x])
			{
				sum++;
			}
		}
	}
	if (sum>100)
	{
		return true;
	}
	return false;
}
//img_TargetShow是已经分配空间的
void ThrStepSchBlkMonEstBase::ThrStepSchBlk(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*展示目标图像数据*/)
{
	//释放空间
	if (g_amplitude.size()>0)
	{
		g_amplitude.clear();
		//a=g_amplitude.capacity();
		vector<double>(g_amplitude).swap(g_amplitude);
	}
	if (g_angle.size()>0)
	{
		g_angle.clear();
		//int a=g_angle.capacity();
		vector<double>(g_angle).swap(g_angle);
	}
	
	//求运动矢量
	CvSize img_sz= cvGetSize( img_Cur );
	IplImage* imgTemp = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
	cvSaveImage("zcx.bmp",img_Cur);
	int s=clock();
	//帧差处理
	if (imgA==NULL)
	{
		//imgA=cvCloneImage(img_Cur);
		imgA=cvCreateImage(img_sz,8,1);
		cvCvtColor(img_Cur,imgA,CV_RGB2GRAY);
	}
	if (imgB==NULL)
	{
		//imgA=cvCloneImage(img_Cur);
		imgB=cvCreateImage(img_sz,8,1);
		//cvCvtColor(img_Cur,imgA,CV_RGB2GRAY);
	}
	cvCvtColor(img_Cur,imgB,CV_RGB2GRAY);//转化为灰度图像
	cvAbsDiff(imgA,imgB,imgTemp);
	cvThreshold(imgTemp,imgTemp,25,255,CV_THRESH_BINARY);

	//找出比较大运动目标
	IplConvKernel *core = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_RECT);//创建卷积核，参考点位于中心，坐标从0开始。
	cvDilate(imgTemp,imgTemp,core,1);//膨胀
	cvErode(imgTemp,imgTemp,core,1);
	cvDilate(imgTemp,imgTemp,core,2);//膨胀扩大目标
	//找出前景物体的轮廓
	CvMemStorage* storage = cvCreateMemStorage(0);//存放轮廓的容器
	CvSeq* firstcontour = 0;//标示第一个轮廓指针
	int contour_num = cvFindContours(imgTemp, storage, &firstcontour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	CvScalar color = CV_RGB( 0, 255, 255 );

	cvReleaseStructuringElement(&core);

	int sum=0,l_min=10000000,minM=-6,minN=-6;
	int t_QuitCount=0;//退出计数
	for (int i=16;i<img_sz.height-16;i+=16)//块16*16共44x36块
	{
		for (int j=16;j<img_sz.width-16;j+=16)
		{
			if(m_Mask)
				if (!m_Mask[i*img_sz.width+j])
				{
					continue;
				}
			t_QuitCount=0;
			if (!is_MotionArea(imgTemp,j,i))//判断是否是前景区域
			{
				continue;
			}
			minM=-6,minN=-6,l_min=10000000;
			for (int m=-4;m<5;m++)//17个点
			{
				for (int n=-4;n<5;n++)
				{

					if (array9x9[m+4][n+4])
					{
						sum=0;
						for (int l=-7;l<=8;l++)//相减矩阵 求16x16矩阵差值
						{
							for (int k=-7;k<=8;k++)
							{
								sum+=abs(int(imgA->imageData[(i+m+l)*img_sz.width+j+n+k])-int(imgB->imageData[(i+l)*img_sz.width+j+k]));
							}
						}
						if (sum<l_min||(sum==l_min&&m==0&&n==0))
						{
							minM=m;
							minN=n;
							l_min=sum;

						}
					}
				}

			}
			//如果为17个点中心，说明无运动，则继续求下一组数据
			if ((minM==0&&minN==0)||l_min==10000000)
			{
				continue;
			}
			else 
				if (is_outRect(minM,minN))//如果为外围正方形点
				{   
					t_QuitCount++;
					int t_minM=-6,t_minN=-6,t_min=10000000;
					for (int m=-2;m<3;m+=2)//8个点 暂时求8个
					{
						for (int n=-2;n<3;n+=2)
						{

							if (array5x5[m+2][n+2])
							{
								sum=0;
								for (int l=-7;l<=8;l++)//相减矩阵 求16x16矩阵差值
								{
									for (int k=-7;k<=8;k++)
									{
										sum+=abs(int(imgA->imageData[(i+minM+m+l)*img_sz.width+j+minN+n+k])-int(imgB->imageData[(i+l)*img_sz.width+j+k]));
									}
								}
								if (sum<l_min)
								{
									t_minM=m;
									t_minN=n;
									l_min=sum;

								}
							}
						}
					}
					//如果为中心，无递归
					if (t_minM==-6&&t_minN==-6)
					{
						CvPoint p0 = cvPoint(
							cvRound(j ),
							cvRound( i )
							);
						CvPoint p1 = cvPoint(
							cvRound(j+minN  ),
							cvRound(i+minM  )
							);
						double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//求距离
						g_amplitude.push_back(t_amplitude);
						double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//求角度，注意坐标表示
						//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//求角度
						g_angle.push_back(t_angle);
						cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0代表目标图像像素，所以在p0画圆点表箭头
						cvLine( img_TargetShow, p1, p0, CV_RGB(255,0,0),2 ,CV_AA);
					}
					else
					{

						// //递归求3x3小正方形
						int flagz=1;
						while (flagz)
						{
							t_QuitCount++;
							int tt_minM=-6,tt_minN=-6,tt_min=10000000;
							for (int m=-1;m<2;m++)//8个点 暂时求8个
							{
								for (int n=-1;n<2;n++)
								{

									if (array3x3Z1[m+1][n+1])
									{
										sum=0;
										for (int l=-7;l<=8;l++)//相减矩阵 求16x16矩阵差值
										{
											for (int k=-7;k<=8;k++)
											{
												sum+=abs(int(imgA->imageData[(i+minM+t_minM+m+l)*img_sz.width+j+minN+t_minN+n+k])-int(imgB->imageData[(i+l)*img_sz.width+j+k]));
											}
										}
										if (sum<l_min)
										{
											tt_minM=m;
											tt_minN=n;
											l_min=sum;

										}
									}
								}
							}
							if ((tt_minM==-6&&tt_minN==-6)||t_QuitCount==10)
							{
								CvPoint p0 = cvPoint(
									cvRound(j ),
									cvRound( i )
									);
								CvPoint p1 = cvPoint(
									cvRound( j+minN+t_minN ),
									cvRound( i+minM+t_minM )
									);
								double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//求距离
								g_amplitude.push_back(t_amplitude);
								double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//求角度，注意坐标表示
								//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//求角度
								g_angle.push_back(t_angle);
								cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0代表目标图像像素，所以在p0画圆点表箭头
								cvLine( img_TargetShow, p1, p0, CV_RGB(255,0,0),2 ,CV_AA);
								flagz=0;
							}
							else
							{
								t_minM+=tt_minM;
								t_minN+=tt_minN;
							}				

						}
					}
				}//最外层大正方形
				else//如果为菱形
				{
					int flagz=1;
					while (flagz)
					{
						t_QuitCount++;
						int t_minM=-6,t_minN=-6,t_min=10000000;
						for (int m=-1;m<2;m++)
						{
							for (int n=-1;n<2;n++)
							{
								if (array3x3L1[m+1][n+1])
								{
									sum=0;
									for (int l=-7;l<=8;l++)//相减矩阵 求16x16矩阵差值
									{
										for (int k=-7;k<=8;k++)
										{
											sum+=abs(int(imgA->imageData[(i+m+minM+l)*img_sz.width+j+minN+n+k])-int(imgB->imageData[(i+l)*img_sz.width+j+k]));
										}
									}
									if (sum<l_min)
									{
										t_minM=m;
										t_minN=n;
										l_min=sum;

									}
								}
							}
						}
						//如果为中心，无递归
						if ((t_minM==-6&&t_minN==-6)||t_QuitCount==10)
						{
							CvPoint p0 = cvPoint(
								cvRound(j ),
								cvRound( i )
								);
							CvPoint p1 = cvPoint(
								cvRound( j+minN ),
								cvRound( i+minM )
								);
							double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//求距离
							g_amplitude.push_back(t_amplitude);
							double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//求角度，注意坐标表示
							//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//求角度
							g_angle.push_back(t_angle);
							cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0代表目标图像像素，所以在p0画圆点表箭头
							cvLine( img_TargetShow, p1, p0, CV_RGB(255,0,0),2 ,CV_AA);//p1->p0
							flagz=0;
						}
						else //暂时保存，需修改
						{
							
							minM+=t_minM;
							minN+=t_minN;
						}

					}

				}//菱形
		}//双重for循环
	}
	//保存幅值到文件,后面统计分析幅值和人关系
	for (int i=0;i<g_amplitude.size();i++)
	{
		char str[20];//保存数据
		memset(str,0,20);
		sprintf(str,"%4f",g_amplitude[i]);
		//fputc(',',saveMaxAmpl[0]);	
		fwrite(str,sizeof(str),1,m_FileAmpli);
		fputc(',',m_FileAmpli);
	}
	fputc('\n',m_FileAmpli);
	//画幅度直方图
	cvZero(histimg_Angle);
	cvZero(histimg_Amplitude);
	int bins_amplitude[500]={0};	
	drawHistogram(histimg_Amplitude,g_amplitude,AmplitudeBinNumbs,bins_amplitude,0,12);
	//画角度直方图
	int bins_angle[500]={0};
	drawHistogram(histimg_Angle,g_angle,AngleBinNumbs,bins_angle,0,360);
	//标示图例
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 0.5f,0.5f);
	cvPutText(histimg_Amplitude,("width length:1pixel    total: 40 histograms "),cvPoint(200,50),&font,CV_RGB(255,0,0));
	
	//画相减幅度直方图
	//DrawSubHistogram(histimg_Amplitude_sub,AmplitudeBinNumbs,bins_amplitude,bins_amplitude_pre);
	//画相减角度直方图
	//DrawSubHistogram(histimg_Angle_sub,AngleBinNumbs,bins_angle,bins_angle_pre);
	int e=clock();
	//计算熵
	CalcEntropy();
	//std::cout<<e-s<<"\n";
	//打架斗殴判断
	//if (g_angle.size()>30)
	//{
	//	if (isFighting(g_amplitude,g_angle,bins_amplitude,bins_angle))
	//	{
	//		//cout<<"正在处理第 "<< ii<<"帧；\n";
	//		cout<<"警告打架斗殴！！！！！";
	//	}


	//}
	//画外围轮廓
	//for( ; firstcontour != 0; firstcontour = firstcontour->h_next )  //h_next为绿色；v_next为蓝色
	//{  

	//	double tmparea = fabs(cvContourArea(firstcontour));
	//	if(tmparea < MinArea)   
	//	{  
	//		cvSeqRemove(firstcontour, 0); // 删除面积小于设定值的轮廓
	//		continue;
	//	}  
	//	//CvRect aRect = cvBoundingRect( firstcontour, 0 ); 
	//	//if ((aRect.width/aRect.height)<1)  
	//	//{  
	//	//	cvSeqRemove(contour, 0); //删除宽高比例小于设定值的轮廓
	//	//	continue;
	//	//}  

	//	// 创建一个色彩值
	//	CvScalar color = CV_RGB( 0, 255, 255 );

	//	//max_level 绘制轮廓的最大等级。如果等级为0，绘制单独的轮廓。如果为1，绘制轮廓及在其后的相同的级别下轮廓
	//	//如果值为2，所有的轮廓。如果等级为2，绘制所有同级轮廓及所有低一级轮廓，诸此种种
	//	//如果值为负数，函数不绘制同级轮廓，但会升序绘制直到级别为abs(max_level)-1的子轮廓
	//	cvDrawContours(img_TargetShow, firstcontour, color, color, -1, 1, 8);	//绘制外部和内部的轮廓
	//}  

	/*cvZero(histimg_Angle);
	cvZero(histimg_Amplitude);
	cvZero(histimg_Angle_sub);
	cvZero(histimg_Amplitude_sub);*/
	swap(imgA,imgB);
	memcpy(bins_angle_pre,bins_angle,sizeof(bins_angle));//重置之前直方图
	memcpy(bins_amplitude_pre,bins_amplitude,sizeof(bins_amplitude));
	//swap(g_amplitude_pre,g_amplitude);
	//swap(g_angle_pre,g_angle);
	//g_angle.clear();
	//int a=g_angle.capacity();
	//vector<double>(g_angle).swap(g_angle);
	//a=g_amplitude.capacity();
	
	cvReleaseImage(&imgB);
	cvReleaseImage(&imgTemp);
	//cvReleaseImage(&imgC);
}

void ThrStepSchBlkMonEstBase::drawHistogram(IplImage *dst,vector<double> vectordata,int binsNums,int bins[],int min,int max)
{
	CalcHistogram(vectordata,binsNums,bins,max,min);
	int binl_w = dst->width/binsNums;
	if (vectordata.size()==0)
	{
		return;
	}
	for( int i = 0; i < binsNums; i++ )
	{
		double val = ( double(bins[i])*dst->height/vectordata.size()/*double(ArrayMax(bins_amplitude,binNum_amplitude) )*/);//得到一维矩阵第i的高度
		CvScalar color = CV_RGB(255,255,0); //(hsv2rgb(i*180.f/hdims);
		cvRectangle( dst, cvPoint(i*binl_w,dst->height),
			cvPoint((i+1)*binl_w,(int)(dst->height - val)),//在目标图像上画矩形，左上到右下
			color, 1, 8, 0 );
	}
	return;
}
void ThrStepSchBlkMonEstBase::CalcHistogram(vector<double> VectorScr,int BinNum,int bins[],int max,int min)//计算直方图2
{

	double step = (max-min)/double(BinNum);
	for (unsigned int i=0;i<VectorScr.size();i++)//统计每个块大小
	{
		int pos = (VectorScr[i]-min)/step;
		bins[pos]++;
	}


}
//计算方向熵
double ThrStepSchBlkMonEstBase::CalcEntropy()
{
	//统计方向角度的总和
	double sum=0.0;
	dirEntropy=0.0;
	for (int i=0;i<AngleBinNumbs;i++)
	{
		sum+=bins_angle_pre[i];
	}
	if (sum)
	{
		for (int i=0;i<AngleBinNumbs;i++)
		{
			double t=bins_angle_pre[i]/sum;
			if (t)
			{
				dirEntropy+=-(t)*log(t)/log(2.0);
			}
		}
	}
	return 0;
}
IplImage * ThrStepSchBlkMonEstBase::GetAmplHist(){return histimg_Amplitude;};
void ThrStepSchBlkMonEstBase::initFile(FILE *fileampli)
{
	m_FileAmpli=fileampli;
}