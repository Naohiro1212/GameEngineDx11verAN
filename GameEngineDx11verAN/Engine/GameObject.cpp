#include "GameObject.h"
#include <assert.h>
#include "Global.h"

// コンストラクタ（親も名前もなし）
GameObject::GameObject(void) : 
    GameObject(nullptr, "")
{

}

// コンストラクタ（名前なし）
GameObject::GameObject(GameObject* parent) : 
    GameObject(parent, "")
{
}

// コンストラクタ（標準）
GameObject::GameObject(GameObject* parent, const std::string& name) : pParent_(parent),objectName_(name)
{
    childList_.clear();
    state_ = { 0, 1, 1, 0 };
    if (parent)
    {
        transform_.pParent_ = &parent->transform_;
    }
}

// デストラクタ
GameObject::~GameObject()
{
    for (auto it = colliderList_.begin(); it != colliderList_.end();it++)
    {
        SAFE_DELETE(*it);
    }
    colliderList_.clear();
}

// 削除するかどうか
bool GameObject::IsDead()
{
    return (state_.dead != 0);
}

// 自分を削除する
void GameObject::KillMe()
{
    state_.dead = 1;
}

// Updateを許可
void GameObject::Enter()
{
    state_.entered = 1;
}

// Updateを拒否
void GameObject::Leave()
{
    state_.entered = 0;
}

// Drawを許可
void GameObject::Visible()
{
    state_.visible = 1;
}

// Drawを拒否
void GameObject::Invisible()
{
    state_.visible = 0;
}

// 初期化済みかどうか
bool GameObject::IsInitialized()
{
    return (state_.initialized != 0);
}

// 初期化済みにする
bool GameObject::SetInitialized()
{
    state_.initialized = 1;
}

// Update実行していいか
bool GameObject::IsEntered()
{
    return (state_.entered != 0);
}

// Draw実行していいか
bool GameObject::IsVisibled()
{
    return (state_.visible != 0);
}

// 子オブジェクトリストを取得
std::list<GameObject*>* GameObject::GetChildList()
{
    return &childList_;
}

// 親オブジェクトを取得
GameObject* GameObject::GetParent()
{
    return pParent_;
}

// 名前でオブジェクトを検索（対象は自分の子供以下）
GameObject* GameObject::FindChildObject(const std::string& name)
{
    // 子供がいないなら終わり
    if (childList_.empty())
    {
        return nullptr;
    }

    // イテレータ
    auto it = childList_.begin(); // 先頭
    auto end = childList_.end();  // 末尾

    // 子オブジェクトから探す
    while (it != end)
    {
        // 同じ名前のオブジェクトを見つけたらそれを返す
        if ((*it)->GetObjectName() == name)
        {
            return *it;
        }

        // その子供（孫）以降にいないか探す
        GameObject* obj = (*it)->FindChildObject(name);
        if (obj != nullptr)
        {
            return obj;
        }

        // 次の子へ
        it++;
    }
    
    // 見つからなかった
    return nullptr;
}

// オブジェクトの名前を取得
const std::string& GameObject::GetObjectName(void) const
{
    return objectName_;
}

// 子オブジェクトを追加（リストの最後へ）
void GameObject::PushBackChild(GameObject* obj)
{
    assert(obj != nullptr);
    obj->pParent_ = this;
    childList_.push_back(obj);
}

// 子オブジェクトを追加（リストの先頭へ）
void GameObject::PushFrontChild(GameObject* obj)
{
    assert(obj != nullptr);
    obj->pParent_ = this;
    childList_.push_front(obj);
}

// 子オブジェクトを全て削除
void GameObject::KillAllChildren(void)
{
    // 子供がいないなら終わり
    if (childList_.empty())
    {
        return;
    }

    // イテレータ
    auto it = childList_.begin(); // 先頭
    auto end = childList_.end();  // 末尾

    // 子オブジェクトを1個ずつ削除
    while (it != end)
    {
        KillObjectSub(*it);
        delete* it;
        it = childList_.erase(it);
    }

    // リストをクリア
    childList_.clear();
}

void GameObject::SetScaleMatrix(XMMATRIX& m)
{
    transform_.matScale_ = m;
    transform_.isSetDirect = true;
}



void GameObject::AddCollider(Collider* collider)
{
}
