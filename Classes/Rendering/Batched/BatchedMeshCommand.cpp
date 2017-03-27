//
//  BatchedMeshCommand.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/18/17.
//
//

#include "BatchedMeshCommand.hpp"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
#include <OpenGLES/ES2/glext.h>
#endif

using namespace mikedotcpp;

void BatchedMeshCommand::batchDraw()
{
    if( _material )
    {
        for( const auto& pass: _material->getTechnique()->getPasses() )
        {
            pass->bind(_mv);
            
            //FOR MOBILE
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
            glDrawElementsInstancedEXT( _primitive, (GLsizei)_indexCount, _indexFormat, 0, instanceCount );
#else
            //FOR DESKTOP:
            glDrawElementsInstancedARB( _primitive, (GLsizei)_indexCount, _indexFormat, 0, instanceCount );
#endif
            CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _indexCount);
            
            pass->unbind();
        }
    }
    else
    {
        _glProgramState->applyGLProgram(_mv);
        
        // set render state
        applyRenderState();
        
        // Draw
        glDrawElements(_primitive, (GLsizei)_indexCount, _indexFormat, 0);
        CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _indexCount);
    }
}
