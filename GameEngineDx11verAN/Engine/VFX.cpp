#include "VFX.h"
#include "Camera.h"

namespace VFX
{
    std::list<Emitter*> emitterList_; // エミッターたち
    std::list<Particle*> particleList_; // パーティクルたち
};

// 更新
void VFX::Update()
{
    // 各エミッターの処理
    EmitterUpdate();

    // 各パーティクルの処理
    ParticleUpdate();
}

// 発生中のパーティクルの更新
void VFX::ParticleUpdate()
{
    for (auto particle = particleList_.begin(); particle != particleList_.end();)
    {
        // 寿命が尽きたので消す
        if ((*particle)->life == 0)
        {
            (*particle)->pEmitter->particleNum--;
            delete (*particle);
            particle = particleList_.erase(particle);
        }

        // まだ生きてる
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

// 各エミッターの更新
void VFX::EmitterUpdate()
{
    for (auto emitter = emitterList_.begin(); emitter != emitterList_.end();)
    {
        // もう死んでるエミッター
        if ((*emitter)->isDead)
        {
            // そのエミッターから出たパーティクルが全部消えてたらエミッターも削除
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

        // まだ生きてる
        else
        {
            // パーティクルを発生させるタイミングなら
            if ((*emitter)->data.delay == 0 || (*emitter)->frameCount % (*emitter)->data.delay == 0)
            {
                // パーティクル発生
                CreateParticle(emitter);
            }

            (*emitter)->frameCount++;

            // delayが0のエミッターは一発出したら削除
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
