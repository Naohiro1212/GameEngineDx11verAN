#pragma once
#include <DirectXMath.h>
#include <list>
#include <string>
#include <assert.h>
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Transform.h"

using namespace DirectX;

// すべてのゲームオブジェクト（シーンも含めて）が継承するインターフェース
// ゲームオブジェクトは親子構造になっていて
// マトリクスの影響を受けることになる
class GameObject
{
protected:

	// 位置や向きなどを管理するオブジェクト
	Transform transform_;

	// オブジェクトの名前
	std::string objectName_;

	// 衝突判定リスト
	std::list<Collider*> colliderList_;

public:
	// コンストラクタ
	GameObject();
	GameObject(GameObject* parent);
	GameObject(GameObject* parent, const std::string& name);

	// デストラクタ
	virtual ~GameObject();

	// 各オブジェクトで必ず作る関数
	virtual void Initialize(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw() = 0;
	virtual void Release(void) = 0;

	// 自分の該当関数を呼んだ後、子供の関数も呼ぶ
	void UpdateSub();
	void DrawSub();
	void ReleaseSub();

	// ワールド行列の取得（親の影響を受けた最終的な行列)
	// 戻り値：ワールド行列
	XMMATRIX GetWorldMatrix();

	// 各フラグの制御
	bool IsDead();			// 削除するかどうか
	void KillMe();			// 自分を削除する
	void Enter();			// Updateを許可
	void Leave();			// Updateを拒否
	void Visible();			// Drawを許可
	void Invisible();		// Drawを拒否
	bool IsInitialized();	// 初期化済みかどうか
	bool SetInitialized();  // 初期化済みにする
	bool IsEntered();       // Update実行していいか
	bool IsVisibled();      // Draw実行していいか

	// 子オブジェクトリストを取得
	// 戻り値：子オブジェクトリスト
	std::list<GameObject*>* GetChildList();

	// 親オブジェクトを取得
	// 戻り値：親オブジェクトのアドレス
	GameObject* GetParent();

	// 名前でオブジェクトを検索（対象は自分の子供以下）
	// 引数：name 検索する名前
	// 戻り値：見つけたオブジェクトのアドレス（見つからなければnullptr)
	GameObject* FindChildObject(const std::string& name);

	// 名前でオブジェクトを検索（対象は全体）
	// 引数：検索する名前
	// 戻り値：見つけたオブジェクトのアドレス
	GameObject* FindObject(const std::string& name) { return GetRootJob()->FindChildObject(name); }

	// オブジェクトの名前を取得
	// 戻り値：名前
	const std::string& GetObjectName(void) const;

	// 子オブジェクトを追加（リストの最後へ）
	// 引数：obj 追加するオブジェクト
	void PushBackChild(GameObject* obj);

	// 子オブジェクトを追加（リストの最後へ）
	// 引数：obj 追加するオブジェクト
	void PushFrontChild(GameObject* obj);

	// 子オブジェクトを全て削除 
	void KillAllChildren();

	//コライダー（衝突判定）を追加する
	void AddCollider(Collider* collider);

	//何かと衝突した場合に呼ばれる（オーバーライド用）
	//引数：pTarget	衝突した相手
	virtual void OnCollision(GameObject* pTarget) {};

	//コライダー（衝突判定）を削除
	void ClearCollider();

	//衝突判定
	//引数：pTarget	衝突してるか調べる相手
	void Collision(GameObject* pTarget);

	// テスト用の衝突判定枠を表示
	void CollisionDraw();

	// RootJobを取得
	GameObject* GetRootJob();

	// 各アクセス関数
	XMFLOAT3 GetPosition() { return transform_.position_; }
	XMFLOAT3 GetRotate() { return transform_.rotate_; }
	XMFLOAT3 GetScale() { return transform_.scale_; }
	XMFLOAT3 GetWorldPosition() { return Transform::Float3Add(GetParent()->transform_.position_, transform_.position_); }
	XMFLOAT3 GetWorldRotate() { return Transform::Float3Add(GetParent()->transform_.rotate_, transform_.rotate_); }
	XMFLOAT3 GetWorldScale() { return Transform::Float3Add(GetParent()->transform_.scale_, transform_.scale_); }
	void SetPosition(XMFLOAT3 position) { transform_.position_ = position; }
	void SetPosition(float x, float y, float z) { SetPosition(XMFLOAT3(x, y, z)); }
	void SetRotate(XMFLOAT3 rotate) { transform_.rotate_ = rotate; }
	void SetRotate(float x, float y, float z) { SetRotate(XMFLOAT3(x, y, z)); }
	void SetRotateX(float x) { SetRotate(x, transform_.rotate_.y, transform_.rotate_.z); }
	void SetRotateY(float y) { SetRotate(transform_.rotate_.x, y, transform_.rotate_.z); }
	void SetRotateZ(float z) { SetRotate(transform_.rotate_.x, transform_.rotate_.y, z); }
	void SetScale(XMFLOAT3 scale) { transform_.scale_ = scale; }
	void SetScale(float x, float y, float z) { SetScale(XMFLOAT3(x, y, z)); }
	void SetScaleMatrix(XMMATRIX& m);
	void SetRotateMatrix(XMMATRIX& m);
	void SetTranslateMatrix(XMMATRIX& m);

private:
	// オブジェクト削除（再帰）
	// 引数：obj 削除するオブジェクト
	void KillObjectSub(GameObject* obj);

private:
	// フラグ
	struct OBJECT_STATE
	{
		unsigned initialized : 1; // 初期化
		unsigned entered : 1;     // 更新するか
		unsigned visible : 1;	  // 描画するか
		unsigned dead : 1;        // 削除するか
	};
	OBJECT_STATE state_;

	// 親オブジェクト
	GameObject* pParent_;

	// 子オブジェクトリスト
	std::list<GameObject*> childList_;
};

// オブジェクトを作成するテンプレート
template <class T>
T* Instantiate(GameObject* pParent)
{
	T* pNewObject = new T(pParent);
	if (pParent != nullptr)
	{
		pParent->PushBackChild(pNewObject);
	}
	pNewObject->Initialize();
	return pNewObject;
}