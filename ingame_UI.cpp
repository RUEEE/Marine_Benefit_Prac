#include "ingame_UI.h"
#include <format>

std::function<bool(DWORD)> g_InputFunc;
bool InGameInputGetConfirm()
{
    return InGameInputGet('Z');
}

void SetGameInputFunc(std::function<bool(DWORD)> func)
{
    g_InputFunc = func;
}


static uint8_t __ki_keystatus[0xFF]{};
int KeyboardInputUpdate(int v_key)
{
    if (THPrac::Gui::ImplWin32CheckForeground() && GetAsyncKeyState(v_key) < 0)
    {
        if (__ki_keystatus[v_key] != 0xff)
            ++__ki_keystatus[v_key];
    }
    else
        __ki_keystatus[v_key] = 0;
    return __ki_keystatus[v_key];
}

bool InGameInputGet(DWORD key)
{
    if (g_InputFunc)
        return g_InputFunc(key);
    return KeyboardInputUpdate(key) == 1;
}

void GameGuiWnd::Update()
{
    OnPreUpdate();

    ImGuiIO& io = ImGui::GetIO();
    size_t styleCount = 0;

    if (mStatus == 1) {
        mAlpha += mAlphaStep;
        if (mAlpha >= mAlphaMax) {
            mAlpha = mAlphaMax;
            mStatus = 2;
        }
    }
    else if (mStatus == 3) {
        io.MouseDrawCursor = false;
        mAlpha -= mAlphaStep;
        if (mAlpha <= 0.0f) {
            mAlpha = 0.0f;
            mStatus = 0;
        }
    }


    if (mStatus) {
        for (auto& style : mStyle) {
            ImGui::PushStyleVarAlt(style.style, style.value);
            styleCount++;
        }
        if (mSizeFlag) {
            ImGui::SetNextWindowSize(mSize, ImGuiCond_Always); // ImGuiCond_FirstUseEver
            mSizeFlag = false;
        }
        if (mPosFlag) {

            ImGui::SetNextWindowPos(mPos, ImGuiCond_Always);
            mPosFlag = false;
        }
        if (mFocus)
            ImGui::SetNextWindowFocus();

        ImGui::SetNextWindowBgAlpha(mAlpha);
        ImGui::Begin(mTitle.c_str(), nullptr, mWndFlag);
        ImGui::PushItemWidth(mItemWidth);

        if (mStatus == 2) {
            OnContentUpdate();
        }

        ImGui::PopItemWidth();
        ImGui::End();
        for (size_t i = 0; i < styleCount; i++)
            ImGui::PopStyleVar();
    }

    OnPostUpdate();
}

void GameGuiWnd::Open()
{
    if (mStatus != 2)
        mStatus = 1;
}
void GameGuiWnd::Close()
{
    if (mStatus)
        mStatus = 3;
}

bool GameGuiWnd::IsOpen()
{
    return mStatus == 2;
}
bool GameGuiWnd::IsClosed()
{
    return mStatus == 0;
}

