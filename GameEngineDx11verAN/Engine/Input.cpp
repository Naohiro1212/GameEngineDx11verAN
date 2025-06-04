#include "Input.h"
#include "Global.h"

namespace Input
{
    // ウィンドウハンドル
    HWND hWnd_;

    // DirectInputオブジェクト
    LPDIRECTINPUT8 pDInput_;

    // キーボード
    LPDIRECTINPUTDEVICE8 pKeyDevice_; // デバイスオブジェクト
    BYTE keyState_[256];     // 現在の各キーの状態
    BYTE prevKeyState_[256]; // 前フレームでの各キーの状態

    // マウス
    LPDIRECTINPUTDEVICE8 pMouseDevice_; // デバイスオブジェクト
    DIMOUSESTATE mouseState_;     //　マウスの状態
    DIMOUSESTATE prevMouseState_; // 前フレームのマウスの状態
    POINT mousePos_;              // マウスカーソルの位置

    // コントローラー
    const int MAX_PAD_NUM = 4;
    XINPUT_STATE controllerState_[MAX_PAD_NUM];
    XINPUT_STATE prevControllerState_[MAX_PAD_NUM];

    // 初期化
    void Initialize(HWND hWnd)
    {
        // ウィンドウハンドル
        hWnd_ = hWnd;

        // DirectInput本体
        DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
            IID_IDirectInput8, (VOID**)&pDInput_, nullptr);

        // キーボード
        pDInput_->CreateDevice(GUID_SysKeyboard, &pKeyDevice_, nullptr);
        pKeyDevice_->SetDataFormat(&c_dfDIKeyboard);
        pKeyDevice_->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

        // マウス
        pDInput_->CreateDevice(GUID_SysMouse, &pMouseDevice_, nullptr);
        pMouseDevice_->SetDataFormat(&c_dfDIMouse);
        pMouseDevice_->SetCooperativeLevel(hWnd_, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
    }

    // 更新
    void Update()
    {
        // キーボード
        pKeyDevice_->Acquire();
        memcpy(prevKeyState_, keyState_, sizeof(keyState_));
        pKeyDevice_->GetDeviceState(sizeof(keyState_), &keyState_);

        // マウス
        pMouseDevice_->Acquire();
        memcpy(&prevMouseState_, &mouseState_, sizeof(mouseState_));
        pMouseDevice_->GetDeviceState(sizeof(mouseState_), &mouseState_);

        // コントローラー
        for (int i = 0;i < MAX_PAD_NUM;i++)
        {
            memcpy(&prevControllerState_[i], &controllerState_[i], sizeof(controllerState_[i]));
            XInputGetState(i, &controllerState_[i]);
        }
    }

    // 解放
    void Release()
    {
        SAFE_RELEASE(pMouseDevice_);
        SAFE_RELEASE(pKeyDevice_);
        SAFE_RELEASE(pDInput_);
    }

    // キーボード情報取得 //
    // キーが押されているか調べる
    bool IsKey(int keyCode)
    {
        // 押してる
        if (keyState_[keyCode] & 0x80)
        {
            return true;
        }
        return false;
    }

    // キーを今押したか調べる
    bool IsKeyDown(int keyCode)
    {
        // 今は押してて、前回は押してない
        if (IsKey(keyCode) && !(prevKeyState_[keyCode] & 0x80))
        {
            return true;
        }
        return false;
    }

    // キーを今離したか調べる
    bool IsKeyUp(int keyCode)
    {
        // 今押してなくて、前回は押してる
        if (!IsKey(keyCode) && prevKeyState_[keyCode] & 0x80)
        {
            return true;
        }
        return false;
    }

    // マウス情報取得 // 
    // マウスのボタンが押されているか調べる
    bool IsMouseButton(int buttonCode)
    {
        // 押してる
        if (mouseState_.rgbButtons[buttonCode] & 0x80)
        {
            return true;
        }
        return false;
    }

    // マウスのボタンを今押したか調べる（押しっぱなしは無効）
    bool IsMouseButtonDown(int buttonCode)
    {
        // 今は押してて、前回は押してない
        if (IsMouseButton(buttonCode) && !(prevMouseState_.rgbButtons[buttonCode] & 0x80))
        {
            return true;
        }
        return false;
    }

    // マウスのボタンを今離したか調べる
    bool IsMouseButtonUp(int buttonCode)
    {

    }
}
