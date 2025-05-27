#include "Global.h"
#include "Model.h"

// 3D���f���iFBX�t�@�C���j���Ǘ�����
namespace Model
{
    // ���[�h�ς݂̃��f���f�[�^�ꗗ
    std::vector<ModelData*> _datas;

    // ������
    void Initialize()
    {
        AllRelease();
    }

    // ���f�������[�h
    int Load(std::string fileName)
    {
        ModelData* pData = new ModelData;

        // �J�����t�@�C���ꗗ���瓯���t�@�C�����̂��̂��Ȃ����T��
        bool isExist = false;
        for (int i = 0; i < _datas.size(); i++)
        {
            // ���ɊJ���Ă���ꍇ
            if (_datas[i] != nullptr && _datas[i]->fileName == fileName)
            {
                pData->pFbx = _datas[i]->pFbx;
                isExist = true;
                break;
            }
        }

        // �V���Ƀt�@�C�����J��
        if (isExist == false)
        {
            // fbx.h��U�����܂�
        }
    }
}

void Model::Release(int handle)
{
}

void Model::AllRelease()
{
}

void Model::SetAnimFrame(int handle, int startFrame, int endFrame, float animSpeed)
{
}

int Model::GetAnimFrame(int handle)
{
    return 0;
}

XMFLOAT3 Model::GetBonePosition(int handle, std::string boneName)
{
    return XMFLOAT3();
}

XMFLOAT3 Model::GetAnimBonePosition(int handle, std::string boneName)
{
    return XMFLOAT3();
}

void Model::SetTransform(int handle, Transform& transform)
{
}

XMMATRIX Model::GetMatrix(int handle)
{
    return XMMATRIX();
}

void Model::RayCast(int handle, RayCastDeta* data)
{
}