void GameGuiWnd::SetViewport(void* ptr)
{
    mViewport = (Viewport*)ptr;
}
void GameGuiWnd::SetSize(float width, float height)
{
    mSizeFlag = true;
    mSize.x = width;
    mSize.y = height;
}
void GameGuiWnd::SetPos(float x, float y)
{
    mPosFlag = true;
    mPos.x = x;
    mPos.y = y;
}
void GameGuiWnd::SetItemWidth(float item_width)
{
    mItemWidth = item_width;
}
void GameGuiWnd::SetSizeRel(float width_prop, float height_prop)
{
    ImGuiIO& io = ImGui::GetIO();
    mSizeFlag = true;
    if (mViewport) {
        mSize.x = width_prop * (float)mViewport->Width;
        mSize.y = height_prop * (float)mViewport->Height;
    }
    else {
        mSize.x = width_prop * io.DisplaySize.x;
        mSize.y = height_prop * io.DisplaySize.y;
    }
}
void GameGuiWnd::SetPosRel(float x_prop, float y_prop)
{
    ImGuiIO& io = ImGui::GetIO();
    mPosFlag = true;
    if (mViewport) {
        mPos.x = x_prop * (float)mViewport->Width + (float)mViewport->X;
        mPos.y = y_prop * (float)mViewport->Height + (float)mViewport->Y;
    }
    else {
        mPos.x = x_prop * io.DisplaySize.x;
        mPos.y = y_prop * io.DisplaySize.y;
    }
}
void GameGuiWnd::SetItemWidthRel(float item_width_prop)
{
    ImGuiIO& io = ImGui::GetIO();
    if (mViewport) {
        mItemWidth = item_width_prop * (float)mViewport->Width;
    }
    else {
        mItemWidth = item_width_prop * io.DisplaySize.x;
    }
}
void GameGuiWnd::SetItemSpacing(float x, float y)
{
    auto itemSpacing = GetStyleIt(ImGuiStyleVar_ItemSpacing);
    itemSpacing->value = ImVec2(x, y);
}
void GameGuiWnd::SetItemSpacingRel(float x_prop, float y_prop)
{
    ImGuiIO& io = ImGui::GetIO();
    auto itemSpacing = GetStyleIt(ImGuiStyleVar_ItemSpacing);
    if (mViewport) {
        itemSpacing->value = ImVec2(x_prop * (float)mViewport->Width, y_prop * (float)mViewport->Height);
    }
    else {
        itemSpacing->value = ImVec2(x_prop * io.DisplaySize.x, y_prop * io.DisplaySize.y);
    }
}
void GameGuiWnd::SetAutoSpacing(bool toggle)
{
    if (toggle) {
        ImGuiIO& io = ImGui::GetIO();
        if (mViewport) {
            auto framePadding = GetStyleIt(ImGuiStyleVar_FramePadding);
            framePadding->value = ImVec2(
                0.00625f * (float)mViewport->Width, 0.00625f * (float)mViewport->Height);
            auto itemSpacing = GetStyleIt(ImGuiStyleVar_ItemSpacing);
            itemSpacing->value = ImVec2(
                0.0125f * (float)mViewport->Width, 0.00833f * (float)mViewport->Height);
        }
        else {
            auto framePadding = GetStyleIt(ImGuiStyleVar_FramePadding);
            framePadding->value = ImVec2(
                0.00625f * io.DisplaySize.x, 0.00625f * io.DisplaySize.y);
            auto itemSpacing = GetStyleIt(ImGuiStyleVar_ItemSpacing);
            itemSpacing->value = ImVec2(
                0.0125f * io.DisplaySize.x, 0.00833f * io.DisplaySize.y);
        }
    }
    else {
        auto framePadding = GetStyleIt(ImGuiStyleVar_FramePadding);
        mStyle.erase(framePadding);
        auto itemSpacing = GetStyleIt(ImGuiStyleVar_ItemSpacing);
        mStyle.erase(itemSpacing);
    }
}
void GameGuiWnd::AutoSize(ImVec2 size,
    const char* content_1, const char* content_2, const char* label_1, const char* label_2,
    float widiget_counts, ImVec2 size_max, ImVec2 size_min)
{
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();

    ImVec2 label_size = { -1.0f, -1.0f };
    if (label_1) {
        auto label_size_1 = ImGui::CalcTextSize(label_1);
        if (label_2) {
            auto label_size_2 = ImGui::CalcTextSize(label_2);
            label_size = label_size_1.x > label_size_2.x ? label_size_1 : label_size_2;
        }
        else
            label_size = label_size_1;
    }
    bool label_valid = label_size.x >= 0.0f && label_size.y >= 0.0f;

    ImVec2 content_size = { -1.0f, -1.0f };
    if (content_1) {
        auto content_size_1 = ImGui::CalcTextSize(content_1);
        if (content_2) {
            auto content_size_2 = ImGui::CalcTextSize(content_2);
            content_size = content_size_1.x > content_size_2.x ? content_size_1 : content_size_2;
        }
        else
            content_size = content_size_1;
    }
    bool content_valid = content_size.x >= 0.0f && content_size.y >= 0.0f;

    if (size.x >= 1.0f) {
        mSize.x = size.x;
    }
    else if (size.x > 0.0f) {
        mSize.x = io.DisplaySize.x * size.x;
    }
    else if (size.x == 0.0f && content_valid && label_valid) {
        mSize.x = content_size.x + label_size.x + ImGui::GetFrameHeight() * 3;
    }
    if (size_max.x >= 0.0f && mSize.x > size_max.x)
        mSize.x = size_max.x;
    else if (size_min.x >= 0.0f && mSize.x < size_min.x)
        mSize.x = size_min.x;

    if (size.y > 1.0f) {
        mSize.y = size.y;
    }
    else if (size.y > 0.0f) {
        mSize.y = io.DisplaySize.y * size.y;
    }
    else if (size.y == 0.0f) {
        float font_y = 0.0f;
        if (label_valid)
            font_y = label_size.y;
        else
            font_y = ImGui::GetFont()->FontSize;

        mSize.y = (font_y + style.FramePadding.y * 2.0f + style.ItemSpacing.y) * widiget_counts;
    }
    if (size_max.y >= 0.0f && mSize.y > size_max.y)
        mSize.y = size_max.y;
    else if (size_min.y >= 0.0f && mSize.y < size_min.y)
        mSize.y = size_min.y;

    if (label_valid)
        mItemWidth = label_size.x * -1.0f;

    mSizeFlag = true;
}
void GameGuiWnd::AutoSize(float x, float y, const char* content, const char* label, float widigit_counts, float max_y)
{
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();

    if (x >= 1.0001f) {
        mSize.x = x;
    }
    else if (x > 0.0f) {
        mSize.x = io.DisplaySize.x * x;
    }
    else if (x == 0.0f && content && label) {
        mSize.x = ImGui::CalcTextSize(content).x + ImGui::CalcTextSize(label).x + ImGui::GetFrameHeight() * 3;
    }

    if (y > 1.0001f) {
        mSize.y = y;
    }
    else if (y > 0.0f) {
        mSize.y = io.DisplaySize.y * y;
    }
    else if (y == 0.0f) {
        float font_y = 0.0f;
        if (content || label)
            font_y = ImGui::CalcTextSize(content ? content : label).y;
        else
            font_y = ImGui::GetFont()->FontSize;

        mSize.y = (font_y + style.FramePadding.y * 2.0f + style.ItemSpacing.y) * widigit_counts;
        if (mSize.y > max_y)
            mSize.y = max_y;
    }

    if (label)
        mItemWidth = ImGui::CalcTextSize(label).x * -1.0f;

    mSizeFlag = true;
}
void GameGuiWnd::AutoPos(float x, float y)
{
    auto& io = ImGui::GetIO();

    if (x > 1.0001f) {
        mPos.x = x;
    }
    else if (x >= 0.0f) {
        mPos.x = (io.DisplaySize.x - mSize.x) * x;
    }

    if (y > 1.0001f) {
        mPos.y = y;
    }
    else if (y >= 0.0f) {
        mPos.y = (io.DisplaySize.y - mSize.y) * y;
    }

    mPosFlag = true;
}
void GameGuiWnd::AutoItemWidth(const char* label)
{
    mItemWidth = ImGui::CalcTextSize(label).x * -1.0f;
}

