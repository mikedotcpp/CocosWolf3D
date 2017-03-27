//
//  FPRenderLayer.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#ifndef FPRenderLayer_hpp
#define FPRenderLayer_hpp

#include "cocos2d.h"
#include "../Rendering/Raycaster/GBRaycaster.hpp"
#include "../Rendering/BlockManager.hpp"
#include "../Map/MapInfo.hpp"

namespace mikedotcpp
{
    /**
     * A behavior function is triggered by one of two specific events:
     *
     * 1. Upon entering a tile.
     * 2. Upon exiting a tile.
     *
     * Subclass BehaviorObject to define the functionality for onEnter and onExit triggers (functions). Additional
     * parameters can be defined as member variables to subclassed objects.
     *
     * See the default behavior HaltMove as an example for subclassing.
     */
    class BehaviorObject
    {
    public:
        virtual void onEnter( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) = 0;
        virtual void onExit( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) = 0;
        virtual ~BehaviorObject() = 0;
    };
    inline BehaviorObject::~BehaviorObject(){}
    
    /**
     * A default BehaviorFunction which prevents the player from moving further. It acts as a wall or physical
     * barrier to player movement. Override for custom functionality.
     */
    class HaltMove : public BehaviorObject
    {
    public:
        HaltMove( cocos2d::Camera* playerCamera ) : _playerCamera( playerCamera ) {};
        void onEnter( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) override
        {
            _playerCamera->setPosition3D( exitPosition );
        };
        void onExit( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) override {};
    private:
        cocos2d::Camera* _playerCamera;
    };
    
    /**
     * Manages the object pool and display of Sprite3D objects used by the GBRaycaster class.
     */
    class FPRenderLayer : public cocos2d::Layer, public mikedotcpp::GBRaycasterInterface
    {
    public:
        /**
         * Creates a new FPRenderLayer object.
         */
        CREATE_FUNC( FPRenderLayer );
        
        /**
         * Creates and adds a special CCLayer where all 3D-related objects are stored, enables depth-testing, sets
         * up player controls and schedules the update loop.
         */
        virtual bool init() override;
        
        /**
         * This is where the Behavior setup takes place. The player camera should have already been initialized.
         */
        virtual void onEnter() override;
        
        /**
         * Resets blocks from previous frame, performs raycasting, updates block info with current position/instance
         * count data.
         */
        virtual void visit( cocos2d::Renderer *renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags ) override;
        
        /**
         * Process input flags for movement, update player positioning, call behavior functions.
         */
        virtual void update( float delta ) override;
        
        /**
         * This function will be called when the GBRaycaster detects a wall, for each unique wall that is hit. That
         * is, it should never be called twice in the same loop through the raycasting algorithm.
         *
         * TRUE  - Continue checking for hits along the path of this ray.
         * FALSE - Quit checking hits on this ray path and move to the next ray.
         */
        virtual bool processHit( int index, float angle, mikedotcpp::Point3f hit, int tileIndex, int planeIndex ) override;
        
        /**
         * Adds the player camera to this layer with a custom field of view, near plane, and far plane. This layer
         * keeps track of the player camera and it can only be instantiated once.
         */
        void addFPSCamera( float fieldOfView=80.0f, float nearPlane=0.10f, float farPlane=10000.0f );
        
        /**
         * Performs setup of MapInfo, GBRaycaster, and BlockManager classes according to the map defined by the
         * filename argument.
         */
        void loadMap( const std::string& filename );
        
        /**
         * Add user-defined behaviors to the onEnter and onExit triggers.
         */
        void addBehavior( BehaviorObject* behaviorObject );
        
    protected:
        /**
         * There is a difference between the camera's rotation and the raycaster's viewpoint. It needs a counter-
         * rotation to sync up correctly with the CCCAmera's view. This magic number is in degrees.
         */
        float _cameraRotationOffset = 180.0f;
        
        /**
         * How tall the viewer (player) is in pixels.
         */
        float _viewerHeight = 512.0f;
        
        /**
         * A container layer where all 3D objects are added (including blocks/faces). 
         */
        cocos2d::Layer* _layer3D;
        
        /**
         * The player camera (first person only). Currently, only one camera can be added to this layer. Create
         * and customize using the addFPSCamera() function.
         */
        cocos2d::Camera* _fpsCamera = nullptr;
        
        /**
         * Parses and stores map data to be used by BlockManager and GBRaycaster.
         */
        mikedotcpp::MapInfo* _mapInfo;
        
