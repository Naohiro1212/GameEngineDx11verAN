#pragma once
#include <vector>
#include <string>
#include "Sprite.h"
#include "Transform.h"

// 2D�摜���Ǘ�����
namespace Image
{
	// �摜���
	struct ImageData
	{
		// �t�@�C����
		std::string fileName;

		// ���[�h�����摜�f�[�^�̃A�h���X
		Sprite* pSprite;
	};
}