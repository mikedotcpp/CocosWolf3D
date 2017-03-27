//
//  Raycaster.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#ifndef Raycaster_hpp
#define Raycaster_hpp

#include <stdio.h>
#include "GBRTypes.hpp"
#include "../../Map/MapInfo.hpp"

namespace mikedotcpp
{
#define MATH_PI 3.14159265359f
#define TWO_PI 2 * MATH_PI
#define ONE_QUARTER TWO_PI * 0.25f
#define THREE_QUARTERS TWO_PI * 0.75f
    
    /**
     * Provides an interface for classes interested in GBRaycaster hit-events such as walls/floors/ceilings/etc.
     */
    class GBRaycasterInterface
    {
    public:
        /**
         * Provides an interface for the calling code to decide what to do with the valid tile. The caller also
         * determines if the GBRaycaster should continue from this point or move on to the next raycast.
         *
         * TRUE  - Continue checking for hits along the path of this ray.
         * FALSE - Quit checking hits on this ray path and move to the next ray.
         */
        virtual bool processHit( int index, float angle, Point3f hit, int tileIndex, int planeIndex ) = 0;
    };
    
    /**
     * Performs the classic grid-based raycasting algorithm seen in old-school first person
     * games such as: Wolfenstein3D, Shadowcaster, In Pursuit of Greed, Blake Stone,
     * etc.
     */
    class GBRaycaster
    {
    public:
        /**
         * Constructor; relies on the MapInfo class for setup and *requires* a delegate for delivering visibility
         * results from the raycasting algorithm.
         */
        GBRaycaster( const MapInfo& mapInfo, GBRaycasterInterface* delegate );
        
        /**
         * Destructor.
         */
        ~GBRaycaster();
        
        /**
         * Converts a world position to an integer tile position.
         */
        Point2i tileCoordForPosition( float x, float y );
        Point2i tileCoordForPosition( Point3f pos );
        
        /**
         * Converts a tile position to a floating-point world position.
         */
        Point3f tilePositionForCoord( int x, int y );
        Point3f tilePositionForCoord( Point2i coord );
        
        /**
         * Find the map coord that this ray hits tracing vertically.
         */
        void traceRayVertically( float rayAngle, Point3f raySinCos, Point3f playerTilePosition, Point3f playerPosition );
        
        /**
         * Find the map coord that this ray hits tracing vertically.
         */
        void traceRayHorizontally( float rayAngle, Point3f raySinCos, Point3f playerTilePosition, Point3f playerPosition );
        
        /**
         * Loops through the tile map(s) checking for intersections.
         */
        void traceRay( Point3f rayPoint, Point3f rayPointChange, Point3f increment, float rayAngle );
        
        /**
         * Casts a number of rays from the playerPosition, at some starting rotation (represented by the camera
         * yaw), out to the world. When any object in the world is hit by the ray GBRaycaster notifies the
         * delegate. Please see the following for more information:
         *
         * http://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/
         */
        void castRays( Point3f playerPosition, float rotation );
        
        /**
         * Returns the tile resource index for a given position. The position is assumed to be the player position.
         */
        int getTileResourceIndex( Point3f position );
        
        /**
         */
        float getTileResourceHeight( Point3f position );
        
        /**
         * Resets the tile resource index to 0 at this position in the tilemap.
         */
        void clearTileResourceAt( Point3f position );
        
    protected:
        /**
         * Refers to the number of rays fired in raycasting algorithm. The classic
         * algorithm calls for one ray per column on screen.
         *
         * Since that could be an absurd number of rays on today's displays, the default value is set to
         * the original Wolfenstein3D.
         */
        int _rayCount = 640; // Use for faster platforms?
        
        /**
         * Width, in pixels, of the tiles in this map.
         */
        float _tileWidth = 64;
        
        /**
         * Height, in pixels, of the tiles in this map.
         */
        float _tileHeight = 64;
        
        /**
         * Width, in tiles, of the map.
         */
        float _mapWidth;
        
        /**
         * Height, in tiles, of the map. Note that height here is interpreted as depth for a 3D landscape.
         */
        float _mapHeight;
        
        /**
         * The extent of the observable world that is seen at any given moment. Measured in degrees.
         */
        float _fov = 120.0f;
        
        /**
         */
        float _tileWidthDivisor;
        
        /**
         */
        float _tileHeightDivisor;
        
        /**
         * Pre-computed ray angles based on the field of view (FOV).
         */
        std::vector< float > _rayAngles;
        
        /**
         */
        mikedotcpp::PlaneCollection _planes;
        
        /**
         * Conforms to the GBRaycasterInterface protocol. Intended to be used to notify the calling code when
         * specific types of entities are hit (walls/floors/ceilings/doors/objects/actors/etc.)
         */
        GBRaycasterInterface* _delegate;
        
        /**
         * Resets the array(s) of visited planes before each round of raycasting.
         */
        void addVisibleEntity( Point2i hitCoord, int* plane );
        
        /**
         */
        void preComputeRayAngles();
        
        /**
         */
        void castRay( int stripIdx, float rayAngle, Point2i playerTileCoords, Point3f playerTilePosition, Point3f playerPosition );
        
        /**
         */
        int getIndexFromMapCoord( Point2i coord );
        
        /*
            TODO: Verify that this is actually necessary...it may not be.
         */
        Point2i getShortestMapCoord( Point3f playerPosition, Point2i vCoord, Point2i hCoord );
        
        /**
         */
        void transposeAboutY( Point3f& vector );
        
        /**
         */
        void setPlayerTile( Point2i playerTile, Point3f playerPosition );
        
        //-----------------------------------------------------
        //
        // GETTERS/SETTERS
        //
        //-----------------------------------------------------
    public:
        /**
         * Returns the _mapWidth * _mapHeight.
         */
        int getMapSize();
        
        /**
         * _rayCount
         */
        void setRayCount( int count );
        int getRayCount();
        
        /**
         * _tileWidth
         */
        void setTileWidth( float width );
        float getTileWidth();
        
        /**
         * _tileHeight
         */
        void setTileHeight( float height );
        float getTileHeight();
        
        /**
         * _mapWidth
         */
        void setMapWidth( float width );
        float getMapWidth();
        
        /**
         * _mapHeight
         */
        void setMapHeight( float height );
        float getMapHeight();
        
        /**
         * _delegate
         */
        void setDelegate( GBRaycasterInterface* delegate );
        
    };
    
}

#endif /* Raycaster_hpp */
