//
//  BlockManager.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/18/16.
//
//

#include "BlockManager.hpp"
#include "Batched/BatchedSprite3D.hpp"
#include "Batched/BatchedGLProgram.hpp"
#include "FPBillboard.hpp"

using namespace mikedotcpp;

#define RENDERING_PREREQUISITES_NOT_MET_MSG "!!!Geometry instancing unsupported on this device!!! Use the Sprite rendering path instead."

BlockManager::BlockManager()
{
    CCLOG( "BlockManager::BlockManager() - Please pass a layer to the constructor!" );
}

BlockManager::BlockManager( const mikedotcpp::MapInfo& mapInfo, cocos2d::Layer* layer )
{
    cocos2d::Configuration* config = cocos2d::Configuration::getInstance();
    bool geometryInstancingSupported = config->checkForGLExtension( "GL_EXT_draw_instanced" ) || config->checkForGLExtension( "GL_ARB_draw_instanced" );
    bool prepRenderingSystem = !mapInfo.useRealtimeLighting || ( mapInfo.useRealtimeLighting && geometryInstancingSupported );
    
    CCASSERT( prepRenderingSystem, RENDERING_PREREQUISITES_NOT_MET_MSG );
    
    if( prepRenderingSystem )
    {
        _contentScaleFactor = cocos2d::Director::getInstance()->getContentScaleFactor();
        loadTextures( mapInfo );
        initBlockManager( mapInfo, layer );
    }
}

void BlockManager::loadTextures(  const mikedotcpp::MapInfo& mapInfo  )
{
    cocos2d::SpriteFrameCache* frameCache = cocos2d::SpriteFrameCache::getInstance();
    if( mapInfo.useRealtimeLighting )
    {
        for( int i = 0; i < mapInfo.tiles.size(); ++i )
        {
            mikedotcpp::Tile tile = mapInfo.tiles[i];
            std::string diffuse[] = { tile.textureAll, tile.textureEast, tile.textureWest,
                tile.textureNorth, tile.textureSouth, tile.textureCeiling, tile.textureFloor };
            std::string normal[] = { tile.normalAll, tile.normalEast, tile.normalWest,
                tile.normalNorth, tile.normalSouth, tile.normalCeiling, tile.normalFloor };
            for( int j = 0; j < sizeof( diffuse )/sizeof( *diffuse ); ++j )
            {
                std::string diffuseName = diffuse[j];
                std::string normalName = normal[j];
                if( !diffuseName.empty() )
                {
                    cocos2d::Sprite* sprite = cocos2d::Sprite::create( diffuseName );
                    addCustomTexParams( tile, *sprite );
                    frameCache->addSpriteFrame( sprite->getSpriteFrame(), diffuseName );
                    sprite = cocos2d::Sprite::create( normalName );
                    if( sprite )
                    {
                        addCustomTexParams( tile, *sprite );
                        frameCache->addSpriteFrame( sprite->getSpriteFrame(), normalName );
                    }
                }
            }
        }
    }
    else
    {
        for( int i = 0; i < mapInfo.spritesheets.size(); ++i )
        {
            std::string name = mapInfo.spritesheets.at( i );
            std::string image = name + ".png";
            std::string plist = name + ".plist";
            frameCache->addSpriteFramesWithFile( plist, image );
        }
    }
}

void BlockManager::addCustomTexParams( const mikedotcpp::Tile& tileData, cocos2d::Sprite& sprite )
{
    if( tileData.textureWrapMode.empty() )
    {
        return;
    }
    cocos2d::Texture2D::TexParams texParams;
    texParams.magFilter = ( tileData.textureMagFilter == "GL_LINEAR" ) ? GL_LINEAR : GL_NEAREST;
    texParams.minFilter = ( tileData.textureMinFilter == "GL_LINEAR" ) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
    texParams.wrapS = ( tileData.textureWrapMode == "GL_REPEAT" ) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    texParams.wrapT = ( tileData.textureWrapMode == "GL_REPEAT" ) ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    sprite.getTexture()->setTexParameters( texParams );
}

