#include "game_init.h"
#include "ingame_ui.h"
#include "game_util.h"
#include "3rd/thp/thprac_hook.h"
#include "3rd/thp/thprac_gui_impl_win32.h"
#include "3rd/thp/thprac_gui_impl_dx9.h"
#include "3rd/thp/utils/utils.h"

#include <d3d9.h>

#include <format>
#include <vector>
#include <string>

#pragma warning(disable:4996)
#define MSG(str) MessageBoxA(NULL,std::string(str).c_str(),"",MB_OK);
#define MSGX(x) MessageBoxA(NULL,std::format("{:x}",x).c_str(),"",MB_OK);
#define DW(x) (*(DWORD*)(x))
#define I32(x) (*(int32_t*)(x))
using namespace THPrac;

IDirect3DDevice9** g_gameGuiDevice;
HWND* g_gameGuiHwnd;
HIMC g_gameIMCCtx;

int GameGuiProgress = 0;
bool g_is_full_screen = false;
bool g_is_rep_version = false;

void Init(IDirect3DDevice9** ppDevice, HWND* phwnd,float* x)
{
    ::ImGui::CreateContext();
    g_gameGuiDevice = ppDevice;
    g_gameGuiHwnd = phwnd;
    g_gameIMCCtx = ImmAssociateContext(*phwnd, 0);
    Gui::ImplDX9Init(*g_gameGuiDevice);
    Gui::ImplWin32Init(*phwnd);
    
    // Hooks
    Gui::ImplDX9HookReset();
    Gui::ImplWin32HookWndProc();
    Gui::ImplDX9AdjustDispSize();



    // LocaleCreateFont
    auto& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SimHei.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    
}

void GuiCreate()
{
    Init((IDirect3DDevice9**)(0x9D87F8), (HWND*)0x8C604C,(float*)0x8CC460);
}

void GameGuiBegin()
{
    Gui::ImplDX9NewFrame();
    Gui::ImplWin32NewFrame();
    ::ImGui::NewFrame();
    GameGuiProgress = 1;
}

void GameGuiEnd(bool draw_cursor = true)
{
    if (GameGuiProgress != 1)
        return;
    if (draw_cursor && Gui::ImplWin32CheckFullScreen()) {
        auto& io = ::ImGui::GetIO();
        io.MouseDrawCursor = true;
    }
    ::ImGui::EndFrame();
    GameGuiProgress = 2;
}

void GameGuiRender()
{
    if (GameGuiProgress != 2)
        return;
    Gui::ImplDX9Check((IDirect3DDevice9*)*g_gameGuiDevice);
    ::ImGui::Render();
    Gui::ImplDX9RenderDrawData(::ImGui::GetDrawData());
    GameGuiProgress = 0;
}


enum Addrs
{
    UI_Page              = 0xA87F48,
    UI_CoolDown          = 0xA87F4C,
    SE_OK                = 0xA73230,

    UI_Select_Stage      = 0xA87F30,
    UI_Select_Diff       = 0xA7E610,

    IsPrac               = 0xA7E618,

    InitLife             = 0xA8CD7C,
    InitBomb             = 0xA8CD80,
    InitPower            = 0xA8CD70,
    InitHydrogen         = 0xA8CD88,
    InitOxygen           = 0xA8CD84,
    InitGraze            = 0xA8CE10,
    InitScore            = 0xA8CD78,

    PlayerState          = 0xA8CD68,

    StageTime            = 0xB3E2E8,
    DramaState           = 0xA77390,
    BossState            = 0xA733C0,
};

enum JmpType
{
    JFront,JLatter,JBoss,JMBoss
};

struct PracParam
{
    bool mode = 0;
    int32_t stage = 0;
    int32_t life = 0;
    int32_t bomb = 0;
    int32_t power = 0;
    int32_t hydrogen = 0;
    int32_t oxygen = 0;
    int32_t graze = 0;
    int32_t score = 0;

    int32_t type;
    int32_t jmp;
    
}pracParam;


std::vector<const char*> front_jmps[7];
std::vector<int> front_time[7];


