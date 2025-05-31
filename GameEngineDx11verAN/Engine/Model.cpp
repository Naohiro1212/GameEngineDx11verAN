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
            pData->pFbx = new Fbx;
            if (FAILED(pData->pFbx->Load(fileName)))
            {
                // 開けなかった
                SAFE_DELETE(pData->pFbx);
                SAFE_DELETE(pData);
                return -1;
            }

            // 無事開けた
            pData->fileName = fileName;
        }

        // 使ってない番号がないか探す
        for (int i = 0; i < _datas.size(); i++)
        {
            if (_datas[i] == nullptr)
            {
                _datas[i] = pData;
                return i;
            }
        }

        // 新たに追加
        _datas.push_back(pData);
        return (int)_datas.size() - 1;
    }

    // 描画
    void Draw(int handle)
    {
        // handleが不正な値かデータが存在しない場合に処理を中断する
        if (handle < 0 || handle >= _datas.size() || _datas[handle] == nullptr)
        {
            return;
        }

        // アニメーションを進める
        _datas[handle]->nowFrame += _datas[handle]->animSpeed;

        // 最後までアニメーションしたら戻す
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

        // 同じモデルをほかでも使っていないか
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