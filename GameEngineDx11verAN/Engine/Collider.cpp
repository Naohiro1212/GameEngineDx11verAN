#include "BoxCollider.h"
#include "SphereCollider.h"
#include "GameObject.h"
#include "Model.h"
#include "Collider.h"

Collider::Collider() : pGameObject_(nullptr)
{
}

Collider::~Collider()
{
}

//” Œ^“¯Žm‚ÌÕ“Ë”»’è
//ˆø”FboxA	‚P‚Â–Ú‚Ì” Œ^”»’è
//ˆø”FboxB	‚Q‚Â–Ú‚Ì” Œ^”»’è
//–ß’lFÚG‚µ‚Ä‚¢‚ê‚Îtrue
bool Collider::IsHitBoxVsBox(BoxCollider* boxA, BoxCollider* boxB)
{

	XMFLOAT3 boxPosA = Transform::Float3Add(boxA->pGameObject_->GetWorldPosition(), boxA->center_);
	XMFLOAT3 boxPosB = Transform::Float3Add(boxB->pGameObject_->GetWorldPosition(), boxB->center_);


	if ((boxPosA.x + boxA->size_.x / 2) > (boxPosB.x - boxB->size_.x / 2) &&
		(boxPosA.x - boxA->size_.x / 2) < (boxPosB.x + boxB->size_.x / 2) &&
		(boxPosA.y + boxA->size_.y / 2) > (boxPosB.y - boxB->size_.y / 2) &&
		(boxPosA.y - boxA->size_.y / 2) < (boxPosB.y + boxB->size_.y / 2) &&
		(boxPosA.z + boxA->size_.z / 2) > (boxPosB.z - boxB->size_.z / 2) &&
		(boxPosA.z - boxA->size_.z / 2) < (boxPosB.z + boxB->size_.z / 2))
	{
		return true;
	}
	return false;
}

bool Collider::IsHitBoxVsCircle(BoxCollider* box, SphereCollider* sphere)
{
    return false;
}

bool Collider::IsHitCircleVsCircle(SphereCollider* circleA, SphereCollider* circleB)
{
    return false;
}

void Collider::Draw(XMFLOAT3 position)
{
}
