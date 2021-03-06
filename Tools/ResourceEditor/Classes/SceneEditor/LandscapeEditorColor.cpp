#include "LandscapeEditorColor.h"

#include "LandscapeTool.h"
#include "LandscapeToolsPanelColor.h"
#include "PropertyControlCreator.h"
#include "ErrorNotifier.h"
#include "EditorScene.h"

#include "UNDOManager.h"

#pragma mark --LandscapeEditorColor
LandscapeEditorColor::LandscapeEditorColor(LandscapeEditorDelegate *newDelegate, 
                                           EditorBodyControl *parentControl, const Rect &toolsRect)
    :   LandscapeEditorBase(newDelegate, parentControl)
{
	wasTileMaskToolUpdate = false;
    tileMaskEditorShader = new Shader();
	tileMaskEditorShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask-editor.shader");
	tileMaskEditorShader->Recompile();

    maskSprite = NULL;
	oldMaskSprite = NULL;
	toolSprite = NULL;
    savedTexture = NULL;
    settings = NULL;
    
    //init draw params
    srcBlendMode = BLEND_SRC_ALPHA;
    dstBlendMode = BLEND_ONE;
    paintColor = Color(1.f, 1.f, 1.f, 1.0f);
    
    toolsPanel = new LandscapeToolsPanelColor(this, toolsRect);

    editingIsEnabled = false;
}

LandscapeEditorColor::~LandscapeEditorColor()
{
    SafeRelease(tileMaskEditorShader);

    SafeRelease(savedTexture);
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);
}


void LandscapeEditorColor::Draw(const DAVA::UIGeometricData &geometricData)
{
    if(wasTileMaskToolUpdate)
	{
		UpdateTileMask();
        
		RenderManager::Instance()->SetRenderTarget(toolSprite);
		RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
		RenderManager::Instance()->RestoreRenderTarget();
		
		wasTileMaskToolUpdate = false;
	}
}

void LandscapeEditorColor::CreateMaskTexture()
{
    SafeRelease(savedTexture);
    savedTexture = SafeRetain(workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK));
    if(savedTexture)
    {
        savedPath = savedTexture->relativePathname;
    }
    else 
    {
        savedPath = "";
    }
    
    CreateMaskFromTexture(savedTexture);
    
    UNDOManager::Instance()->ClearHistory(UNDOAction::ACTION_TILEMASK);
    UNDOManager::Instance()->SaveTilemask(maskSprite->GetTexture());
}

void LandscapeEditorColor::CreateMaskFromTexture(Texture *tex)
{
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
    SafeRelease(toolSprite);
    
    int32 texSize = settings->maskSize;
    if(tex)
    {
        texSize = tex->width;
    }
    
    maskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	oldMaskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
    toolSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
    
    if(tex)
    {
        RenderManager::Instance()->LockNonMain();
        
        Sprite *oldMask = Sprite::CreateFromTexture(tex, 0, 0, tex->width, tex->height);
        
        RenderManager::Instance()->SetRenderTarget(oldMaskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();
        
        RenderManager::Instance()->SetRenderTarget(maskSprite);
        oldMask->SetPosition(0.f, 0.f);
        oldMask->Draw();
        RenderManager::Instance()->RestoreRenderTarget();
        
        SafeRelease(oldMask);
        
        RenderManager::Instance()->UnlockNonMain();
    }
    
	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, oldMaskSprite->GetTexture());
}



void LandscapeEditorColor::UpdateTileMask()
{
	int32 colorType;
	if(settings->redMask)
	{
		colorType = 0;
	}
	else if(settings->greenMask)
	{
		colorType = 1;
	}
	else if(settings->blueMask)
	{
		colorType = 2;
	}
	else if(settings->alphaMask)
	{
		colorType = 3;
	}
	else
	{
		return; // no color selected
	}
    
	RenderManager::Instance()->SetRenderTarget(maskSprite);
    
	srcBlendMode = RenderManager::Instance()->GetSrcBlend();
	dstBlendMode = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
    
	RenderManager::Instance()->SetShader(tileMaskEditorShader);
	oldMaskSprite->PrepareSpriteRenderData(0);
	RenderManager::Instance()->SetRenderData(oldMaskSprite->spriteRenderObject);
	RenderManager::Instance()->SetTexture(oldMaskSprite->GetTexture(), 0);
	RenderManager::Instance()->SetTexture(toolSprite->GetTexture(), 1);
	RenderManager::Instance()->FlushState();
    
	int32 tex0 = tileMaskEditorShader->FindUniformLocationByName("texture0");
	tileMaskEditorShader->SetUniformValue(tex0, 0);
	int32 tex1 = tileMaskEditorShader->FindUniformLocationByName("texture1");
	tileMaskEditorShader->SetUniformValue(tex1, 1);
	int32 colorTypeUniform = tileMaskEditorShader->FindUniformLocationByName("colorType");
	tileMaskEditorShader->SetUniformValue(colorTypeUniform, colorType);
	int32 intensityUniform = tileMaskEditorShader->FindUniformLocationByName("intensity");
    
	tileMaskEditorShader->SetUniformValue(intensityUniform, currentTool->strength);
    
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
    
	RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
	RenderManager::Instance()->RestoreRenderTarget();
    
	workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, maskSprite->GetTexture());
	Sprite * temp = oldMaskSprite;
	oldMaskSprite = maskSprite;
	maskSprite = temp;
}

