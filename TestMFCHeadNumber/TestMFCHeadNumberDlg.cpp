
// TestMFCHeadNumberDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestMFCHeadNumber.h"
#include "TestMFCHeadNumberDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestMFCHeadNumberDlg �Ի���



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


// CTestMFCHeadNumberDlg ��Ϣ�������

BOOL CTestMFCHeadNumberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_winThdReadVideo=NULL;
	m_cWndShow[0]=GetDlgItem(IDC_STATIC_ORIGIN);
	m_cWndShow[1]=GetDlgItem(IDC_STATIC_RESULT);
	saveAmpl=fopen("imageAmpl.csv","w+");
	m_MtnAls.initFile(saveAmpl);
	m_hwndHeadNumber=GetDlgItem(IDC_EDIT_SHOWRESULT);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTestMFCHeadNumberDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTestMFCHeadNumberDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestMFCHeadNumberDlg::OnBnClickedBtnOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CFileDialog dlg(TRUE,_T("*.mp4"),NULL,OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST/*|OFN_HIDEREADONLY*/
		,_T("video files (*.mp4;*.avi)|*.mp4;*.avi|ALL Files(*.*)|*.*||"),NULL); // ѡ����Ƶ��Լ��
	dlg.m_ofn.lpstrTitle=_T("Open Video"); // ���ļ��Ի���ı�����
	if (dlg.DoModal()!=IDOK)
	{
		return; // �ж��Ƿ�����Ƶ
	}
	m_spath=dlg.GetPathName(); // ��ȡ��Ƶ·��
}

//�����߳�
UINT ThreadProcessReadVideo(LPVOID pParam)
{
	CTestMFCHeadNumberDlg *l_pVPD2=(CTestMFCHeadNumberDlg *)pParam;
	CvCapture* capture= cvCaptureFromFile(l_pVPD2->m_spath);
	IplImage *frame,*t_TargetShow;//ץȡ����ÿһ֡
	bool *t_bMask=NULL;
	int t_nPersonNumbs=0;
	MSG msg;//��Ϣ��Ϣ
	CvvImage target[2];
	CRect rect[2];//��ʾģ��
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
			if (t_bMask==NULL)//������������
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
			//��ʾ��Ƶ
			//cvvimage��ʾ
			target[0].CopyOf(frame);
			target[0].DrawToHDC(l_Hdc,rect[0]);
			//cvvimage��ʾ
			target[1].CopyOf(t_TargetShow);
			target[1].DrawToHDC(l_Hdc2,rect[1]);
			//�ж�һ���ǲ���
			vector<double> t_vdAmpl=l_pVPD2->m_MtnAls.Getg_amplitude();
			t_nTrd=0;
			for (int i=0;i<t_vdAmpl.size();++i)
			{
				if (t_vdAmpl[i]>8.1)//5.5 ��ֵ����
				{ 
					t_nTrd++;
				}
			}
			if (t_nTrd>2*t_vdAmpl.size()/7)//4//����
			{
				++t_nContTrd;
			}
			else
				t_nContTrd=0;
			if (t_nContTrd>20)
			{
				++t_nPersonNumbs;
				//cout<<"��⵽һ��,������Ϊ��"<<t_nPersonNumbs<<endl;
				CString str;
				str.Format("%d",t_nPersonNumbs);
				l_pVPD2->m_hwndHeadNumber->SetWindowTextA("��⵽һ��,������Ϊ��"+str);
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
				
				AfxEndThread(0);//����յ���Ϣ�˳��߳�
				return 0;
			}
			cvWaitKey(1);
			cvReleaseImage(&t_TargetShow);
		}
	}
//l_pVPD2->m_VideoPostProcess.ClimbDetect();//�ʸ߼��
	return 0;
}
void CTestMFCHeadNumberDlg::OnBnClickedBtnStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_winThdReadVideo!=NULL) 
	{
		GetDlgItem(IDC_BTN_START)->SetWindowTextA("��⿪ʼ");
		int i=PostThreadMessage(m_winThdReadVideo->m_nThreadID,WM_QUIT,0,0);
		m_winThdReadVideo=nullptr;
	}
	else
	{
		GetDlgItem(IDC_BTN_START)->SetWindowTextA("�����");
		m_winThdReadVideo=AfxBeginThread(ThreadProcessReadVideo,this);
	}
}