std::vector<const char*> latter_jmps[7];
std::vector<int> latter_time[7];

std::vector<const char*> boss_jmps[7];
std::vector<int> boss_jmps_boss_state[7];
std::vector<bool> boss_need_create[7];


std::vector<const char*> Mboss_jmps[7];
std::vector<int> Mboss_jmps_boss_state[7];
std::vector<bool> Mboss_need_create[7];

JmpType GetJmpType(int stage, int type)
{
    JmpType normal[] = { JFront,JMBoss,JLatter,JBoss };
    JmpType no_later[] = { JFront,JMBoss,JBoss };
    if (stage == 5)//stage 3/6
        return no_later[type];
    return normal[type];
}
void InsertLatterJmpData(int stage, const char8_t* name, int time)
{
    latter_jmps[stage].push_back((const char*)name);
    latter_time[stage].push_back(time);
}
void InsertFrontJmpData(int stage, const char8_t* name, int time)
{
    front_jmps[stage].push_back((const char*)name);
    front_time[stage].push_back(time);
}

void InsertBossJmpData(int stage,const char8_t* name, int state, bool createbs = true)
{
    boss_jmps[stage].push_back((const char*)name);
    boss_jmps_boss_state[stage].push_back(state);
    boss_need_create[stage].push_back(createbs);
}

void InsertMBossJmpData(int stage, const char8_t* name, int state, bool createbs = true)
{
    Mboss_jmps[stage].push_back((const char*)name);
    Mboss_jmps_boss_state[stage].push_back(state);
    Mboss_need_create[stage].push_back(createbs);
}

