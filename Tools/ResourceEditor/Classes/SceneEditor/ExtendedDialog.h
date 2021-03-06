#ifndef __EXTENDED_DIALOG_H__
#define __EXTENDED_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"

using namespace DAVA;

class ExtendedDialog : public UIControl
{
public:
    
    ExtendedDialog();
    ~ExtendedDialog();
    
    virtual void Close();

protected:

    virtual const Rect DialogRect();
    DraggableDialog *draggableDialog;
};

#endif //__EXTENDED_DIALOG_H__