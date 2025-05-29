#include "FbxParts.h"
#include "Fbx.h"
#include "Global.h"
#include "Direct3D.h"
#include "Camera.h"
#include "Debug.h"

FbxParts::FbxParts() :
    ppIndexBuffer_(nullptr), pMaterial_(nullptr),
    pVertexBuffer_(nullptr), pConstantBuffer_(nullptr),
	pVertexData_(nullptr), ppIndexData_(nullptr)
{

}

FbxParts::FbxParts(Fbx* parent) : 
    ppIndexBuffer_(nullptr), pMaterial_(nullptr),
    pVertexBuffer_(nullptr), pConstantBuffer_(nullptr),
    pVertexData_(nullptr), ppIndexData_(nullptr)
{
    parent_ = parent;
}

// デストラクタ
FbxParts::~FbxParts()
{
    SAFE_DELETE_ARRAY(pBoneArray_);
    SAFE_DELETE_ARRAY(ppCluster_);

    if (pWeightArray_ != nullptr)
    {
        for (DWORD i = 0; i < vertexCount_; i++)
        {
			SAFE_DELETE_ARRAY(pWeightArray_[i].pBoneIndex);
			SAFE_DELETE_ARRAY(pWeightArray_[i].pBoneWeight);
        }
        SAFE_DELETE_ARRAY(pWeightArray_);
    }

	SAFE_DELETE_ARRAY(pVertexData_);
    for (DWORD i = 0; i < materialCount_; i++)
    {
        SAFE_RELEASE(ppIndexBuffer_[i]);
        SAFE_DELETE(ppIndexData_[i]);
        SAFE_DELETE(pMaterial_[i].pTexture);
    }
    SAFE_DELETE_ARRAY(ppIndexBuffer_);
    SAFE_DELETE_ARRAY(ppIndexData_);
    SAFE_DELETE_ARRAY(pMaterial_);

    SAFE_RELEASE(pVertexBuffer_);
    SAFE_RELEASE(pConstantBuffer_);
}

// FBXファイルから情報をロードして諸々準備する
HRESULT FbxParts::Init(FbxNode* pNode)
{
    // ノードからメッシュの情報を取得
    FbxMesh* mesh = pNode->GetMesh();

    mesh->SplitPoints(FbxLayerElement::eTextureDiffuse);

    // 各情報の個数を取得
    vertexCount_ = mesh->GetControlPointsCount(); // 頂点の数
    polygonCount_ = mesh->GetPolygonCount();      // ポリゴンの数
    polygonVertexCount_ = mesh->GetPolygonVertexCount(); // ポリゴン頂点インデックス数

    InitVertex(mesh);    // 頂点バッファ準備
    InitMaterial(pNode); // マテリアル準備
    InitIndex(mesh);     // インデックスバッファ準備
    InitSkelton(mesh);   // 骨の情報を準備
	IntConstantBuffer();  // コンスタントバッファ（シェーダーに準備を送るやつ）準備tConstant

    return E_NOTIMPL;
}

HRESULT FbxParts::Init(FbxMesh* pMesh)
{
    // メッシュの情報を取得

    // メッシュのコントロールポイントをマテリアルをベースに分割する
    pMesh->SplitPoints(FbxLayerElement::eTextureDiffuse);

	vertexCount_ = pMesh->GetControlPointsCount(); // 頂点の数
	polygonCount_ = pMesh->GetPolygonCount();      // ポリゴンの数
	polygonVertexCount_ = pMesh->GetPolygonVertexCount(); // ポリゴン頂点インデックス数

    InitVertex(pMesh);   // 頂点バッファ準備
	InitMaterial(pMesh); // マテリアル準備
    InitIndex(pMesh);    // インデックスバッファ準備
	InitSkelton(pMesh);  // 骨の情報を準備
    IntConstantBuffer(); // コンスタントバッファ（シェーダーに準備を送るやつ）準備

    return E_NOTIMPL;
}