void InitData()
{
    //st1
    InsertFrontJmpData(0, u8"一开",            0);
    InsertFrontJmpData(0, u8"一前半1",         2200);
                                               
    InsertLatterJmpData(0, u8"一后半",          0);
    //st2
    InsertFrontJmpData(1, u8"二开",            0);
    InsertLatterJmpData(1, u8"二后半1",         0);

    InsertLatterJmpData(1, u8"二后半2",         790);//800

    //st3
    InsertFrontJmpData(2, u8"三开",            0);
    InsertFrontJmpData(2, u8"三前半飞行阵",    710);//720
    InsertFrontJmpData(2, u8"三前半飞行阵",    2490);//2500

    InsertLatterJmpData(2, u8"三后半1",         0);
    InsertLatterJmpData(2, u8"三后半飞行阵",    890);//900

    //st4
    InsertFrontJmpData(3, u8"四开",            0);
    InsertFrontJmpData(3, u8"四前半1",         540);//550
    InsertFrontJmpData(3, u8"四前半2",         3660);//3700

    InsertLatterJmpData(3, u8"四后半",         0);

    
    //st5
    InsertFrontJmpData(4, u8"五开",            0);
    InsertFrontJmpData(4, u8"五前半1",         2350);//2400
    InsertFrontJmpData(4, u8"五前半2",         3450);//3500
    InsertFrontJmpData(4, u8"五前半3",         4450);//4500

    InsertLatterJmpData(4, u8"五后半1",         0);
    InsertLatterJmpData(4, u8"五后半2",         2360);//2400

    
    //st6
    InsertFrontJmpData(5, u8"六开",            0);
    InsertFrontJmpData(5, u8"六前半1",         2060);//2100
    InsertFrontJmpData(5, u8"六前半2",         4800);

    
    //stex
    InsertFrontJmpData(6, u8"ex开",            0);
    InsertFrontJmpData(6, u8"ex前半1",         2250);//2300
    InsertFrontJmpData(6, u8"ex前半2",         3260);//3300
    InsertFrontJmpData(6, u8"ex前半3",         4950);//5000
    InsertFrontJmpData(6, u8"ex前半4",         7000);//7050

    InsertLatterJmpData(6, u8"ex后半1",         0);
    InsertLatterJmpData(6, u8"ex后半2",         600);//660
    InsertLatterJmpData(6, u8"ex后半3",         1360);//1400
    InsertLatterJmpData(6, u8"ex后半4",         3460);//3500
    InsertLatterJmpData(6, u8"ex后半5",         4960);//5000


    // mboss
    InsertMBossJmpData(0, u8"Mnormal 1"              , 0, false);
    InsertMBossJmpData(0, u8"Mspell 1 高风险高回报"  , 2);

    InsertMBossJmpData(1, u8"Mnormal 1"             , 0, false);
    InsertMBossJmpData(1, u8"Mspell 1 俄勒冈旋涡"   , 2);

    InsertMBossJmpData(2, u8"道中对话"                  , 0, false);
    InsertMBossJmpData(2, u8"Mnormal 1"                 , 1);
    InsertMBossJmpData(2, u8"Mspell 1 无底的安产大炮"   , 3);

    InsertMBossJmpData(3, u8"Mnormal 1"                 , 0, false);

    InsertMBossJmpData(4, u8"道中对话"                  , 0, false);
    InsertMBossJmpData(4, u8"Mnormal 1"                 , 1);
    InsertMBossJmpData(4, u8"Mspell 1 咒语的合唱"       , 2);
    
    InsertMBossJmpData(5, u8"Mspell 1 反正一颜色"       , 0, false);
    InsertMBossJmpData(5, u8"Mspell 2 梦幻的泡沫"       , 2, false);
    
    InsertMBossJmpData(6, u8"Mspell 1 盐盈珠"           , 0, false);
    InsertMBossJmpData(6, u8"Mspell 1 盐干珠"           , 2);
    InsertMBossJmpData(6, u8"Mspell 1 出盐的神秘石臼"   , 3);

    // boss
    InsertBossJmpData(0, u8"对话"                       ,0, false);
    InsertBossJmpData(0, u8"normal 1"                   ,1);
    InsertBossJmpData(0, u8"spell 1 木舟与泥舟"         ,2);
    InsertBossJmpData(0, u8"normal 2"                   ,3);
    InsertBossJmpData(0, u8"spell 2 三光姬狸"           ,4);

    InsertBossJmpData(1, u8"对话"                       ,0, false);
    InsertBossJmpData(1, u8"normal 1"                   ,1);
    InsertBossJmpData(1, u8"spell 1 刻于石上的基因"     ,2);
    InsertBossJmpData(1, u8"normal 2"                   ,3);
    InsertBossJmpData(1, u8"spell 2 埃舍尔螺旋"         ,4);
    InsertBossJmpData(1, u8"spell 3 潜在海中的龙尾"     ,5);

    InsertBossJmpData(2, u8"normal 1"                   ,0, false);
    InsertBossJmpData(2, u8"spell 1 未来预知"           ,3);
    InsertBossJmpData(2, u8"normal 2"                   ,4);
    InsertBossJmpData(2, u8"spell 2 右满舵海上火灾"     ,5);
    InsertBossJmpData(2, u8"normal 3"                   ,6);
    InsertBossJmpData(2, u8"spell 3 牧马玄上"           ,7);

    InsertBossJmpData(3, u8"对话"                       ,0, false);
    InsertBossJmpData(3, u8"normal 1"                   ,1);
    InsertBossJmpData(3, u8"spell 1 幻之楼阁"           ,2);
    InsertBossJmpData(3, u8"normal 2"                   ,3);
    InsertBossJmpData(3, u8"spell 2 海与空的混沌"       ,4);
    InsertBossJmpData(3, u8"normal 3"                   ,5);
    InsertBossJmpData(3, u8"spell 3 猪如的视角"         ,6);
    InsertBossJmpData(3, u8"spell 4 自机相关符卡"       ,8);

    InsertBossJmpData(4, u8"对话"                       ,0, false);
    InsertBossJmpData(4, u8"normal 1"                   ,1);
    InsertBossJmpData(4, u8"spell 1 泪的八百万遍大念珠" ,2);
    InsertBossJmpData(4, u8"normal 2"                   ,4);
    InsertBossJmpData(4, u8"spell 2 星之舞"             ,5);
    InsertBossJmpData(4, u8"normal 3"                   ,7);
    InsertBossJmpData(4, u8"spell 3 命酒"               ,8);
    InsertBossJmpData(4, u8"spell 4 大笑的八百万学士"   ,10);

    InsertBossJmpData(5, u8"对话"                       ,0,false);
    InsertBossJmpData(5, u8"normal 1"                   ,1);
    InsertBossJmpData(5, u8"spell 1 生命创造"           ,2);
    InsertBossJmpData(5, u8"normal 2"                   ,3);
    InsertBossJmpData(5, u8"spell 2 46亿年的岁月"       ,4);
    InsertBossJmpData(5, u8"normal 3"                   ,5);
    InsertBossJmpData(5, u8"spell 3 水压临界值突破"     ,6);
    InsertBossJmpData(5, u8"normal 4"                   ,7);
    InsertBossJmpData(5, u8"spell 4 自机相关符卡"       ,8);
    InsertBossJmpData(5, u8"spell 5 海之恩惠"           ,10);

    InsertBossJmpData(6, u8"对话"                       ,0,false);
    InsertBossJmpData(6, u8"normal 1"                   ,1);
    InsertBossJmpData(6, u8"spell 1 玉匣"               ,2);
    InsertBossJmpData(6, u8"normal 2"                   ,3);
    InsertBossJmpData(6, u8"spell 2 高维"               ,4);
    InsertBossJmpData(6, u8"normal 3"                   ,5);
    InsertBossJmpData(6, u8"spell 3 唱片机"             ,6);
    InsertBossJmpData(6, u8"normal 4"                   ,7);
    InsertBossJmpData(6, u8"spell 4 超美丽弹幕"         ,8);
    InsertBossJmpData(6, u8"normal 5"                   ,10);
    InsertBossJmpData(6, u8"spell 5 迷宫"               ,11);
    InsertBossJmpData(6, u8"normal 6"                   ,12);
    InsertBossJmpData(6, u8"spell 6 八百比流"           ,13);
    InsertBossJmpData(6, u8"normal 7"                   ,15);
    InsertBossJmpData(6, u8"spell 7 因果律操作"         ,16);
    InsertBossJmpData(6, u8"spell 8 集中豪无"           ,17);
    InsertBossJmpData(6, u8"normal 8"                   ,19);
    InsertBossJmpData(6, u8"spell 9 THE过去现在未来"    ,20);
    InsertBossJmpData(6, u8"spell 10 终符"              ,21);
}


