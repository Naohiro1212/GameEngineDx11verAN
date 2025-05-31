#pragma once
#include <fbxsdk.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Texture.h"
#include "Transform.h"
#include <string>
#include <unordered_map>

using namespace DirectX;

class Fbx;
struct RayCastData;
using std::string;
using std::unordered_map;

// FBX�̈�̃p�[�c�������N���X
class FbxParts
{
	// ��̒��_�����i�[����\����
	struct VERTEX
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 uv;
	};

	// �y�R���X�^���g�o�b�t�@�[�z
	// GPU�i�V�F�[�_���j�֑��鐔�l���܂Ƃ߂��\����
	// Simple3D.hlsl�̃O���[�o���ϐ��ƑΉ�������
	struct CONSTANT_BUFFER
	{
		XMMATRIX worldViewProj; // ���[���h�A�r���[�A�v���W�F�N�V�����s����|�����킹�����́i���_�ϊ��Ɏg�p�j
		XMMATRIX normalTrans;   // ��]�s��Ɗg��s��̋t�s��������������́i�@���̕ό`�Ɏg�p�j
		XMMATRIX world;         // ���[���h�s��
		XMFLOAT4 lightDirection;      // ���C�g�̌���
		XMFLOAT4 diffuse;       // �f�B�t���[�Y�J���[�A�}�e���A���̐F�i�e�N�X�`���𒣂��Ă��鎞�͎g��Ȃ��j
		XMFLOAT4 ambient;       // �A���r�G���g
		XMFLOAT4 specular;      // �X�؃L�����[�iLambert�̏ꍇ��0�j
		XMFLOAT4 cameraPosition;// �J�����̈ʒu�i�n�C���C�g�̌v�Z�ɕK�v�j
		FLOAT shiniess;         // �n�C���C�g�̋����iMaya��CosinePower)
		BOOL isTexture;         // �e�N�X�`���̗L��
	};

	// �}�e���A�����i�����̏��j
	struct MATERIAL
	{
		DWORD polygonCount; // �}�e���A���̃|���S����
		XMFLOAT4 diffuse;   // �g�U���ˌ��i�f�B�t���[�Y�j�ւ̔��ˋ��x
		XMFLOAT4 ambient;   // �����i�A���r�G���g�j�ւ̔��ˋ��x
		XMFLOAT4 specular;  // ���ʔ��ˌ��i�X�y�L�����[�j�ւ̔��ˋ��x
		float shininess;    // �n�C���C�g�̋����i�T�C�Y�j
		Texture* pTexture;  // �e�N�X�`��
	}*pMaterial_;

	// �{�[���\���́i�֐ߏ��j
	struct Bone
	{
		XMMATRIX bindPose;   // �����|�[�Y���̃{�[���ϊ��s��
		XMMATRIX newPose;    // �A�j���[�V�����ŕω������Ƃ��̃{�[���ϊ��s��
		XMMATRIX diffPose;   // mBindPose�ɑ΂���mNowPose �̕ω���
	};

	// �E�G�C�g�\���́i�{�[���ƒ��_�̊֘A�t���j
	struct Weight
	{
		XMFLOAT3 posOrigin;    // ���X�̒��_���W
		XMFLOAT3 normalOrigin; // ���X�̒��_�@��
		int* pBoneIndex;       // �֘A����{�[����ID
		float* pBoneWeight;    // �{�[���̏d��
	};

	// �e�f�[�^�̌�
	DWORD vertexCount_;        // ���_��
	DWORD polygonCount_;       // �|���S����
	DWORD indexCount_;         // �C���f�b�N�X��
	DWORD materialCount_;      // �}�e���A���̌�
	DWORD polygonVertexCount_; // �|���S�����_�C���f�b�N�X��

	VERTEX* pVertexData_;
	DWORD** ppIndexData_;

	//�y���_�o�b�t�@�z
	//�e���_�̏��i�ʒu�Ƃ��F�Ƃ��j���i�[����Ƃ���
	//���_�����̔z��ɂ��Ďg��
	ID3D11Buffer* pVertexBuffer_;

	//�y�C���f�b�N�X�o�b�t�@�z
	//�u�ǂ̒��_�v�Ɓu�ǂ̒��_�v�Ɓu�ǂ̒��_�v�łR�p�`�|���S���ɂȂ邩�̏����i�[����Ƃ���
	ID3D11Buffer** ppIndexBuffer_ = NULL;

	// �y�萔�o�b�t�@�z
	// �V�F�[�_�[�iSimple3D.hlsl)�̃O���[�o���ϐ��ɒl��n�����߂̂���
	ID3D11Buffer* pConstantBuffer_;

	// �{�[��������
	FbxSkin* pSkinInfo_;      // �X�L�����b�V�����i�X�L�����b�V���A�j���[�V�����̃f�[�^�{�́j
	FbxCluster** ppCluster_;  // �N���X�^���i�֐߂��ƂɊ֘A�t����ꂽ���_���j
	int numBone_;             // FBX�Ɋ܂܂�Ă���֐߂̐�
	Bone* pBoneArray_;        // �e�֐߂̏��
	std::unordered_map<string, Bone*> bonePair;
	Weight* pWeightArray_;    // �E�G�C�g���i���_�̑΂���e�֐߂̉e���x�����j

	// private�֐�
	void InitVertex(fbxsdk::FbxMesh* pMesh);   // ���_�o�b�t�@����
	void InitMaterial(fbxsdk::FbxNode* pNode); // �}�e���A������
	void InitMaterial(fbxsdk::FbxMesh* pMesh); // �}�e���A������
	void InitTexture(fbxsdk::FbxSurfaceMaterial* pMaterial, const DWORD& i); // �e�N�X�`������
	void InitIndex(fbxsdk::FbxMesh* pMesh);    // �C���f�b�N�X�o�b�t�@����
	void InitSkelton(FbxMesh* pMesh);          // ���̏�������
	void IntConstantBuffer();                   // �R���X�^���g�o�b�t�@�i�V�F�[�_�[�ɏ����𑗂��j����

