//
//  Game.hpp
//  GridBasedRaycaster
//
//  Created by Michael Edwards on 3/14/17.
//
//

#ifndef Game_hpp
#define Game_hpp

#include "Layers/FPRenderLayer.hpp"

class Game : public mikedotcpp::FPRenderLayer
{
public:
    CREATE_FUNC( Game );
    
    virtual bool init() override;
    virtual void onEnter() override;
    
    void showIntro();
};

/**
 * A BehaviorFunction that allows players to walk up to tiles defined as items and pick them up
 * (remove them from the world) as the player walks over the item. **NOTE** there is no storage mechanism
 * or inventory implemented; override for custom functionality.
 */
class PickupObject : public mikedotcpp::BehaviorObject
{
public:
    PickupObject( mikedotcpp::GBRaycaster* gbRaycaster, cocos2d::Layer* layer ) : _raycaster( gbRaycaster ), _layer( layer ){};
    void onEnter( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) override;
    void onExit( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition ) override {};
private:
    mikedotcpp::GBRaycaster* _raycaster;
    cocos2d::Layer* _layer;
    void flashScreen();
};

#endif /* Game_hpp */
