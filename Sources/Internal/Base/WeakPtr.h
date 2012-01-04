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
#include "RefPtr.h"

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
        
        WeakPtr(T * p) : refcounter(NULL)
        {
            Assign(p);
        }
        
        ~WeakPtr()
        {
            ReleaseOldCounter();
        }
        
        WeakPtr(const WeakPtr<T> & rp) : refcounter(NULL)
        {            
            Assign(rp.refcounter);
        }
        
        WeakPtr(const RefPtr<T> & p)
        {
            Assign(p.Get());
        }
        
        RefPtr<T> GetRefPtr()
        {
            RefPtr<T> r;
            r = Get();
            return r;
        }
        
        WeakPtr & operator = (const WeakPtr & rp)
        {
            Assign(rp.refcounter);
            return *this;
        }
        
        /// use GetRefPtr instead!
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
        
        
    private:
        BaseObject::RefCounter * refcounter;
        
        void Assign(BaseObject::RefCounter * ptr)
        {
            if (ptr == refcounter)
                return;
            
            ReleaseOldCounter();
            refcounter = ptr;
            refcounter->IncWeak();
        }
        
        void Assign(T * ptr)
        {            
            if (ptr)
            {
                BaseObject::RefCounter * tmp_pt = refcounter;
                refcounter = ptr->GetRefCounter();
                
                if (refcounter)
                {
                    refcounter->IncWeak();
                }
                
                if (tmp_pt)
                {
                    ReleaseCounter(tmp_pt);
                }
            }
            else
            {
                ReleaseOldCounter();
                refcounter = NULL;
            }
        }
        
        void ReleaseOldCounter()
        {
            ReleaseCounter(refcounter);
        }
        
        static void ReleaseCounter(BaseObject::RefCounter * &refcounter)
        {
            if (refcounter)
            {
                if (refcounter->DecWeak())
                    refcounter = NULL;
                //if (refcounter->NoRefsLeft())
                //    delete refcounter;
            }
        }
    };
} //ns

#endif
