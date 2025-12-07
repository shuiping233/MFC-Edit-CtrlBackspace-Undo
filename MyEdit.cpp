#include "pch.h"          // 如果你的项目叫 pch，或 stdafx.h
#include "CMyEdit.h"

BOOL CMyEdit::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_BACK &&
        (GetKeyState(VK_CONTROL) & 0x8000))
    {
        CString txt;
        GetWindowText(txt);
        int a, b;
        GetSel(a, b);          // 获取光标
        if (a != b) return FALSE;  // 有选区，让系统处理

        // 找单词边界
        int p = a;
        while (p > 0 && _istspace(txt[p - 1])) --p;
        while (p > 0 && !_istspace(txt[p - 1])) --p;
        if (p < a)
        {
            txt.Delete(p, a - p);
            SetWindowText(txt);
            SetSel(p, p);
            return TRUE;       // 吃掉消息
        }
    }
    return CEdit::PreTranslateMessage(pMsg);
}