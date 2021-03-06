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
=====================================================================================*/

#include "PropertyLineEditControl.h"

REGISTER_CLASS(PropertyLineEditControl); 

PropertyLineEditControl::PropertyLineEditControl()
{
	minX = 0.0f;
	maxX = 1.0f;
	
	minY = 0.0f;
	maxY = 1.0f;
	
	values.push_back(PropertyRect(0.0f, 0.5f));
	values.push_back(PropertyRect(1.0f, 0.5f));

	SetClipContents(true);
	
	activeValueIndex = -1;
    selectedValueIndex = -1;

	curTime = 0;
    
    text = new UIStaticText(Rect(0, 0, 10, 15));
    Font *font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    font->SetSize(12);
    font->SetColor(Color(1,1,1,1));
    text->SetFont(font);
    AddControl(text);
    SafeRelease(font);
}

PropertyLineEditControl::~PropertyLineEditControl()
{
	
}

void PropertyLineEditControl::SetMaxY(float32 max)
{
    maxY = max;
}

void PropertyLineEditControl::SetMinY(float32 min)
{
    minY = min;
}

void PropertyLineEditControl::SetMinX(float32 value)
{
    minX = value;
}

void PropertyLineEditControl::SetMaxX(float32 value)
{
    maxX = value;
}

const Rect & PropertyLineEditControl::GetWorkZone()
{
	workZone = GetRect(true);
	workZone.x += 10;
	workZone.y += 10;
	workZone.dx -= 20;
	workZone.dy -= 20;
	return workZone;
}

void PropertyLineEditControl::Reset()
{
    values.clear();
	values.push_back(PropertyRect(0.0f, 0.5f));
	values.push_back(PropertyRect(1.0f, 0.5f));
	activeValueIndex = -1;
    selectedValueIndex = -1;
    delegate->OnPointSelected(this, -1, Vector2(0, 0));
}

int32 PropertyLineEditControl::FindActiveValueFromPosition(const Vector2 & absolutePoint)
{
	for (int32 k = 0; k < (int32)values.size(); ++k)
	{
		Rect valRect = RectFromPosition(CalcRealPosition(values[k]));
		if (valRect.PointInside(absolutePoint))
		{
            return k;
		}
	}
	return -1;
}

int32 PropertyLineEditControl::FindValueForInsertion(const Vector2 & absolutePoint)
{
	for (int32 k = 0; k < (int32)values.size(); ++k)
	{
		Vector2 pos = CalcRealPosition(values[k]);
		if (absolutePoint.x < pos.x)
			return k;
	}
	return (int32)values.size();
}

void PropertyLineEditControl::SetDelegate(PropertyLineEditControlDelegate *controlDelegate)
{
    delegate = controlDelegate;
}

void PropertyLineEditControl::AddPoint(float32 t, float32 value)
{
    int32 k;
	for (k = 0; k < (int32)values.size(); k++)
	{
		if (t < values[k].x)
			break;
	}
    Vector<PropertyRect>::iterator r = values.insert(values.begin() + k, PropertyRect(t, value));
}

void PropertyLineEditControl::DeletePoint(float32 t)
{
    int32 k = -1;
	for (int32 i = 0; i < (int32)values.size(); i++)
	{
		if(values[i].x == t)
        {
            k = i;
            break;
        }
	}
    if (k != -1)
    {
        values.erase( values.begin() + k );
    }
    selectedValueIndex = -1;
    delegate->OnPointSelected(this, -1, Vector2(0, 0));
}

void PropertyLineEditControl::MovePoint(float32 lastT, float32 newT, float32 newV, bool changeV)
{
	int32 size = values.size();
    for(int32 i = 0; i < size; i++)
    {
        if(values.at(i).x == lastT)
        {
            values.at(i).x = newT;
            if(changeV)
            {
                values.at(i).y = newV;
            }
            if(i < size - 1 && values.at(i+1).x == newT)
            {
                values.erase(values.begin()+i+1);
                selectedValueIndex = -1;
            }
            if(i > 0 && values.at(i-1).x == newT)
            {
                values.erase(values.begin()+i-1);
                selectedValueIndex = -1;
            }
            break;
        }
    }
}

