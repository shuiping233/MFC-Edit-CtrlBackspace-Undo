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
    int start, end;
    GetSel(start, end);
    // 如果本身有选区，我们按“起点”当光标
    int cur = (start == end) ? start : start;

    // 截断分支
    if (m_histIndex + 1 < m_history.size())
        m_history.erase(m_history.begin() + m_histIndex + 1,
            m_history.end());

    m_history.push_back({ txt.GetString(), cur });
    if (m_history.size() > MAX_HIST)
        m_history.erase(m_history.begin());
    m_histIndex = m_history.size() - 1;
}

// 撤销
void CMyEdit::Undo()
{
    if (m_histIndex > 0)
    {
        --m_histIndex;
        m_bUserEdit = FALSE;
        const auto& snap = m_history[m_histIndex];
        SetWindowTextW(snap.text.c_str());
        SetSel(snap.cursor, snap.cursor);   // ← 关键：光标也还原
        m_bUserEdit = TRUE;
    }
}

// 重做
void CMyEdit::Redo()
{
    if (m_histIndex + 1 < m_history.size())
    {
        ++m_histIndex;
        m_bUserEdit = FALSE;
        const auto& snap = m_history[m_histIndex];
        SetWindowTextW(snap.text.c_str());
        SetSel(snap.cursor, snap.cursor);
        m_bUserEdit = TRUE;
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

    // 2 普通 Backspace 删除
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == VK_BACK &&
        !(GetKeyState(VK_CONTROL) & 0x8000))
    {
        int a, b;
        GetSel(a, b);

        // 只有有内容可删才记录
        if (a > 0 || b > 0)
        {
            PushSnapshot();  // 删除前记录
        }
        return FALSE;  // 让系统处理，EN_UPDATE 会再记录一次
    }

    // 3. Ctrl+Z 撤销
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == 'Z' &&
        (GetKeyState(VK_CONTROL) & 0x8000))
    {
        Undo();
        return TRUE;
    }

    // 4. Ctrl+Y 恢复
    if (pMsg->message == WM_KEYDOWN &&
        pMsg->wParam == 'Y' &&
        (GetKeyState(VK_CONTROL) & 0x8000))
    {
        Redo();
        return TRUE;
    }

    // 5. 普通字符输入（可打印键）也先拍快照，再交给系统
    if (pMsg->message == WM_CHAR && pMsg->wParam >= 0x20)
    {
        // 只记录“用户真正想输入”的可见字符
        PushSnapshot();
        // 不拦截，返回 FALSE 让系统继续处理
    }

    return CEdit::PreTranslateMessage(pMsg);
}


void CMyEdit::OnEnUpdate()
{
    if (m_bInUpdate) return;
    m_bInUpdate = TRUE;

    if (m_bUserEdit)
        PushSnapshot();  // 恢复这一行！

    m_bInUpdate = FALSE;
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
    ON_CONTROL_REFLECT(EN_UPDATE, &CMyEdit::OnEnUpdate)
END_MESSAGE_MAP()