//
//  FPRenderLayer.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#include "FPRenderLayer.hpp"

using namespace mikedotcpp;

bool FPRenderLayer::init()
{
    bool result = cocos2d::Layer::init();
    _layer3D = cocos2d::Layer::create();
    addChild( _layer3D );
    
    cocos2d::Director::getInstance()->setDepthTest( true );
    setupPlayerControls();
    scheduleUpdate();
    
    return result;
}

void FPRenderLayer::setupPlayerControls()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
    setKeyboardEnabled( true );
    cocos2d::EventListenerMouse* mouseListener = cocos2d::EventListenerMouse::create();
    mouseListener->onMouseMove = CC_CALLBACK_1( FPRenderLayer::onMouseMove, this );
    mouseListener->onMouseUp = CC_CALLBACK_1( FPRenderLayer::onMouseUp, this );
    mouseListener->onMouseDown = CC_CALLBACK_1( FPRenderLayer::onMouseDown, this );
    _eventDispatcher->addEventListenerWithSceneGraphPriority( mouseListener, this );
#else
    cocos2d::EventListenerTouchOneByOne* listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(FPRenderLayer::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(FPRenderLayer::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(FPRenderLayer::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
}

void FPRenderLayer::onEnter()
{
    cocos2d::Layer::onEnter();
    addDefaultBehaviors();
}

void FPRenderLayer::addDefaultBehaviors()
{
    addBehavior( new HaltMove( _fpsCamera ) );
}

void FPRenderLayer::addBehavior( BehaviorObject* behaviorObject )
{
    _behaviorFunctions.push_back( behaviorObject );
}

void FPRenderLayer::update(float delta)
{
    cocos2d::Vec3 rotation =  cocos2d::Vec3( _fpsCamera->getRotation3D().x, _fpsCamera->getRotation3D().y, _fpsCamera->getRotation3D().z );
    float adjustedRotation = ( rotation.y + _cameraRotationOffset ) * ( MATH_PI/180.0f );
    
    cocos2d::Vec3 pos = _fpsCamera->getPosition3D();
    float playerSpeed = ( ( _updateForwardPosition || _updateBackwardPosition ) && ( _updateStrafeLeft || _updateStrafeRight ) ) ? 3.0f : 4.0f;
    
    static int multiplier = 1;
    float heightTarget = _raycaster->getTileResourceHeight( Point3f( pos.x, pos.y, pos.z ) ) + 128;
    multiplier = ( heightTarget < pos.y ) ? ++multiplier : 1;
    float playerHeight = MAX( heightTarget, pos.y - ( delta * 9.81f * multiplier ) );
    
    if( _updateForwardPosition || _updateBackwardPosition )
    {
        if( _updateForwardPosition )
        {
            pos += cocos2d::Vec3( playerSpeed * sin( adjustedRotation ), 0, playerSpeed * cos( adjustedRotation ) );
        }
        else
        {
            pos -= cocos2d::Vec3( playerSpeed * sin( adjustedRotation ), 0, playerSpeed * cos( adjustedRotation ) );
        }
    }
    if( _updateStrafeLeft || _updateStrafeRight )
    {
        if( _updateStrafeLeft )
        {
            pos += cocos2d::Vec3( playerSpeed * cos( adjustedRotation ), 0, playerSpeed * -sin( adjustedRotation ) ); // STRAFE
        }
        else
        {
            pos -= cocos2d::Vec3( playerSpeed * cos( adjustedRotation ), 0, playerSpeed * -sin( adjustedRotation ) ); // STRAFE
        }
    }
    pos.y = playerHeight;
    triggerBehaviors( _fpsCamera->getPosition3D(), pos );
}

void FPRenderLayer::triggerBehaviors( const cocos2d::Vec3& previousPosition, const cocos2d::Vec3& currentPosition )
{
    _fpsCamera->setPosition3D( currentPosition );
    
    Point3f enterPosition = Point3f( currentPosition.x, currentPosition.y, currentPosition.z );
    Point3f exitPosition = Point3f( previousPosition.x, previousPosition.y, previousPosition.z );
    Point3f triggers[] = { enterPosition, exitPosition };
    
    for( int i = 0; i < sizeof( triggers[0] )/sizeof( Point3f ); ++i )
    {
        int tileResourceIndex = _raycaster->getTileResourceIndex( triggers[i] );
        if( tileResourceIndex >= 0 )
        {
            int behaviorIndex = _mapInfo->tiles[tileResourceIndex].tag;
            if( behaviorIndex >= 0 )
            {
                if( behaviorIndex < _behaviorFunctions.size() )
                {
                    _behaviorFunctions[behaviorIndex]->onEnter( currentPosition, previousPosition );
                }
            }
        }
    }
}

void FPRenderLayer::visit( cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
    cocos2d::Vec3 rotation =  cocos2d::Vec3( _fpsCamera->getRotation3D().x, _fpsCamera->getRotation3D().y, _fpsCamera->getRotation3D().z );
    float adjustedRotation = ( rotation.y + _cameraRotationOffset ) * ( MATH_PI/180.0f );
    Point3f playerPosition( _fpsCamera->getPosition3D().x, _fpsCamera->getPosition3D().y, _fpsCamera->getPosition3D().z );

    _blockManager->reclaimAllBlocks();
    
    resetVisitedPlanes();
    _raycaster->castRays( playerPosition, adjustedRotation );
    
    if( _mapInfo->useRealtimeLighting )
    {
        for( int i = 0; i < _tileCounter.size(); ++i )
        {
            BatchedSprite3D* blockMesh = _blockManager->getMeshBlock( i );
            blockMesh->setInstanceCount( _tileCounter[i] );
            blockMesh->setPositionPalette( _tilePositions[i] );
        }
    }
    
    cocos2d::Layer::visit( renderer, parentTransform, parentFlags );
}

void FPRenderLayer::resetVisitedPlanes()
{
    int mapSize = _mapInfo->width * _mapInfo->height;
    unsigned long planeCount = _mapInfo->planes.size();
    if( _visitedPlanes.size() > 0 )
    {
        for( int i = 0; i < planeCount; ++i )
        {
            delete _visitedPlanes[i];
        }
        _visitedPlanes.clear();
    }
    
    for( int i = 0; i < planeCount; ++i )
    {
        _visitedPlanes.push_back( new int[mapSize]() );
    }
    
    for( int i = 0; i < _tileCounter.size(); ++i )
    {
        _tileCounter[i] = 0;
        _tilePositions[i].clear();
    }
}

bool FPRenderLayer::processHit( int index, float angle, Point3f hit, int tileIndex, int planeIndex )
{
    CCASSERT( tileIndex < _mapInfo->tiles.size(), "Index out of range!" );
    
    int tag = _mapInfo->tiles[tileIndex].tag;
    bool continueProcessing = ( planeIndex != getPlaneIndexForHeight( _fpsCamera->getPosition3D().y ) || tag != 0 );
    
    if( _visitedPlanes[planeIndex][index] == 0 )
    {
        drawBlock( hit, tileIndex );
        _visitedPlanes[planeIndex][index] = 1;
    }
    return continueProcessing;
}

int FPRenderLayer::getPlaneIndexForHeight( float height )
{
    int index = -1;
    for( int i = 0; i < _mapInfo->planes.size(); ++i )
    {
        if( _mapInfo->planes[i].height == height )
        {
            index = i;
            break;
        }
    }
    return index;
}

void FPRenderLayer::drawBlock( Point3f hit, int tileIndex )
{
    cocos2d::Vec3 point = cocos2d::Vec3( hit.x, hit.y, hit.z );
    
    if( _mapInfo->useRealtimeLighting )
    {
        _tileCounter[ tileIndex ] += 1;
        _tilePositions[ tileIndex ].push_back( point );
    }
    else
    {
        cocos2d::Sprite3D* block = _blockManager->getBlock( tileIndex );
        if( block )
        {
            block->setPosition3D( point );
        }
    }
}

void FPRenderLayer::loadMap( const std::string& filename )
{
    CC_SAFE_DELETE( _mapInfo );
    _mapInfo = new MapInfo( filename.c_str() );
    
    CC_SAFE_DELETE( _raycaster );
    _raycaster = new GBRaycaster( *_mapInfo, this );
    
    CC_SAFE_DELETE( _blockManager );
    _blockManager = new BlockManager( *_mapInfo, _layer3D );
    
    _visitedPlanes.clear();
    _visitedPlanes.reserve( _mapInfo->width * _mapInfo->height );
    resetVisitedPlanes();
    
    if( _mapInfo->useRealtimeLighting )
    {
        for( int i = 0; i < _mapInfo->tiles.size(); ++i )
        {
            std::vector< cocos2d::Vec3 > positions;
            _tilePositions.push_back( positions );
            _tileCounter.push_back( 0 );
        }
    }
}

void FPRenderLayer::addFPSCamera( float fieldOfView, float nearPlane, float farPlane )
{
    if( _fpsCamera == nullptr )
    {
        cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();
        Actor player = _mapInfo->actors[0];
        _viewerHeight = player.y;
        
        Point3f startingPos( _raycaster->tilePositionForCoord( Point2i( player.z, player.x ) ) );
        _fpsCamera = cocos2d::Camera::createPerspective( fieldOfView, (GLfloat)winSize.width/winSize.height, nearPlane, farPlane );
        _fpsCamera->setPosition3D( cocos2d::Vec3( startingPos.x, _viewerHeight, startingPos.y ) );
        _fpsCamera->setRotation3D( cocos2d::Vec3( 0.0f, -_cameraRotationOffset - player.yaw, 0.0f ) );
        _fpsCamera->setCameraFlag( cocos2d::CameraFlag::USER1 );
        _fpsCamera->retain();
        _layer3D->addChild( _fpsCamera );
    }
}

cocos2d::Vec3 FPRenderLayer::calculateCameraRotation( cocos2d::Vec2 currentScreenPosition, bool invertPitch )
{
    cocos2d::Vec3 cameraRotation = _fpsCamera->getRotation3D();
    static cocos2d::Size screenSize = cocos2d::Director::getInstance()->getWinSize();
    float xRotation = ( currentScreenPosition.y - _lastScreenPosition.y) * -0.5f;
    float yRotation = ( currentScreenPosition.x - _lastScreenPosition.x) * -0.5f;
    if( invertPitch )
    {
        xRotation *= -1;
    }
    _lastScreenPosition = currentScreenPosition;
    float pitch = MAX( -90.0f, MIN( 90.0f, xRotation + cameraRotation.x ) );
    float yaw = yRotation + cameraRotation.y;
    float roll = 0.f;
    return cocos2d::Vec3( pitch, yaw, roll );
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)

void FPRenderLayer::onKeyPressed( cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event )
{
    if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW || keyCode == cocos2d::EventKeyboard::KeyCode::KEY_W )
    {
        _updateForwardPosition = true;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A )
    {
        _updateStrafeLeft = true;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW || keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S  )
    {
        _updateBackwardPosition = true;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D )
    {
        _updateStrafeRight = true;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE )
    {
        unscheduleUpdate();
        cocos2d::Director::getInstance()->end();   //Close the cocos2d-x game scene and quit the application
    }
}

void FPRenderLayer::onKeyReleased( cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event )
{
    if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW || keyCode == cocos2d::EventKeyboard::KeyCode::KEY_W )
    {
        _updateForwardPosition = false;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_A )
    {
        _updateStrafeLeft = false;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW || keyCode == cocos2d::EventKeyboard::KeyCode::KEY_S )
    {
        _updateBackwardPosition = false;
    }
    else if( keyCode == cocos2d::EventKeyboard::KeyCode::KEY_D )
    {
        _updateStrafeRight = false;
    }
}

