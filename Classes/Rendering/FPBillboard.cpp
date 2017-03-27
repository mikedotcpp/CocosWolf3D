//
//  FPBillboard.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 1/31/17.
//
//
#include "FPBillboard.hpp"
#include "2d/CCSpriteFrameCache.h"
#include "base/CCDirector.h"
#include "2d/CCCamera.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCGLProgramCache.h"

using namespace mikedotcpp;

FPBillBoard::FPBillBoard()
: _mode(Mode::VIEW_POINT_ORIENTED)
, _modeDirty(false)
{
    cocos2d::Node::setAnchorPoint( cocos2d::Vec2(0.5f,0.5f) );
}

FPBillBoard::~FPBillBoard()
{
}

FPBillBoard* FPBillBoard::createWithTexture( cocos2d::Texture2D *texture, Mode mode)
{
    FPBillBoard *billboard = new (std::nothrow) FPBillBoard();
    if (billboard && billboard->initWithTexture(texture))
    {
        billboard->_mode = mode;
        billboard->autorelease();
        return billboard;
    }
    CC_SAFE_DELETE(billboard);
    return nullptr;
}

FPBillBoard* FPBillBoard::create(const std::string& filename, Mode mode)
{
    FPBillBoard *billboard = new (std::nothrow) FPBillBoard();
    if (billboard && billboard->initWithFile(filename))
    {
        billboard->_mode = mode;
        billboard->autorelease();
        return billboard;
    }
    CC_SAFE_DELETE(billboard);
    return nullptr;
}

FPBillBoard* FPBillBoard::create(const std::string& filename, const cocos2d::Rect& rect, Mode mode)
{
    FPBillBoard *billboard = new (std::nothrow) FPBillBoard();
    if (billboard && billboard->initWithFile(filename, rect))
    {
        billboard->_mode = mode;
        billboard->autorelease();
        return billboard;
    }
    CC_SAFE_DELETE(billboard);
    return nullptr;
}

FPBillBoard* FPBillBoard::create(Mode mode)
{
    FPBillBoard *billboard = new (std::nothrow) FPBillBoard();
    if (billboard && billboard->init())
    {
        billboard->_mode = mode;
        billboard->autorelease();
        return billboard;
    }
    CC_SAFE_DELETE(billboard);
    return nullptr;
}

void FPBillBoard::visit( cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags )
{
    // quick return if not visible. children won't be drawn.
    if (!_visible)
    {
        return;
    }
    bool visibleByCamera = isVisitableByVisitingCamera();
    
    uint32_t flags = processParentFlags(parentTransform, parentFlags);
    
    //Add 3D flag so all the children will be rendered as 3D object
    flags |= FLAGS_RENDER_AS_3D;
    
    //Update Billboard transform
    bool dirty = calculateBillbaordTransform();
    if(dirty)
    {
        flags |= FLAGS_TRANSFORM_DIRTY;
    }
    
    cocos2d::Director* director = cocos2d::Director::getInstance();
    director->pushMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);
    
    int i = 0;
    
    if(!_children.empty())
    {
        sortAllChildren();
        // draw children zOrder < 0
        for( ; i < _children.size(); i++ )
        {
            auto node = _children.at(i);
            
            if (node && node->getLocalZOrder() < 0)
                node->visit(renderer, _modelViewTransform, flags);
            else
                break;
        }
        // self draw
        if (visibleByCamera)
            this->draw(renderer, _modelViewTransform, flags);
        
        for(auto it=_children.cbegin()+i; it != _children.cend(); ++it)
            (*it)->visit(renderer, _modelViewTransform, flags);
    }
    else if (visibleByCamera)
    {
        this->draw(renderer, _modelViewTransform, flags);
    }
    
    director->popMatrix( cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW );
}

