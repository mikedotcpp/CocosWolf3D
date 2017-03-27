//
//  BatchedMesh.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/9/17.
//
//

#include "BatchedMesh.hpp"
#include "BatchedMeshCommand.hpp"

using namespace mikedotcpp;

void BatchedMesh::draw( cocos2d::Renderer* renderer, float globalZOrder, const cocos2d::Mat4& transform, uint32_t flags, unsigned int lightMask,
                       const cocos2d::Vec4& color, bool forceDepthWrite, int instanceCount, const std::vector< cocos2d::Vec3 >& positionPalette )
{
    if( ! isVisible() )
    {
        return;
    }
    
    bool isTransparent = (_isTransparent || color.w < 1.f);
    float globalZ = isTransparent ? 0 : globalZOrder;
    if (isTransparent)
        flags |= cocos2d::Node::FLAGS_RENDER_AS_3D;
    
    _meshCommand.init(globalZ,
                      _material,
                      getVertexBuffer(),
                      getIndexBuffer(),
                      getPrimitiveType(),
                      getIndexFormat(),
                      getIndexCount(),
                      transform,
                      flags);
    
    _material->getStateBlock()->setDepthWrite(true);
    
    
    _meshCommand.setSkipBatching(isTransparent);
    _meshCommand.setTransparent(isTransparent);
    _meshCommand.set3D(!_force2DQueue);
    _material->getStateBlock()->setBlend(_force2DQueue || isTransparent);
    
    _meshCommand.instanceCount = instanceCount;
    
    // set default uniforms for Mesh
    // 'u_color' and others
    const auto scene = cocos2d::Director::getInstance()->getRunningScene();
    auto technique = _material->getTechnique();
    for( const auto pass : technique->getPasses() )
    {
        auto programState = pass->getGLProgramState();
        programState->setUniformVec4("u_color", color);
        
        if( _skin )
        {
            programState->setUniformVec4v("u_matrixPalette", (GLsizei)_skin->getMatrixPaletteSize(), _skin->getMatrixPalette());
        }
        
        if( scene && scene->getLights().size() > 0 )
        {
            setLightUniforms(pass, scene, color, lightMask);
        }
        
        if( positionPalette.size() > 0 )
        {
            programState->setUniformVec3v( "u_posPalette", (GLsizei)positionPalette.size(), &positionPalette[0] );
        }
    }
    
    renderer->addCommand( &_meshCommand );
}

BatchedMesh* BatchedMesh::create(const std::vector<float>& vertices, int perVertexSizeInFloat,
                                 const IndexArray& indices, const std::vector<cocos2d::MeshVertexAttrib>& attribs)
{
    cocos2d::MeshData meshdata;
    meshdata.attribs = attribs;
    meshdata.vertex = vertices;
    meshdata.subMeshIndices.push_back(indices);
    meshdata.subMeshIds.push_back("");
    auto meshvertexdata = cocos2d::MeshVertexData::create(meshdata);
    auto indexData = meshvertexdata->getMeshIndexDataByIndex(0);
    
    return create("", indexData);
}

BatchedMesh* BatchedMesh::create(const std::string& name, cocos2d::MeshIndexData* indexData, cocos2d::MeshSkin* skin)
{
    auto state = new (std::nothrow) BatchedMesh();
    state->autorelease();
    state->bindMeshCommand();
    state->_name = name;
    state->setMeshIndexData(indexData);
    state->setSkin(skin);
    
    return state;
}
