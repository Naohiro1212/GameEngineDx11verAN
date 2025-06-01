#pragma once
#include <vector>
#include <string>
#include "Sprite.h"
#include "Transform.h"

// 2D画像を管理する
namespace Image
{
	// 画像情報
	struct ImageData
	{
		// ファイル名
		std::string fileName;

		// ロードした画像データのアドレス
		Sprite* pSprite;
	};
}