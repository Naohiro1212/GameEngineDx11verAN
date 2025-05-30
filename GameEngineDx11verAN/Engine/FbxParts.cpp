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

        // フォンシェーディングを想定したマテリアルバッファの抽出
        FbxSurfaceMaterial* pMaterial = pMesh->GetNode()->GetMaterial(i);
        FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

        // 環境光＆拡散反射光＆鏡面反射光の反射成分値を取得
        FbxDouble3  ambient = FbxDouble3(0, 0, 0);
        FbxDouble3  diffuse = FbxDouble3(0, 0, 0);
        FbxDouble3  specular = FbxDouble3(0, 0, 0);
        // Ambientのプロパティを見つける
        FbxProperty prop;
        prop = pPhong->FindProperty(FbxSurfaceMaterial::sAmbient);
        if (prop.IsValid())
        {
            //Debug::Log("Ambient OK", true);
            ambient = pPhong->Ambient;
        }
        prop = pPhong->FindProperty(FbxSurfaceMaterial::sDiffuse);
        if (prop.IsValid())
        {
            //Debug::Log("Diffuse OK", true);
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
                //Debug::Log("Specular OK", true);
                specular = pPhong->Specular;
            }

            pMaterial_[i].specular = XMFLOAT4((float)specular[0], (float)specular[1], (float)specular[2], 1.0f);
            prop = pPhong->FindProperty(FbxSurfaceMaterial::sShininess);
            if (prop.IsValid())
            {
                //Debug::Log("Shininess OK", true);
                pMaterial_[i].shininess = (float)pPhong->Shininess;
            }
            else
                pMaterial_[i].shininess = (float)(1.0);
        }
        InitTexture(pMaterial, i);
    }
}

// テクスチャ準備
void FbxParts::InitTexture(fbxsdk::FbxSurfaceMaterial* pMaterial, const DWORD& i)
{
    pMaterial_[i].pTexture = nullptr;

    // テクスチャー情報の取得
    FbxProperty lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

    // テクスチャの数
	int fileTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

    if (fileTextureCount > 0)
    {
        FbxFileTexture* texture = lProperty.GetSrcObject<FbxFileTexture>(0);

        // ファイル名+拡張だけにする
        char name[_MAX_FNAME]; // ファイル名
        char ext[_MAX_EXT];    // 拡張子
        _splitpath_s(texture->GetRelativeFileName(), nullptr, 0, nullptr, 0, name, _MAX_FNAME, ext, _MAX_EXT);
        wsprintf(name, "%s%s", name, ext);

        pMaterial_[i].pTexture = new Texture;
        pMaterial_[i].pTexture->Load(name);
    }
}

// インデックスバッファ準備
void FbxParts::InitIndex(fbxsdk::FbxMesh* mesh)
{
    // マテリアルの数だけインデックスバッファーを作成
    ppIndexBuffer_ = new ID3D11Buffer * [materialCount_];
    ppIndexData_ = new DWORD * [materialCount_];

    int count = 0;

    // マテリアルから「ポリゴン平面」の情報を抽出する
    for (DWORD i = 0;i < materialCount_;i++)
    {
        count = 0;
        DWORD* pIndex = new DWORD[polygonCount_ * 3];
        ZeroMemory(&pIndex[i], sizeof(pIndex[i]));

        // ポリゴンを構成する三角形平面が、
        // 「頂点バッファ」内の度の頂点を利用しているかを調べる
        for (DWORD j = 0; j < polygonCount_; j++)
        {
            FbxLayerElementMaterial* mtl = mesh->GetLayer(0)->GetMaterials();
            int mtlId = mtl->GetIndexArray().GetAt(j);
            if (mtlId == i)
            {
                for (DWORD k = 0; k < 3; k++)
                {
                    pIndex[count + k] = mesh->GetPolygonVertex(j, 2 - k);
                }
                count += 3;
            }
        }

        // インデックスバッファを生成する
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(int) * count;
        bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = pIndex;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        if (FAILED(Direct3D::pDevice_->CreateBuffer(&bd, &InitData, &ppIndexBuffer_[i])))
        {
            MessageBox(0, "インデックスバッファの生成に失敗しました", fbxFileName, MB_OK);
        }
        pMaterial_[i].polygonCount = count / 3; // ポリゴンの数を保存
        ppIndexData_[i] = new DWORD[count];
        memcpy(ppIndexData_[i], pIndex, sizeof(DWORD) * count);
        SAFE_DELETE_ARRAY(pIndex);
    }
}

