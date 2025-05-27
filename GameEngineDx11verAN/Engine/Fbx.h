#pragma once
#include <d3d11.h>
#include <fbxsdk.h>
#include <vector>
#include <string>
#include "Transform.h"

class FbxParts;

// レイキャスト用構造体
struct RayCastData
{
	XMFLOAT3 start; // レイ発射位置
	XMFLOAT3 dir;   // レイの向きベクトル
	float dist;     // 衝突点までの距離
	BOOL hit;       // レイが当たったか
	XMFLOAT3 normal;// 法線

	RayCastData() { dist = 99999.0f; }
};

// FBXファイルを扱うクラス
// ほとんどの処理は各パーツ

class Fbx
{
	// FbxPartクラスをフレンドクラスにする
	// FbxPartのprivateな関数にもアクセス可能
	friend class FbxParts;

	// モデルの各パーツ（複数あるかも）
	std::vector<FbxParts*> parts_;

	// FBXファイルを扱う機能の本体
	FbxManager* pFbxManager_;

	// FBXファイルのシーン（Mayaで作ったすべての物体）を扱う
	FbxScene* pFbxScene_;

	// アニメーションのフレームレート
	FbxTime::EMode _frameRate;

	// アニメーション速度
	float _animSpeed;

	// アニメーションの最初と最後のフレーム
	int _startFrame, _endFrame;

	// ノードの中身を調べる
	// 引数：pNode 調べるノード
	// 引数：pPartsList パーツのリスト
	void CheckNode(FbxNode* pNode, std::vector<FbxParts*>* pPartsList);

public:
	Fbx();
	~Fbx();

	FbxManager* GetFbxManager()
	{
		return pFbxManager_;
	}
	FbxScene* GetFbxScene()
	{
		return pFbxScene_;
	}

	// ロード
};