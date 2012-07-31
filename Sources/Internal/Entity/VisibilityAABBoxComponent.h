#ifndef __DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__
#define __DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__

#include "Entity/Component.h"
#include "Math/AABBox3.h"

namespace DAVA
{

DECLARE_COMPONENT(VisibilityAABBoxComponent);  

void VisibilityAABBoxComponent::Register()
{   
	RegisterData<AABBox3>(DataName::MESH_AABOX);
	RegisterData<uint32>(DataName::FLAGS);
}

};

#endif //__DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__