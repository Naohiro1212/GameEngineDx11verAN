#pragma once
#include "Direct3D.h"

// ���S�Ƀ�������������邽�߂̃}�N��
#define SAFE_DELETE(p) {if ((p)!=nullptr){delete(p); (p)=nullptr;}}
#define SAFE_DELETE_ARRAy(p) {if((p)!=nullptr) {delete[] (p); (p)=nullptr;}}
#define SAFE_RELEASE(p) {if((p)!=nullptr){p->Release();(p)=nullptr;}}