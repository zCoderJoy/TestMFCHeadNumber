
// TestMFCHeadNumber.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestMFCHeadNumberApp:
// �йش����ʵ�֣������ TestMFCHeadNumber.cpp
//

class CTestMFCHeadNumberApp : public CWinApp
{
public:
	CTestMFCHeadNumberApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestMFCHeadNumberApp theApp;