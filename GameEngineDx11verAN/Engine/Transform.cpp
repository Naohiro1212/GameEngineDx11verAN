#include "Transform.h"

Transform::Transform() : pParent_(nullptr), isSetDirect(false)
{
    position_ = XMFLOAT3(0, 0, 0);
    rotate_ = XMFLOAT3(0, 0, 0);
    scale_ = XMFLOAT3(1, 1, 1);
    matTranslate_ = XMMatrixIdentity();
    matRotate_ = XMMatrixIdentity();
    matScale_ = XMMatrixIdentity();
}

Transform::~Transform()
{
}

void Transform::Calclation()
{
    matTranslate_ = XMMatrixTranslation(position_.x, position_.y, position_.z);

    XMMATRIX rotateX, rotateY, rotateZ;
    rotateX = XMMatrixRotationX(XMConvertToRadians(rotate_.x));
    rotateY = XMMatrixRotationY(XMConvertToRadians(rotate_.y));
    rotateZ = XMMatrixRotationZ(XMConvertToRadians(rotate_.z));
    matRotate_ = rotateZ * rotateX * rotateY;

    matScale_ = XMMatrixScaling(sacle_.x, scale_.y, scale_z);
}

XMMATRIX Transform::GetWorldMatrix()
{
    if (!isSetDirect)
    {
        Calclation();
    }
    if (pParent_)
    {
        return matScale_ * matRotate_ * matTranslate_ * pParent_->GetWorldMatrix();
    }

    return matScale_ * matRotate_ * matTranslate_;
}
