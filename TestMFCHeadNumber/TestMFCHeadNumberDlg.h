
// TestMFCHeadNumberDlg.h : 头文件
//

#pragma once
#include "MontionAnalysis.h"
#include "CvvImage.h"
// CTestMFCHeadNumberDlg 对话框
class CTestMFCHeadNumberDlg : public CDialogEx
{
// 构造
public:
	CTestMFCHeadNumberDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTMFCHEADNUMBER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnOpen();
	//视频路径
	CString m_spath;
	afx_msg void OnBnClickedBtnStart();
	//读视频处理线程
	CWinThread *m_winThdReadVideo;
	//数人数
	MontionAnalysis m_MtnAls;
	//显示窗口句柄
	CWnd *m_cWndShow[2];
	FILE *saveAmpl;
	//显示人数消息
	CWnd *m_hwndHeadNumber;
};
