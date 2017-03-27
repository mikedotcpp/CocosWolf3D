//
//  FPHUDLayer.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#ifndef FPHUDLayer_hpp
#define FPHUDLayer_hpp

#include "cocos2d.h"

namespace mikedotcpp
{
    
    /**
     * Organizes and manages anything considered part of the HUD.
     */
    class FPHUDLayer : public cocos2d::Layer
    {
    public:
        /**
         * Creates a new FPHUDLayer object.
         */
        CREATE_FUNC( FPHUDLayer );
        
    protected:
        /**
         * Constructor/destructor
         */
        FPHUDLayer();
        ~FPHUDLayer();
        
    private:
    };
    
}

#endif /* FPHUDLayer_hpp */
