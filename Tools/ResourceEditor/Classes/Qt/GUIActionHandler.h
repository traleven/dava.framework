#ifndef __GUI_ACTION_MANAGER_H__
#define __GUI_ACTION_MANAGER_H__

#include <QObject>
#include "DAVAEngine.h"
#include "../Constants.h"
#include "Classes/SceneEditor/EditorSettings.h"

#include <QAction>
#include <QMenu>


class Command;
class GUIActionHandler: public QObject
{
    Q_OBJECT
    
public:
    GUIActionHandler(QObject *parent = 0);
    virtual ~GUIActionHandler();

    void RegisterNodeActions(DAVA::int32 count, ...);
    void RegisterViewportActions(DAVA::int32 count, ...);
    void RegisterDockActions(DAVA::int32 count, ...);
    
    void SetResentMenu(QMenu *menu);
    
    
public slots:
    //menu
    void MenuFileWillShow();
    void MenuToolsWillShow();

    void CreateNodeTriggered(QAction *nodeAction);
    void ViewportTriggered(QAction *viewportAction);
    void ResentSceneTriggered(QAction *resentScene);

    //File
    void NewScene();
    void OpenScene();
    void OpenProject();
    void OpenResentScene(DAVA::int32 index);
    void SaveScene();
    void ExportAsPNG();
    void ExportAsPVR();
    void ExportAsDXT();
    
    //View
    void RestoreViews();

    //tools
    void Materials();
    void ConvertTextures();
    void HeightmapEditor();
    void TilemapEditor();
    
private:
    //create node
    void CreateNode(ResourceEditor::eNodeType type);
    //viewport
    void SetViewport(ResourceEditor::eViewportType type);
    
    void Execute(Command *command);
    
    void RegisterActions(QAction **actions, DAVA::int32 count, va_list &vl);
    
    void ClearActions(int32 count, QAction **actions);
    
private:
    
    QAction *resentSceneActions[EditorSettings::RESENT_FILES_COUNT];
    QAction *nodeActions[ResourceEditor::NODE_COUNT];
    QAction *viewportActions[ResourceEditor::VIEWPORT_COUNT];
    QAction *dockActions[ResourceEditor::VIEWPORT_COUNT];

    QMenu *menuResentScenes;
};

#endif // __GUI_ACTION_MANAGER_H__
