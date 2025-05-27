#pragma once
#include <d3d11.h>
#include <fbxsdk.h>
#include <vector>
#include <string>
#include "Transform.h"

class FbxParts;

// ���C�L���X�g�p�\����
struct RayCastData
{
	XMFLOAT3 start; // ���C���ˈʒu
	XMFLOAT3 dir;   // ���C�̌����x�N�g��
	float dist;     // �Փ˓_�܂ł̋���
	BOOL hit;       // ���C������������
	XMFLOAT3 normal;// �@��

	RayCastData() { dist = 99999.0f; }
};

// FBX�t�@�C���������N���X
// �قƂ�ǂ̏����͊e�p�[�c

class Fbx
{
	// FbxPart�N���X���t�����h�N���X�ɂ���
	// FbxPart��private�Ȋ֐��ɂ��A�N�Z�X�\
	friend class FbxParts;

	// ���f���̊e�p�[�c�i�������邩���j
	std::vector<FbxParts*> parts_;

	// FBX�t�@�C���������@�\�̖{��
	FbxManager* pFbxManager_;

	// FBX�t�@�C���̃V�[���iMaya�ō�������ׂĂ̕��́j������
	FbxScene* pFbxScene_;

	// �A�j���[�V�����̃t���[�����[�g
	FbxTime::EMode _frameRate;

	// �A�j���[�V�������x
	float _animSpeed;

	// �A�j���[�V�����̍ŏ��ƍŌ�̃t���[��
	int _startFrame, _endFrame;

	// �m�[�h�̒��g�𒲂ׂ�
	// �����FpNode ���ׂ�m�[�h
	// �����FpPartsList �p�[�c�̃��X�g
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

	// ���[�h
};