// 骨の情報を準備
void FbxParts::InitSkelton(FbxMesh* pMesh)
{
    // でフォーマ情報（ボーンとモデルの関連付け）の取得

    FbxDeformer* pDeformer = pMesh->GetDeformer(0);
    if (pDeformer == nullptr)
    {
        // ボーン情報なし
        return;
    }

    // デフォーマ情報からスキンメッシュ情報を取得
    pSkinInfo_ = (FbxSkin*)pDeformer;

    // 頂点からポリゴンを逆引きするための情報を作成する
    struct POLY_INDEX
    {
        int* polyIndex; // ポリゴンの情報
        int* vertexIndex; // 頂点の番号
        int numRef;     // 頂点を共有するポリゴンの数
    };

    // ボーン情報を取得する
    numBone_ = pSkinInfo_->GetClusterCount();
    ppCluster_ = new FbxCluster * [numBone_];
    for (int i = 0; i < numBone_; i++)
    {
        ppCluster_[i] = pSkinInfo_->GetCluster(i);
    }

    // ボーンの数に合わせてウェイト情報を準備する
    pWeightArray_ = new FbxParts::Weight[vertexCount_];
    for (DWORD i = 0; i < vertexCount_; i++)
    {
        pWeightArray_[i].posOrigin = pVertexData_[i].position;
        pWeightArray_[i].normalOrigin = pVertexData_[i].normal;
        pWeightArray_[i].pBoneIndex = new int[numBone_];
        pWeightArray_[i].pBoneWeight = new float[numBone_];
        for (int j = 0; j < numBone_; j++)
        {
            pWeightArray_[i].pBoneIndex[j] = -1;
            pWeightArray_[i].pBoneWeight[i] = 0.0f;
        }
    }

    // それぞれのボーンに影響を受ける頂点を調べる
    // そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
    for (int i = 0; i < numBone_; i++)
    {
        int numIndex = ppCluster_[i]->GetControlPointIndicesCount(); // このボーンに影響を受ける頂点数
        int* piIndex = ppCluster_[i]->GetControlPointIndices();      // ボーン/ウェイト情報の番号
		double* pdWeight = ppCluster_[i]->GetControlPointWeights(); // ボーン/ウェイト情報の重み

        // 頂点側からインデックスをたどって、頂点サイドで整理する
        for (int k = 0; k < numIndex; k++)
        {
            // 頂点に関連付けられたウェイト情報がボーン5本以上の場合は、重みの大きい順に4本を絞る
            for (int m = 0; m < 4; m++)
            {
                if (m >= numBone_) break;

                if (pdWeight[k] > pWeightArray_[piIndex[k]].pBoneIndex[m])
                {
                    for (int n = numBone_ - 1; n > m; n--)
                    {
                        pWeightArray_[piIndex[k]].pBoneIndex[n] = pWeightArray_[piIndex[k]].pBoneIndex[n - 1];
						pWeightArray_[piIndex[k]].pBoneWeight[n] = pWeightArray_[piIndex[k]].pBoneWeight[n - 1];
                    }
                    pWeightArray_[piIndex[k]].pBoneIndex[m] = i; // ボーンのインデックスを保存
					pWeightArray_[piIndex[k]].pBoneWeight[m] = (float)pdWeight[k]; // ボーンの重みを保存
                    break;
                }
            }
        }
    }

    // ボーン生成
    pBoneArray_ = new FbxParts::Bone[numBone_];
    for (int i = 0; i < numBone_; i++)
    {
        // ボーンのデフォルト位置を取得
        FbxAMatrix matrix;
        ppCluster_[i]->GetTransformLinkMatrix(matrix);

        // 行列コピー（Fbx形式からDirectXへの変換）
        XMFLOAT4X4 pose;
        for (DWORD x = 0; x < 4; x++)
        {
            for (DWORD y = 0; y < 4; y++)
            {
                pose(x, y) = (float)matrix.Get(x, y);
            }
        }
        pBoneArray_[i].bindPose = XMLoadFloat4x4(&pose);
        bonePair[ppCluster_[i]->GetLink()->GetName()] = pBoneArray_ + i;
    }
}

