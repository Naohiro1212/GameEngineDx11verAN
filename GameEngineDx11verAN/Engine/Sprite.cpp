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

// 準備
HRESULT Sprite::Load(std::string fileName)
{
    // テクスチャ準備
    pTexture_ = new Texture();
    if (FAILED(pTexture_->Load(fileName)))
    {
        return E_FAIL;
    }

    // 頂点情報準備
    InitVertex();

    // インデックス情報準備
    InitIndex();

    // コンスタントバッファ準備
    InitConstantBuffer();

    return S_OK;
}

// コンスタントバッファ準備
void Sprite::InitConstantBuffer()
{
    // 必要な設定項目
    D3D11_BUFFER_DESC cb;
    cb.ByteWidth = sizeof(CONSTANT_BUFFER);
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;

    // 定数バッファの作成
    Direct3D::pDevice_->CreateBuffer(&cb, NULL, &pConstantBuffer_);
}

void Sprite::InitVertex()
{
    // 頂点データ宣言
    VERTEX vertices[] =
    {
        {XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)}, // 四角形の頂点（左上）
        {XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)}, // 四角形の頂点（右上）
        {XMFLOAT3(-1.0f,-1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)}, // 四角形の頂点（左下）
        {XMFLOAT3(1.0f,-1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)}, // 四角形の頂点（右下）
    };

    // 頂点データ用バッファの設定
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

    // インデックスバッファを生成する
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
    // いろいろ設定
    Direct3D::SetShader(Direct3D::SHADER_2D);
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);
    Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);
    Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);
    Direct3D::SetDepthBafferWriteEnable(false);


}
