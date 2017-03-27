//
//  BatchedMeshCommand.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/18/17.
//
//

#ifndef BatchedMeshCommand_hpp
#define BatchedMeshCommand_hpp

#include "cocos2d.h"

namespace mikedotcpp
{
    /**
     * Created in support of instanced geometry rendering.
     */
    class BatchedMeshCommand : public cocos2d::MeshCommand
    {
    public:
        int instanceCount = 0;
        
        void batchDraw();
    };
}

#endif /* BatchedMeshCommand_hpp */
