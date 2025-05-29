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

// �f�X�g���N�^
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

// FBX�t�@�C������������[�h���ď��X��������
HRESULT FbxParts::Init(FbxNode* pNode)
{
    // �m�[�h���烁�b�V���̏����擾
    FbxMesh* mesh = pNode->GetMesh();

    mesh->SplitPoints(FbxLayerElement::eTextureDiffuse);

    // �e���̌����擾
    vertexCount_ = mesh->GetControlPointsCount(); // ���_�̐�
    polygonCount_ = mesh->GetPolygonCount();      // �|���S���̐�
    polygonVertexCount_ = mesh->GetPolygonVertexCount(); // �|���S�����_�C���f�b�N�X��

    InitVertex(mesh);    // ���_�o�b�t�@����
    InitMaterial(pNode); // �}�e���A������
    InitIndex(mesh);     // �C���f�b�N�X�o�b�t�@����
    InitSkelton(mesh);   // ���̏�������
	IntConstantBuffer();  // �R���X�^���g�o�b�t�@�i�V�F�[�_�[�ɏ����𑗂��j����tConstant

    return E_NOTIMPL;
}

HRESULT FbxParts::Init(FbxMesh* pMesh)
{
    // ���b�V���̏����擾

    // ���b�V���̃R���g���[���|�C���g���}�e���A�����x�[�X�ɕ�������
    pMesh->SplitPoints(FbxLayerElement::eTextureDiffuse);

	vertexCount_ = pMesh->GetControlPointsCount(); // ���_�̐�
	polygonCount_ = pMesh->GetPolygonCount();      // �|���S���̐�
	polygonVertexCount_ = pMesh->GetPolygonVertexCount(); // �|���S�����_�C���f�b�N�X��

    InitVertex(pMesh);   // ���_�o�b�t�@����
	InitMaterial(pMesh); // �}�e���A������
    InitIndex(pMesh);    // �C���f�b�N�X�o�b�t�@����
	InitSkelton(pMesh);  // ���̏�������
    IntConstantBuffer(); // �R���X�^���g�o�b�t�@�i�V�F�[�_�[�ɏ����𑗂��j����

    return E_NOTIMPL;
}

void FbxParts::InitVertex(fbxsdk::FbxMesh* mesh)
{
    pVertexData_ = new VERTEX[vertexCount_];

    for (DWORD poly = 0; poly < polygonCount_; poly++)
    {
        // 3���_��
        for (int vertex = 0; vertex < 3; vertex++)
        {
            int index = mesh->GetPolygonVertex(poly, vertex);

            // ���_�̈ʒu
            FbxVector4 pos = mesh->GetControlPointAt(index);
            pVertexData_[index].position = XMFLOAT3((float)pos[0], (float)pos[1], -(float)pos[2]);

            // ���_�̖@��
            FbxVector4 Normal;
            mesh->GetPolygonVertexNormal(poly, vertex, Normal); // i�Ԗڂ̃|���S���́Aj�Ԗڂ̖@�����Q�b�g
            pVertexData_[index].normal = XMFLOAT3((float)Normal[0], (float)Normal[1], -(float)Normal[2]);

            // ���_��UV
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

    // ���_�f�[�^�p�o�b�t�@�̐ݒ�
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

// �}�e���A������
void FbxParts::InitMaterial(fbxsdk::FbxNode* pNode)
{
    // �}�e���A���o�b�t�@�̐���
    materialCount_ = pNode->GetMaterialCount();
    pMaterial_ = new MATERIAL[materialCount_];

    for (DWORD i = 0; i < materialCount_; i++)
    {
        ZeroMemory(&pMaterial_[i], sizeof(pMaterial_[i]));

        // �t�H���V�F�[�f�B���O��z�肵���}�e���A���o�b�t�@�̒��o
        FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

        FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

        // �������g�U���ˌ������ʔ��ˌ��̔��ː����l���擾
        FbxDouble3 ambient = FbxDouble3(0, 0, 0);
        FbxDouble3 diffuse = FbxDouble3(0, 0, 0);
        FbxDouble3 specular = FbxDouble3(0, 0, 0);
        // Ambient�̃v���p�e�B��������
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

        // �������g�U���ˌ������ʔ��ˌ��̔��ː����l���}�e���A���o�b�t�@�ɃR�s�[
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
    // �}�e���A���o�b�t�@�̐���
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