void BlockManager::initBlockManager( const mikedotcpp::MapInfo& mapInfo, cocos2d::Layer* layer  )
{
    int tileCount = (int)mapInfo.tiles.size();
    int planeCount = (int)mapInfo.planes.size();
    int mapSize = mapInfo.width * mapInfo.height;
    int* countCollection = getTileCollectionCounts( tileCount, planeCount, mapSize, mapInfo.planes );
    
    _freeBlocks.reserve( planeCount );
    _inUseBlocks.reserve( planeCount );
    _instancedMeshes.reserve( tileCount );
    
    for( int i = 0; i < tileCount; ++i )
    {
        Pool tileSet;
        int count = countCollection[i];
        int j = 0;
        do
        {
            Tile tileData = mapInfo.tiles[i];
            if( mapInfo.useRealtimeLighting )
            {
                // Only do this once for instanced geometry tilesets.
                if( j == 0 ) 
                {
                    BatchedSprite3D* block = createMeshBlock( tileData );
                    _instancedMeshes.push_back( block );
                    if( count > 0 )
                    {
                        layer->addChild( block );
                    }
                }
            }
            else
            {
                cocos2d::Sprite3D* block = createSpriteBlock( tileData );
                tileSet.push_back( block );
                layer->addChild( block );
            }
            ++j;
        } while( j < count );
        _freeBlocks.push_back( tileSet );
        _inUseBlocks.push_back( mikedotcpp::Pool() );
    }
    
    delete countCollection;
}

mikedotcpp::BatchedSprite3D* BlockManager::createMeshBlock( const Tile& tileData )
{
    BatchedSprite3D* block = BatchedSprite3D::create();
    block->setCameraMask( (unsigned short)cocos2d::CameraFlag::USER1 );
    block->retain();
    initMeshBlock( *block, tileData );
    block->addChild( cocos2d::Node::create() ); // Note: This is hack to ensure the node is always visited (not culled).
    
    BatchedGLProgram* shader = BatchedGLProgram::createWithFilenames( tileData.vertexShader, tileData.fragmentShader );
    auto glProgramState = cocos2d::GLProgramState::getOrCreateWithGLProgram( shader );
    block->setMaterial( cocos2d::Sprite3DMaterial::createWithGLStateProgram( glProgramState ) );
    return block;
}

cocos2d::Sprite3D* BlockManager::createSpriteBlock( const Tile& tileData )
{
    cocos2d::Sprite3D* block = cocos2d::Sprite3D::create();
    block->setVisible( false );
    block->setCameraMask( (unsigned short)cocos2d::CameraFlag::USER1 );
    block->retain();
    initSpriteBlock( *block, tileData );
    return block;
}

void BlockManager::initMeshBlock( mikedotcpp::BatchedSprite3D& block, const mikedotcpp::Tile& tileData )
{
    configureMeshFaces( block, tileData );
}

int* BlockManager::getTileCollectionCounts( int tileCount, int planeCount, int mapSize, const std::vector< Plane >& planes ) const
{
    int* countCollection = new int[ tileCount ]{0};
    for( int i = 0; i < planeCount; ++i )
    {
        Plane mapPlane = planes[i];
        for( int j = 0; j < mapSize; ++j )
        {
            int tileIndex = mapPlane.map[j];
            if( tileIndex > 0 )
            {
                countCollection[ tileIndex - 1 ] += 1;
            }
        }
    }
    //
    for( int i = 0; i < tileCount; ++i )
    {
        CCLOG( "COUNT FOR TILESET %i: %i", i+1, countCollection[i]  );
    }
    return countCollection;
}

void BlockManager::initSpriteBlock( cocos2d::Sprite3D& block, const mikedotcpp::Tile& tileData )
{
    std::vector<std::string> names;
    if( !tileData.billboardTexture.empty() )
    {
        configureSpriteBillboard( block, tileData.billboardTexture );
    }
    else
    {
        if( !tileData.textureAll.empty() )
        {
            names = { tileData.textureAll, tileData.textureAll, tileData.textureAll,
                      tileData.textureAll, tileData.textureAll, tileData.textureAll,
                      tileData.textureAll, tileData.textureAll };
        }
        else
        {
            names = { tileData.textureNorth, tileData.textureEast, tileData.textureSouth,
                      tileData.textureWest, tileData.textureCeiling, tileData.textureFloor,
                      tileData.textureCenterSpanNS, tileData.textureCenterSpanEW };
        }
        configureSpriteFaces( block, names );
    }
}

void BlockManager::configureSpriteBillboard( cocos2d::Sprite3D& block, const std::string& filename )
{
    mikedotcpp::FPBillBoard* board = mikedotcpp::FPBillBoard::create();
    board->setSpriteFrame( filename );
    board->setGlobalZOrder( -1 );
    board->setScale( _contentScaleFactor );
    board->setCameraMask( (unsigned short) cocos2d::CameraFlag::USER1 );
    board->setVisible( true );
    block.addChild( board );
}

