//
//  FPScene.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#ifndef FPScene_hpp
#define FPScene_hpp

#include "cocos2d.h"
#include "Game.hpp"
#include "../Layers/FPRenderLayer.hpp"
#include "../Layers/FPHUDLayer.hpp"

/**
 * Subclass FPScene to create an FPS game.
 */
class FPScene : public cocos2d::Scene
{
public:
    /**
     * Creates a new FPScene object.
     */
    CREATE_FUNC( FPScene );
    
    /**
     * Do special initialization work here.
     */
    virtual bool init();
    
protected:
    /**
     * The extent of the observable world that is seen at any given moment.
     */
    float _fieldOfView = 60.0f;
    
    /**
     * The plane closest to the player that cut the view frustum perpendicular to the viewing direction.
     */
    float _nearPlane = 0.10f;
    
    /**
     * The plane farthest from the player that cut the view frustum perpendicular to the viewing direction.
     */
    float _farPlane = 10000.0f;
    
    /**
     * This layer manages the sprite pool used for rendering and has an
     * instance of the Raycaster class to perform the visibility algorithm.
     */
    Game* _game;
    
    /**
     * Manages and organizes all HUD level elements.
     */
    mikedotcpp::FPHUDLayer* _hudLayer;
    
    /**
     * Constructor.
     */
    FPScene();
    
    /**
     * Destructor.
     */
    ~FPScene();
};

#endif /* FPScene_hpp */
