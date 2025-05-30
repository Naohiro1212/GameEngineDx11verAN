#include "Camera.h"
#include "Direct3D.h"

XMFLOAT3 _position;
XMFLOAT3 _target;
XMMATRIX _view;
XMMATRIX _proj;
XMMATRIX _billBoard;

// 初期化（プロジェクション行列作成）
void Camera::Initialize()
{
    _position = XMFLOAT3(0, 3, -5); // カメラの位置
    _target = XMFLOAT3(0, 1, 0);    // カメラの焦点

    // プロジェクション行列
    _proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (FLOAT)Direct3D::screenWidth_ / (FLOAT)Direct3D::screenHeight_, 0.1f, 1000.0f);
}

// 更新（ビュー行列作成）
void Camera::Update()
{
    // ビュー行列
    _view = XMMatrixLookAtLH(XMVectorSet(_position.x, _position.y, _position.z, 0),
            XMVectorSet(_target.x, _target.y, _target.z, 0), XMVectorSet(0, 1, 0, 0));

    _billBoard = XMMatrixLookAtLH(XMVectorSet(_position.x, _position.y, _position.z, 0),
        XMVectorSet(_target.x, _target.y, _target.z, 0), XMVectorSet(0, 1, 0, 0));

    // ビルボード行列
    // （常にカメラのほうを向くように回転させる行列。パーティクルでしか使わない）
    _billBoard = XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), XMLoadFloat3(&_target) - XMLoadFloat3(&_position), XMVectorSet(0, 1, 0, 0));
    _billBoard = XMMatrixInverse(nullptr, _billBoard);
}

// 位置を設定
void Camera::SetPosition(XMFLOAT3 position) { _position = position; }

//焦点を取得
XMFLOAT3 Camera::GetTarget() { return _target; }

//位置を取得
XMFLOAT3 Camera::GetPosition() { return _position; }

//ビュー行列を取得
XMMATRIX Camera::GetViewMatrix() { return _view; }

//プロジェクション行列を取得
XMMATRIX Camera::GetProjectionMatrix() { return _proj; }

//ビルボード用回転行列を取得
XMMATRIX Camera::GetBillboardMatrix() { return _billBoard; }