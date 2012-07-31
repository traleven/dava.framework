#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"
#include "Entity/ComponentTypes.h"
#include "Entity/DataNames.h"
#include "Entity/EntityManager.h"
#include <typeinfo>


#define DECLARE_COMPONENT(ComponentClassName) \
class ComponentClassName : public Component \
{ \
public:	\
	static ComponentClassName * Get() { return instance; } \
	\
	static void Create() \
	{   \
		ComponentClassName * component = new ComponentClassName(); \
		RegisterComponent(#ComponentClassName, component); \
		component->Register(); \
	}\
private: \
	static ComponentClassName * instance; \
	ComponentClassName() \
	{ \
		instance = this; \
	} \
	inline void Register(); \
\
}; 


#define IMPLEMENT_COMPONENT(ComponentClassName) \
ComponentClassName * ComponentClassName::instance = 0;

namespace DAVA 
{

class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    
    Component()
    {
        //type = ComponentType(); //this duplicates field declaration
        componentsByIndex[type.GetIndex()] = this;
    };
    
    
    template <class T>
    void RegisterData(int32 dataName)
    {
        EntityManager::CreatePoolAllocator<T>(dataName);
        dataNames.insert(dataName);
    }

//	template<class T>
//	TemplatePool<T>* CreatePool(T a, const char * name)
//	{
//		TemplatePool<T> * pool = new TemplatePool<T>(100);
//		pools.push_back(pool);
//        return pool;
//	}
//    

    const ComponentType & GetType() { return type; };
    
    static Component * GetComponentByIndex(uint64 index);
    
    Set<int32> & GetDataNames() {return dataNames; };

	static Map<const char *, Component * > cache;//<name, component>
    
private:
	ComponentType type;
    Set<int32> dataNames;
    static Map<uint64, Component*>  componentsByIndex;
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
