#pragma once
#include <imgui.h>
#include <vector>
#include <functional>
#include <initializer_list>
#include <utility>
#define NOMINMAX
#include <Windows.h>

#include "3rd/thp/thprac_gui_impl_win32.h"
#include "3rd/thp/thprac_hook.h"
#include "3rd/thp/utils/utils.h"

bool InGameInputGet(DWORD key);
bool InGameInputGetConfirm();

void SetGameInputFunc(std::function<bool(DWORD)> func);
int KeyboardInputUpdate(int v_key);

class GameGuiWnd {
public:
    struct Viewport {
        DWORD X;
        DWORD Y;
        DWORD Width;
        DWORD Height;
    };
    struct StyleValue {
        ImGuiStyleVar style;
        ImVec2 value;
    };
    GameGuiWnd() noexcept
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    }

    static constexpr ImGuiWindowFlags STYLE_DEFAULT = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 0;

    void Update();

    void Open();
    void Close();

    bool IsOpen();
    bool IsClosed();

    void SetViewport(void* ptr);
    void SetSize(float width, float height);
    void SetPos(float x, float y);
    void SetItemWidth(float item_width);
    void SetSizeRel(float width_prop, float height_prop);
    void SetPosRel(float x_prop, float y_prop);
    void SetItemWidthRel(float item_width_prop);
    void SetItemSpacing(float x, float y);
    void SetItemSpacingRel(float x_prop, float y_prop);
    void SetAutoSpacing(bool toggle);
    void AutoSize(ImVec2 size,
        const char* content_1 = nullptr, const char* content_2 = nullptr, const char* label_1 = nullptr, const char* label_2 = nullptr,
        float widiget_counts = 0.0f, ImVec2 size_max = { -1.0f, -1.0f }, ImVec2 size_min = { -1.0f, -1.0f });
    void AutoSize(float x, float y, const char* content = nullptr, const char* label = nullptr, float widigit_counts = 0.0f, float max_y = 0.0f);
    void AutoPos(float x, float y);
    void AutoItemWidth(const char* label);

    void SetStyle(ImGuiStyleVar style, float param1, float param2 = 0.0f);
    void SetWndFlag(ImGuiWindowFlags flags);
    void SetFade(float transparency, float step);
    void SetTitle(const char* title);
    void SetWndFoucs(bool toggle);

    void WndDebugOutput();

protected:
    std::vector<StyleValue>::iterator GetStyleIt(ImGuiStyleVar style);

    virtual void OnLocaleChange() {};
    virtual void OnContentUpdate() {};
    virtual void OnPreUpdate() {};
    virtual void OnPostUpdate() {};

    int mStatus = 0;
    std::vector<StyleValue> mStyle;
    std::string mTitle;
    ImVec2 mSize;
    ImVec2 mPos;
    ImGuiWindowFlags mWndFlag = STYLE_DEFAULT;
    float mItemWidth = -100.0f;
    float mAlpha = 0.0f;
    float mAlphaMax = 0.0f;
    float mAlphaStep = 0.0f;
    bool mSizeFlag = true;
    bool mPosFlag = true;
    bool mFocus = false;
    Viewport* mViewport = nullptr;
};

class PPGuiWnd : public GameGuiWnd {
protected:
    void InitUpdFunc(
        std::function<void(void)>&& contentUpdFunc,
        std::function<void(void)>&& localeUpdFunc,
        std::function<void(void)>&& preUpdFunc,
        std::function<void(void)>&& postUpdFunc)
    {
        mContentUpdFuncTmp = mContentUpdFunc = contentUpdFunc;
        mLocaleUpdFuncTmp = mLocaleUpdFunc = localeUpdFunc;
        mPreUpdFuncTmp = mPreUpdFunc = preUpdFunc;
        mPostUpdFuncTmp = mPostUpdFunc = postUpdFunc;
    }
    void SetContentUpdFunc(std::function<void(void)>&& contentUpdFunc)
    {
        mContentUpdFuncTmp = contentUpdFunc;
    }
    void SetPreUpdFunc(std::function<void(void)>&& preUpdFunc)
    {
        mPreUpdFuncTmp = preUpdFunc;
    }
    void SetPostUpdFunc(std::function<void(void)>&& postUpdFunc)
    {
        mPostUpdFuncTmp = postUpdFunc;
    }
    void SetLocaleUpdFunc(std::function<void(void)>&& localeUpdFunc)
    {
        mLocaleUpdFuncTmp = localeUpdFunc;
    }

