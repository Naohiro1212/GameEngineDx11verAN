#include "SceneManager.h"

#include "Model.h"
#include "Image.h"
#include "Audio.h"

// コンストラクタ
SceneManger::SceneManger(GameObject* parent) : GameObject(parent, "SceneManager")
{
}

// 初期化
void SceneManger::Initialize()
{
	// 最初のシーンを準備
	currentSceneID_ = SCENE_ID_TEST;
	nextSceneID_ = currentSceneID_;
	Instantiate<TestScene>(this);
}

// 更新
void SceneManger::Update()
{
	// 次のシーンが現在のシーンと違う = シーンを切り替えなければならない
	if (currentSceneID_ != nextSceneID_)
	{
		// そのシーンのオブジェクトを全削除
		KillAllChildren();

		// ロードしたデータを全削除
		Audio::Release();
		Model::AllRelease();
		Image::AllRelease();

		// 次のシーンを作成
		switch (nextSceneID_)
		{
		case SCENE_ID_TEST:
			Instantiate<TestScene>(this); break;
		}
		Audio::Initialize();
		currentSceneID_ = nextSceneID_;
	}
}

// 描画
void SceneManger::Draw()
{
}

void SceneManger::Release()
{
}

// シーン切り替え（実際に切り替わるのはこの次のフレーム)
void SceneManger::ChangeScene(SCENE_ID next)
{
	nextSceneID_ = next;
}
