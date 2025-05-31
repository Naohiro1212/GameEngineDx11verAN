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
            pData->pFbx = new Fbx;
            if (FAILED(pData->pFbx->Load(fileName)))
            {
                // �J���Ȃ�����
                SAFE_DELETE(pData->pFbx);
                SAFE_DELETE(pData);
                return -1;
            }

            // �����J����
            pData->fileName = fileName;
        }

        // �g���ĂȂ��ԍ����Ȃ����T��
        for (int i = 0; i < _datas.size(); i++)
        {
            if (_datas[i] == nullptr)
            {
                _datas[i] = pData;
                return i;
            }
        }

        // �V���ɒǉ�
        _datas.push_back(pData);
        return (int)_datas.size() - 1;
    }

    // �`��
    void Draw(int handle)
    {
        // handle���s���Ȓl���f�[�^�����݂��Ȃ��ꍇ�ɏ����𒆒f����
        if (handle < 0 || handle >= _datas.size() || _datas[handle] == nullptr)
        {
            return;
        }

        // �A�j���[�V������i�߂�
        _datas[handle]->nowFrame += _datas[handle]->animSpeed;

        // �Ō�܂ŃA�j���[�V����������߂�
        if (_datas[handle]->nowFrame > (float)_datas[handle]->endFrame)
        {
            _datas[handle]->nowFrame = (float)_datas[handle]->startFrame;
        }

        if (_datas[handle]->pFbx)
        {
            _datas[handle]->pFbx->Draw(_datas[handle]->transform, (int)_datas[handle]->nowFrame);
        }
    }

    void Release(int handle)
    {
        if (handle < 0 || handle >= _datas.size() || _datas[handle] == nullptr)
        {
            return;
        }

        // �������f�����ق��ł��g���Ă��Ȃ���
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
}