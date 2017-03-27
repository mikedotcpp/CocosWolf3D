//
//  BatchedMesh.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/9/17.
//
//

#ifndef BatchedMesh_hpp
#define BatchedMesh_hpp

#include "cocos2d.h"
#include "BatchedMeshCommand.hpp"

namespace mikedotcpp
{
    /**
     * Created in support of instanced geometry rendering.
     */
    class BatchedMesh : public cocos2d::Mesh
    {
    public:
        BatchedMeshCommand _meshCommand;
        
        BatchedMesh(){};
        
        static BatchedMesh* create(const std::string& name, cocos2d::MeshIndexData* indexData, cocos2d::MeshSkin* skin = nullptr);
        
        static BatchedMesh* create(const std::vector<float>& vertices, int perVertexSizeInFloat,
                                   const IndexArray& indices, const std::vector<cocos2d::MeshVertexAttrib>& attribs);
        
        void draw( cocos2d::Renderer* renderer, float globalZOrder, const cocos2d::Mat4& transform, uint32_t flags,
                  unsigned int lightMask, const cocos2d::Vec4& color, bool forceDepthWrite,
                  int instanceCount, const std::vector< cocos2d::Vec3 >& positionPalette );
        
    };
}

#endif /* BatchedMesh_hpp */