// コンスタントバッファ（シェーダーに準備を送るやつ）準備
void FbxParts::IntConstantBuffer()
{
    // 定数バッファの作成（パラメータ受け渡し用）
    D3D11_BUFFER_DESC cb;
    cb.ByteWidth = sizeof(CONSTANT_BUFFER);
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;
    Direct3D::pDevice_->CreateBuffer(&cb, NULL, &pConstantBuffer_);
}

void FbxParts::Draw(Transform& transform)
{
    // 今から描画する頂点情報をシェーダに伝える
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);

    // 使用するコンスタントバッファをシェーダに伝える
    Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);
    Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);

    // シェーダーのコンスタントバッファーに各種データを渡す
    for (DWORD i = 0; i < materialCount_; i++)
    {
        // インデックスバッファーをセット
        UINT stride = sizeof(int);
        UINT offset = 0;
        Direct3D::pContext_->IASetIndexBuffer(ppIndexBuffer_[i], DXGI_FORMAT_R32_UINT, 0);

        // パラメータの受け渡し
        D3D11_MAPPED_SUBRESOURCE pdata;
        CONSTANT_BUFFER cb;
        cb.worldViewProj = XMMatrixTranspose(transform.GetWorldMatrix() * Camera::GetViewMatrix() * Camera::GetProjectionMatrix()); //　リソースへ送る値をセット
        cb.world = XMMatrixTranspose(transform.GetWorldMatrix());

        cb.normalTrans = XMMatrixTranspose(transform.matRotate_ * XMMatrixInverse(nullptr, transform.matScale_));

        cb.ambient = pMaterial_[i].ambient;
        cb.diffuse = pMaterial_[i].diffuse;
        cb.specular = pMaterial_[i].specular;
        cb.shiniess = pMaterial_[i].shininess;
        cb.cameraPosition = XMFLOAT4(Camera::GetPosition().x, Camera::GetPosition().y, Camera::GetPosition().z, 0);
        cb.lightDirection = XMFLOAT4(-1, -1, 1, 0);
        cb.isTexture = pMaterial_[i].pTexture != nullptr;

        Direct3D::pContext_->Map(pConstantBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata); // GPUからのリソースアクセスを一時止める
        memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb)); // リソースへ値を送る

        // テクスチャをシェーダーに設定
        if (cb.isTexture)
        {
            ID3D11SamplerState* pSampler = pMaterial_[i].pTexture->GetSampler();
            Direct3D::pContext_->PSSetSamplers(0, 1, &pSampler);

            ID3D11ShaderResourceView* pSRV = pMaterial_[i].pTexture->GetSRV();
            Direct3D::pContext_->PSSetShaderResources(0, 1, &pSRV);
        }
        Direct3D::pContext_->Unmap(pConstantBuffer_, 0);

        // ポリゴンメッシュを描画する
        Direct3D::pContext_->DrawIndexed(pMaterial_[i].polygonCount * 3, 0, 0);
    }
}