void FPRenderLayer::onMouseMove( cocos2d::Event* event )
{
    cocos2d::EventMouse* ms = (cocos2d::EventMouse*)event;
    static bool firstMove = true;
    if( firstMove )
    {
        _lastScreenPosition = ms->getLocation();
        firstMove = false;
    }
    _fpsCamera->setRotation3D( calculateCameraRotation( ms->getLocation() ) );
}

void FPRenderLayer::onMouseDown( cocos2d::Event* event )
{
}

void FPRenderLayer::onMouseUp( cocos2d::Event* event )
{
}

#else

bool FPRenderLayer::onTouchBegan( cocos2d::Touch *touch, cocos2d::Event *unused_event )
{
    if( _firstTouch )
    {
        if( _secondTouch == nullptr )
        {
            _secondTouch = touch;
            _updateForwardPosition = true;
        }
    }
    else
    {
        _firstTouch = touch;
        _lastScreenPosition = touch->getLocation();
    }
    return true;
}

void FPRenderLayer::onTouchMoved( cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
    if( _firstTouch && _firstTouch->getID() == touch->getID() )
    {
        _fpsCamera->setRotation3D( calculateCameraRotation( touch->getLocation(), true ) );
    }
}

void FPRenderLayer::onTouchEnded( cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
    if( _firstTouch && _firstTouch->getID() == touch->getID() )
    {
        _lastScreenPosition = touch->getLocation();
        _firstTouch = nullptr;
    }
    else if( _secondTouch && _secondTouch->getID() == touch->getID() )
    {
        _updateForwardPosition = false;
        _secondTouch = nullptr;
    }
}

void FPRenderLayer::onTouchCancelled( cocos2d::Touch *touch, cocos2d::Event *unused_event)
{
    _firstTouch = nullptr;
    _secondTouch = nullptr;
}

#endif

FPRenderLayer::FPRenderLayer()
{
}

FPRenderLayer::~FPRenderLayer()
{
    if( _raycaster )
    {
        delete _raycaster;
    }
    _raycaster = nullptr;
    if( _blockManager )
    {
        delete _blockManager;
    }
    _blockManager = nullptr;
}

void FPRenderLayer::setViewerHeight( float height )
{
    _viewerHeight = height;
    if( _fpsCamera != nullptr )
    {
        cocos2d::Vec3 pos = _fpsCamera->getPosition3D();
        _fpsCamera->setPosition3D( cocos2d::Vec3( pos.x, _viewerHeight, pos.y ) );
    }
}

float FPRenderLayer::getViewerHeight()
{
    return _viewerHeight;
}