    void OnLocaleChange() override final
    {
        mLocaleUpdFunc();
    }
    void OnContentUpdate() override final
    {
        mContentUpdFunc();
    }
    void OnPreUpdate() override final
    {
        mContentUpdFunc = mContentUpdFuncTmp;
        mLocaleUpdFunc = mLocaleUpdFuncTmp;
        mPreUpdFunc = mPreUpdFuncTmp;
        mPostUpdFunc = mPostUpdFuncTmp;

        mPreUpdFunc();
    }
    void OnPostUpdate() override
    {
        mPostUpdFunc();
    }

    int mIndicator = 0;

private:
    std::function<void(void)> mContentUpdFunc = []() {};
    std::function<void(void)> mLocaleUpdFunc = []() {};
    std::function<void(void)> mPreUpdFunc = []() {};
    std::function<void(void)> mPostUpdFunc = []() {};

    std::function<void(void)> mContentUpdFuncTmp = []() {};
    std::function<void(void)> mLocaleUpdFuncTmp = []() {};
    std::function<void(void)> mPreUpdFuncTmp = []() {};
    std::function<void(void)> mPostUpdFuncTmp = []() {};
};


class GuiCombo {
public:
    GuiCombo(const char* label, std::vector<const char*> items)
        : mLabel(const_cast<char*>(label))
        , mItems(items)
    {
    }

    inline void SetItems(std::vector<const char*> items)
    {
        mItems = items;
        if (mCurrent >= items.size())
            mCurrent = items.size() - 1;
    }

    inline void SetLabel(const char* label)
    {
        mLabel = const_cast<char*>(label);
    }

    inline int& operator*()
    {
        return mCurrent;
    }
    bool operator()()
    {
        auto hasChanged = ImGui::Combo(mLabel, &mCurrent, mItems.data(), mItems.size());

        if (ImGui::IsItemFocused()) {
            if (InGameInputGet(VK_LEFT)) {
                hasChanged = true;
                --mCurrent;
                if (mCurrent < 0) {
                    mCurrent = mItems.size() - 1;
                }
            }
            else if (InGameInputGet(VK_RIGHT)) {
                hasChanged = true;
                ++mCurrent;
                if (mCurrent >= mItems.size()) {
                    mCurrent = 0;
                }
            }
        }

        return hasChanged;
    }

private:
    char* mLabel = nullptr;

    std::vector<const char*>  mItems;

    int mCurrent = 0;
};


class GuiCheckBox {
private:
    char* mLabel = nullptr;
    bool mToggle = false;

public:
    GuiCheckBox(const char* label)
        : mLabel(const_cast<char*>(label))
    {
    }

    inline void SetLabel(const char* label)
    {
        mLabel = const_cast<char*>(label);
    }

    inline bool& operator*()
    {
        return mToggle;
    }

    bool operator()();
};


class GuiButton {
private:
    ImVec2 mSize;
    char* mLabel = nullptr;

public:

    GuiButton(const char* label, float width, float height)
        : mSize{ width, height }
        , mLabel(const_cast<char*>(label))
    {
    }

    inline void SetSize(float width, float height)
    {
        mSize.x = width;
        mSize.y = height;
    }
    inline void SetLabel(const char* label)
    {
        mLabel = const_cast<char*>(label);
    }

    bool operator()();
};



template <typename T, ImGuiDataType type>
class GuiDrag {
private:
    char* mLabel = nullptr;
    T mValue = 0;
    T mValueMin = 0;
    T mValueMax = 0;
    T mStep = 1;
    T mStepMin = 1;
    T mStepMax = 1;
    T mStepX = 1;

public:
    GuiDrag(const char* label, const T&& minimum, const T&& maximum,
        T step_min = 1, T step_max = 1, T step_x = 10)
        : mLabel(const_cast<char*>(label))
        , mValueMin(minimum)
        , mValueMax(maximum)
        , mStep(step_min)
        , mStepMin(step_min)
        , mStepMax(step_max)
        , mStepX(step_x)
    {
    }

