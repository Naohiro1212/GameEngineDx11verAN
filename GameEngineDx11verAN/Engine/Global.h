#pragma once
#include "Direct3D.h"

// 安全にメモリを解放するためのマクロ
#define SAFE_DELETE(p) {if ((p)!=nullptr){delete(p); (p)=nullptr;}}
#define SAFE_DELETE_ARRAY(p) {if((p)!=nullptr) {delete[] (p); (p)=nullptr;}}
#define SAFE_RELEASE(p) {if((p)!=nullptr){p->Release();(p)=nullptr;}}