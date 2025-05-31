#include "Fbx.h"
#include "Direct3D.h"
#include "FbxParts.h"

#pragma comment(lib, "LibFbxSDK-MT.lib")
#pragma comment(lib, "LibXml2-MT.lib")
#pragma comment(lib, "zlib-MT.lib")

Fbx::Fbx():_animSpeed(0)
{
}

Fbx::~Fbx()
{
    for (int i = 0l; i < parts_.size(); i++)
    {
        delete parts_[i];
    }
    parts_.clear();

    pFbxScene_->Destroy();
    pFbxManager_->Destroy();
}

HRESULT Fbx::Load(std::string fileName)
{
    // FBX�̓ǂݍ���
    pFbxManager_ = FbxManager::Create();
    pFbxScene_ = FbxScene::Create(pFbxManager_, "fbxscene");
    FbxString FileName(fileName.c_str());
    FbxImporter* fbxImporter = FbxImporter::Create(pFbxManager_, "imp");

    if (!fbxImporter->Initialize(FileName.Buffer(), -1, pFbxManager_->GetIOSettings()));
    {
        // ���s
        return E_FAIL;
    }

    fbxImporter->Import(pFbxScene_);
    fbxImporter->Destroy();

    FbxGeometryConverter geometryConverter(pFbxManager_);

    // �A�j���[�V�����̃^�C�����[�h�̎擾
    _frameRate = pFbxScene_->GetGlobalSettings().GetTimeMode();

    //���݂̃J�����g�f�B���N�g�����o���Ă���
    char defaultCurrentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, defaultCurrentDir);

    // �J�����g�f�B���N�g�����t�@�C�����������ꏊ�ɕύX
    char dir[MAX_PATH];
    _splitpath_s(fileName.c_str(), nullptr, 0, dir, MAX_PATH, nullptr, 0, nullptr, 0);
    SetCurrentDirectoryA(dir);

    int meshCount = pFbxScene_->GetSrcObjectCount<FbxMesh>();
    for (int i = 0; i < meshCount; ++i)
    {
        // ���ꂾ���ł��ׂẴ��b�V���f�[�^���擾�ł���c�͂�
        FbxMesh* mesh = pFbxScene_->GetSrcObject<FbxMesh>(i);
        // �p�[�c��p��
        FbxParts* pParts = new FbxParts(this);
        pParts->Init(mesh);

        // �p�[�c���𓮓I�z��ɒǉ�
        parts_.push_back(pParts);
    }

    // �J�����g�f�B���N�g�������̈ʒu�ɖ߂�
    SetCurrentDirectoryA(defaultCurrentDir);

    return S_OK;
}

void Fbx::CheckNode(FbxNode* pNode, std::vector<FbxParts*>* pPartsList)
{
    // ���b�V����񂪂��邩�ǂ���
    FbxNodeAttribute* attr = pNode->GetNodeAttribute();
    if (attr != nullptr && attr->GetAttributeType() == FbxNodeAttribute::eMesh)
    {
        // �p�[�c��p��
        FbxParts* pParts = new FbxParts(this);
        pParts->Init(pNode);

        // �p�[�c���𓮓I�z��ɒǉ�
        pPartsList->push_back(pParts);
    }

    // �q�̃m�[�h�̃f�[�^�����ׂ�
    {
        // �q���̐��𐔂���
        int childCount = pNode->GetChildCount();

        // ��l���`�F�b�N
        for (int i = 0; i < childCount; i++)
        {
            CheckNode(pNode->GetChild(i), pPartsList);
        }
    }
}

void Fbx::Release()
{
}

XMFLOAT3 Fbx::GetBonePosition(std::string boneName)
{
    XMFLOAT3 position = XMFLOAT3(0, 0, 0);
    for (int i = 0; i < parts_.size(); i++)
    {
        if (parts_[i]->GetBonePosition(boneName, &position))
            break;
    }
    return position;
}

XMFLOAT3 Fbx::GetAnimBonePosition(std::string boneName)
{
    XMFLOAT3 position = XMFLOAT3(0, 0, 0);
    for (int i = 0; i < parts_.size(); i++)
    {
        if (parts_[i]->GetBonePositionAtNow(boneName, &position))
            break;
    }
    return position;
}

void Fbx::Draw(Transform& transform, int frame)
{
    Direct3D::SetBlendMode(Direct3D::BLEND_DEFAULT);

    // �p�[�c������`��
    for (int k = 0; k < parts_.size(); k++)
    {
        // ���̏u�Ԃ̎����̎p���s��𓾂�
        FbxTime time;
        time.SetTime(0, 0, 0, frame, 0, 0, _frameRate);

        // �X�L���A�j���[�V�����i�{�[������j�̏ꍇ
        if (parts_[k]->GetSkinInfo() != nullptr)
        {
            parts_[k]->DrawSkinAnime(transform, time);
        }

        // ���b�V���A�j���[�V�����̏ꍇ
        else
        {
            parts_[k]->DrawMeshAnime(transform, time, pFbxScene_);
        }
    }
}

// ���C�L���X�g�i���C���΂��ē����蔻��j
void Fbx::RayCast(RayCastData* data)
{
    // ���ׂẴp�[�c�Ɣ���
    for (int i = 0; i < parts_.size(); i++)
    {
        parts_[i]->RayCast(data);
    }
}