bool IsPracMode()
{
    return DW(IsPrac);
}

class THOverlay : public GameGuiWnd {
    THOverlay() noexcept
    {
        SetTitle("Mod Menu");
        SetFade(0.1f, 0.5f);
        SetPos(10.0f, 10.0f);
        SetSize(0.0f, 0.0f);
        SetWndFlag(
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | 0);
        OnLocaleChange();
    }
    SINGLETON(THOverlay);
public:

protected:
    virtual void OnLocaleChange() override
    {
        float x_offset_1 = 0.0f;
        float x_offset_2 = 0.0f;
        x_offset_1 = 0.1f;
        x_offset_2 = 0.14f;

        mMenu.SetTextOffsetRel(x_offset_1, x_offset_2);
        mMuteki.SetTextOffsetRel(x_offset_1, x_offset_2);
    }
    virtual void OnContentUpdate() override
    {
        ImGui::Text("drama state: %d", I32(DramaState));
        ImGui::Text("boss state:  %d", I32(BossState));
        ImGui::Text("stage time:  %d", I32(StageTime));
        mMuteki();
    }
    virtual void OnPreUpdate() override
    {
        if (mMenu(false) && !ImGui::IsAnyItemActive()) {
            if (*mMenu) {
                Open();
            }
            else {
                Close();
            }
        }
    }

    GuiHotKey mMenu{ "ModMenuToggle", "BACKSPACE", VK_BACK };