public:
	FbxParts();
	FbxParts(Fbx* parent);
	~FbxParts();

	Fbx* parent_;
	// Fbx�t�@�C������������[�h���ď��X��������
	// �����FpNode ��񂪓����Ă���m�[�h
	// �߂�l�F����
	HRESULT Init(FbxNode* pNode);

	// Fbx�t�@�C������������[�h���ď��X��������
	// �����FpNode ��񂪓����Ă���m�[�h
	// �߂�l�F����
	HRESULT Init(FbxMesh* pMesh);

	// �`��
	// �����Fworld ���[���h�s��
	void Draw(Transform& transform);

	// �{�[������̃��f����`��
	// �����Ftransform �s����
	// �����Ftime �t���[�����i1�A�j���[�V�������̍��ǂ����j
	void DrawSkinAnime(Transform& transform, FbxTime time);

	// �{�[�������̃��f����`��
	// �����Ftransform �s����
	// �����Ftime �t���[�����i1�A�j���[�V�������̍��ǂ����j
	// �����Fscene Fbx�t�@�C������ǂݍ��񂾃V�[�����
	void DrawMeshAnime(Transform& transform, FbxTime time, FbxScene* scene);

	// �C�ӂ̃{�[���̈ʒu���擾
	// �����FboneName �擾�������{�[���̈ʒu
	// �����Fposition ���[���h���W�ł̈ʒu�yout�z
	// �߂�l�F�������true
	bool GetBonePosition(std::string boneName, XMFLOAT3* position);

	// �C�ӂ̃{�[���̈ʒu���擾�i�X�L�����b�V���A�j���[�V�����̎��j
	// �����FboneName �擾�������{�[���̈ʒu
	// �����Fposition ���[���h���W�ł̈ʒu�yout�z
	// �߂�l�F�������true
	bool GetBonePositionAtNow(std::string boneName, XMFLOAT3* position);

	// �X�L�����b�V�������擾
	// �߂�l�F�X�L�����b�V�����
	FbxSkin* GetSkinInfo() { return pSkinInfo_; }

	// ���C�L���X�g�i���C���΂��ē����蔻��j
	// �����Fdata�@�K�v�Ȃ��̂��܂Ƃ߂��f�[�^
	void RayCast(RayCastData *data);
};