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
        bool isExist = false;
        for (int i = 0; i < _datas.size(); i++)
        {
            // すでに開いている場合
            if (_datas[i] != nullptr && i != handle && _datas[i]->pFbx == _datas[handle]->pFbx)
            {
                isExist = true;
                break;
            }
        }

        // 使ってなければモデル解放
        if (isExist == false)
        {
            SAFE_DELETE(_datas[handle]->pFbx);
        }

        SAFE_DELETE(_datas[handle]);
    }

    void AllRelease()
    {
        for (int i = 0; i < _datas.size(); i++)
        {
            if (_datas[i] != nullptr)
            {
                Release(i);
            }
        }
        _datas.clear();
    }

    // アニメーションのフレーム数をセット
    void SetAnimFrame(int handle, int startFrame, int endFrame, float animSpeed)
    {
        _datas[handle]->SetAnimFrame(startFrame, endFrame, animSpeed);
    }

    // 現在のアニメーションのフレームを取得
    int GetAnimFrame(int handle)
    {
        return (int)_datas[handle]->nowFrame;
    }

    // 任意のボーンの位置を取得
    XMFLOAT3 GetBonePosition(int handle, std::string boneName)
    {
        XMFLOAT3 pos = _datas[handle]->pFbx->GetBonePosition(boneName);
        XMVECTOR vec = XMVector3TransformCoord(XMLoadFloat3(&pos), _datas[handle]->transform.GetWorldMatrix());
        XMStoreFloat3(&pos, vec);
        return pos;
    }

    XMFLOAT3 GetAnimBonePosition(int handle, std::string boneName)
    {
        XMFLOAT3 pos = _datas[handle]->pFbx->GetAnimBonePosition(boneName);
        XMVECTOR vec = XMVector3TransformCoord(XMLoadFloat3(&pos), _datas[handle]->transform.GetWorldMatrix());
        XMStoreFloat3(&pos, vec);
        return pos;
    }

    // ワールド行列を設定
    void SetTransform(int handle, Transform& transform)
    {
        if (handle < 0 || handle >= _datas.size())
        {
            return;
        }
        _datas[handle]->transform = transform;
    }

    XMMATRIX GetMatrix(int handle)
    {
        return _datas[handle]->transform.GetWorldMatrix();
    }

    void RayCast(int handle, RayCastData* data)
    {
        XMFLOAT3 target = Transform::Float3Add(data->start, data->dir);
        XMMATRIX matInv = XMMatrixInverse(nullptr, _datas[handle]->transform.GetWorldMatrix());
        XMVECTOR vecStart = XMVector3TransformCoord(XMLoadFloat3(&data->start), matInv);
        XMVECTOR vecTarget = XMVector3TransformCoord(XMLoadFloat3(&target), matInv);
        XMVECTOR vecDir = vecTarget - vecStart;

        XMStoreFloat3(&data->start, vecStart);
        XMStoreFloat3(&data->dir, vecDir);

        _datas[handle]->pFbx->RayCast(data);
    }
}