    HOTKEY_DEFINE(mMuteki, (char*)u8"无敌", "F1", VK_F1)
        EHOOK_HK(0x660863, 7, {
        I32(PlayerState) = 0;
         })
    HOTKEY_ENDDEF();

public:
};
std::vector<const char*> stage_select = { "stage 1", "stage 2", "stage 3", "stage 4", "stage 5", "stage 6", "stage ex" };
std::vector<const char*> type_select_1 = std::vector<const char*>{ (const char*)u8"前半", (const char*)u8"道中boss", (const char*)u8"后半", (const char*)u8"关底boss" };
std::vector<const char*> type_select_2 = std::vector<const char*>{ (const char*)u8"前半", (const char*)u8"道中boss", (const char*)u8"关底boss" };
class PracUI : public GameGuiWnd
{
    PracUI() noexcept
    {
        SetFade(0.8f, 0.1f);
        SetStyle(ImGuiStyleVar_WindowRounding, 0.0f);
        SetStyle(ImGuiStyleVar_WindowBorderSize, 0.0f);
        SetTitle("prac");
        SetSizeRel(0.55f, 0.7f);
        SetPosRel(0.215f, 0.18f);
        // SetSizeRel(0.65f, 0.81f);
        // SetPosRel(0.20f, 0.1f);
        SetItemWidthRel(-0.15f);
        SetAutoSpacing(true);

        *mStage = 1;
        *mLife = 8;
        *mBomb = 8;
        *mPower = 400;
        *mOxygen = 0;
        *mHydrogen = 0;
        *mGraze = 0;
        *mScore = 0;
        *mJmpSelect = 0;
        *mType = 0;

        *mNavFocus = 0;
    }
    SINGLETON(PracUI)
private:
    GuiCombo mStage{ "stage",stage_select };
    GuiCombo mType{ "type",type_select_1 };
    GuiCombo mJmpSelect{ "jmp",std::vector<const char*>{""}};
    
    
    GuiDrag<int32_t, ImGuiDataType_S32> mLife      { "life",0,8 };
    GuiDrag<int32_t, ImGuiDataType_S32> mBomb      { "bomb",0,8 };
    GuiDrag<int32_t, ImGuiDataType_S32> mPower     { "power",0,400 };
    GuiSlider<int32_t, ImGuiDataType_S32> mOxygen    { "oxygen",0,64 };
    GuiSlider<int32_t, ImGuiDataType_S32> mHydrogen  { "hydrogen",0,384};
    GuiDrag<int32_t, ImGuiDataType_S32> mGraze        { "graze",0,2147483647 };
    GuiDrag<int32_t, ImGuiDataType_S32> mScore        { "score",0,2147483647};

    GuiNavFocus mNavFocus{ "stage","type","jmp","life","bomb","power","oxygen","hydrogen","graze","score"};

public:
    void FillParam()
    {
        pracParam.mode = true;
        pracParam.stage = *mStage;
        pracParam.life = *mLife;
        pracParam.bomb = *mBomb;
        pracParam.power = *mPower;

        pracParam.oxygen = *mPower;
        pracParam.hydrogen = *mHydrogen;
        pracParam.graze = *mGraze;
        pracParam.score = *mScore;

        pracParam.type = *mType;
        pracParam.jmp = *mJmpSelect;
    }

    void PracticeMenu()
    {
        bool sel_changed = false;
        sel_changed = mStage();
        if (sel_changed)
        {
            if (*mStage == 5)//no later
                mType.SetItems(type_select_2);
            else
                mType.SetItems(type_select_1);
        }
        sel_changed |= mType();

        if (sel_changed)
        {
            switch (GetJmpType(*mStage,*mType))
            {
            default:
            case JFront:
                mJmpSelect.SetItems(front_jmps[*mStage]);
                break;
            case JLatter:
                mJmpSelect.SetItems(latter_jmps[*mStage]);
                break;
            case JMBoss:
                mJmpSelect.SetItems(Mboss_jmps[*mStage]);
                break;
            case JBoss://boss
                mJmpSelect.SetItems(boss_jmps[*mStage]);
                break;
            }
        }
        mJmpSelect();

        mLife();
        mBomb();
        mPower();
        mOxygen();
        mHydrogen();
        mGraze();
        mScore();

        mNavFocus();
    }
    virtual void OnContentUpdate() override
    {
        PracticeMenu();
    }
};

enum Boss
{
    ST1_MID  = 0,
    ST1_BS   = 0,