void BlockManager::configureSpriteFaces( cocos2d::Sprite3D& block, const std::vector<std::string>& filenames )
{
    CCASSERT( filenames.size() == 8, "A filename is required for each side of a cube (8 including the center spaces)." );
    
    int directions[] = { FaceDirection::north, FaceDirection::east, FaceDirection::south,
                         FaceDirection::west, FaceDirection::top, FaceDirection::bottom,
                         FaceDirection::centerSpanNS, FaceDirection::centerSpanEW };
    for( int i = 0; i < filenames.size(); ++i )
    {
        std::string textureName = filenames[i];
        if( !textureName.empty() )
        {
            cocos2d::Sprite* face;
            if( textureName[0] == '#' )
            {
                unsigned long hex = std::strtoul( textureName.substr( 1, 6 ).c_str(), nullptr, 16 );
                int b = hex & 0xFF; hex >>= 8;
                int g = hex & 0xFF; hex >>= 8;
                int r = hex & 0xFF;
                face = configureSpriteFace( WHITE_TILE );
                face->setColor( cocos2d::Color3B( r, g, b ) );
            }
            else
            {
                face = configureSpriteFace( textureName );
            }
            if( face )
            {
                orientSpriteFace( face, directions[i] );
                cocos2d::V3F_C4B_T2F_Quad quad = face->getQuad();
                block.addChild( face );
            }
        }
    }
}

/*
 
    TODO: Break this function up a bit.
 
 */
