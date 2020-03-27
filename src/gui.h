///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "font.h"

#include <vector>
#include <string>


class GUIControl
{
protected:
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    std::string mTextureFile = "";
    bool mVisible = true;;

public:
    GUIControl() {}
    virtual ~GUIControl() {}

    // For now, empty string means use the font texture/shader... let's not overengineer this yet :)
    std::string TextureFile() const { return mTextureFile; }

    void Visible(bool visible) { mVisible = visible; }
    bool Visible() const { return mVisible; }

    virtual SpriteVertex* Draw(float viewportWidth, float viewportHeight, SpriteVertex* outVertex) const = 0;

    bool HitTest(int x, int y) const
    {
        return mVisible && (x >= mX && x < (mX + mWidth) && y > mY && y < (mY + mHeight));
    }
};


class GUIText : public GUIControl
{
private:
    std::string mText;
    const BitmapFont* mFont;

    void ComputeDimensions()
    {
        mFont->GetDimensions(mText.c_str(), &mWidth, &mHeight);
    }

public:
    // Font lifetime managed by caller
    GUIText(int x, int y, const BitmapFont* font, const std::string& text)
        : mFont(font), mText(text)
    {
        mX = x;
        mY = y;
        ComputeDimensions();
    }

    void Text(const std::string& text)
    {
        mText = text;
        ComputeDimensions();
    }

    virtual SpriteVertex* Draw(float viewportWidth, float viewportHeight, SpriteVertex* outVertex) const override
    {
        return mFont->DrawString(mText.c_str(), float(mX), float(mY), viewportWidth, viewportHeight, outVertex);
    }
};


class GUISprite : public GUIControl
{
private:
    std::string mSpriteFile;
    
public:
    GUISprite(int x, int y, int width, int height, const std::string& spriteFile)
        : mSpriteFile(spriteFile)
    {
        mX = x;
        mY = y;
        mWidth = width;
        mHeight = height;
        mTextureFile = spriteFile;
    }
    
    virtual SpriteVertex* Draw(float viewportWidth, float viewportHeight, SpriteVertex* outVertex) const override
    {
        return DrawSprite(float(mX), float(mY), float(mWidth), float(mHeight), viewportWidth, viewportHeight, outVertex);
    }
};


class GUI
{
private:
    std::vector<GUIControl*> mControls;
    IntelClearBold mFont; // Single font for the entire GUI works for now

public:
    GUI()
    {
    }
    ~GUI()
    {
        for (auto i : mControls) {
            delete i;
        }
    }
    
    const BitmapFont* Font() const { return &mFont; }

    GUIText* AddText(int x, int y, const std::string& text = "")
    {
        auto control = new GUIText(x, y, &mFont, text);
        mControls.push_back(control);
        return control;
    }

    GUISprite* AddSprite(int x, int y, int width, int height, const std::string& spriteFile)
    {
        auto control = new GUISprite(x, y, width, height, spriteFile);
        mControls.push_back(control);
        return control;
    }

    // NOTE: Caller needs to preadjust x/y for any stretching/scaling going on
    GUIControl* HitTest(int x, int y) const
    {
        for (auto i : mControls) {
            if (i->HitTest(x, y)) return i;
        }
        return nullptr;
    }

    GUIControl* operator[](size_t i) { return mControls[i]; }
    size_t size() const { return mControls.size(); }
};