        /**
         * Performs the raycasting algorithm and provides a potentially visible set for rendering.
         */
        mikedotcpp::GBRaycaster* _raycaster;
        
        /**
         * Manages pools (blocks) of sprites.
         */
        mikedotcpp::BlockManager* _blockManager;
        
        /**
         * Keeps count of each tile that is visible at any given moment. This is used specifically for geometry-
         * instancing.
         */
        std::vector< int > _tileCounter;
        
        /**
         * Keeps track of which plane was visited during the raycasting algorithm so as not to render the same 
         * object more than once. Reset to 0 before running the raycast algorithm.
         */
        std::vector< int* > _visitedPlanes;
        
        /**
         * A collection that stores unique positions of tile objects. Used in geometry-instanced rendering. This
         * list is built up as a result of the raycasting algorithm and cleared before each iteration.
         */
        std::vector< std::vector< cocos2d::Vec3 > > _tilePositions;
        
        /**
         * Pulls the next availalbe block from the BlockManager and draws it in the world. For instanced rendering
         * this code simply updates the _tileCounter and _tilePositions for the tile at tileIndex. 
         */
        void drawBlock( mikedotcpp::Point3f hit, int tileIndex );
        
        /**
         * Keeps track of the tiles that have already been visited during the raycasting algorithm. Records a 1 for each
         * tile hit per plane. This is reset each frame.
         */
        void resetVisitedPlanes();
        
        /**
         * Returns the layer index for the height provided. 
         */
        int getPlaneIndexForHeight( float height );
        
        /**
         * Constructor/destructor
         */
        FPRenderLayer();
        ~FPRenderLayer();
        
        /**
         * _viewerHeight
         */
        void setViewerHeight( float height );
        float getViewerHeight();
        
        //-----------------------------------------------------
        //
        // BEHAVIOR CODE
        //
        //-----------------------------------------------------
    protected:
        /**
         * Collections of functions that are mapped to tiles by the "tag" property. Conditionally invoked in the
         * update() function depending on the tile tag and player position.
         */
        std::vector< BehaviorObject* > _behaviorFunctions;
        
        /**
         * Simply appends any behavior functions to the _behaviorFunctions trigger list.
         */
        void addDefaultBehaviors();
        
        /**
         * Maps previous and current positions to enter/exit position variables to determine what (if any)
         * BehaviorFunctions should be called. 
         */
        void triggerBehaviors( const cocos2d::Vec3& previousPosition, const cocos2d::Vec3& currentPosition );
        
        //-----------------------------------------------------
        //
        // PLAYER CONTROL CODE
        //
        //-----------------------------------------------------
    protected:
        /**
         * Flow control flags that modulate when a new position should be calculated for various movements (walk,
         * strafe, etc.)
         */
        bool _updateForwardPosition = false;
        bool _updateStrafeLeft = false;
        bool _updateStrafeRight = false;
        bool _updateBackwardPosition = false;
        
        /**
         * The last recorded screen position; used to calculate a new screen offset. 
         */
        cocos2d::Vec2 _lastScreenPosition;
        
        /**
         * Calculates a new rotation based on the last known screen coordinates. The resulting Vec3 is made up of
         * [ pitch, yaw, roll ] in that order.
         */
        cocos2d::Vec3 calculateCameraRotation( cocos2d::Vec2 currentScreenPosition, bool invertPitch=false );
        
        /**
         * Perfomrs first-time setup of player controls using mouse/keyboard or touch input functions depending on
         * the platform.
         */
        void setupPlayerControls();
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        /**
         * Overridden event methods for desktop controls.
         */
        virtual void onKeyPressed( cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event ) override;
        virtual void onKeyReleased( cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event ) override;
        void onMouseMove( cocos2d::Event* event );
        void onMouseDown( cocos2d::Event* event );
        void onMouseUp( cocos2d::Event* event );
#else
        
        /**
         * The first and second touches that were detected. Only supported on touch-enabled devices (obvs).
         */
        cocos2d::Touch* _firstTouch = nullptr;
        cocos2d::Touch* _secondTouch = nullptr;
        
        /**
         * Overridden event methods for touchscreen controls.
         */
        virtual bool onTouchBegan( cocos2d::Touch *touch, cocos2d::Event *unused_event ) override;
        virtual void onTouchMoved( cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
        virtual void onTouchEnded( cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
        virtual void onTouchCancelled( cocos2d::Touch *touch, cocos2d::Event *unused_event) override;
#endif
        
    };
    
}

#endif /* FPRenderLayer_hpp */