void BlockManager::configureMeshFaces( BatchedSprite3D& block, const mikedotcpp::Tile& tileData )
{
    std::vector< cocos2d::MeshVertexAttrib > attributes =
    {
        { 3, GL_FLOAT, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION,   3*sizeof(float) },
        { 3, GL_FLOAT, cocos2d::GLProgram::VERTEX_ATTRIB_NORMAL,     3*sizeof(float) },
        { 3, GL_FLOAT, cocos2d::GLProgram::VERTEX_ATTRIB_TANGENT,    3*sizeof(float) },
        { 3, GL_FLOAT, cocos2d::GLProgram::VERTEX_ATTRIB_BINORMAL,   3*sizeof(float) },
        { 2, GL_FLOAT, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD,  2*sizeof(float) }
    };
    float x = 0.0f, y = 0.0f, z = 0.0f, s = 64.0f;
    std::vector< float > vertices;
    cocos2d::MeshData::IndexArray meshIndices;
    cocos2d::MeshData::IndexArray indices;
    int perVertexSizeInFloat = 14;
    //
    FaceCoords northFace = getFaceTextureCoordinates();
    FaceCoords eastFace = getFaceTextureCoordinates();
    FaceCoords southFace = getFaceTextureCoordinates();
    FaceCoords westFace = getFaceTextureCoordinates();
    FaceCoords topFace = getFaceTextureCoordinates();
    FaceCoords bottomFace = getFaceTextureCoordinates();
    //
    vertices.insert ( vertices.end(),
    {
        // position  normal  tangent binormal uv
        // +x
        x+s,y-s,z-s, 1,0,0,  0,0,-1, 0,-1,0, westFace.br.x, westFace.br.y,
        x+s,y+s,z-s, 1,0,0,  0,0,-1, 0,-1,0, westFace.tr.x, westFace.tr.y,
        x+s,y-s,z+s, 1,0,0,  0,0,-1, 0,-1,0, westFace.bl.x, westFace.bl.y,
        x+s,y+s,z+s, 1,0,0,  0,0,-1, 0,-1,0, westFace.tl.x, westFace.tl.y,
        // +y
        x-s,y+s,z-s, 0,1,0,  1,0,0,  0,0,1,  topFace.br.x, topFace.br.y,
        x+s,y+s,z-s, 0,1,0,  1,0,0,  0,0,1,  topFace.tr.x, topFace.tr.y,
        x-s,y+s,z+s, 0,1,0,  1,0,0,  0,0,1,  topFace.bl.x, topFace.bl.y,
        x+s,y+s,z+s, 0,1,0,  1,0,0,  0,0,1,  topFace.tl.x, topFace.tl.y,
        // +z
        x-s,y-s,z+s, 0,0,1,  0,0,-1, 1,0,0,  northFace.bl.x, northFace.bl.y,
        x+s,y-s,z+s, 0,0,1,  0,0,-1, 1,0,0,  northFace.br.x, northFace.br.y,
        x-s,y+s,z+s, 0,0,1,  0,0,-1, 1,0,0,  northFace.tl.x, northFace.tl.y,
        x+s,y+s,z+s, 0,0,1,  0,0,-1, 1,0,0,  northFace.tr.x, northFace.tr.y,
        // -x
        x-s,y-s,z-s, -1,0,0, 0,0,1,  0,-1,0, eastFace.br.x, eastFace.br.y,
        x-s,y+s,z-s, -1,0,0, 0,0,1,  0,-1,0, eastFace.tr.x, eastFace.tr.y,
        x-s,y-s,z+s, -1,0,0, 0,0,1,  0,-1,0, eastFace.bl.x, eastFace.bl.y,
        x-s,y+s,z+s, -1,0,0, 0,0,1,  0,-1,0, eastFace.tl.x, eastFace.tl.y,
        // -y
        x-s,y-s,z-s, 0,-1,0, -1,0,0, 0,0,1,  bottomFace.bl.x, bottomFace.bl.y,
        x+s,y-s,z-s, 0,-1,0, -1,0,0, 0,0,1,  bottomFace.br.x, bottomFace.br.y,
        x-s,y-s,z+s, 0,-1,0, -1,0,0, 0,0,1,  bottomFace.tl.x, bottomFace.tl.y,
        x+s,y-s,z+s, 0,-1,0, -1,0,0, 0,0,1,  bottomFace.tr.x, bottomFace.tr.y,
        // -z
        x-s,y-s,z-s, 0,0,-1,  0,0,1, 1,0,0,  southFace.bl.x, southFace.bl.y,
        x+s,y-s,z-s, 0,0,-1,  0,0,1, 1,0,0,  southFace.br.x, southFace.br.y,
        x-s,y+s,z-s, 0,0,-1,  0,0,1, 1,0,0,  southFace.tl.x, southFace.tl.y,
        x+s,y+s,z-s, 0,0,-1,  0,0,1, 1,0,0,  southFace.tr.x, southFace.tr.y,
     });
    //
    if( !tileData.textureAll.empty() )
    {
        meshIndices.insert( meshIndices.end(),
        {
            0, 3, 2, 1, 3, 0,     // EAST
            4, 6, 7, 4, 7, 5,     // TOP
            8, 11, 10, 9, 11, 8,  // NORTH
            12,  14,  15,  13,  12,  15, // WEST
            16,  19,  18,  17,  19,  16, // BOTTOM
            20,  22,  23,  21,  20,  23  // SOUTH
        } );
        BatchedMesh* proceduralMesh = BatchedMesh::create( vertices, perVertexSizeInFloat, meshIndices, attributes );
        proceduralMesh->setTexture( tileData.textureAll, cocos2d::NTextureData::Usage::Diffuse );
        proceduralMesh->setTexture( tileData.normalAll, cocos2d::NTextureData::Usage::Normal );
        block.addMesh( proceduralMesh );
    }
    else
    {
        std::string diffuseTextures[] = { tileData.textureSouth, tileData.textureNorth, tileData.textureEast,
                                          tileData.textureWest, tileData.textureCeiling, tileData.textureFloor };
        std::string normalTextures[] = { tileData.normalSouth, tileData.normalNorth, tileData.normalEast,
                                          tileData.normalWest, tileData.normalCeiling, tileData.normalFloor };
        int directions[] = { FaceDirection::south, FaceDirection::north, FaceDirection::east,
                             FaceDirection::west, FaceDirection::top, FaceDirection::bottom };
        for( int i = 0; i < sizeof( diffuseTextures )/sizeof( *diffuseTextures ); ++i )
        {
            std::string diffuse = diffuseTextures[i];
            std::string normal = normalTextures[i];
            if( !diffuse.empty() )
            {
                int faceDirection = directions[i];
                
                if( faceDirection == FaceDirection::south )
                {
                    meshIndices.insert( meshIndices.end(), { 20,  22,  23,  21,  20,  23 } );
                }
                else if( faceDirection == FaceDirection::north )
                {
                    meshIndices.insert( meshIndices.end(), { 8, 11, 10, 9, 11, 8 } );
                }
                else if( faceDirection == FaceDirection::east )
                {
                    meshIndices.insert( meshIndices.end(), { 0, 3, 2, 1, 3, 0 } );
                }
                else if( faceDirection == FaceDirection::west )
                {
                    meshIndices.insert( meshIndices.end(), { 12,  14,  15,  13,  12,  15 } );
                }
                else if( faceDirection == FaceDirection::top )
                {
                    meshIndices.insert( meshIndices.end(), { 4, 6, 7, 4, 7, 5 } );
                }
                else if( faceDirection == FaceDirection::bottom )
                {
                    meshIndices.insert( meshIndices.end(), { 16,  19,  18,  17,  19,  16 } );
                }
                BatchedMesh* proceduralMesh = BatchedMesh::create( vertices, perVertexSizeInFloat, meshIndices, attributes );
                proceduralMesh->setTexture( diffuse, cocos2d::NTextureData::Usage::Diffuse );
                proceduralMesh->setTexture( normal, cocos2d::NTextureData::Usage::Normal );
                block.addMesh( proceduralMesh );
            }
        }
    }
}