void PropertyLineEditControl::Input(UIEvent * touch)
{
	Vector2 absolutePoint = touch->point;
	
	if (touch->tid == UIEvent::BUTTON_1)
	{
        static float32 lastX = 0;
        static float32 lastY = 0;
		if (touch->phase == UIEvent::PHASE_BEGAN)
		{
			activeValueIndex = FindActiveValueFromPosition(absolutePoint);
            selectedValueIndex = activeValueIndex;
            delegate->OnPointSelected(this, activeValueIndex, Vector2(values[activeValueIndex].x, values[activeValueIndex].y));
            lastX = values[activeValueIndex].x;
            lastY = values[activeValueIndex].y;
		}
		
		if (touch->phase == UIEvent::PHASE_DRAG)
		{
			if (activeValueIndex != -1)
			{                
				FromMouseToPoint(absolutePoint, values[activeValueIndex]);
			}
		}
		
		if (touch->phase == UIEvent::PHASE_ENDED)
		{
            if (activeValueIndex != -1)
            {
                float32 tx = values[activeValueIndex].x;
                float32 ty = values[activeValueIndex].y;
                values[activeValueIndex].x = lastX;
                values[activeValueIndex].y = lastY;
                
                delegate->OnPointMove(this, lastX, tx, ty);
                delegate->OnPointSelected(this, activeValueIndex, Vector2(values[activeValueIndex].x, values[activeValueIndex].y));
            }
			activeValueIndex = -1;
		}
	}
	
	if (touch->tid == UIEvent::BUTTON_2)
	{
		if (touch->phase == UIEvent::PHASE_ENDED)
		{
			int32 k = FindActiveValueFromPosition(absolutePoint);
			if (k != -1)
			{
                if(values.size() > 1)
                    delegate->OnPointDelete(this, values[k].x);
			}
            else
			{
                PropertyRect r(0, 0);
                FromMouseToPoint(absolutePoint, r);
                
                delegate->OnPointAdd(this, r.x, r.y);
			}
		}
	}
    
    if (touch->phase == UIEvent::PHASE_MOVE)
    {
        PropertyRect r(0, 0);
        FromMouseToPoint(absolutePoint, r);
        curTime = r.x;
        delegate->OnMouseMove(this, r.x);
    }
}

void PropertyLineEditControl::SetCurTime(float32 t)
{
    curTime = t;
}

float32 PropertyLineEditControl::GetCurTime()
{
    return curTime;
}


void PropertyLineEditControl::Update(float32 timeElapsed)
{
	
}

PropertyLineKeyframes<float32> * PropertyLineEditControl::GetPropertyLine()
{
    PropertyLineKeyframes<float32> *prop = new PropertyLineKeyframes<float32>();
	int32 size = values.size();
    for(int i = 0; i < size; i++)
    {
        prop->AddValue(values.at(i).x, values.at(i).y);
    }
    return prop;
}

void PropertyLineEditControl::SetByPropertyLine(PropertyLineKeyframes<float32> *props)
{
    values.clear();
	int32 size = props->keys.size();
    for(int i = 0; i < size; i++)
    {
        values.push_back(PropertyRect(props->keys.at(i).t, props->keys.at(i).value));
    }
}

Vector<PropertyLineEditControl::PropertyRect> & PropertyLineEditControl::GetValues()
{
    return values;
}

bool PropertyLineEditControl::GetSelectedValue(Vector2 &v)
{
    if(selectedValueIndex != -1)
        v = Vector2(values[selectedValueIndex].x, values[selectedValueIndex].y);
    else
        return false;
    return true;
}

void PropertyLineEditControl::SetSelectedValue(const Vector2 &v)
{
    if(selectedValueIndex != -1)
    {
        Vector2 v2 = v;
        if(v2.x < minX)
            v2.x = minX;
        if(v2.x > maxX)
            v2.x = maxX;
        delegate->OnPointMove(this, values[selectedValueIndex].x, v2.x, v2.y);
    }
}

void PropertyLineEditControl::DeselectPoint()
{
    selectedValueIndex = -1;
}