void GameGuiWnd::SetStyle(ImGuiStyleVar style, float param1, float param2)
{
    auto s = GetStyleIt(style);
    s->value = ImVec2(param1, param2);
}
void GameGuiWnd::SetWndFlag(ImGuiWindowFlags flags)
{
    mWndFlag = flags;
}
void GameGuiWnd::SetFade(float transparency, float step)
{
    mAlphaMax = transparency;
    mAlphaStep = step;
}
void GameGuiWnd::SetTitle(const char* title)
{
    mTitle = title;
}
void GameGuiWnd::SetWndFoucs(bool toggle)
{
    mFocus = toggle;
}

void GameGuiWnd::WndDebugOutput()
{
    SetWndFlag(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    auto pos = ImGui::GetWindowPos();
    auto size = ImGui::GetWindowSize();
    auto& io = ImGui::GetIO();

    ImGui::Text("Size: %f (%f), %f (%f)", size.x, size.x / io.DisplaySize.x, size.y, size.y / io.DisplaySize.y);
    ImGui::Text("Pos X: %f (%f)", pos.x, pos.x / (io.DisplaySize.x - size.x));
    ImGui::Text("Pos Y: %f (%f)", pos.y, pos.y / (io.DisplaySize.y - size.y));
}
std::vector<GameGuiWnd::StyleValue>::iterator GameGuiWnd::GetStyleIt(ImGuiStyleVar style)
{
    for (auto it = mStyle.begin(); it != mStyle.end(); ++it) {
        if (it->style == style) {
            return it;
        }
    }
    mStyle.emplace_back();
    auto it = std::prev(mStyle.end());
    it->style = style;
    return it;
}


bool GuiCheckBox::operator()()
{
    const char* label = mLabel;
    bool pressed = ImGui::Checkbox(label, &mToggle);
    if (ImGui::IsItemFocused() && (InGameInputGet(VK_LEFT) || InGameInputGet(VK_RIGHT))) {
        mToggle = !mToggle;
        pressed = true;
    }
    return pressed;
}


bool GuiButton::operator()()
{
    const char* label = mLabel;
    bool res = ImGui::Button(label, mSize);
    if (ImGui::IsItemFocused() && InGameInputGetConfirm())
        res = true;
    return res;
}


void GuiNavFocus::operator()()
{

    bool has_active_popup = false;
    int last_activated_id = -1;

    for (int it = 0; it < mNavName.size();it++) {
        auto name = mNavName[it];
        if (ImGui::IsPopupOpen((std::string(name)+ "##ComboPopup").c_str()))
            has_active_popup = true;
        if (ImGui::IsItemActiveAlt(name))
            last_activated_id = it;
        else if (ImGui::IsItemFocusedAlt(name))
            mFocusId = it;
    }

    bool force = false;
    if (!has_active_popup)
    {
        ImGui::SetWindowFocus();

        if (last_activated_id != -1)
            mFocusId = last_activated_id;


        if (InGameInputGet(VK_UP))
        {
            mFocusId--;
            if (mFocusId < 0)
                mFocusId = 0;
            force = true;
        }
        else if (InGameInputGet(VK_DOWN))
        {
            mFocusId++;
            if (mFocusId >= mNavName.size())
                mFocusId = mNavName.size() - 1;
            force = true;
        }
        if (mFocusId != -1)
            ImGui::SetItemFocusAlt(mNavName[mFocusId], force);
        else
            ImGui::SetItemFocusAlt(mNavName[0], force);
    }
}


bool GuiHotKey::OnWidgetUpdate()
{
    const char* text = mText;
    std::string realText;
    if (mStatus) {
        realText = std::format("[{}: {}]", mKeyText, text);
        ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 0.0f, 1.0f });
    }
    else {
        realText = std::format("{}: {}", mKeyText, text);
    }

    auto cursor = ImGui::GetCursorPos();
    ImGui::TextUnformatted(realText.c_str());
    ImGui::SetCursorPos(cursor);

    if (mStatus)
        ImGui::PopStyleColor();

    if (ImGui::InvisibleButton(mKeyText, ImGui::CalcTextSize(realText.c_str())))
        return true;
    else
        return false;
}
bool GuiHotKey::operator()(bool use_widget)
{
    bool flag = KeyboardInputUpdate(mKey) == 1;
    if (use_widget)
        flag |= OnWidgetUpdate();

    if (flag) {
        mStatus = !mStatus;
        if (mStatus) {
            for (size_t i = 0; i < mHooks.len; i++) {
                mHooks.ptr[i].Enable();
            }
        }
        else {
            for (size_t i = 0; i < mHooks.len; i++) {
                mHooks.ptr[i].Disable();
            }
        }
    }

    return flag;
}