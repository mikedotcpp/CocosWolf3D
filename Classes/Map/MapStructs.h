//
//  MapStructs.h
//  CocosWolf3D
//
//  Created by Michael Edwards on 1/12/17.
//
//

#ifndef MapStructs_h
#define MapStructs_h
    
namespace mikedotcpp
{
    /**
     * Forward-declarations and Type declarations
     */
    struct Tile;
    typedef std::vector< Tile > TileCollection;
    struct Plane;
    typedef std::vector< Plane > PlaneCollection;
    struct Actor;
    typedef std::vector< Actor > ActorCollection;
    struct Behavior;
    typedef std::vector< Behavior > BehaviorCollection;
    struct Trigger;
    typedef std::vector< Trigger > TriggerCollection;
    
    /**
     * The basic building block for tile sets in raycast games. The basic default struct conforms to the data
     * format specified by UWMF (Universal Wolfenstein Map Format). Please see the following for reference:
     * 
     * http://maniacsvault.net/ecwolf/wiki/Universal_Wolfenstein_Map_Format
     *
     */
    struct Tile
    {
        /**
         * Sets the texture to use on each side of the tile. A hexadecimal color prefixed with '#' or '-' may be
         * used in leiu of a texture.
         */
        std::string textureNorth   = "";
        std::string textureEast    = "";
        std::string textureSouth   = "";
        std::string textureWest    = "";
        std::string textureFloor   = "";
        std::string textureCeiling = "";
        std::string textureAll     = "";
        
        /**
         * This texture is placed at the center of the tile, oriented either North to South or East to West.
         */
        std::string textureCenterSpanNS = "";
        std::string textureCenterSpanEW = "";
        
        /**
         * Sets the normal map texture to use on each side of the tile.
         */
        std::string normalNorth   = "";
        std::string normalEast    = "";
        std::string normalSouth   = "";
        std::string normalWest    = "";
        std::string normalFloor   = "";
        std::string normalCeiling = "";
        std::string normalAll     = "";
        
        /**
         * If this texture name is present, then this Tile resource gets drawn as a billboard. This may have to be
         * changed to an array when writing in support for Actors.
         */
        std::string billboardTexture = "";
        
        /**
         */
        std::string vertexShader = "";
        
        /**
         */
        std::string fragmentShader = "";
        
        /**
         */
        std::string model = "";
        
        /**
         */
        std::string textureWrapMode = "";
        
        /**
         */
        std::string textureMinFilter = "";
        
        /**
         */
        std::string textureMagFilter = "";
        
        /**
         * An optional tag to link this tile to a trigger or other action. Used for doors, pushwalls, etc.
         */
        int tag = -1;
    };
    
    /**
     * Defines a single plane of tiles.
     */
    struct Plane
    {
        /**
         * Height of the plane in pixels.
         */
        int height;
        
        /**
         * Defines a layer of map spots that create the layout of the level. Indexes into tiles array. Index zero
         * (0) is considered void and is ignored.
         */
        int* map;
    };
    
    /**
     * An entity that interacts with the world and the player. "Player1" itself is the only REQUIRED actor per map.
     */
    struct Actor
    {
        /**
         * The actor name to spawn. "Player1" by default, which is reserved for index 0.
         */
        std::string type = "Player1";
        
        /**
         * Represents the COLUMN index into the map array.
         */
        int x;
        
        /**
         * Represents the ROW index into the map array.
         */
        int y;
        
        /**
         * Refers to the index of the specific plane-map in the "planes" array.
         */
        int z;
        
        /**
         * Angle of the spawning point in degrees. 0 = East, 90 = North, 180 = West, 270 = South.
         */
        float yaw;
    };
    
    /**
     * Defines how objects interact with the player and each other.
     */
    struct Behavior
    {
        std::string onEnter;
        std::string onExit;
        std::string onCreate;
    };
    
    /**
     *  -- DEPRECATED --
     */
    struct Trigger
    {
        bool continueRaycast = false;
    };
}


#endif /* MapStructs_h */
