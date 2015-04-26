#include "stdafx.h"
#include "ThrStepSchBlkMonEstBase.h"

ThrStepSchBlkMonEstBase::ThrStepSchBlkMonEstBase()
{
	//9X9��ʼ��
	array9x9[0][0]=1;array9x9[0][4]=1;array9x9[0][8]=1;array9x9[2][4]=1;
	array9x9[3][3]=1;array9x9[3][5]=1;array9x9[4][0]=1;array9x9[4][2]=1;
	array9x9[4][4]=1;array9x9[4][6]=1;array9x9[4][8]=1;array9x9[5][3]=1;
	array9x9[5][5]=1;array9x9[6][4]=1;array9x9[8][0]=1;array9x9[8][4]=1;
	array9x9[8][8]=1;
	//5x5������
	array5x5[0][0]=1;array5x5[0][2]=1;array5x5[0][4]=1;array5x5[2][0]=1;
	array5x5[2][4]=1;array5x5[4][0]=1;array5x5[4][2]=1;array5x5[4][4]=1;
	//3x3����
	array3x3L1[0][1]=1;array3x3L1[1][0]=1;array3x3L1[1][2]=1;array3x3L1[2][1]=1;
	//3x3������
	array3x3Z1[0][0]=1;array3x3Z1[1][0]=1;array3x3Z1[1][2]=1;array3x3Z1[2][1]=1;
	array3x3Z1[2][2]=1;array3x3Z1[0][1]=1;array3x3Z1[0][2]=1;array3x3Z1[2][0]=1;

	imgA=NULL;
	imgB=NULL;
	m_gaussMix=new GaussMix();
	m_Mask=NULL;
	histimg_Amplitude = cvCreateImage( cvSize(640,300), 8, 3 );//����ֱ��ͼͳ��
	histimg_Angle = cvCreateImage( cvSize(640,300), 8, 3 );//�Ƕ�ֱ��ͼͳ��
	//	histimg_Angle_sub = cvCreateImage( cvSize(320,200), 8, 3 );//�Ƕ�ֱ��ͼͳ��
	//	histimg_Amplitude_sub = cvCreateImage( cvSize(320,200), 8, 3 );//�Ƕ�ֱ��ͼͳ��
	memset(bins_angle_pre,0,sizeof(bins_angle_pre)/*/sizeof(int)*/);//�洢֮ǰ�Ƕ�ֱ��ͼ����
	memset(bins_amplitude_pre,0,sizeof(bins_amplitude_pre)/*/sizeof(int)*/);//�洢֮ǰ����ֱ��ͼ����
	cvZero( histimg_Amplitude );
	cvZero( histimg_Angle );
}
ThrStepSchBlkMonEstBase::~ThrStepSchBlkMonEstBase()
{
	delete m_gaussMix;
}
vector<double> ThrStepSchBlkMonEstBase::Getg_amplitude()//����
{
	return g_amplitude;
}
void ThrStepSchBlkMonEstBase::LoadRoi(bool *mask)
{
	m_Mask=mask;
}
double ThrStepSchBlkMonEstBase::distants(double x1,double y1,double x2,double y2)//���������󷵻ؾ���
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}
double ThrStepSchBlkMonEstBase::Angle(double x1,double y1,double x2,double y2)//�󷵻ؽǶ� 360��
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
bool ThrStepSchBlkMonEstBase::is_outRect(int m,int n)//�Ƿ�����Χ������
{
	return (m==-4&&n==-4)||(m==-4&&n==0)||(m==-4&&n==4)||(m==0&&n==-4)||(m==0&&n==4)||(m==4&&n==-4)||(m==4&&n==0)||(m==4&&n==4);
}
bool ThrStepSchBlkMonEstBase::is_MotionArea(IplImage *src,int x,int y)//16*16����y�Ǹ߶ȣ�x�ǿ��
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
//img_TargetShow���Ѿ�����ռ��
void ThrStepSchBlkMonEstBase::ThrStepSchBlk(IplImage *img_Cur,IplImage *img_FrameSubBin,IplImage *img_TargetShow/*չʾĿ��ͼ������*/)
{
	//�ͷſռ�
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
	
	//���˶�ʸ��
	CvSize img_sz= cvGetSize( img_Cur );
	IplImage* imgTemp = cvCreateImage( img_sz, IPL_DEPTH_8U, 1 );
	cvSaveImage("zcx.bmp",img_Cur);
	int s=clock();
	//֡���
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
	cvCvtColor(img_Cur,imgB,CV_RGB2GRAY);//ת��Ϊ�Ҷ�ͼ��
	cvAbsDiff(imgA,imgB,imgTemp);
	cvThreshold(imgTemp,imgTemp,25,255,CV_THRESH_BINARY);

	//�ҳ��Ƚϴ��˶�Ŀ��
	IplConvKernel *core = cvCreateStructuringElementEx(5,5,2,2,CV_SHAPE_RECT);//��������ˣ��ο���λ�����ģ������0��ʼ��
	cvDilate(imgTemp,imgTemp,core,1);//����
	cvErode(imgTemp,imgTemp,core,1);
	cvDilate(imgTemp,imgTemp,core,2);//��������Ŀ��
	//�ҳ�ǰ�����������
	CvMemStorage* storage = cvCreateMemStorage(0);//�������������
	CvSeq* firstcontour = 0;//��ʾ��һ������ָ��
	int contour_num = cvFindContours(imgTemp, storage, &firstcontour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	CvScalar color = CV_RGB( 0, 255, 255 );

	cvReleaseStructuringElement(&core);

	int sum=0,l_min=10000000,minM=-6,minN=-6;
	int t_QuitCount=0;//�˳�����
	for (int i=16;i<img_sz.height-16;i+=16)//��16*16��44x36��
	{
		for (int j=16;j<img_sz.width-16;j+=16)
		{
			if(m_Mask)
				if (!m_Mask[i*img_sz.width+j])
				{
					continue;
				}
			t_QuitCount=0;
			if (!is_MotionArea(imgTemp,j,i))//�ж��Ƿ���ǰ������
			{
				continue;
			}
			minM=-6,minN=-6,l_min=10000000;
			for (int m=-4;m<5;m++)//17����
			{
				for (int n=-4;n<5;n++)
				{

					if (array9x9[m+4][n+4])
					{
						sum=0;
						for (int l=-7;l<=8;l++)//������� ��16x16�����ֵ
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
			//���Ϊ17�������ģ�˵�����˶������������һ������
			if ((minM==0&&minN==0)||l_min==10000000)
			{
				continue;
			}
			else 
				if (is_outRect(minM,minN))//���Ϊ��Χ�����ε�
				{   
					t_QuitCount++;
					int t_minM=-6,t_minN=-6,t_min=10000000;
					for (int m=-2;m<3;m+=2)//8���� ��ʱ��8��
					{
						for (int n=-2;n<3;n+=2)
						{

							if (array5x5[m+2][n+2])
							{
								sum=0;
								for (int l=-7;l<=8;l++)//������� ��16x16�����ֵ
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
					//���Ϊ���ģ��޵ݹ�
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
						double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//�����
						g_amplitude.push_back(t_amplitude);
						double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//��Ƕȣ�ע�������ʾ
						//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//��Ƕ�
						g_angle.push_back(t_angle);
						cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0����Ŀ��ͼ�����أ�������p0��Բ����ͷ
						cvLine( img_TargetShow, p1, p0, CV_RGB(255,0,0),2 ,CV_AA);
					}
					else
					{

						// //�ݹ���3x3С������
						int flagz=1;
						while (flagz)
						{
							t_QuitCount++;
							int tt_minM=-6,tt_minN=-6,tt_min=10000000;
							for (int m=-1;m<2;m++)//8���� ��ʱ��8��
							{
								for (int n=-1;n<2;n++)
								{

									if (array3x3Z1[m+1][n+1])
									{
										sum=0;
										for (int l=-7;l<=8;l++)//������� ��16x16�����ֵ
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
								double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//�����
								g_amplitude.push_back(t_amplitude);
								double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//��Ƕȣ�ע�������ʾ
								//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//��Ƕ�
								g_angle.push_back(t_angle);
								cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0����Ŀ��ͼ�����أ�������p0��Բ����ͷ
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
				}//������������
				else//���Ϊ����
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
									for (int l=-7;l<=8;l++)//������� ��16x16�����ֵ
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
						//���Ϊ���ģ��޵ݹ�
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
							double t_amplitude = distants(p0.x,p0.y,p1.x,p1.y);//�����
							g_amplitude.push_back(t_amplitude);
							double t_angle = Angle(p1.x,-p1.y,p0.x,-p0.y);//��Ƕȣ�ע�������ʾ
							//double t_angle = Angle(p0.x,p0.y,p1.x,p1.y);//��Ƕ�
							g_angle.push_back(t_angle);
							cvCircle(img_TargetShow,p0,3,CV_RGB(0,0,0),-1);//p0����Ŀ��ͼ�����أ�������p0��Բ����ͷ
							cvLine( img_TargetShow, p1, p0, CV_RGB(255,0,0),2 ,CV_AA);//p1->p0
							flagz=0;
						}
						else //��ʱ���棬���޸�
						{
							
							minM+=t_minM;
							minN+=t_minN;
						}

					}

				}//����
		}//˫��forѭ��
	}
	//�����ֵ���ļ�,����ͳ�Ʒ�����ֵ���˹�ϵ
	for (int i=0;i<g_amplitude.size();i++)
	{
		char str[20];//��������
		memset(str,0,20);
		sprintf(str,"%4f",g_amplitude[i]);
		//fputc(',',saveMaxAmpl[0]);	
		fwrite(str,sizeof(str),1,m_FileAmpli);
		fputc(',',m_FileAmpli);
	}
	fputc('\n',m_FileAmpli);
	//������ֱ��ͼ
	cvZero(histimg_Angle);
	cvZero(histimg_Amplitude);
	int bins_amplitude[500]={0};	
	drawHistogram(histimg_Amplitude,g_amplitude,AmplitudeBinNumbs,bins_amplitude,0,12);
	//���Ƕ�ֱ��ͼ
	int bins_angle[500]={0};
	drawHistogram(histimg_Angle,g_angle,AngleBinNumbs,bins_angle,0,360);
	//��ʾͼ��
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, 0.5f,0.5f);
	cvPutText(histimg_Amplitude,("width length:1pixel    total: 40 histograms "),cvPoint(200,50),&font,CV_RGB(255,0,0));
	
	//���������ֱ��ͼ
	//DrawSubHistogram(histimg_Amplitude_sub,AmplitudeBinNumbs,bins_amplitude,bins_amplitude_pre);
	//������Ƕ�ֱ��ͼ
	//DrawSubHistogram(histimg_Angle_sub,AngleBinNumbs,bins_angle,bins_angle_pre);
	int e=clock();
	//������
	CalcEntropy();
	//std::cout<<e-s<<"\n";
	//��ܶ�Ź�ж�
	//if (g_angle.size()>30)
	//{
	//	if (isFighting(g_amplitude,g_angle,bins_amplitude,bins_angle))
	//	{
	//		//cout<<"���ڴ���� "<< ii<<"֡��\n";
	//		cout<<"�����ܶ�Ź����������";
	//	}


	//}
	//����Χ����
	//for( ; firstcontour != 0; firstcontour = firstcontour->h_next )  //h_nextΪ��ɫ��v_nextΪ��ɫ
	//{  

	//	double tmparea = fabs(cvContourArea(firstcontour));
	//	if(tmparea < MinArea)   
	//	{  
	//		cvSeqRemove(firstcontour, 0); // ɾ�����С���趨ֵ������
	//		continue;
	//	}  
	//	//CvRect aRect = cvBoundingRect( firstcontour, 0 ); 
	//	//if ((aRect.width/aRect.height)<1)  
	//	//{  
	//	//	cvSeqRemove(contour, 0); //ɾ����߱���С���趨ֵ������
	//	//	continue;
	//	//}  

	//	// ����һ��ɫ��ֵ
	//	CvScalar color = CV_RGB( 0, 255, 255 );

	//	//max_level �������������ȼ�������ȼ�Ϊ0�����Ƶ��������������Ϊ1��������������������ͬ�ļ���������
	//	//���ֵΪ2�����е�����������ȼ�Ϊ2����������ͬ�����������е�һ���������������
	//	//���ֵΪ����������������ͬ�������������������ֱ������Ϊabs(max_level)-1��������
	//	cvDrawContours(img_TargetShow, firstcontour, color, color, -1, 1, 8);	//�����ⲿ���ڲ�������
	//}  

	/*cvZero(histimg_Angle);
	cvZero(histimg_Amplitude);
	cvZero(histimg_Angle_sub);
	cvZero(histimg_Amplitude_sub);*/
	swap(imgA,imgB);
	memcpy(bins_angle_pre,bins_angle,sizeof(bins_angle));//����֮ǰֱ��ͼ
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
		double val = ( double(bins[i])*dst->height/vectordata.size()/*double(ArrayMax(bins_amplitude,binNum_amplitude) )*/);//�õ�һά�����i�ĸ߶�
		CvScalar color = CV_RGB(255,255,0); //(hsv2rgb(i*180.f/hdims);
		cvRectangle( dst, cvPoint(i*binl_w,dst->height),
			cvPoint((i+1)*binl_w,(int)(dst->height - val)),//��Ŀ��ͼ���ϻ����Σ����ϵ�����
			color, 1, 8, 0 );
	}
	return;
}
void ThrStepSchBlkMonEstBase::CalcHistogram(vector<double> VectorScr,int BinNum,int bins[],int max,int min)//����ֱ��ͼ2
{

	double step = (max-min)/double(BinNum);
	for (unsigned int i=0;i<VectorScr.size();i++)//ͳ��ÿ�����С
	{
		int pos = (VectorScr[i]-min)/step;
		bins[pos]++;
	}


}
//���㷽����
double ThrStepSchBlkMonEstBase::CalcEntropy()
{
	//ͳ�Ʒ���Ƕȵ��ܺ�
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