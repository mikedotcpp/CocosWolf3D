/****************************************************************************
 Copyright (c) 2014 Chukong Technologies Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 Modified by Michael Edwards on 1/31/17
 ****************************************************************************/

#ifndef __FPBILLBOARD_H__
#define __FPBILLBOARD_H__

#include "cocos2d.h"
#include "2d/CCSprite.h"

namespace mikedotcpp
{
    /**
     * A First-Person Billboard class that is essentially the same as the cocos2d::Billboard class. The major
     * difference here is the need to lock rotation along the z-axis when viewing billboards with a 6DF first
     * person camera.
     */
    class FPBillBoard : public cocos2d::Sprite
    {
    public:
        
        enum class Mode
        {
            VIEW_POINT_ORIENTED, // orient to the camera
            VIEW_PLANE_ORIENTED // orient to the XOY plane of camera
        };
        
        /**
         * Creates an empty BillBoard without texture. You can call setTexture method subsequently.
         *
         * @return An autoreleased BillBoard object.
         */
        static FPBillBoard* create(Mode mode = Mode::VIEW_POINT_ORIENTED);
        
        /**
         * Creates a BillBoard with an image filename.
         *
         * After creation, the rect of BillBoard will be the size of the image,
         * and the offset will be (0,0).
         *
         * @param   filename A path to image file, e.g., "scene1/monster.png"
         * @return  An autoreleased BillBoard object.
         */
        static FPBillBoard* create(const std::string& filename, Mode mode = Mode::VIEW_POINT_ORIENTED);
        
        /**
         * Creates a BillBoard with an image filename and a rect.
         *
         * @param   filename A path to image file, e.g., "scene1/monster.png"
         * @param   rect     A subrect of the image file
         * @return  An autoreleased BillBoard object
         */
        static FPBillBoard* create(const std::string& filename, const cocos2d::Rect& rect, Mode mode = Mode::VIEW_POINT_ORIENTED);
        
        /**
         * Creates a BillBoard with a Texture2D object.
         *
         * After creation, the rect will be the size of the texture, and the offset will be (0,0).
         *
         * @param   texture    A pointer to a Texture2D object.
         * @return  An autoreleased BillBoard object
         */
        static FPBillBoard* createWithTexture( cocos2d::Texture2D *texture, Mode mode = Mode::VIEW_POINT_ORIENTED);
        
        /** Set the billboard rotation mode. */
        void setMode(Mode mode);
        
        /** Get the billboard rotation mode. */
        Mode getMode() const;
        
        //override
        
        /** update billboard's transform and turn it towards camera */
        virtual void visit( cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags ) override;
        
        /**
         * draw BillBoard object.
         *
         * @lua NA
         */
        virtual void draw( cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags ) override;
        
        
    CC_CONSTRUCTOR_ACCESS:
        FPBillBoard();
        virtual ~FPBillBoard();
        
    protected:
        
        /**
         * calculate a model matrix which keep original translate & scaling but always face to the camera
         */
        bool calculateBillbaordTransform();
        
        cocos2d::Mat4 _camWorldMat;
        cocos2d::Mat4 _mvTransform;
        
        Mode _mode;
        bool _modeDirty;
        
    private:
        CC_DISALLOW_COPY_AND_ASSIGN(FPBillBoard);
        
    };
}

#endif // __CCBILLBOARD_H__