// ボーンありのモデルを描画
void FbxParts::DrawSkinAnime(Transform& transform, FbxTime time)
{
    // ボーンごとの現在の行列を取得する
    for (int i = 0; i < numBone_; i++)
    {
        FbxAnimEvaluator* evaluator = ppCluster_[i]->GetLink()->GetScene()->GetAnimationEvaluator();
        FbxMatrix mCurrentOrentation = evaluator->GetNodeGlobalTransform(ppCluster_[i]->GetLink(), time);

        // 行列コピー（Fbx形式からDirectXへの変換）
        XMFLOAT4X4 pose;
        for (DWORD x = 0; x < 4; x++)
        {
            for (DWORD y = 0; y < 4; y++)
            {
                pose(x, y) = (float)mCurrentOrentation.Get(x, y);
            }
        }

        XMFLOAT4X4 mmat;
        XMMATRIX mMirror;
        mMirror = XMMatrixIdentity();
        XMStoreFloat4x4(&mmat, mMirror);
        mmat.m[2][2] = -1.0f;
        mMirror = XMLoadFloat4x4(&mmat);

        // オフセット時のポーズの差分を計算する
        pBoneArray_[i].newPose = XMLoadFloat4x4(&pose) * mMirror;
        pBoneArray_[i].diffPose = XMMatrixInverse(nullptr, pBoneArray_[i].bindPose * mMirror);
        pBoneArray_[i].diffPose = pBoneArray_[i].diffPose * pBoneArray_[i].newPose;
    }

    // 各ボーンに対応した頂点の変形制御
    for (DWORD i = 0; i < vertexCount_; i++)
    {
        // 各頂点ごとに、「影響するボーン×ウエイト値」を反映させた間接行列を作成する
        XMMATRIX matrix;
        ZeroMemory(&matrix, sizeof(matrix));
        for (int m = 0; m < numBone_; m++)
        {
            if (pWeightArray_[i].pBoneIndex[m] < 0)
            {
                break;
            }
            matrix += pBoneArray_[pWeightArray_[i].pBoneIndex[m]].diffPose * pWeightArray_[i].pBoneWeight[m];
        }

        // 作成された間接行列を使って、頂点を変形する
        XMVECTOR Pos = XMLoadFloat3(&pWeightArray_[i].posOrigin);
        XMVECTOR Normal = XMLoadFloat3(&pWeightArray_[i].normalOrigin);

        XMStoreFloat3(&pVertexData_[i].position, XMVector3TransformCoord(Pos, matrix));
        XMFLOAT3X3 mat33;
        XMStoreFloat3x3(&mat33, matrix);
        XMMATRIX matrix33 = XMLoadFloat3x3(&mat33);
        XMStoreFloat3(&pVertexData_[i].normal, XMVector3TransformCoord(Normal, matrix33));
    }

    // 頂点バッファをロックして、変形させた後の頂点情報で上書きする
    D3D11_MAPPED_SUBRESOURCE msr = {};
    Direct3D::pContext_->Map(pVertexBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    if (msr.pData)
    {
        memcpy_s(msr.pData, msr.RowPitch, pVertexData_, sizeof(VERTEX) * vertexCount_);
        Direct3D::pContext_->Unmap(pVertexBuffer_, 0);
    }

    Draw(transform);
}

void FbxParts::DrawMeshAnime(Transform& transform, FbxTime time, FbxScene* scene)
{
    Draw(transform);
}

bool FbxParts::GetBonePosition(std::string boneName, XMFLOAT3* position)
{
    for (int i = 0; i < numBone_; i++)
    {
        if (boneName == ppCluster_[i]->GetLink()->GetName())
        {
            FbxAMatrix matrix;
            ppCluster_[i]->GetTransformLinkMatrix(matrix);

            position->x = (float)matrix[3][0];
            position->y = (float)matrix[3][1];
            position->z = (float)matrix[3][2];

            return true;
        }
    }
    return false;
}

bool FbxParts::GetBonePositionAtNow(std::string boneName, XMFLOAT3* position)
{
    decltype(bonePair)::iterator it = bonePair.find(boneName);
    if(it != bonePair.end()) // 見つかったら
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, it->second->newPose);
        position->x = m._41;
        position->y = m._42;
        position->z = m._43;

        return true;
    }
    return false;
}

void FbxParts::RayCast(RayCastData* data)
{
    data->hit = FALSE;

    // マテリアル毎
    for (DWORD i = 0; i < materialCount_; i++)
    {
        // そのマテリアルのポリゴン毎
        for (DWORD j = 0; j < pMaterial_[i].polygonCount; j++)
        {
            // 3頂点
            XMFLOAT3 ver[3];
            ver[0] = pVertexData_[ppIndexData_[i][j * 3 + 0]].position;
            ver[1] = pVertexData_[ppIndexData_[i][j * 3 + 1]].position;
            ver[2] = pVertexData_[ppIndexData_[i][j * 3 + 2]].position;

            BOOL hit = FALSE;
            float dist = 0.0f;

            hit = Direct3D::Intersect(data->start, data->dir, ver[0], ver[1], ver[2], &dist);

            if (hit && dist < data->dist)
            {
                data->hit = TRUE;
                data->dist = dist;
            }
        }
    }
}
