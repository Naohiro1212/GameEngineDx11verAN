#pragma once
#include "Global.h"
#include "GameObject.h"

// ゲームに登場するシーン
enum SCENE_ID
{
	SCENE_ID_TEST = 0,
};

// シーン切り替えを担当するオブジェクト
class SceneManger : public GameObject
{
public:
	// コンストラクタ
	// 引数：parent 親オブジェクト（基本的にゲームマネージャー）
	SceneManger(GameObject* parent);

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Release() override;

	// シーン切り替え
	// 引数：next 次のシーンのID
	void ChangeScene(SCENE_ID next);

private:
	SCENE_ID currentSceneID_; // 現在のシーン
	SCENE_ID nextSceneID_;    // 次のシーン
};