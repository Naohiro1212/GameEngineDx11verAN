#include "VFX.h"
#include "Camera.h"

namespace VFX
{
    std::list<Emitter*> emitterList_; // �G�~�b�^�[����
    std::list<Particle*> particleList_; // �p�[�e�B�N������
};

// �X�V
void VFX::Update()
{
    // �e�G�~�b�^�[�̏���
    EmitterUpdate();

    // �e�p�[�e�B�N���̏���
    ParticleUpdate();
}

// �������̃p�[�e�B�N���̍X�V
void VFX::ParticleUpdate()
{
    for (auto particle = particleList_.begin(); particle != particleList_.end();)
    {
        // �������s�����̂ŏ���
        if ((*particle)->life == 0)
        {
            (*particle)->pEmitter->particleNum--;
            delete (*particle);
            particle = particleList_.erase(particle);
        }

        // �܂������Ă�
        else
        {
            (*particle)->life--;
            (*particle)->now.position.x += (*particle)->delta.position.x;
            (*particle)->now.position.y += (*particle)->delta.position.y;
            (*particle)->now.position.z += (*particle)->delta.position.z;
            (*particle)->delta.position.x *= (*particle)->accel;
            (*particle)->delta.position.y *= (*particle)->accel;
            (*particle)->delta.position.z *= (*particle)->accel;
            (*particle)->delta.position.y -= (*particle)->gravity;
            
            (*particle)->now.rotation.x += (*particle)->delta.rotation.x;
            (*particle)->now.rotation.y += (*particle)->delta.rotation.y;
            (*particle)->now.rotation.z += (*particle)->delta.rotation.z;

            (*particle)->now.scale.x *= (*particle)->delta.scale.x;
            (*particle)->now.scale.y *= (*particle)->delta.scale.y;
            (*particle)->now.color.x += (*particle)->delta.color.x; // R
            (*particle)->now.color.y += (*particle)->delta.color.y; // G
            (*particle)->now.color.z += (*particle)->delta.color.z; // B
            (*particle)->now.color.w += (*particle)->delta.color.w; // A

            particle++;
        }
    }
}

// �e�G�~�b�^�[�̍X�V
void VFX::EmitterUpdate()
{
    for (auto emitter = emitterList_.begin(); emitter != emitterList_.end();)
    {
        // ��������ł�G�~�b�^�[
        if ((*emitter)->isDead)
        {
            // ���̃G�~�b�^�[����o���p�[�e�B�N�����S�������Ă���G�~�b�^�[���폜
            if ((*emitter)->particleNum <= 0)
            {
                (*emitter)->pBillBoard->Release();
                delete ((*emitter)->pBillBoard);
                delete (*emitter);
                emitter = emitterList_.erase(emitter);
            }
            else
            {
                emitter++;
            }
        }

        // �܂������Ă�
        else
        {
            // �p�[�e�B�N���𔭐�������^�C�~���O�Ȃ�
            if ((*emitter)->data.delay == 0 || (*emitter)->frameCount % (*emitter)->data.delay == 0)
            {
                // �p�[�e�B�N������
                CreateParticle(emitter);
            }

            (*emitter)->frameCount++;

            // delay��0�̃G�~�b�^�[�͈ꔭ�o������폜
            if ((*emitter)->data.delay == 0)
            {
                (*emitter)->isDead = true;
            }

            emitter++;
        }
    }
}

void VFX::CreateParticle(std::list<VFX::Emitter*>::iterator& emitter)
{

}

void VFX::Draw()
{
}

void VFX::Release()
{
}

int VFX::Start(EmitterData emitterData)
{
    return 0;
}

void VFX::End(int handle)
{
}
