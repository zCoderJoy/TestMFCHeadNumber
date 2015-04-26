
// TestMFCHeadNumberDlg.h : ͷ�ļ�
//

#pragma once
#include "MontionAnalysis.h"
#include "CvvImage.h"
// CTestMFCHeadNumberDlg �Ի���
class CTestMFCHeadNumberDlg : public CDialogEx
{
// ����
public:
	CTestMFCHeadNumberDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTMFCHEADNUMBER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpen();
	//��Ƶ·��
	CString m_spath;
	afx_msg void OnBnClickedBtnStart();
	//����Ƶ�����߳�
	CWinThread *m_winThdReadVideo;
	//������
	MontionAnalysis m_MtnAls;
	//��ʾ���ھ��
	CWnd *m_cWndShow[2];
	FILE *saveAmpl;
	//��ʾ������Ϣ
	CWnd *m_hwndHeadNumber;
};
