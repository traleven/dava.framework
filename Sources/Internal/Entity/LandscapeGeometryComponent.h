#ifndef __DAVAENGINE_COMPONENT_LANDSCAPE_GEOMETRY__
#define __DAVAENGINE_COMPONENT_LANDSCAPE_GEOMETRY__

#include "Entity/Component.h"

namespace DAVA
{
	DECLARE_COMPONENT(LandscapeGeometryComponent);  

	class LandscapeNode;
	void LandscapeGeometryComponent::Register()
	{   
		RegisterData<LandscapeNode*>(DataName::LANDSCAPE_NODE);
	}

};

#endif //__DAVAENGINE_COMPONENT_LANDSCAPE_GEOMETRY__