#include "SceneExporter.h"
#include "SceneValidator.h"

using namespace DAVA;

SceneExporter::SceneExporter()
{
    dataFolder = String("");
    dataSourceFolder = String(""); 
    workingFolder = String("");
    
    SetExportingFormat(String("png"));
}

SceneExporter::~SceneExporter()
{
    
}

void SceneExporter::CleanFolder(const String &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.c_str())));
        }
    }
}

void SceneExporter::SetInFolder(const String &folderPathname)
{
    dataSourceFolder = NormalizeFolderPath(folderPathname);
}

void SceneExporter::SetOutFolder(const String &folderPathname)
{
    dataFolder = NormalizeFolderPath(folderPathname);
}

void SceneExporter::SetExportingFormat(const String &newFormat)
{
    format = newFormat;
    if(0 < format.length() && format.at(0) != '.')
    {
        format = "." + format;
    }
}



void SceneExporter::ExportFile(const String &fileName, Set<String> &errorLog)
{
    //Load scene with *.sc2
    Scene *scene = new Scene();
    SceneNode *rootNode = scene->GetRootNode(dataSourceFolder + fileName);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            SceneNode *node = rootNode->GetChild(i);
            scene->AddNode(node);
        }
    }
    
    ExportScene(scene, fileName, errorLog);

    SafeRelease(scene);
}

void SceneExporter::ExportScene(Scene *scene, const String &fileName, Set<String> &errorLog)
{
    //Create destination folder
    String normalizedFileName = FileSystem::Instance()->NormalizePath(fileName);

    
    String workingFile;
    FileSystem::SplitPath(normalizedFileName, workingFolder, workingFile);
    FileSystem::Instance()->CreateDirectory(dataFolder + workingFolder, true); 
    
    //Export scene data
    RemoveEditorNodes(scene);

    String oldPath = SceneValidator::Instance()->SetPathForChecking(dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);
	SceneValidator::Instance()->ValidateScales(scene, errorLog);

    ExportMaterials(scene, errorLog);
    ExportLandscape(scene, errorLog);
    ExportMeshLightmaps(scene, errorLog);
    
    //save scene to new place
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    outFile->SaveScene(dataFolder + fileName, scene);
    SafeRelease(outFile);
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}



void SceneExporter::RemoveEditorNodes(DAVA::SceneNode *rootNode)
{
    //Remove scene nodes
    
    Set<SceneNode *> scenenodesForDeletion;
    Vector<SceneNode *> scenenodes;
    rootNode->GetChildNodes(scenenodes);
    
    //Find "editor." nodes
    Vector<SceneNode *>::const_iterator endIt = scenenodes.end();
    for (Vector<SceneNode *>::const_iterator it = scenenodes.begin(); it != endIt; ++it)
    {
        SceneNode * node = *it;
        String::size_type pos = node->GetName().find(String("editor."));
        if(String::npos != pos)
        {
            Set<SceneNode *>::const_iterator setIt = scenenodesForDeletion.find(node);
            if(setIt == scenenodesForDeletion.end())
            {
                scenenodesForDeletion.insert(SafeRetain(node));
            }
        }
    }
        
    //remove nodes from hierarhy
    Set<SceneNode *>::const_iterator endItDeletion = scenenodesForDeletion.end();
    for (Set<SceneNode *>::const_iterator it = scenenodesForDeletion.begin(); it != endItDeletion; ++it)
    {
        SceneNode * node = *it;
        if (node->GetParent())
        {
            node->GetParent()->RemoveNode(node);
        }
    }
    
    //release nodes
	for (Set<SceneNode *>::const_iterator it = scenenodesForDeletion.begin(); it != endItDeletion; ++it)
	{
		SceneNode * node = *it;
		SafeRelease(node);
	}
    scenenodesForDeletion.clear();
}


void SceneExporter::ExportFolder(const String &folderName, Set<String> &errorLog)
{
    String folderPathname = dataSourceFolder + NormalizeFolderPath(folderName);
	FileList * fileList = new FileList(folderPathname);
    for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
        String pathname = fileList->GetPathname(i);
		if(fileList->IsDirectory(i))
		{
            String curFolderName = fileList->GetFilename(i);
            if((String(".") != curFolderName) && (String("..") != curFolderName))
            {
                String workingPathname = RemoveFolderFromPath(pathname, dataSourceFolder);
                ExportFolder(workingPathname, errorLog);
            }
        }
        else 
        {
            String extension = FileSystem::Instance()->GetExtension(pathname);
            if(String(".sc2") == extension)
            {
                String workingPathname = RemoveFolderFromPath(pathname, dataSourceFolder);
                ExportFile(workingPathname, errorLog);
            }
        }
    }
    
    SafeRelease(fileList);
}

String SceneExporter::NormalizeFolderPath(const String &pathname)
{
    String normalizedPathname = FileSystem::Instance()->NormalizePath(pathname);

    int32 lastPos = normalizedPathname.length() - 1;
    if((0 <= lastPos) && ('/' != normalizedPathname.at(lastPos)))
    {
        normalizedPathname += "/";
    }
    
    return normalizedPathname;
}


void SceneExporter::ExportMaterials(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportMaterials]");

    Vector<Material*> materials;
    scene->GetDataNodes(materials);
    for (int32 i = 0; i < (int32)materials.size(); i++)
    {
        Material *m = materials[i];
        if (m->GetName().find("editor.") == String::npos)
        {
			if (m->textures[Material::TEXTURE_DIFFUSE])
			{
				if (!m->textures[Material::TEXTURE_DIFFUSE]->relativePathname.empty()) 
				{
                    m->names[Material::TEXTURE_DIFFUSE] = ExportTexture(m->names[Material::TEXTURE_DIFFUSE], errorLog);
				}
			}
            
			if (m->textures[Material::TEXTURE_DECAL])
			{
				if (!m->textures[Material::TEXTURE_DECAL]->relativePathname.empty()) 
				{
                    m->names[Material::TEXTURE_DECAL] = ExportTexture(m->names[Material::TEXTURE_DECAL], errorLog);
				}
			}
        }
    }
}