bool FPBillBoard::calculateBillbaordTransform()
{
    //Get camera world position
    auto camera = cocos2d::Camera::getVisitingCamera();
    const cocos2d::Mat4& camWorldMat = camera->getNodeToWorldTransform();
    
    //TODO: use math lib to calculate math lib Make it easier to read and maintain
    if (memcmp(_camWorldMat.m, camWorldMat.m, sizeof(float) * 16) != 0 || memcmp(_mvTransform.m, _modelViewTransform.m, sizeof(float) * 16) != 0 || _modeDirty || true)
    {
        //Rotate based on anchor point
        cocos2d::Vec3 anchorPoint( _anchorPointInPoints.x, _anchorPointInPoints.y , 0.0f);
        cocos2d::Mat4 localToWorld = _modelViewTransform;
        localToWorld.translate(anchorPoint);
        
        //Decide billboard mode
        cocos2d::Vec3 camDir;
        switch (_mode)
        {
            case Mode::VIEW_POINT_ORIENTED:
                camDir.set(localToWorld.m[12] - camWorldMat.m[12], localToWorld.m[13] - camWorldMat.m[13], localToWorld.m[14] - camWorldMat.m[14]);
                break;
            case Mode::VIEW_PLANE_ORIENTED:
                camWorldMat.transformVector( cocos2d::Vec3(0.0f, 0.0f, -1.0f), &camDir );
                break;
            default:
                CCASSERT(false, "invalid billboard mode");
                break;
        }
        _modeDirty = false;
        
        if (camDir.length() < MATH_TOLERANCE)
        {
            camDir.set(camWorldMat.m[8], camWorldMat.m[9], camWorldMat.m[10]);
        }
        camDir.normalize();
        
        cocos2d::Quaternion rotationQuaternion;
        this->getNodeToWorldTransform().getRotation(&rotationQuaternion);
        
        cocos2d::Mat4 rotationMatrix;
        rotationMatrix.setIdentity();
        
        cocos2d::Vec3 upAxis(rotationMatrix.m[4],rotationMatrix.m[5],rotationMatrix.m[6]);
        cocos2d::Vec3 x, y;
        camWorldMat.transformVector(upAxis, &y);
        cocos2d::Vec3::cross(camDir, y, &x);
        x.normalize();
        cocos2d::Vec3::cross(x, camDir, &y);
        y.normalize();
        
        float xlen = sqrtf(localToWorld.m[0] * localToWorld.m[0] + localToWorld.m[1] * localToWorld.m[1] + localToWorld.m[2] * localToWorld.m[2]);
        float ylen = sqrtf(localToWorld.m[4] * localToWorld.m[4] + localToWorld.m[5] * localToWorld.m[5] + localToWorld.m[6] * localToWorld.m[6]);
        float zlen = sqrtf(localToWorld.m[8] * localToWorld.m[8] + localToWorld.m[9] * localToWorld.m[9] + localToWorld.m[10] * localToWorld.m[10]);
        
        cocos2d::Mat4 billboardTransform;
        
        billboardTransform.m[0] = x.x * xlen; billboardTransform.m[1] = 0; billboardTransform.m[2] = x.z * xlen;
        billboardTransform.m[4] = 0; billboardTransform.m[5] = y.y * ylen; billboardTransform.m[6] = 0;
        billboardTransform.m[8] = -camDir.x * zlen; billboardTransform.m[9] = 0; billboardTransform.m[10] = -camDir.z * zlen;
        billboardTransform.m[12] = localToWorld.m[12]; billboardTransform.m[13] = localToWorld.m[13]; billboardTransform.m[14] = localToWorld.m[14];
        
        billboardTransform.translate(-anchorPoint);
        _mvTransform = _modelViewTransform = billboardTransform;
        
        _camWorldMat = camWorldMat;
        
        return true;
    }
    
    return false;
}

void FPBillBoard::draw( cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags )
{
    //FIXME: frustum culling here
    flags |= Node::FLAGS_RENDER_AS_3D;
    _trianglesCommand.init(0, _texture->getName(), getGLProgramState(), _blendFunc, _polyInfo.triangles, _modelViewTransform, flags);
    _trianglesCommand.setTransparent(true);
    _trianglesCommand.set3D(true);
    renderer->addCommand(&_trianglesCommand);
}

void FPBillBoard::setMode( Mode mode )
{
    _mode = mode;
    _modeDirty = true;
}

FPBillBoard::Mode FPBillBoard::getMode() const
{
    return _mode;
}
