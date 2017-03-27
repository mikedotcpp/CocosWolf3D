//
//  MapInfo.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 1/10/17.
//
//

#ifndef MapInfo_hpp
#define MapInfo_hpp

#include "cocos2d.h"
#include "external/json/document.h"
#include "MapStructs.h"

namespace mikedotcpp
{
    /**
     * Defines the properties that compose a map for the GBRaycaster.
     */
    class MapInfo
    {
    public:
        
        /**
         * The namespace of the textures/actors/etc used by the map (path in Resources).
         */
        std::string path;
        
        /**
         * Name of the map.
         */
        std::string name;
        
        /**
         * Version string of the map format.
         */
        std::string version;
        
        /**
         * The width (in pixels) of a map tile. Wolfenstein3D uses 64.
         */
        int tileSize;
        
        /**
         * Width of the map (in tiles).
         */
        int width;
        
        /**
         * Height of the map (in tiles).
         */
        int height;
        
        /**
         * TODO: Define the light positions in the map itself. Realtime lighting requires the use of instanced
         *       geometry and may not be possible on some hardware (requires OpenGL ES extensions to 2.0 or just
         *       3.0).
         */
        bool useRealtimeLighting = false;
        
        /**
         * Collection of spritesheet names that this map depends on (includes the file extension).
         */
        std::vector< std::string > spritesheets;
        
        /**
         * Collection of the Tile objects.
         */
        TileCollection tiles;
        
        /**
         * Collection of the Plane objects.
         */
        PlaneCollection planes;
        
        /**
         * Collection of Actor objects in the map. "Player1" is REQUIRED.
         */
        ActorCollection actors;
        
        /**
         * Actions that affect actors (including the player). Currently, they can only be activated by entering or
         * exiting a tile-space. More complex interactions can be added and should be evaluated by the implementer.
         */
        BehaviorCollection behaviors;
        
        /**
         *  -- DEPRECATED --
         * Special hint to the underlying system for help in optimizing raycasting.
         */
        TriggerCollection triggers;
        
        /**
         * Can load map data from either a compatible JSON or TMX format.
         */
        void loadMapInfo( std::string fullPath );
        
        /**
         * Returns the behavior index for the provided tile resource.
         */
        int getBehaviorIndex( int tileResourceIndex );
        
        /**
         * Instantiates MapInfo and calls MapInfo::loadMapInfo(...)
         */
        MapInfo( std::string fullPath );
        
        /**
         * Default constructor/destructor.
         */
        MapInfo();
        ~MapInfo();
        
    private:
        /**
         * These functions simply parse JSON file data into the appropriate data structures for use with the rest 
         * of the system.
         */
        void loadJSONData( cocos2d::Data fileData );
        void loadJSONProperties( const rapidjson::Document& doc );
        void loadJSONTiles( const rapidjson::Document& doc );
        void loadJSONPlanes( const rapidjson::Document& doc );
        void loadJSONActors( const rapidjson::Document& doc );
        void loadJSONBehaviors( const rapidjson::Document& doc );
        void loadJSONTriggers( const rapidjson::Document& doc );
        void loadTMXData();
        
    };
}

#endif /* MapInfo_hpp */
