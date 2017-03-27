//
//  FPScene.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#include "FPScene.hpp"

FPScene::FPScene()
{
    cocos2d::log( "FPScene created, allocate resources." );
}

FPScene::~FPScene()
{
    cocos2d::log( "FPScene deleted, release resources." );
}

bool FPScene::init()
{
    cocos2d::log( "Initializing the FPScene." );
    bool ret = cocos2d::Scene::init();
    
    _game = Game::create();
    _game->addFPSCamera( _fieldOfView, _nearPlane, _farPlane );
    addChild( _game );
    
    _hudLayer = mikedotcpp::FPHUDLayer::create();
    addChild( _hudLayer );
    
    return ret;
}
