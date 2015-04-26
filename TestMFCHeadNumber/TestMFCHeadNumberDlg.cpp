
// TestMFCHeadNumberDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestMFCHeadNumber.h"
#include "TestMFCHeadNumberDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestMFCHeadNumberDlg 对话框



CTestMFCHeadNumberDlg::CTestMFCHeadNumberDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestMFCHeadNumberDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestMFCHeadNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestMFCHeadNumberDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, &CTestMFCHeadNumberDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_START, &CTestMFCHeadNumberDlg::OnBnClickedBtnStart)
END_MESSAGE_MAP()


// CTestMFCHeadNumberDlg 消息处理程序

BOOL CTestMFCHeadNumberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_winThdReadVideo=NULL;
	m_cWndShow[0]=GetDlgItem(IDC_STATIC_ORIGIN);
	m_cWndShow[1]=GetDlgItem(IDC_STATIC_RESULT);
	saveAmpl=fopen("imageAmpl.csv","w+");
	m_MtnAls.initFile(saveAmpl);
	m_hwndHeadNumber=GetDlgItem(IDC_EDIT_SHOWRESULT);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestMFCHeadNumberDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTestMFCHeadNumberDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestMFCHeadNumberDlg::OnBnClickedBtnOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE,_T("*.mp4"),NULL,OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST/*|OFN_HIDEREADONLY*/
		,_T("video files (*.mp4;*.avi)|*.mp4;*.avi|ALL Files(*.*)|*.*||"),NULL); // 选项视频的约定
	dlg.m_ofn.lpstrTitle=_T("Open Video"); // 打开文件对话框的标题名
	if (dlg.DoModal()!=IDOK)
	{
		return; // 判断是否获得视频
	}
	m_spath=dlg.GetPathName(); // 获取视频路径
}

//创建线程
UINT ThreadProcessReadVideo(LPVOID pParam)
{
	CTestMFCHeadNumberDlg *l_pVPD2=(CTestMFCHeadNumberDlg *)pParam;
	CvCapture* capture= cvCaptureFromFile(l_pVPD2->m_spath);
	IplImage *frame,*t_TargetShow;//抓取到的每一帧
	bool *t_bMask=NULL;
	int t_nPersonNumbs=0;
	MSG msg;//消息信息
	CvvImage target[2];
	CRect rect[2];//显示模块
	CDC* l_PostScreen=l_pVPD2->m_cWndShow[0]->GetDC();
	CDC* l_PostScreen2=l_pVPD2->m_cWndShow[1]->GetDC();
	HDC l_Hdc=l_PostScreen->GetSafeHdc();
	HDC l_Hdc2=l_PostScreen2->GetSafeHdc();
	l_pVPD2->m_cWndShow[0]->GetClientRect(&rect[0]);
	l_pVPD2->m_cWndShow[1]->GetClientRect(&rect[1]);
	if( capture )
	{	
		int t_nTrd=0,t_nContTrd=0;
		while(1)
		{
			frame = cvQueryFrame(capture);
			if (t_bMask==NULL)//测试掩码区域
			{
				t_bMask=new bool[frame->width*frame->height];
				memset(t_bMask,0,frame->width*frame->height);
				for (int i=100;i<frame->height-100;i++)
				{
					for (int j=300;j<frame->width-200;j++)
					    t_bMask[i*frame->width+j]=true;
				}
				l_pVPD2->m_MtnAls.LoadRoi(t_bMask,0);
			}
			if (!frame)
				return 0;
			t_TargetShow=cvCloneImage(frame);
			l_pVPD2->m_MtnAls.BusVideDetectPersonNumb(frame,NULL,t_TargetShow);
			//显示视频
			//cvvimage显示
			target[0].CopyOf(frame);
			target[0].DrawToHDC(l_Hdc,rect[0]);
			//cvvimage显示
			target[1].CopyOf(t_TargetShow);
			target[1].DrawToHDC(l_Hdc2,rect[1]);
			//判断一个是不是
			vector<double> t_vdAmpl=l_pVPD2->m_MtnAls.Getg_amplitude();
			t_nTrd=0;
			for (int i=0;i<t_vdAmpl.size();++i)
			{
				if (t_vdAmpl[i]>8.1)//5.5 幅值长度
				{ 
					t_nTrd++;
				}
			}
			if (t_nTrd>2*t_vdAmpl.size()/7)//4//数量
			{
				++t_nContTrd;
			}
			else
				t_nContTrd=0;
			if (t_nContTrd>20)
			{
				++t_nPersonNumbs;
				//cout<<"检测到一人,总人数为："<<t_nPersonNumbs<<endl;
				CString str;
				str.Format("%d",t_nPersonNumbs);
				l_pVPD2->m_hwndHeadNumber->SetWindowTextA("检测到一人,总人数为："+str);
				t_nContTrd=0;
			}
			/*cvShowImage( "RawVideo", frame);
			cvShowImage( "ResultVideo", t_TargetShow);*/
			char t_cTemp[10];
			/*static int count=0;
			count++;
			string t_sFileName="../ImageAmpl/RawImageAmpl";
			itoa(count,t_cTemp,10);
			t_sFileName+=t_cTemp;
			t_sFileName+=".bmp";*/
			//cvSaveImage(t_sFileName.c_str(),t_TargetShow);
			//cvShowImage("amplhis1",t_MtnAls.GetAmplHist());
			if (PeekMessage(&msg,0,0,0,0))
			{
				
				//cvReleaseVideoWriter(&writertest);
				
				AfxEndThread(0);//如果收到信息退出线程
				return 0;
			}
			cvWaitKey(1);
			cvReleaseImage(&t_TargetShow);
		}
	}
//l_pVPD2->m_VideoPostProcess.ClimbDetect();//攀高检测
	return 0;
}
void CTestMFCHeadNumberDlg::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_winThdReadVideo!=NULL) 
	{
		GetDlgItem(IDC_BTN_START)->SetWindowTextA("检测开始");
		int i=PostThreadMessage(m_winThdReadVideo->m_nThreadID,WM_QUIT,0,0);
		m_winThdReadVideo=nullptr;
	}
	else
	{
		GetDlgItem(IDC_BTN_START)->SetWindowTextA("检测中");
		m_winThdReadVideo=AfxBeginThread(ThreadProcessReadVideo,this);
	}
}
