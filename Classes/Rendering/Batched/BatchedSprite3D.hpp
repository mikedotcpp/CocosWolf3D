//
//  BatchedSprite3D.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/9/17.
//
//

#ifndef BatchedSprite3D_hpp
#define BatchedSprite3D_hpp

#include "cocos2d.h"
#include "BatchedMesh.hpp"

namespace mikedotcpp
{
    /**
     * This is intended to be the same as Sprite3D, but relies on a different type of RenderCommand to allow for
     * batched render calls. It may not be as flexible as the default Sprite3D, but it should be a lot more
     * performant for rendering many instances of the same Sprite3D.
     */
    class BatchedSprite3D : public cocos2d::Sprite3D
    {
    public:
        static BatchedSprite3D* create();
        
        static BatchedSprite3D* create( const std::string &modelPath );
        
        virtual void draw( cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags ) override;
        
    private:
        int _instanceCount = 0;
        
        std::vector< cocos2d::Vec3 > _positionPalette;
        
        BatchedSprite3D(){};
        
    public:
        void setInstanceCount( int count );
        int getInstanceCount();
        
        void setPositionPalette( const std::vector< cocos2d::Vec3 >& positionArray );
        cocos2d::Vec3* getPositionPalette();
    };
}

#endif /* BatchedSprite3D_hpp */