    ST2_MID  = 1,
    ST2_BS   = 1,

    ST3_MID  = 2,
    ST3_BS   = 3,

    ST4_MID  = 4,
    ST4_BS   = 4,

    ST5_MID  = 5,
    ST5_BS   = 5,

    ST6_MID  = 4, // st6 has 2 boss in mid butt it should ok because we dont need to create it
    ST6_BS   = 6,
    
    ST7_MID  = 6,
    ST7_BS   = 7,
};

void CreateBoss(int stage, bool is_mid)
{
    Boss Bosses_end[] = { ST1_BS, ST2_BS, ST3_BS, ST4_BS, ST5_BS, ST6_BS, ST7_BS };
    Boss Bosses_mid[] = { ST1_MID,ST2_MID,ST3_MID,ST4_MID,ST5_MID,ST6_MID,ST7_MID };
    Boss bs = is_mid?Bosses_mid[stage]:Bosses_end[stage];
    int32_t index = asm_call<0x643840,Cdecl,int32_t>(bs);
    DW(0xA87FF0+ 62 * index * 4) = 14;
    *((double*)0xA88020 + 31 * index) = 172.0;
    *((double*)0xA88028 + 31 * index) = -40.0;
    *((double*)0xA88090 + 31 * index) = 192.0;
    *((double*)0xA88098 + 31 * index) = 80.0;
}
void SetBgm(int stage, bool is_boss)
{
    if(is_boss)
        asm_call<0x652d80, Cdecl>((stage + 1) * 2, 1);
    else
        asm_call<0x652d80, Cdecl>((stage + 1) * 2 - 1, 1);
}

void PracEnterStage()
{
    PracUI::singleton().FillParam();
    I32(UI_Select_Stage) = pracParam.stage;
    if (pracParam.stage == 6)
        I32(UI_Select_Diff) = 4;//ex

    DW(UI_Page) = 21;
    DW(UI_CoolDown) = 0;
    DW(SE_OK) = 0;
}

void RenderUpdate()
{
    PracUI::singleton().Update();
    THOverlay::singleton().Update();
}

HOOKSET_DEFINE(Prac)
EHOOK_DY(UI_Prac, 0x5E8730, 7,
    {
        PracUI::singleton().Open();
        if (GetKeyTime(ZOfs) == 1)
        {
            PracEnterStage();
            PracUI::singleton().Close();
        }
        if (GetKeyTime(XOfs) == 1)
        {
            PracUI::singleton().Close();
            pracParam.mode = false;//disable
        }
        pCtx->Eip = 0x05E89C9; // remain X key
    }
)
EHOOK_DY(Prac_Param_Set, 0x65775f, 6,
    {
        if (IsPracMode()) {
            I32(InitLife) = pracParam.life;
            I32(InitBomb) = pracParam.bomb;
            I32(InitPower) = pracParam.power;

            I32(InitOxygen) = pracParam.oxygen;
            I32(InitHydrogen) = pracParam.hydrogen;
            I32(InitGraze) = pracParam.graze;
            I32(InitScore) = pracParam.score;
            switch (GetJmpType(pracParam.stage, pracParam.type))
            {
            default:
            case JFront:
                SetBgm(pracParam.stage, false);
                I32(DramaState) = 0;
                I32(StageTime) = front_time[pracParam.stage][pracParam.jmp];
                break;
            case JMBoss:
                if (boss_need_create[pracParam.stage][pracParam.jmp])//not dlg
                {
                    CreateBoss(pracParam.stage, true);
                }
                SetBgm(pracParam.stage, false);
                I32(DramaState) = 1;
                I32(BossState) = Mboss_jmps_boss_state[pracParam.stage][pracParam.jmp];
                break;
            case JLatter:
                SetBgm(pracParam.stage, false);
                I32(DramaState) = 2;
                I32(StageTime) = latter_time[pracParam.stage][pracParam.jmp];
                break;
            case JBoss:
                if (boss_need_create[pracParam.stage][pracParam.jmp])//not dlg
                {
                    CreateBoss(pracParam.stage,false);
                    SetBgm(pracParam.stage, true);
                } else{
                    SetBgm(pracParam.stage, false);
                }
                I32(DramaState) = 3;
                I32(BossState) = boss_jmps_boss_state[pracParam.stage][pracParam.jmp];
                break;
            }
        }
    }
)
HOOKSET_ENDDEF()

