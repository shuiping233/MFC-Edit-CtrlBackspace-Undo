#include "pch.h"
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 判断是否为分隔符（英文符号）
static bool IsDelimiter(wchar_t ch)
{
    return ch == L' ' || ch == L',' || ch == L'.' || ch == L';' ||
        ch == L':' || ch == L'?' || ch == L'!' || ch == L'"' ||
        ch == L'\'' || ch == L'(' || ch == L')' || ch == L'[' ||
        ch == L']' || ch == L'{' || ch == L'}';
}

// 记录快照
void CMyEdit::PushSnapshot()
{
    CStringW txt;
    GetWindowText(txt);
    std::wstring s(txt);
    if (m_histIndex < m_history.size() && m_history[m_histIndex] == s)
        return; // 没变化，不重复记录
    m_history.resize(m_histIndex + 1);          // 扔掉后面的
    m_history.push_back(s);
    if (m_history.size() > MAX_HIST)            // 超上限，丢弃最旧
        m_history.erase(m_history.begin());
    else
        ++m_histIndex;
}

// 撤销
void CMyEdit::Undo()
{
    if (m_histIndex > 0)
    {
        --m_histIndex;
        SetWindowTextW(m_history[m_histIndex].c_str());
        SetSel(0, -1);        // 全选，方便继续输入
    }
}

BOOL CMyEdit::PreTranslateMessage(MSG* pMsg)
{
    // 1. Ctrl+Backspace 删除单词
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_BACK &&
        (GetKeyState(VK_CONTROL) & 0x8000))
    {
        CStringW txt;
        GetWindowText(txt);
        if (txt.IsEmpty())          // ← 空文本框直接屏蔽
            return TRUE;

        int a, b;
        GetSel(a, b);
        if (a != b) return FALSE;   // 有选区，系统处理

        PushSnapshot();             // 先记录，方便撤销

        int p = a;
        // 跳过左侧连续分隔符（保留）
        while (p > 0 && IsDelimiter(txt[p - 1]))
            --p;
        // 再跳过单词
        while (p > 0 && !IsDelimiter(txt[p - 1]))
            --p;
        if (p < a)
        {
            txt.Delete(p, a - p);
            SetWindowText(txt);
            SetSel(p, p);
            return TRUE;            // 吃掉消息
        }
    }

    // 2. Ctrl+Z 撤销
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == 'Z' &&
        (GetKeyState(VK_CONTROL) & 0x8000))
    {
        Undo();
        return TRUE;
    }

    return CEdit::PreTranslateMessage(pMsg);
}

// 新增成员函数
void CMyEdit::OnEnUpdate()
{
    if (m_bInUpdate) return;     // 递归保护
    m_bInUpdate = TRUE;
    PushSnapshot();              // 真正修改前拍照
    m_bInUpdate = FALSE;
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
    ON_CONTROL_REFLECT(EN_UPDATE, &CMyEdit::OnEnUpdate)
END_MESSAGE_MAP()