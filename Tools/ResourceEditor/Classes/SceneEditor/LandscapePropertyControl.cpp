#include "LandscapePropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"
#include "ControlsFactory.h"


LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    tiledModes.push_back("Tile mask mode");
    tiledModes.push_back("Texture mode");
    tiledModes.push_back("Mixed mode");
}

LandscapePropertyControl::~LandscapePropertyControl()
{

}

void LandscapePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
	DVASSERT(landscape);

    propertyList->AddSection("property.landscape.landscape", GetHeaderState("property.landscape.landscape", true));
    
    propertyList->AddFloatProperty("property.landscape.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.landscape.height", PropertyList::PROPERTY_IS_EDITABLE); 
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox;
        bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
        size = transformedBox.max - transformedBox.min;
    }
    propertyList->SetFloatPropertyValue("property.landscape.size", size.x);
    propertyList->SetFloatPropertyValue("property.landscape.height", size.z);

    propertyList->AddComboProperty("property.landscape.tilemode", tiledModes);
    propertyList->SetComboPropertyIndex("property.landscape.tilemode", landscape->GetTiledShaderMode());

    propertyList->AddFilepathProperty("property.landscape.heightmap", ".png;.heightmap", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("property.landscape.heightmap", landscape->GetHeightmapPathname());
    
    
    propertyList->AddSubsection("property.landscape.subsection.textures");
    AddFilepathProperty(String("property.landscape.texture.color"), String(".png;.pvr"), LandscapeNode::TEXTURE_COLOR);
    AddFilepathProperty(String("property.landscape.texture.tile0"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE0);
    AddFilepathProperty(String("property.landscape.texture.tile1"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE1);
    AddFilepathProperty(String("property.landscape.texture.tile2"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE2);
    AddFilepathProperty(String("property.landscape.texture.tile3"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE3);
    AddFilepathProperty(String("property.landscape.texture.tilemask"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE_MASK);
    AddFilepathProperty(String("property.landscape.texture.tiledtexture"), String(".png;.pvr"), LandscapeNode::TEXTURE_TILE_FULL);
    propertyList->AddMessageProperty(String("property.landscape.generatefulltiled"), 
                                     Message(this, &LandscapePropertyControl::GenerateFullTiledTexture));

    propertyList->AddSubsection("property.landscape.subsection.build_mask");
    propertyList->AddFilepathProperty("property.landscape.lightmap", String(".png;.pvr"), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.alphamask", String(".png;.pvr"), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("property.landscape.lightmap", String(""));
    propertyList->SetFilepathPropertyValue("property.landscape.alphamask", String(""));

    
    propertyList->AddBoolProperty("property.landscape.showgrid", PropertyList::PROPERTY_IS_EDITABLE);
    bool showGrid =  (0 != (landscape->GetDebugFlags() & SceneNode::DEBUG_DRAW_GRID));
    propertyList->SetBoolPropertyValue("property.landscape.showgrid", showGrid);
    

	propertyList->AddIntProperty("lightmap.size");
	propertyList->SetIntPropertyValue("lightmap.size", currentSceneNode->GetCustomProperties()->GetInt32("lightmap.size", 1024));

    propertyList->AddFloatProperty("property.landscape.texture0.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).x);
    propertyList->AddFloatProperty("property.landscape.texture0.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).y);
    
    propertyList->AddFloatProperty("property.landscape.texture1.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).x);
    propertyList->AddFloatProperty("property.landscape.texture1.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).y);

    propertyList->AddFloatProperty("property.landscape.texture2.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).x);
    propertyList->AddFloatProperty("property.landscape.texture2.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).y);
    
    propertyList->AddFloatProperty("property.landscape.texture3.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).x);
    propertyList->AddFloatProperty("property.landscape.texture3.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).y);
    
    ControlsFactory::AddFogSubsection(propertyList, landscape->IsFogEnabled(), landscape->GetFogDensity(), landscape->GetFogColor());
}

void LandscapePropertyControl::AddFilepathProperty(const String &key, const String &filter, LandscapeNode::eTextureLevel level)
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
    Texture * t = landscape->GetTexture(level);
    
    propertyList->AddFilepathProperty(key, filter, true, PropertyList::PROPERTY_IS_EDITABLE);
    if(t && !t->isRenderTarget)
    {
        propertyList->SetFilepathPropertyValue(key, landscape->GetTextureName(level));
    }
    else 
    {
        propertyList->SetFilepathPropertyValue(key, String(""));
    }
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.landscape.size" == forKey || "property.landscape.height" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        
        
        String heightMap = propertyList->GetFilepathPropertyValue("property.landscape.heightmap");
        if(EditorSettings::IsValidPath(heightMap) && heightMap.length())
        {
            landscape->BuildLandscapeFromHeightmapImage(heightMap, bbox);
        }
    }
    if ("property.landscape.texture0.tilex" == forKey || "property.landscape.texture0.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture0.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture0.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE0, tiling);
    }
    if ("property.landscape.texture1.tilex" == forKey || "property.landscape.texture1.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture1.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture1.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE1, tiling);
    }
	if ("property.landscape.texture2.tilex" == forKey || "property.landscape.texture2.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture2.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture2.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE2, tiling);
	}
	if ("property.landscape.texture3.tilex" == forKey || "property.landscape.texture3.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture3.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture3.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE3, tiling);
	}
    
    if ("property.material.dencity" == forKey)
    {
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetFogDensity(newValue);
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}


void LandscapePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if("lightmap.size" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetInt32("lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(EditorSettings::IsValidPath(newValue))
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        if("property.landscape.heightmap" == forKey)
        {
            Vector3 size(
                         propertyList->GetFloatPropertyValue("property.landscape.size"),
                         propertyList->GetFloatPropertyValue("property.landscape.size"),
                         propertyList->GetFloatPropertyValue("property.landscape.height"));
            AABBox3 bbox;
            bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
            bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
            
            if(newValue.length())
            {
                landscape->BuildLandscapeFromHeightmapImage(newValue, bbox);
            }
        }
        else if("property.landscape.texture.tile0" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE0, newValue);
        }
        else if("property.landscape.texture.tile1" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE1, newValue);
        }
        else if("property.landscape.texture.tile2" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE2, newValue);
        }
        else if("property.landscape.texture.tile3" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE3, newValue);
        }
        else if("property.landscape.texture.tilemask" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_MASK, newValue);
        }        
        else if("property.landscape.texture.color" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_COLOR, newValue);
        }
        else if("property.landscape.texture.tiledtexture" == forKey)
        {
            SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_FULL, newValue);
        }
        else if(    "property.landscape.lightmap" == forKey 
                ||  "property.landscape.alphamask" == forKey)
        {
            String lightMap = propertyList->GetFilepathPropertyValue("property.landscape.lightmap");
            String alphaMask = propertyList->GetFilepathPropertyValue("property.landscape.alphamask");
            
            CreateMaskTexture(lightMap, alphaMask);
        }
    }

    NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::SetLandscapeTexture(LandscapeNode::eTextureLevel level, const String &texturePathname)
{
    Texture::EnableMipmapGeneration();
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
    landscape->SetTexture(level, texturePathname);
    SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(level));
    Texture::DisableMipmapGeneration();

    if(LandscapeNode::TEXTURE_TILE_FULL != level)
    {
        landscape->UpdateFullTiledTexture();   
    }
}



void LandscapePropertyControl::OnComboIndexChanged(
                                PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.landscape.tilemode" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetTiledShaderMode((LandscapeNode::eTiledShaderMode)newItemIndex);
    }
        
    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LandscapePropertyControl::CreateMaskTexture(const String &lightmapPath, const String &alphamaskPath)
{
    Image *lightMap = Image::CreateFromFile(lightmapPath);
    Image *alphaMask = Image::CreateFromFile(alphamaskPath);
    
    if(lightMap && alphaMask)
    {
        if(     (lightMap->GetPixelFormat() == FORMAT_RGBA8888)
           &&   (alphaMask->GetPixelFormat() == FORMAT_RGBA8888))
        {
            if(     (lightMap->GetHeight() == alphaMask->GetHeight()) 
               &&   (lightMap->GetWidth() == alphaMask->GetWidth()) )
            {
                uint8 *lightMapData = lightMap->GetData();
                uint8 *alphaMaskData = alphaMask->GetData();
                
                int32 dataSize = lightMap->GetHeight() * lightMap->GetWidth() * 4;
                for(int32 i = 0; i < dataSize; i += 4)
                {
                    lightMapData[i + 3] = alphaMaskData[i];
                }
                
                String extension = FileSystem::Instance()->GetExtension(lightmapPath);
                String path, fileName;
                FileSystem::Instance()->SplitPath(lightmapPath, path, fileName);
                
                String resultPath = path + "EditorMaskTexture" + extension;
                FileSystem::Instance()->DeleteFile(resultPath);
                lightMap->Save(resultPath);
                
                propertyList->SetFilepathPropertyValue("property.landscape.lightmap", "");
                propertyList->SetFilepathPropertyValue("property.landscape.alphamask", "");

                propertyList->SetFilepathPropertyValue("property.landscape.texture.color", resultPath);
                LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
                if(landscape)
                {
                    Texture::EnableMipmapGeneration();
                    landscape->SetTexture(LandscapeNode::TEXTURE_COLOR, resultPath);
                    SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_COLOR));
                    Texture::DisableMipmapGeneration();
                }
            }
        }
    }
    
    SafeRelease(lightMap);
    SafeRelease(alphaMask);
}


void LandscapePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.landscape.showgrid" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
        if(newValue)
        {
            landscape->SetDebugFlags(landscape->GetDebugFlags() | SceneNode::DEBUG_DRAW_GRID);
        }
        else 
        {
            landscape->SetDebugFlags(landscape->GetDebugFlags() & ~SceneNode::DEBUG_DRAW_GRID);
        }
    }
    else if (String("property.material.fogenabled") == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetFog(newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.material.fogcolor" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetFogColor(newColor);
    }
    
    PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
}


void LandscapePropertyControl::GenerateFullTiledTexture(DAVA::BaseObject *object, void *userData, void *callerData)
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
    String texPathname = landscape->SaveFullTiledTexture();
    
    propertyList->SetFilepathPropertyValue(String("property.landscape.texture.tiledtexture"), texPathname);
    landscape->SetTexture(LandscapeNode::TEXTURE_TILE_FULL, texPathname);
}