void FbxParts::InitVertex(fbxsdk::FbxMesh* mesh)
{
    pVertexData_ = new VERTEX[vertexCount_];

    for (DWORD poly = 0; poly < polygonCount_; poly++)
    {
        // 3頂点分
        for (int vertex = 0; vertex < 3; vertex++)
        {
            int index = mesh->GetPolygonVertex(poly, vertex);

            // 頂点の位置
            FbxVector4 pos = mesh->GetControlPointAt(index);
            pVertexData_[index].position = XMFLOAT3((float)pos[0], (float)pos[1], -(float)pos[2]);

            // 頂点の法線
            FbxVector4 Normal;
            mesh->GetPolygonVertexNormal(poly, vertex, Normal); // i番目のポリゴンの、j番目の法線をゲット
            pVertexData_[index].normal = XMFLOAT3((float)Normal[0], (float)Normal[1], -(float)Normal[2]);

            // 頂点のUV
            FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();

            if (pUV->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            {
                int uvIndex = mesh->GetTextureUVIndex(poly, vertex, FbxLayerElement::eTextureDiffuse);
                FbxVector2  uv = pUV->GetDirectArray().GetAt(uvIndex);
                pVertexData_[index].uv = { (float)uv.mData[0], (float)(1.0 - uv.mData[1]), 0.0f };
            }
            else if (pUV->GetReferenceMode() == FbxLayerElement::eDirect)
            {
                FbxVector2 vUV;
                bool res = true;
                FbxStringList sUVSetNames;
                mesh->GetUVSetNames(sUVSetNames);
                FbxString sUVSetName = sUVSetNames.GetStringAt(0);
                mesh->GetPolygonVertexUV(poly, vertex, sUVSetName, vUV, res);
                int uvIndex = mesh->GetTextureUVIndex(poly, vertex, FbxLayerElement::eTextureDiffuse);
                pVertexData_[index].uv = { (float)vUV[0], (float)(1.0 - vUV[1]), 0.0f };
            }
        }
    }

    // 頂点データ用バッファの設定
    D3D11_BUFFER_DESC bd_vertex;
    bd_vertex.ByteWidth = sizeof(VERTEX) * mesh->GetControlPointsCount();
    bd_vertex.Usage = D3D11_USAGE_DYNAMIC;
    bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd_vertex.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd_vertex.MiscFlags = 0;
    bd_vertex.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA data_vertex;
    data_vertex.pSysMem = pVertexData_;
    Direct3D::pDevice_->CreateBuffer(&bd_vertex, &data_vertex, &pVertexBuffer_);
}

// マテリアル準備
void FbxParts::InitMaterial(fbxsdk::FbxNode* pNode)
{
    // マテリアルバッファの生成
    materialCount_ = pNode->GetMaterialCount();
    pMaterial_ = new MATERIAL[materialCount_];

    for (DWORD i = 0; i < materialCount_; i++)
    {
        ZeroMemory(&pMaterial_[i], sizeof(pMaterial_[i]));

        // フォンシェーディングを想定したマテリアルバッファの抽出
        FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

        FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

        // 環境光＆拡散反射光＆鏡面反射光の反射成分値を取得
        FbxDouble3 ambient = FbxDouble3(0, 0, 0);
        FbxDouble3 diffuse = FbxDouble3(0, 0, 0);
        FbxDouble3 specular = FbxDouble3(0, 0, 0);
        // Ambientのプロパティを見つける
        FbxProperty prop;
        prop = pPhong->FindProperty(FbxSurfaceMaterial::sAmbient);
        if (prop.IsValid())
        { 
            ambient = pPhong->Ambient;
        }
		prop = pPhong->FindProperty(FbxSurfaceMaterial::sDiffuse);
        if (prop.IsValid())
        {
            diffuse = pPhong->Diffuse;
        }

        // 環境光＆拡散反射光＆鏡面反射光の反射成分値をマテリアルバッファにコピー
        pMaterial_[i].ambient = XMFLOAT4((float)ambient[0], (float)ambient[1], (float)ambient[2], 1.0f);
        pMaterial_[i].diffuse = XMFLOAT4((float)diffuse[0], (float)diffuse[1], (float)diffuse[2], 1.0f);
        pMaterial_[i].specular = XMFLOAT4(0, 0, 0, 0);
        pMaterial_[i].shininess = 0;

        if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
        {
            prop = pPhong->FindProperty(FbxSurfaceMaterial::sSpecular);
            if (prop.IsValid())
            {
                specular = pPhong->Specular;
            }

            pMaterial_[i].specular = XMFLOAT4((float)specular[0], (float)specular[1], (float)specular[2], 1.0f);
            prop = pPhong->FindProperty(FbxSurfaceMaterial::sShininess);
            if (prop.IsValid())
            {
                pMaterial_[i].shininess = (float)pPhong->Shininess;
            }
            else
            {
                pMaterial_[i].shininess = (float)(1.0);
            }
        }
        InitTexture(pMaterial, i);
    }
}

void FbxParts::InitMaterial(fbxsdk::FbxMesh* pMesh)
{
    // マテリアルバッファの生成
    materialCount_ = pMesh->GetNode()->GetMaterialCount();
    pMaterial_ = new MATERIAL[materialCount_];

    for (DWORD i = 0; i < materialCount_; i++)
    {
		ZeroMemory(&pMaterial_[i], sizeof(pMaterial_[i]));
    }
}

void FbxParts::InitTexture(fbxsdk::FbxSurfaceMaterial* pMaterial, const DWORD& i)
{
}

void FbxParts::InitIndex(fbxsdk::FbxMesh* pMesh)
{
}

void FbxParts::InitSkelton(FbxMesh* pMesh)
{
}

void FbxParts::IntConstantBuffer()
{
}

void FbxParts::Draw(Transform& transform)
{
}

void FbxParts::DrawSkinAnime(Transform& transform, FbxTime time)
{
}

void FbxParts::DrawSkinAnime(std::string takeName, Transform& transform, FbxTime time)
{
}

void FbxParts::DrawMeshAnime(Transform& transform, FbxTime time, FbxScene* scene)
{
}

bool FbxParts::GetBonePosition(std::string boneName, XMFLOAT3* position)
{
    return false;
}

bool FbxParts::GetBonePositionAtNow(std::string boneName, XMFLOAT3* position)
{
    return false;
}

void FbxParts::RayCast(RayCastData* data)
{
}
