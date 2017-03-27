//
//  BlockManager.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/18/16.
//
//

#ifndef BlockManager_hpp
#define BlockManager_hpp

#include "cocos2d.h"
#include "../Map/MapInfo.hpp"
#include "FPBillboard.hpp"
#include "Batched/BatchedSprite3D.hpp"

#define WHITE_TILE "whiteTile.png"

enum FaceDirection
{
    north, south, east, west, top, bottom, centerSpanNS, centerSpanEW
};

namespace mikedotcpp
{
    typedef std::vector<cocos2d::Sprite3D*> Pool;
    typedef std::vector<Pool> PoolCollection;
    
    struct FaceCoords
    {
        cocos2d::Vec2 bl, br, tl, tr;
    };
    
    /**
     * This class manages collections of Sprite3D container objects called "blocks".
     * 
     * There are two different rendering paths: sprite-based and mesh-based. The particulars of each are discussed
     * below.
     *
     * Sprite-Based:
     * Each block itself contains six sprites that comprise a cube. BlockManager will create and manage pools of
     * Sprite3D container objects (the blocks), and Sprite objects (the faces).
     *
     * Mesh-Based:
     * Each block in this mode is a Sprite3D, but the mesh is programatically created and assigned to each tile.
     * For every uniquely textured face of a cube there will be a separate mesh generated, which results in 1
     * draw call per mesh. 
     * 
     * The advantage here is we can use realtime lighting and achieve effects like normal mapping. All geometry in
     * meshed-based rendering uses geometry instancing, which is not supported across all hardwarde. Thankfully, it
     * is supported as an extension to many OpenGL ES 2.0 implementations.
     *
     * The BlockManager does not free allocated objects until the end of 
     * program execution.
     */
    class BlockManager
    {
    public:
        /**
         * Checks to see if there are any available blocks and creates one if necessary. The resulting block will be
         * returned with the proper faces specified.
         */
        cocos2d::Sprite3D* getBlock( int tilePropertiesIndex );
        
        /**
         * Returns the BatchedSprite3D object and tileIndex.
         */
        mikedotcpp::BatchedSprite3D* getMeshBlock( int tileIndex );
        
        /**
         * Returns all in-use blocks to the pool (also returns those blocks' sprites to the sprite pool).
         */
        void reclaimAllBlocks();
        
        /**
         * Constructor/Destructor
         */
        BlockManager( const mikedotcpp::MapInfo& mapInfo, cocos2d::Layer* layer );
        BlockManager();
        ~BlockManager();
        
    protected:
        /**
         * Taken from the Director.
         */
        float _contentScaleFactor = 1.0f;
        
        /**
         * A pool of container objects that are used to house [1...6] sprites.
         */
        PoolCollection _freeBlocks;
        PoolCollection _inUseBlocks;
        
        /**
         * A collection containing all of the meshes that are instanced.
         */
        std::vector< mikedotcpp::BatchedSprite3D* > _instancedMeshes;
        
        /**
         * Configures the appropriate set of blocks according to the map settings. At this time it is not possible
         * to mix the two different rendering paths (sprite and mesh).
         */
        void initBlockManager( const mikedotcpp::MapInfo& mapInfo, cocos2d::Layer* layer  );
        
        /**
         * Loads only the texture data specified by the MapInfo class.
         */
        void loadTextures( const mikedotcpp::MapInfo& mapInfo );
        
        /**
         * Adds some texture parameters defined by the map.
         */
        void addCustomTexParams( const mikedotcpp::Tile& tileData, cocos2d::Sprite& sprite );
        
        /**
         * Returns an array of integers that represent the number of instances for each tile defined by the map.
         */
        int* getTileCollectionCounts( int tileCount, int planeCount, int mapSize, const std::vector< Plane >& mapPlane ) const;
        
        //-----------------------------------------------------
        //
        // SPRITE BLOCK
        //
        //-----------------------------------------------------
    protected:
        /**
         * Returns a properly created Sprite3D object.
         */
        cocos2d::Sprite3D* createSpriteBlock( const Tile& tileData );
        
        /**
         * In charge of configuring faces and separate billboard objects.
         */
        void initSpriteBlock( cocos2d::Sprite3D& block, const mikedotcpp::Tile& tileData );
        
        /**
         * Sets each face of a cube to a texture specified by the tile. Each texture is mapped to it's corresponding
         * face according to the cardinal directions. If a filename begins with the '#', it is considered a hex color.
         */
        void configureSpriteFaces( cocos2d::Sprite3D& block, const std::vector<std::string>& filenames );
        
        /**
         * Retrieves sprites from the _faces pool and places/rotates them accordingly within the provided block.
         */
        cocos2d::Sprite* configureSpriteFace( const std::string& faceImage );
        
        /**
         * Places and rotates the face sprite according to it's FaceDirection.
         */
        void orientSpriteFace( cocos2d::Sprite* face, int direction );
        
        /**
         * Special billboarded sprites that keep their z-axis rotation locked. Sprite rendering path only.
         */
        void configureSpriteBillboard( cocos2d::Sprite3D& block, const std::string& filename );
        
        //-----------------------------------------------------
        //
        // MESH BLOCK
        //
        //-----------------------------------------------------
    protected:
        /**
         * Returns instanced Sprite3D objects.
         */
        mikedotcpp::BatchedSprite3D* createMeshBlock( const Tile& tileData );
        
        /**
         * Creates the meshes and assigns textures/materials for the provided block.
         */
        void initMeshBlock( mikedotcpp::BatchedSprite3D& block, const mikedotcpp::Tile& tileData );
        
        /**
         * In charge of building the mesh primitive (a cube) and assigning textures according to the cardinal 
         * directions. Supports diffuse mapping and normal mapping.
         */
        void configureMeshFaces( BatchedSprite3D& block, const mikedotcpp::Tile& tileData );
        
        /**
         * Returns a standard set of normalized device coordinates for texturing.
         */
        FaceCoords getFaceTextureCoordinates();
    };
}

#endif /* BlockManager_hpp */
