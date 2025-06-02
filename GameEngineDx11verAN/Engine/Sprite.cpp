#include "Sprite.h"
#include "Direct3D.h"
#include "Global.h"

Sprite::Sprite():pTexture_(nullptr)
{
}

Sprite::~Sprite()
{
    SAFE_RELEASE(pVertexBuffer_);
    SAFE_RELEASE(pIndexBuffer_);
}

// ����
HRESULT Sprite::Load(std::string fileName)
{
    // �e�N�X�`������
    pTexture_ = new Texture();
    if (FAILED(pTexture_->Load(fileName)))
    {
        return E_FAIL;
    }

    // ���_��񏀔�
    InitVertex();

    // �C���f�b�N�X��񏀔�
    InitIndex();

    // �R���X�^���g�o�b�t�@����
    InitConstantBuffer();

    return S_OK;
}

// �R���X�^���g�o�b�t�@����
void Sprite::InitConstantBuffer()
{
    // �K�v�Ȑݒ荀��
    D3D11_BUFFER_DESC cb;
    cb.ByteWidth = sizeof(CONSTANT_BUFFER);
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;

    // �萔�o�b�t�@�̍쐬
    Direct3D::pDevice_->CreateBuffer(&cb, NULL, &pConstantBuffer_);
}

void Sprite::InitVertex()
{
    // ���_�f�[�^�錾
    VERTEX vertices[] =
    {
        {XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)}, // �l�p�`�̒��_�i����j
        {XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)}, // �l�p�`�̒��_�i�E��j
        {XMFLOAT3(-1.0f,-1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)}, // �l�p�`�̒��_�i�����j
        {XMFLOAT3(1.0f,-1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)}, // �l�p�`�̒��_�i�E���j
    };

    // ���_�f�[�^�p�o�b�t�@�̐ݒ�
    D3D11_BUFFER_DESC bd_vertex;
    bd_vertex.ByteWidth = sizeof(vertices);
    bd_vertex.Usage = D3D11_USAGE_DEFAULT;
    bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd_vertex.CPUAccessFlags = 0;
    bd_vertex.MiscFlags = 0;
    bd_vertex.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA data_vertex;
    data_vertex.pSysMem = vertices;
    Direct3D::pDevice_->CreateBuffer(&bd_vertex, &data_vertex, &pVertexBuffer_);
}

void Sprite::InitIndex()
{
    int index[] = { 2,1,0, 2,3,1 };

    // �C���f�b�N�X�o�b�t�@�𐶐�����
    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(index);
    bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = index;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    Direct3D::pDevice_->CreateBuffer(&bd, &InitData, &pIndexBuffer_);
}

void Sprite::Draw(Transform& transform, RECT rect, float alpha)
{
    // ���낢��ݒ�
    Direct3D::SetShader(Direct3D::SHADER_2D);
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);
    Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);
    Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);
    Direct3D::SetDepthBafferWriteEnable(false);


}
