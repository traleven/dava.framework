//
//  WeakPtr.h
//  Framework
//
//  Created by Yury Benesh on 1/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Framework_WeakPtr_h
#define Framework_WeakPtr_h

#include "Base/BaseObject.h"

namespace DAVA
{
    
    /// reference pointer wrapper for BaseObject refcounted classes.
    template <class T> class WeakPtr
    {
    public:
        WeakPtr()
        {
            refcounter = NULL;
        }
        
        explicit WeakPtr(T * p)
        {
            refcounter = p->GetRefCounter();
            refcounter->IncWeak();
        }
        
        ~WeakPtr()
        {
            ReleaseOldCounter();
        }
        
        WeakPtr(const WeakPtr<T> & rp)
        {
            ReleaseOldCounter();
                
            refcounter = rp.refcounter;
            refcounter->IncWeak();
        }
        
        T * Get() const
        {
            if (refcounter)
                return (T*)refcounter->GetPtr();
            
            return NULL;
        }
        
        bool Valid()
        {
            return Get() != NULL;
        }
        
        WeakPtr& operator = (T * ptr)
        {
            if (ptr == Get())
                return *this;
            
            BaseObject::RefCounter * tmp_pt = refcounter;
            refcounter = ptr->GetRefCounter();
            
            if (refcounter)
            {
                refcounter->IncWeak();
            }
            
            if (tmp_pt)
            {
                tmp_pt->DecWeak();
                if (tmp_pt->NoRefsLeft())
                {
                    delete tmp_pt;
                }
            }
            
            
            return *this;
        }
        
    private:
        BaseObject::RefCounter * refcounter;
        
        void ReleaseOldCounter()
        {
            if (refcounter)
            {
                refcounter->DecWeak();
                //if (refcounter->NoRefsLeft())
                //    delete refcounter;
            }
        }
    };
} //ns

#endif
