//
//  Game.cpp
//  GridBasedRaycaster
//
//  Created by Michael Edwards on 3/14/17.
//
//

#include "Game.hpp"
#include "audio/include/AudioEngine.h"

using namespace mikedotcpp;

bool Game::init()
{
    bool result = FPRenderLayer::init();
    
    loadMap( "maps/e1m1/e1m1.json" );
    
    return result;
}

void Game::onEnter()
{
    FPRenderLayer::onEnter();
    
    /*
     ---------------------------------------------------------------------------------------------------
     
        Add your custom behaviors here. Behaviors **MUST** be added after the base class has already run
        it's onEnter() method.
     
     ---------------------------------------------------------------------------------------------------
     */
    addBehavior( new PickupObject( _raycaster, this ) );
    
    showIntro();
}

void Game::showIntro()
{
    cocos2d::Director* director = cocos2d::Director::getInstance();
    cocos2d::LayerColor* introLayer = cocos2d::LayerColor::create( cocos2d::Color4B::BLACK,
                                                                  director->getWinSize().width,
                                                                  director->getWinSize().height );
    
    cocos2d::Sprite* banner = cocos2d::Sprite::create( "CocosWolf3D_Banner.png" );
    banner->setNormalizedPosition( cocos2d::Vec2( 0.5f, 0.5f ) );
    introLayer->addChild( banner );
    
    introLayer->runAction( cocos2d::Sequence::create( cocos2d::FadeOut::create( 5 ), cocos2d::RemoveSelf::create(), NULL ) );
    addChild( introLayer );
    
    cocos2d::experimental::AudioEngine::play2d( "sounds/getthem.wav" );
}

/**
 * When the player enters this tile, we should have it removed from the map. Additionally, you may want to call 
 * some code that increments a counter or stores a reference to the object in some sort of inventory.
 */
void PickupObject::onEnter( cocos2d::Vec3 enterPosition, cocos2d::Vec3 exitPosition )
{
    _raycaster->clearTileResourceAt( mikedotcpp::Point3f( enterPosition.x, enterPosition.y, enterPosition.z ) );
    flashScreen();
}

/**
 * Some feedback for the player to communicate they picked up an item.
 */
void PickupObject::flashScreen()
{
    cocos2d::Size winSize = cocos2d::Director::getInstance()->getWinSize();
    cocos2d::LayerColor* flash = cocos2d::LayerColor::create( cocos2d::Color4B( 255, 255, 0, 100 ), winSize.width, winSize.height );
    cocos2d::FadeOut* fade = cocos2d::FadeOut::create( 0.5f );
    cocos2d::RemoveSelf* removeSelf = cocos2d::RemoveSelf::create();
    cocos2d::Sequence* sequence = cocos2d::Sequence::create( fade, removeSelf, NULL );
    flash->runAction( sequence );
    _layer->addChild( flash );
    cocos2d::experimental::AudioEngine::play2d( "sounds/dsitemup.wav" );
}
