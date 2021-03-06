/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan Petrochenko
=====================================================================================*/
#ifndef __DAVAENGINE_STATS_H__
#define __DAVAENGINE_STATS_H__

#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "FileSystem/File.h"
#include "Base/DynamicObjectCache.h"

namespace DAVA
{

/**
    \brief Class to measure performance in framework code. 
 
    This class can help you to determine the bottlenecks of your code and measure the functions pefromance. 
 
    First step is initialization: 
    \code
    Scene::Scene()
    {
        Stats::Instance()->RegisterEvent("Scene", "Time spend in scene processing");
        Stats::Instance()->RegisterEvent("Scene.Update", "Time spend in draw function");
        Stats::Instance()->RegisterEvent("Scene.Draw", "Time spend in draw function");
    }
    \endcode
    
    Second step is measurement code: 
    \code
    void Scene::Update(float timeElapsed)
    {
        Stats::Instance()->BeginTimeMeasure("Scene.Update", this);
        // .. function code
        Stats::Instance()->EndTimeMeasure("Scene.Update", this);
    }
    
    void Scene::Draw()
    {
        Stats::Instance()->BeginTimeMeasure("Scene.Draw", this);
        // .. function code
        Stats::Instance()->EndTimeMeasure("Scene.Draw", this);
    }   
    \endcode
 
    Third step is initialization of output somewhere in your code: 
    \code
    void GameCore::OnAppStarted()
    {
        // show statistics every 30 frame, to avoid slowdown because of logger messages
        Stats::Instance()->EnableStatsOutputEventNFrame(30);
    }
 
    \endcode
    
 
 */
class Stats : public StaticSingleton<Stats>
{
public:
    Stats();
    virtual ~Stats();
    
    /**
        \brief Register name of tracking event, and it's description.
        \param[in] eventName name of the tracking event.
        \param[in] eventDescription description of the tracking event. 
     */
    void RegisterEvent(const String & eventName, const String & eventDescription);
    
    /**
        \brief Begin time measure for the particular event and object instance. 
        \param[in] eventName name of event we want to begin tracking for.
        \param[in] owner pointer to instance we want to begin tracking for. 
    */
    void BeginTimeMeasure(const String & eventName, BaseObject * owner);
    
    /**
        \brief End time measure for the particular event and object instance. 
        \param[in] eventName name of event we want to begin tracking for.
        \param[in] owner pointer to instance we want to begin tracking for. 
     */
    void EndTimeMeasure(const String & eventName, BaseObject * owner);
    
    /**
        \brief Function enables automatic output of measured values to log, every N frames. 
        \param[in] skipFrameCount number of frames we should skip before next debug print to log.
     */
    void EnableStatsOutputEventNFrame(int32 skipFrameCount);

    /**
        \brief System function that is called from ApplicationCore::BeginFrame, to initialize the state of the statistics singleton.
     */
    void BeginFrame();

    /**
        \brief System function that is called from ApplicationCore::EndFrame, to finalize the state of the statistics singleton and output debug information.
     */
    void EndFrame();
    
private:
#if defined(__DAVAENGINE_ENABLE_DEBUG_STATS__)
    struct Event
    {
        enum eEventType
        {
            TYPE_EVENT_BEGIN = 0,
            TYPE_EVENT_END,
        };
        uint32 type;
        uint32 id;
        uint64 time;
        BaseObject * owner;
    };
    int32 frame;
    int32 skipFrameCount;
    
    uint32 globalId;
    
    struct EventDescription
    {
        uint32 id;
        String name;
        String description;
        uint32 parent;
        List<uint32> childs;
        uint32 time;
    };
        
    Map<String, uint32> eventIds;
    Map<uint32, EventDescription> eventMap;
    Vector<std::pair<String, uint32> > sortedNames;
    
    DynamicObjectCacheData<Event> cache;
    List<Event*> events;
#endif
};


};

#endif // __DAVAENGINE_STATS_H__