HOOKSET_DEFINE(Hook_Update)
EHOOK_DY(GameGuiRenderUpdate, 0x45877f, 1, {
    GameGuiBegin();
    RenderUpdate();
    GameGuiEnd();
    GameGuiRender();
})
HOOKSET_ENDDEF()


HOOKSET_DEFINE(Hook_Init)
EHOOK_DY(WindowSz, 0x653030, 3, {
 if (pCtx->Edx == '4')
 {
     *(double*)(pCtx->Ebp - 0x30) = 3.0;
     pCtx->Eip = 0x65307D;
 }
 })
 EHOOK_DY(FullScreenBackbufferSz, 0x457fc2, 5, {
     if (g_is_full_screen)
     {
         ((D3DPRESENT_PARAMETERS*)(pCtx->Esp + 0x4))->Windowed = 0;
         ((D3DPRESENT_PARAMETERS*)(pCtx->Esp + 0x10))->BackBufferWidth = 640;
         ((D3DPRESENT_PARAMETERS*)(pCtx->Esp + 0x10))->BackBufferHeight = 480;
     }
     }
 )
EHOOK_DY(FullScreenGet, 0x653196, 2, {
    g_is_full_screen = true;
}
)
EHOOK_DY(FullScreenWindSz, 0x653124, 4, {
     char Destination[6];
     auto Stream = fopen("config.txt", "r");
     if (Stream)
     {
       fgets(Destination, 6, Stream);
     } else {
       strcpy(Destination, "00100");
       auto Streama = fopen("config.txt", "w");
       fputs(Destination, Streama);
       fclose(Streama);
     }
    if (Destination[0]=='1')
    {
        g_is_full_screen = true;
        I32(pCtx->Ebp - 0x1C) = GetSystemMetrics(SM_CXSCREEN);
        I32(pCtx->Ebp - 0x14) = GetSystemMetrics(SM_CYSCREEN);
    }
 })
//#define DISCL_EXCLUSIVE     0x00000001
//#define DISCL_NONEXCLUSIVE  0x00000002
//#define DISCL_FOREGROUND    0x00000004
//#define DISCL_BACKGROUND    0x00000008
//#define DISCL_NOWINKEY      0x00000010
PATCH_DY(DisableKeyOutOfWindow1, 0x43d50c, "06")
PATCH_DY(DisableKeyOutOfWindow2, 0x43d667, "06")
EHOOK_DY(init_1, 0x458323, 2, {
    self->Disable();
    GuiCreate();

    PracUI::singleton(); 
    THOverlay::singleton();
    
    InitData();

    SetGameInputFunc([](DWORD key)->bool
        {
            KeyOfs ofs = ENDOfs;
            switch (key)
            {
            default:
                ofs = ENDOfs; break;
            case VK_UP:
                ofs = UpOfs; break;
            case VK_DOWN:
                ofs = DownOfs; break;
            case VK_RIGHT:
                ofs = RightOfs; break;
            case VK_LEFT:
                ofs = LeftOfs; break;
            case 'Z':
                ofs = ZOfs; break;
            case 'X':
                ofs = XOfs; break;
            case VK_LSHIFT:
            case VK_SHIFT:
            case VK_RSHIFT:
                ofs = ShiftOfs; break;
            case VK_ESCAPE:
                ofs = EscOfs; break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
                ofs = CtrlOfs; break;
            }
            if (ofs == ENDOfs)
                return KeyboardInputUpdate(key) == 1;
            return GetKeyTime(ofs) == 1;
        }
    );

    EnableAllHooks(Hook_Update);
    EnableAllHooks(Prac);
    })
HOOKSET_ENDDEF()

void InitGame()
{
    if (*(BYTE*)(0x6533DA) == 0xE9)
    {
         g_is_rep_version = true;
    }
    EnableAllHooks(Hook_Init);
}