    inline void SetValue(const T&& value)
    {
        mValue = value;
    }
    inline void SetLabel(const char* label)
    {
        mLabel = const_cast<char*>(label);
    }
    inline void SetBound(const T&& minimum, const T&& maximum)
    {
        mValueMin = minimum;
        mValueMax = maximum;
        if (mValue < mValueMin)
            mValue = mValueMin;
        else if (mValue > mValueMax)
            mValue = mValueMax;
    }
    inline void SetStep(const T&& step_min, const T&& step_max, const T&& step_x = 10)
    {
        mStep = step_min;
        mStepMin = step_min;
        mStepMax = step_max;
        mStepX = step_x;
    }

    inline T GetValue()
    {
        return mValue;
    }
    inline T& operator*()
    {
        return mValue;
    }

    inline void RoundDown(T x)
    {
        if (mValue % x)
            mValue -= mValue % x;
    }

    void operator()(const char* format = nullptr)
    {
        bool isFocused;
        const char* label = mLabel;

        ImGui::DragScalar(label, type, &mValue, (float)(mStep * 2), &mValueMin, &mValueMax, format);
        isFocused = ImGui::IsItemFocused();

        if (mValue > mValueMax)
            mValue = mValueMax;
        if (mValue < mValueMin)
            mValue = mValueMin;

        if (isFocused) {
            if (InGameInputGet(VK_LSHIFT)) {
                mStep *= mStepX;
                if (mStep > mStepMax)
                    mStep = mStepMin;
            }

            if (InGameInputGet(VK_LEFT)) {
                mValue -= mStep;
                if (mValue < mValueMin)
                    mValue = mValueMin;
            }
            else if (InGameInputGet(VK_RIGHT)) {
                mValue += mStep;
                if (mValue > mValueMax)
                    mValue = mValueMax;
            }
        }
    }
};



template <typename T, ImGuiDataType type>
class GuiSlider {
private:
    char* mLabel = nullptr;
    T mValue = 0;
    T mValueMin = 0;
    T mValueMax = 0;
    T mStep = 1;
    T mStepMin = 1;
    T mStepMax = 1;
    T mStepX = 1;

public:
    GuiSlider(const char* label, const T&& minimum, const T&& maximum,
        T step_min = 1, T step_max = 1, T step_x = 10)
        : mLabel(const_cast<char*>(label))
        , mValueMin(minimum)
        , mValueMax(maximum)
        , mStep(step_min)
        , mStepMin(step_min)
        , mStepMax(step_max)
        , mStepX(step_x)
    {
    }

    inline void SetValue(const T&& value)
    {
        mValue = value;
    }
    inline void SetLabel(const char* label)
    {
        mLabel = const_cast<char*>(label);
    }
    inline void SetBound(const T&& minimum, const T&& maximum)
    {
        mValueMin = minimum;
        mValueMax = maximum;
        if (mValue < mValueMin)
            mValue = mValueMin;
        else if (mValue > mValueMax)
            mValue = mValueMax;
    }
    inline void SetStep(const T&& step_min, const T&& step_max, const T&& step_x = 10)
    {
        mStep = step_min;
        mStepMin = step_min;
        mStepMax = step_max;
        mStepX = step_x;
    }

    inline T GetValue()
    {
        return mValue;
    }
    inline T& operator*()
    {
        return mValue;
    }

    inline void RoundDown(T x)
    {
        if (mValue % x)
            mValue -= mValue % x;
    }

