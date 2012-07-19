#ifndef __DAVAENGINE_COMPONENT_FLAGS__
#define __DAVAENGINE_COMPONENT_FLAGS__

#include "Entity/Component.h"

namespace DAVA
{

	DECLARE_COMPONENT(FlagsComponent);  

	void FlagsComponent::Register()
	{   
		RegisterData<uint32>("flags");
	}

};

#endif //__DAVAENGINE_COMPONENT_FLAGS__