FaceCoords BlockManager::getFaceTextureCoordinates()
{
    float left = 0;
    float bottom = 0;
    float top = 1;
    float right = 1;
    FaceCoords faceTexCoords;
    faceTexCoords.bl = cocos2d::Vec2( left, bottom );
    faceTexCoords.br = cocos2d::Vec2( right, bottom );
    faceTexCoords.tl = cocos2d::Vec2( left, top );
    faceTexCoords.tr = cocos2d::Vec2( right, top );
    return faceTexCoords;
}

BlockManager::~BlockManager()
{
    CCLOG( "BlockManager deleted, release resources." );
}

cocos2d::Sprite* BlockManager::configureSpriteFace( const std::string& faceImage )
{
    cocos2d::Sprite* face = cocos2d::Sprite::create();
    if( face && !faceImage.empty() )
    {
        cocos2d::SpriteFrame* frame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName( faceImage );
        if( !frame || frame == nullptr )
        {
            cocos2d::log( "BlockManager::configureFace - SpriteFrame named %s missing!", faceImage.c_str() );
            return face;
        }
        face->setSpriteFrame( frame );
        face->setGlobalZOrder( -1 );
        face->setScale( _contentScaleFactor );
        face->setCameraMask( (unsigned short) cocos2d::CameraFlag::USER1 );
        face->setVisible( true );
    }
    return face;
}

void BlockManager::orientSpriteFace( cocos2d::Sprite* face, int direction )
{
    float faceSize = face->getContentSize().width * _contentScaleFactor * 0.5f;
    if( direction == FaceDirection::north )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 180, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, 0, -faceSize ) );
    }
    else if( direction == FaceDirection::south )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 0, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, 0, faceSize ) );
    }
    else if( direction == FaceDirection::east )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 90, 0 ) );
        face->setPosition3D( cocos2d::Vec3( faceSize, 0, 0 ) );
    }
    else if( direction == FaceDirection::west )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 270, 0 ) );
        face->setPosition3D( cocos2d::Vec3( -faceSize, 0, 0 ) );
    }
    else if( direction == FaceDirection::top )
    {
        face->setRotation3D( cocos2d::Vec3( 270, 0, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, faceSize, 0 ) );
    }
    else if( direction == FaceDirection::bottom )
    {
        face->setRotation3D( cocos2d::Vec3( 90, 0, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, -faceSize, 0 ) );
    }
    else if( direction == FaceDirection::centerSpanNS )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 90, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, 0, 0 ) );
    }
    else if( direction == FaceDirection::centerSpanEW )
    {
        face->setRotation3D( cocos2d::Vec3( 0, 180, 0 ) );
        face->setPosition3D( cocos2d::Vec3( 0, 0, 0 ) );
    }
}

cocos2d::Sprite3D* BlockManager::getBlock( int tilePropertiesIndex )
{
    int index = tilePropertiesIndex;
    cocos2d::Sprite3D* block = nullptr;
    if( _freeBlocks[index].size() > 0 )
    {
        block = _freeBlocks[index].back();
        block->setVisible( true );
        block->setTag( index );
        _freeBlocks[index].pop_back();
        _inUseBlocks[index].push_back( block );
    }
    return block;
}

mikedotcpp::BatchedSprite3D* BlockManager::getMeshBlock( int tileIndex )
{
    mikedotcpp::BatchedSprite3D* block = nullptr;
    if( _instancedMeshes.size() > 0 )
    {
        block = _instancedMeshes[ tileIndex ];
    }
    return block;
}

void BlockManager::reclaimAllBlocks()
{
    for( int i = 0; i < _inUseBlocks.size(); ++i )
    {
        long int length = _inUseBlocks[i].size();
        for( int j = 0; j < length; ++j )
        {
            cocos2d::Sprite3D* block = _inUseBlocks[i].back();
            block->setVisible( false );
            _inUseBlocks[i].pop_back();
            _freeBlocks[block->getTag()].push_back( block );
        }
    }
    
    for( int i = 0; i < _instancedMeshes.size(); ++ i )
    {
        _instancedMeshes[i]->setInstanceCount( 0 );
    }
}
