#include "Global.h"
#include "Model.h"

// 3Dモデル（FBXファイル）を管理する
namespace Model
{
    // ロード済みのモデルデータ一覧
    std::vector<ModelData*> _datas;

    // 初期化
    void Initialize()
    {
        AllRelease();
    }

    // モデルをロード
    int Load(std::string fileName)
    {
        ModelData* pData = new ModelData;

        // 開いたファイル一覧から同じファイル名のものがないか探す
        bool isExist = false;
        for (int i = 0; i < _datas.size(); i++)
        {
            // 既に開いている場合
            if (_datas[i] != nullptr && _datas[i]->fileName == fileName)
            {
                pData->pFbx = _datas[i]->pFbx;
                isExist = true;
                break;
            }
        }

        // 新たにファイルを開く
        if (isExist == false)
        {
            // fbx.h一旦書きます
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
