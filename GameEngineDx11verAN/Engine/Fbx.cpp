#include "Fbx.h"
#include "Direct3D.h"
#include "FbxParts.h"

#pragma comment(lib, "LibFbxSDK-MT.lib")
#pragma comment(lib, "LibXml2-MT.lib")
#pragma comment(lib, "zlib-MT.lib")

Fbx::Fbx():_animSpeed(0)
{
}

Fbx::~Fbx()
{
    for(int i = 0)
}

HRESULT Fbx::Load(std::string fileName)
{
    return E_NOTIMPL;
}

void Fbx::Draw(Transform& transform, int frame)
{
}

void Fbx::Release()
{
}

XMFLOAT3 Fbx::GetBonePosition(std::string boneName)
{
    return XMFLOAT3();
}

XMFLOAT3 Fbx::GetAnimBonePosition(std::string boneName)
{
    return XMFLOAT3();
}

void Fbx::RayCast(RayCastData* data)
{
}
