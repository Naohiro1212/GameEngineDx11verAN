#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "string"
#include "Texture.h"
#include "Transform.h"

using namespace DirectX;

// 2D�摜��\�����邽�߂̃N���X�i���ۂɂ̓|���S���𕽖ʓI�ɕ\�����Ă��邾��
class Sprite
{
protected:
	// ��̒��_�����i�[����\����
	struct VERTEX
	{
		XMFLOAT3 position;
		XMFLOAT3 uv;
	};

	// �y�R���X�^���g�o�b�t�@�[�z
	// GPU�i�V�F�[�_���j�֑��鐔�l���܂Ƃ߂��\����
	// Simple2D.hlsl�̃O���[�o���ϐ��ƑΉ�������
	struct CONSTANT_BUFFER
	{
		XMMATRIX world;     // ���_���W�ϊ��s��
		XMMATRIX uvTrans;   // �e�N�X�`�����W�ϊ��s��
		XMFLOAT4 color;     // �e�N�X�`���Ƃ̍����F
	};

	// �y���_�o�b�t�@�z
	// �e���_�̏��i�ʒu�ƐF�Ƃ��j���i�[����Ƃ���
	// ���_�����̔z��ɂ��Ďg��
	ID3D11Buffer* pVertexBuffer_;

	// �y�C���f�b�N�X�o�b�t�@�z
	// �u�ǂ̒��_�v�Ɓu�ǂ̒��_�v�Ɓu�ǂ̒��_�v��3�p�`�|���S���ɂȂ邩�̏����i�[����Ƃ���
	ID3D11Buffer* pIndexBuffer_;

	// �y�e�N�X�`���z
	Texture* pTexture_;

	// �y�萔�o�b�t�@�z
	// �V�F�[�_�[�iSimple2D.hlsl�j�̃O���[�o���ϐ��ɒl��n�����߂̂���
	ID3D11Buffer* pConstantBuffer_;

	// private�֐�
	// Load�֐�����Ă΂��
	void InitVertex(); // ���_�o�b�t�@����
	void InitIndex();  // �C���f�b�N�X�o�b�t�@����
	void InitConstantBuffer(); // �R���X�^���g�o�b�t�@�i�V�F�[�_�[�ɏ����𑗂��j����

public:
	Sprite();
	~Sprite();

	// 


};