    void operator()(const char* format = nullptr)
    {
        bool isFocused;
        const char* label = mLabel;

        ImGui::SliderScalar(label, type, &mValue, &mValueMin, &mValueMax, format);
        isFocused = ImGui::IsItemFocused();

        if (mValue > mValueMax)
            mValue = mValueMax;
        if (mValue < mValueMin)
            mValue = mValueMin;

        if (isFocused) {
            if (InGameInputGet(VK_LSHIFT)) {
                mStep *= mStepX;
                if (mStep > mStepMax)
                    mStep = mStepMin;
            }

            if (InGameInputGet(VK_LEFT)) {
                mValue -= mStep;
                if (mValue < mValueMin)
                    mValue = mValueMin;
            }
            else if (InGameInputGet(VK_RIGHT)) {
                mValue += mStep;
                if (mValue > mValueMax)
                    mValue = mValueMax;
            }
        }
    }
};


class GuiNavFocus {
public:
    GuiNavFocus(std::initializer_list<const char*> arr_name)
    {
        for (auto it = std::begin(arr_name); it != std::end(arr_name); ++it) {
            mNavName.push_back(*it);
        }
    }

    inline int& operator*()
    {
        return mFocusId;
    }

    void operator()();

private:
    std::vector<const char*> mNavName;
    int mFocusId = -1;
};


class GuiHotKey {
private:
    const char* mText = nullptr;
    const char* mKeyText = nullptr;
    int mKey;
    bool mStatus = false;
    THPrac::HookSlice mHooks;
    float mXOffset1 = 0.0f;
    float mXOffset2 = 0.0f;

protected:
    bool OnWidgetUpdate();
public:

    GuiHotKey(const char* text, const char* key_text, int vkey, THPrac::HookSlice hooks)
        : mText(text)
        , mKeyText(key_text)
        , mKey(vkey)
        , mHooks(hooks)
    {
        for (size_t i = 0; i < hooks.len; i++) {
            hooks.ptr[i].Setup();
        }
    }

    GuiHotKey(const char* text, const char* key_text, int vkey,
        float x_offset_1, float x_offset_2, bool use_rel_offset,
        THPrac::HookSlice hooks)
        : mText(text)
        , mKeyText(key_text)
        , mKey(vkey)
        , mHooks(hooks)
    {
        if (use_rel_offset)
            SetTextOffsetRel(x_offset_1, x_offset_2);
        else
            SetTextOffset(x_offset_1, x_offset_2);

        for (size_t i = 0; i < hooks.len; i++) {
            hooks.ptr[i].Setup();
        }
    }

    GuiHotKey(const char* text, const char* key_text, int vkey)
        : mText(text)
        , mKeyText(key_text)
        , mKey(vkey)
        , mHooks({})
    {
    }

    inline void SetText(const char* label)
    {
        mText = label;
    }
    inline void SetKey(const char* key_text, int vkey)
    {
        mKeyText = key_text;
        mKey = vkey;
    }
    inline void SetTextOffset(float x_offset_1, float x_offset_2)
    {
        mXOffset1 = x_offset_1;
        mXOffset2 = x_offset_2;
    }
    inline void SetTextOffsetRel(float x_offset_prop_1, float x_offset_prop_2)
    {
        auto disp_x = ImGui::GetIO().DisplaySize.x;
        mXOffset1 = x_offset_prop_1 * disp_x;
        mXOffset2 = x_offset_prop_2 * disp_x;
    }
    inline void Toggle(bool status)
    {
        mStatus = status;
        if (status) {
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
    inline bool& operator*()
    {
        return mStatus;
    }
    bool operator()(bool use_widget = true);
};

#define HOTKEY_DEFINE(name, txt, keytxt, vkey) GuiHotKey name { txt, keytxt, vkey, make_hook_array<

#if __INTELLISENSE__
#define PATCH_HK(addr_, code_) HookCtx { .data = PatchData() }
#define EHOOK_HK(addr_, inslen_, ...) HookCtx { .data = PatchData() }
#else
#define PATCH_HK(addr_, code_) HookCtx{ .addr = addr_, .data = PatchCode(code_) }
#define EHOOK_HK(addr_, inslen_, ...) HookCtx{ .addr = addr_, .callback = [](PCONTEXT pCtx, HookCtx * self) __VA_ARGS__, .data = PatchHookImpl(inslen_) }
#endif

#define HOTKEY_ENDDEF() >()}
