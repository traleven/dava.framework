#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class UNDOAction: public BaseObject
{
public:
    enum eActionType
    {
        ACTION_NONE = -1,
        ACTION_HEIGHTMAP = 0,
        ACTION_TILEMASK
    };
    
    UNDOAction();

    eActionType type;
    int32 ID;
    String filePathname;
    
    void *actionData;
};


class UNDOManager: public Singleton<UNDOManager>
{    
    enum eConst
    {
        UNDO_HISTORY_SIZE = 20
    };
    
public:
 
    UNDOManager();
	virtual ~UNDOManager();

    void SaveHightmap(Heightmap *heightmap);
    void UndoHeightmap(Heightmap *heightmap);
    void RedoHeightmap(Heightmap *heightmap);

    void SaveTilemask(Texture *tilemask);
    Texture * UndoTilemask();
    Texture * RedoTilemask();

    
    UNDOAction::eActionType GetLastUNDOAction();
    UNDOAction::eActionType GetFirstREDOAction();
    
    void ClearHistory(UNDOAction::eActionType forAction);

protected:

    void ClearHistory(List<UNDOAction *> &actionsHistory, UNDOAction::eActionType forAction);
    void ReleaseHistory(List<UNDOAction *> &actionsHistory);
    
    UNDOAction *CreateHeightmapAction(Heightmap *heightmap);
    UNDOAction *CreateTilemaskAction(Texture *tilemask);
    
    
    String TimeString();
    
    void CheckHistoryLength();
    
    List<UNDOAction *>actionsHistoryUNDO;
    List<UNDOAction *>actionsHistoryREDO;
    
    int32 actionCounter;
};



#endif // __UNDO_MANAGER_H__