void LandscapeEditorColor::UpdateTileMaskTool()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->size * currentTool->size);
        Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;
        if(pos != prevDrawPos)
        {
            wasTileMaskToolUpdate = true;
            
            RenderManager::Instance()->SetRenderTarget(toolSprite);
            currentTool->sprite->SetScaleSize(scaleSize, scaleSize);
            currentTool->sprite->SetPosition(pos);
            currentTool->sprite->Draw();
            RenderManager::Instance()->RestoreRenderTarget();
        }
	}
}


void LandscapeEditorColor::UpdateCursor()
{
	if(currentTool && currentTool->sprite && currentTool->size)
	{
		float32 scaleSize = currentTool->sprite->GetWidth() * (currentTool->size * currentTool->size);
		Vector2 pos = landscapePoint - Vector2(scaleSize, scaleSize)/2;

		workingLandscape->SetCursorTexture(cursorTexture);
		workingLandscape->SetBigTextureSize(workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK)->GetWidth());
		workingLandscape->SetCursorPosition(pos);
		workingLandscape->SetCursorScale(scaleSize);
	}
}


void LandscapeEditorColor::InputAction(int32 phase, bool intersects)
{
    switch(phase)
    {
        case UIEvent::PHASE_BEGAN:
        {
            editingIsEnabled = true;
            break;
        }
            
        case UIEvent::PHASE_DRAG:
        {
            if(editingIsEnabled && !intersects)
            {
                editingIsEnabled = false;
                UNDOManager::Instance()->SaveTilemask(maskSprite->GetTexture());
            }
            else if(!editingIsEnabled && intersects)
            {
                editingIsEnabled = true;
            }
            break;
        }
            
        case UIEvent::PHASE_ENDED:
        {
            editingIsEnabled = false;
            UNDOManager::Instance()->SaveTilemask(maskSprite->GetTexture());
            break;
        }
            
        default:
            break;
    }
    
    
    
    Texture *tex = NULL;
    if(settings->redMask)
    {
        tex = workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE0);
    }
    else if(settings->greenMask)
    {
        tex = workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE1);
    }
    else if(settings->blueMask)
    {
        tex = workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE2);
    }
    else if(settings->alphaMask)
    {
        tex = workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE3);
    }
    
    if(tex)
    {
        UpdateTileMaskTool(); 
    }
}

void LandscapeEditorColor::HideAction()
{
    workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, savedTexture);
    
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
	SafeRelease(toolSprite);

	workingLandscape->CursorDisable();
}

void LandscapeEditorColor::ShowAction()
{
    CreateMaskTexture();
    landscapeSize = maskSprite->GetWidth();

	workingLandscape->CursorEnable();
}

void LandscapeEditorColor::UndoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetLastUNDOAction();
    if(UNDOAction::ACTION_TILEMASK == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->UndoTilemask();
        
        Image::EnableAlphaPremultiplication(true);
        tex->GenerateMipmaps();
        tex->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);

        CreateMaskFromTexture(tex);
    }
}

void LandscapeEditorColor::RedoAction()
{
    UNDOAction::eActionType type = UNDOManager::Instance()->GetFirstREDOAction();
    if(UNDOAction::ACTION_TILEMASK == type)
    {
        Image::EnableAlphaPremultiplication(false);
        
        Texture *tex = UNDOManager::Instance()->RedoTilemask();
        
        Image::EnableAlphaPremultiplication(true);
        tex->GenerateMipmaps();
        tex->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);

        CreateMaskFromTexture(tex);
    }
}

void LandscapeEditorColor::SaveTextureAction(const String &pathToFile)
{
    if(maskSprite)
    {
        Image *img = maskSprite->GetTexture()->CreateImageFromMemory();   
        if(img)
        {
            img->Save(pathToFile);
            SafeRelease(img);
            
            SafeRelease(savedTexture);
            workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, pathToFile); 
            savedTexture = SafeRetain(workingLandscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK));
            workingLandscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, maskSprite->GetTexture());
        }
    }
}

NodesPropertyControl *LandscapeEditorColor::GetPropertyControl(const Rect &rect)
{
    LandscapeEditorPropertyControl *propsControl = 
    (LandscapeEditorPropertyControl *)PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(workingLandscape, rect, LandscapeEditorPropertyControl::MASK_EDITOR_MODE);
    
    propsControl->SetDelegate(this);

    LandscapeEditorSettingsChanged(propsControl->Settings());
    return propsControl;
}


#pragma mark -- LandscapeEditorPropertyControlDelegate
void LandscapeEditorColor::LandscapeEditorSettingsChanged(LandscapeEditorSettings *newSettings)
{
    settings = newSettings;
}

void LandscapeEditorColor::TextureWillChanged(const String &forKey)
{
    if("property.landscape.texture.tilemask" == forKey)
    {
        if(savedPath.length())
        {
            SaveTextureAction(savedPath);
        }
    }
}

void LandscapeEditorColor::TextureDidChanged(const String &forKey)
{
    if("property.landscape.texture.tilemask" == forKey)
    {
        CreateMaskTexture();
    }
}
