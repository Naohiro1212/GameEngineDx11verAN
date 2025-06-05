#pragma once
#include <vector>
#include <string>
#include <Windows.h>

// CSV�t�@�C���������N���X // 
class CsvReader
{
	// �ǂݍ��񂾃f�[�^�����Ă���2�����z��
	std::vector<std::vector<std::string>> data_;

	// �u,�v���u���s�v�܂ł̕�������擾
	// �����Fresult ���ʂ�����A�h���X
	// �����Fdata ���̕�����f�[�^
	// �����Findex �������ڂ��璲�ׂ邩
	// �߂�l�F�Ȃ�
	void GetToComma(std::string* result, std::string data, DWORD* index);

public:
	// �R���X�g���N�^
	CsvReader();

	// �f�X�g���N�^
	~CsvReader();

	// CSV�t�@�C���̃��[�h
	// �����FfileName �t�@�C����
	// �߂�l�F������true ���s��false
	bool Load(std::wstring fileName);

	//�w�肵���ʒu�̃f�[�^�𕶎���Ŏ擾
	//�����Fx,y�@�擾�������ʒu
	//�ߒl�F�擾����������
	std::string GetString(DWORD x, DWORD y);

	// �w�肵���ʒu�̃f�[�^�𕶎���Ŏ擾
	// �����Fx,y �擾�������ʒu
	// �߂�l�F�擾�����l
	int GetValue(DWORD x, DWORD y);

	// �t�@�C���̗񐔂��擾
	// �����F����
	// �߂�l�F��
	size_t GetWidth();

	// �t�@�C���̍s�����擾
	// �����F�Ȃ�
	// �߂�l�F�s��
	size_t GetHeight();
};