void SceneExporter::ExportLandscape(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportLandscape]");

    Vector<LandscapeNode *> landscapes;
    scene->GetChildNodes(landscapes);
    
    if(0 < landscapes.size())
    {
        if(1 < landscapes.size())
        {
            errorLog.insert(String("There are more than one landscapes at level."));
        }
        
        LandscapeNode *landscape = landscapes[0];
        ExportFileDirectly(landscape->GetHeightmapPathname(), errorLog);
        
        String fullTiledTexture = landscape->SaveFullTiledTexture();
        landscape->SetTexture(LandscapeNode::TEXTURE_TILE_FULL, fullTiledTexture);
        
        for(int i = 0; i < LandscapeNode::TEXTURE_COUNT; i++)
        {
            Texture *t = landscape->GetTexture((LandscapeNode::eTextureLevel)i);
            if(t) 
            {
                landscape->SetTextureName((LandscapeNode::eTextureLevel)i, 
                                            ExportTexture(landscape->GetTextureName((LandscapeNode::eTextureLevel)i), errorLog));
            }
            else 
            {
                if(LandscapeNode::TEXTURE_DETAIL != i)
                    errorLog.insert(String(Format("There is no landscape texture for index %d", i)));
            }
        }
    }
}

void SceneExporter::ExportMeshLightmaps(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportMeshLightmaps]");

    Vector<MeshInstanceNode *> meshInstances;
    scene->GetChildNodes(meshInstances);

    for(int32 iMesh = 0; iMesh < (int32)meshInstances.size(); ++iMesh)
    {
        for (int32 li = 0; li < meshInstances[iMesh]->GetLightmapCount(); ++li)
        {
            MeshInstanceNode::LightmapData * ld = meshInstances[iMesh]->GetLightmapDataForIndex(li);
            if (ld)
            {
                ld->lightmapName = ExportTexture(ld->lightmapName, errorLog);
            }
        }
    }
}


bool SceneExporter::ExportFileDirectly(const String &filePathname, Set<String> &errorLog)
{
    Logger::Debug("[ExportFileDirectly] %s", filePathname.c_str());

    String workingPathname = RemoveFolderFromPath(filePathname, dataSourceFolder);
    
    String folder, file;
    FileSystem::SplitPath(workingPathname, folder, file);
    
    String newFolderPath = dataFolder + folder;
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s",newFolderPath.c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + workingPathname);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s", workingPathname.c_str(), dataSourceFolder.c_str(), dataFolder.c_str())));
    }
    
    return retCopy;
}


String SceneExporter::ExportTexture(const String &texturePathname, Set<String> &errorLog)
{
    Logger::Debug("[ExportTexture] %s", texturePathname.c_str());
    
    //TODO: return at format will be enabled
    String exportedPathname = FileSystem::Instance()->ReplaceExtension(texturePathname, format);
    bool ret = ExportFileDirectly(exportedPathname, errorLog);
    if(!ret)
    {
        //TODO: blen textures
        RenderManager::Instance()->LockNonMain();

        Texture *tex = Texture::CreateFromFile(texturePathname);
        if(tex)
        {
            Sprite *fbo = Sprite::CreateAsRenderTarget((float32)tex->width, (float32)tex->height, FORMAT_RGBA8888);
            Sprite *texSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);
            if(fbo && texSprite)
            {
                
                RenderManager::Instance()->SetRenderTarget(fbo);
                texSprite->SetPosition(0.f, 0.f);
                texSprite->Draw();
                
                RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 1.0f, 0.5f));
                RenderHelper::Instance()->FillRect(Rect(0.0f, 0.0f, (float32)tex->width, (float32)tex->height));

                RenderManager::Instance()->RestoreRenderTarget();
                
                //Save new texture
                String workingPathname = RemoveFolderFromPath(texturePathname, dataSourceFolder);
                
                String folder, file;
                FileSystem::SplitPath(workingPathname, folder, file);
                
                String newFolderPath = dataFolder + folder;
                if(!FileSystem::Instance()->IsDirectory(newFolderPath))
                {
                    FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
                    if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
                    {
                        errorLog.insert(String(Format("Can't create folder %s",newFolderPath.c_str())));
                    }
                }
                exportedPathname = dataFolder + workingPathname;
                FileSystem::Instance()->DeleteFile(exportedPathname);
                
                Image *image = fbo->GetTexture()->CreateImageFromMemory();
                if(image)
                {
                    image->Save(exportedPathname);
                }
                else 
                {
                    errorLog.insert(String(Format("Can't create image from fbo for file %s", texturePathname.c_str())));
                }
                
                SafeRelease(image);
            }
            else 
            {
                errorLog.insert(String(Format("Can't create FBO for file %s", texturePathname.c_str())));
            }
            
            SafeRelease(texSprite);
            SafeRelease(fbo);
            SafeRelease(tex);
        }
        else 
        {
            errorLog.insert(String(Format("Can't create texture for file %s", texturePathname.c_str())));
        }
        RenderManager::Instance()->UnlockNonMain();
        
    }
    
    return exportedPathname;
}


String SceneExporter::RemoveFolderFromPath(const String &pathname, const String &folderPathname)
{
    String workingPathname = pathname;
    String::size_type pos = workingPathname.find(folderPathname);
    if(String::npos != pos)
    {
        workingPathname = workingPathname.replace(pos, folderPathname.length(), "");
    }

    return workingPathname;
}