void PropertyLineEditControl::FromMouseToPoint(Vector2 vec, PropertyRect & rect)
{
	Rect controlRect = GetWorkZone();
	
	controlRect.ClampToRect(vec);
	
	// go to coords of work rect
	vec.x -= controlRect.x;
	vec.y -= controlRect.y;
	
	float32 rectX = (maxX - minX) * vec.x / controlRect.dx + minX;
	float32 rectY = -(maxY - minY) * vec.y / controlRect.dy + maxY;
	
	PropertyRect prev(minX, minY);
	PropertyRect next(maxX, maxY);
	
	if (activeValueIndex != -1)
	{
		if (activeValueIndex > 0)
			prev = values[activeValueIndex - 1];
		if (activeValueIndex < (int32)values.size() - 1)
			next = values[activeValueIndex + 1];
	}
	
	if (rectX < prev.x)rectX = prev.x;
	if (rectX > next.x)rectX = next.x;
	
	rect.x = rectX;
	rect.y = rectY;
}

Vector2 PropertyLineEditControl::CalcRealPosition(const PropertyRect & rect, bool absolute)
{
	const Rect & controlRect = GetWorkZone();
	
	Vector2 vec;
	vec.x = (rect.x - minX) * controlRect.dx / (maxX - minX);
	vec.y = (maxY - rect.y) * controlRect.dy / (maxY - minY);
	if (absolute)
	{
		vec.x += controlRect.x;
		vec.y += controlRect.y;
	}
	return vec;
}

Rect PropertyLineEditControl::RectFromPosition(const Vector2 & pos)
{
	Rect r;
	r.x = pos.x - 4;
	r.y = pos.y - 4;
	r.dx = 8;
	r.dy = 8;	
	return r;
}

void PropertyLineEditControl::SetText(WideString string)
{
    text->SetText(string);
}

void PropertyLineEditControl::Draw(const UIGeometricData &geometricData)
{
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, .3f);
	RenderHelper::Instance()->FillRect(geometricData.GetUnrotatedRect());
    
    const Rect & cRect = GetWorkZone();
    
    RenderManager::Instance()->SetColor(0.8f, 0.6f, 0.6f, 1.0f);
    RenderHelper::Instance()->DrawLine(CalcRealPosition(PropertyRect(curTime, maxY)), CalcRealPosition(PropertyRect(curTime, minY)));
    
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	RenderHelper::Instance()->DrawRect(cRect);
    
    RenderHelper::Instance()->DrawLine(Vector2(cRect.x - 3, cRect.y + cRect.dy/2), Vector2(cRect.x + 3, cRect.y + cRect.dy/2));
    RenderHelper::Instance()->DrawLine(Vector2(cRect.x + cRect.dx - 3, cRect.y + cRect.dy/2), Vector2(cRect.x + cRect.dx + 3, cRect.y + cRect.dy/2));
    RenderHelper::Instance()->DrawLine(Vector2(cRect.x + cRect.dx/2, cRect.y - 3), Vector2(cRect.x + cRect.dx/2, cRect.y + 3));
    RenderHelper::Instance()->DrawLine(Vector2(cRect.x + cRect.dx/2, cRect.y + cRect.dy - 3), Vector2(cRect.x + cRect.dx/2, cRect.y + cRect.dy + 3));
    
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	for (int32 k = 0; k < (int32)values.size() - 1; ++k)
	{
		RenderHelper::Instance()->DrawLine(CalcRealPosition(values[k]), CalcRealPosition(values[k + 1]));
	}
	
	Vector2 vlast = CalcRealPosition(values[(int32)values.size() - 1]);
	Vector2 vlast2 = CalcRealPosition(PropertyRect(maxX, maxY));
	vlast2.y = vlast.y;
	
	RenderHelper::Instance()->DrawLine(vlast, vlast2);
	
	for (int32 k = 0; k < (int32)values.size(); ++k)
	{
		Vector2 pos = CalcRealPosition(values[k]);
        if(activeValueIndex == k)
        {
            RenderManager::Instance()->SetColor(0.6f, 0.6f, 0.6f, 1.0f);
            RenderHelper::Instance()->FillRect(RectFromPosition(pos));
        }
        else if(selectedValueIndex == k)
        {
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            RenderHelper::Instance()->FillRect(RectFromPosition(pos));
        }
        else
        {
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            RenderHelper::Instance()->DrawRect(RectFromPosition(pos));
        }
	}
}
