
// CMyEdit.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号
#include <vector>
#include <string>
#include <afxwin.h>


// CCMyEditApp:
// 有关此类的实现，请参阅 CMyEdit.cpp
//

class CCMyEditApp : public CWinApp
{
public:
    CCMyEditApp();

    // 重写
public:
    virtual BOOL InitInstance();

    // 实现

    DECLARE_MESSAGE_MAP()
};

extern CCMyEditApp theApp;


struct Snapshot
{
    std::wstring text;
    int cursor;
};


class CMyEdit : public CEdit
{
public:
    BOOL PreTranslateMessage(MSG* pMsg) override;

protected:
    afx_msg void OnEnUpdate();   // 反射通知
    DECLARE_MESSAGE_MAP()        // 声明消息映射

private:
    void PushSnapshot();               // 记录当前文本
    void Undo();                       // 回退一次
    void Redo();                       // 重做一次

    std::vector<Snapshot> m_history;
    size_t m_histIndex = 0;            // 当前所在历史位置
    static constexpr size_t MAX_HIST = 50;
    BOOL m_bInUpdate = FALSE;   // 防止递归
    BOOL m_bUserEdit = TRUE;   // TRUE 表示“用户造成的改变”

};


