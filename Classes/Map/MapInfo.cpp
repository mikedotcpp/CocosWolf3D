//
//  MapInfo.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 1/10/17.
//
//

#include "MapInfo.hpp"

#define ASSERT_FAILED_NANO "Document is NanO (Not an Object)"

#define PLAYER_ACTOR_STR "Player1"
#define PLAYER_ACTOR_UNDEFINED_ERR_MSG "Player1 actor not defined! Add Player1 as custom Actor in the first position of the map definition file."
#define TRIGGERS_UNDEFINED_ERR_MSG "Must provide default trigger as first element in map definition file."
#define SPRITESHEET_UNDEFINED_ERR_MSG "Spritesheets undefined! You must use a spritesheet with the .png/.plist combination for the sprite-rendering path, see default.json for example."
#define MESH_RENDERING_REMINDER "\n\n**NOTE**\nEnabling realtime lighting automatically means using the mesh-rendering path which is not compatible with spritesheets at this time. Please import individual PNG's into project.\n\n"

#define KEY_PROPERTIES "properties"
#define KEY_TILES "tiles"
#define KEY_PLANES "planes"
#define KEY_ACTORS "actors"
#define KEY_BEHAVIORS "behaviors"
#define KEY_TRIGGERS "triggers"

using namespace mikedotcpp;

void MapInfo::loadMapInfo( std::string fullPath )
{
    if( fullPath.find( ".json" ) != std::string::npos )
    {
        loadJSONData( cocos2d::FileUtils::getInstance()->getDataFromFile( fullPath.c_str() ) );
    }
    else if( fullPath.find( ".tmx" ) != std::string::npos )
    {
        loadTMXData();
    }
    else
    {
        CCASSERT( 0, "Unsupported filetype." );
    }
}

//==============================================================================
//
// TMX PARSER
//
//==============================================================================

/*
 
    TODO: Implement the TMX parser.
 
 */
void MapInfo::loadTMXData()
{
}

//==============================================================================
//
// JSON PARSER
//
//==============================================================================

void MapInfo::loadJSONData( cocos2d::Data fileData )
{
    std::string json( (const char*)fileData.getBytes(), fileData.getSize() );
    rapidjson::Document doc;
    doc.Parse( json.c_str() );
    loadJSONProperties( doc );
    loadJSONTiles( doc );
    loadJSONPlanes( doc );
    loadJSONActors( doc );
    loadJSONBehaviors( doc );
    /*
     
        TODO: Refactor the mechanism for assigning a BEHAVIOR to a TILE and how that is read in by the MapInfo class.
     
     */
    loadJSONTriggers( doc );
}

void MapInfo::loadJSONProperties( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    CCASSERT( doc.HasMember( KEY_PROPERTIES ), "" );
    const rapidjson::Value& props = doc[KEY_PROPERTIES];
    
    CCASSERT( props.IsObject(), "" );
    CCASSERT( props.HasMember( "path" ), "" );
    CCASSERT( props["path"].IsString(), "" );
    path = props["path"].GetString();
    
    CCASSERT( props.HasMember( "name" ), "" );
    CCASSERT( props["name"].IsString(), "" );
    name = props["name"].GetString();
    
    CCASSERT( props.HasMember( "tileSize" ), "" );
    CCASSERT( props["tileSize"].IsInt(), "" );
    tileSize = props["tileSize"].GetInt();
    
    CCASSERT( props.HasMember( "width" ), "" );
    CCASSERT( props["width"].IsInt(), "" );
    width = props["width"].GetInt();
    
    CCASSERT( props.HasMember( "height" ), "" );
    CCASSERT( props["height"].IsInt(), "" );
    height = props["height"].GetInt();
    
    CCASSERT( props.HasMember( "useRealtimeLighting" ), "" );
    CCASSERT( props["useRealtimeLighting"].IsBool(), "" );
    useRealtimeLighting = props["useRealtimeLighting"].GetBool();
    
    CCASSERT( props.HasMember( "version" ), "" );
    CCASSERT( props["version"].IsString(), "" );
    version = props["version"].GetString();
    
    if( !useRealtimeLighting )
    {
        CCASSERT( props.HasMember( "spritesheets" ), SPRITESHEET_UNDEFINED_ERR_MSG );
        CCASSERT( props["spritesheets"].IsArray(), "" );
        const rapidjson::Value& array = props["spritesheets"];
        
        spritesheets.reserve( array.Size() );
        
        for( rapidjson::Value::ConstValueIterator itr = array.Begin(); itr != array.End(); ++itr )
        {
            spritesheets.push_back( itr->GetString() );
        }
    }
    else
    {
        CCLOG( MESH_RENDERING_REMINDER );
    }
}

void MapInfo::loadJSONTiles( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    CCASSERT( doc.HasMember( KEY_TILES ), "" );
    const rapidjson::Value& array = doc[KEY_TILES];
    
    tiles.reserve( array.Size() );
    
    for( rapidjson::SizeType i = 0; i < array.Size(); ++i )
    {
        const rapidjson::Value& obj = array[i];
        
        CCASSERT( obj.IsObject(), ASSERT_FAILED_NANO );
        Tile tile;
        
        if( obj.HasMember( "billboardTexture" ) )
        {
            CCASSERT( obj["billboardTexture"].IsString(), "" );
            tile.billboardTexture = obj["billboardTexture"].GetString();
        }
        else if( obj.HasMember( "model" ) )
        {
            CCASSERT( obj[ "model" ].IsString(), "" );
            tile.model = obj[ "model" ].GetString();
            
            //
            // NOTE: Specifying textureAll and normalAll for a model will override the texture(s) that are bound
            //       with the model.
            //
            if( obj.HasMember( "textureAll" ) )
            {
                CCASSERT( obj["textureAll"].IsString(), "" );
                tile.textureAll = obj["textureAll"].GetString();
            }
            
            if( obj.HasMember( "normalAll" ) )
            {
                CCASSERT( obj["normalAll"].IsString(), "" );
                tile.normalAll = obj["normalAll"].GetString();
            }
        }
        else
        {
            if( obj.HasMember( "textureNorth" ) )
            {
                CCASSERT( obj["textureNorth"].IsString(), "" );
                tile.textureNorth = obj["textureNorth"].GetString();
            }
            
            if( obj.HasMember( "textureEast" ) )
            {
                CCASSERT( obj["textureEast"].IsString(), "" );
                tile.textureEast = obj["textureEast"].GetString();
            }
            
            if( obj.HasMember( "textureSouth" ) )
            {
                CCASSERT( obj["textureSouth"].IsString(), "" );
                tile.textureSouth = obj["textureSouth"].GetString();
            }
            
            if( obj.HasMember( "textureWest" ) )
            {
                CCASSERT( obj["textureWest"].IsString(), "" );
                tile.textureWest = obj["textureWest"].GetString();
            }
            
            if( obj.HasMember( "textureFloor" ) )
            {
                CCASSERT( obj["textureFloor"].IsString(), "" );
                tile.textureFloor = obj["textureFloor"].GetString();
                CCLOG( "textureFloor: %s", tile.textureFloor.c_str() );
            }
            
            if( obj.HasMember( "textureCeiling" ) )
            {
                CCASSERT( obj["textureCeiling"].IsString(), "" );
                tile.textureCeiling = obj["textureCeiling"].GetString();
            }
            
            if( obj.HasMember( "textureAll" ) )
            {
                CCASSERT( obj["textureAll"].IsString(), "" );
                tile.textureAll = obj["textureAll"].GetString();
            }
            
            if( obj.HasMember( "textureCenterSpanNS" ) )
            {
                CCASSERT( obj["textureCenterSpanNS"].IsString(), "" );
                tile.textureCenterSpanNS = obj["textureCenterSpanNS"].GetString();
            }
            
            if( obj.HasMember( "textureCenterSpanEW" ) )
            {
                CCASSERT( obj["textureCenterSpanEW"].IsString(), "" );
                tile.textureCenterSpanEW = obj["textureCenterSpanEW"].GetString();
            }
            
            
            //
            // NORMAL
            //
            if( obj.HasMember( "normalNorth" ) )
            {
                CCASSERT( obj["normalNorth"].IsString(), "" );
                tile.normalNorth = obj["normalNorth"].GetString();
            }
            
            if( obj.HasMember( "normalEast" ) )
            {
                CCASSERT( obj["normalEast"].IsString(), "" );
                tile.normalEast = obj["normalEast"].GetString();
            }
            
            if( obj.HasMember( "normalSouth" ) )
            {
                CCASSERT( obj["normalSouth"].IsString(), "" );
                tile.normalSouth = obj["normalSouth"].GetString();
            }
            
            if( obj.HasMember( "normalWest" ) )
            {
                CCASSERT( obj["normalWest"].IsString(), "" );
                tile.normalWest = obj["normalWest"].GetString();
            }
            
            if( obj.HasMember( "normalFloor" ) )
            {
                CCASSERT( obj["normalFloor"].IsString(), "" );
                tile.normalFloor = obj["normalFloor"].GetString();
                CCLOG( "normalFloor: %s", tile.normalFloor.c_str() );
            }
            
            if( obj.HasMember( "normalCeiling" ) )
            {
                CCASSERT( obj["normalCeiling"].IsString(), "" );
                tile.normalCeiling = obj["normalCeiling"].GetString();
            }
            
            if( obj.HasMember( "normalAll" ) )
            {
                CCASSERT( obj["normalAll"].IsString(), "" );
                tile.normalAll = obj["normalAll"].GetString();
            }
            
            //
            
            if( obj.HasMember( "vertexShader" ) )
            {
                CCASSERT( obj["vertexShader"].IsString(), "" );
                tile.vertexShader = obj["vertexShader"].GetString();
            }
            
            if( obj.HasMember( "fragmentShader" ) )
            {
                CCASSERT( obj["fragmentShader"].IsString(), "" );
                tile.fragmentShader = obj["fragmentShader"].GetString();
            }
                        
            if( obj.HasMember( "textureWrapMode" ) )
            {
                CCASSERT( obj["textureWrapMode"].IsString(), "" );
                tile.textureWrapMode = obj["textureWrapMode"].GetString();
            }
            
            if( obj.HasMember( "textureMinFilter" ) )
            {
                CCASSERT( obj["textureMinFilter"].IsString(), "" );
                tile.textureMinFilter = obj["textureMinFilter"].GetString();
            }
            
            if( obj.HasMember( "textureMagFilter" ) )
            {
                CCASSERT( obj["textureMagFilter"].IsString(), "" );
                tile.textureMagFilter = obj["textureMagFilter"].GetString();
            }
        }
        
        if( obj.HasMember( "tag" ) )
        {
            CCASSERT( obj["tag"].IsInt(), "" );
            tile.tag = obj["tag"].GetInt();
        }
        
        tiles.push_back( tile );
    }
}

void MapInfo::loadJSONPlanes( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    CCASSERT( doc.HasMember( KEY_PLANES ), "" );
    const rapidjson::Value& array = doc[KEY_PLANES];
    
    planes.reserve( array.Size() );
    
    for( rapidjson::SizeType i = 0; i < array.Size(); ++i )
    {
        const rapidjson::Value& obj = array[i];
        
        CCASSERT( obj.IsObject(), "" );
        Plane plane;
        
        CCASSERT( obj.HasMember( "height" ), "" );
        CCASSERT( obj["height"].IsInt(), "" );
        plane.height = obj["height"].GetInt();
        
        CCASSERT( obj.HasMember( "map" ), "" );
        CCASSERT( obj["map"].IsArray(), "" );
        const rapidjson::Value& mapArray = obj["map"];
        
        plane.map = new int[mapArray.Size()]{0};
        
        for( int i = 0; i < mapArray.Size(); ++i )
        {
            plane.map[i] = mapArray[i].GetInt(); 
        }
        
        planes.push_back( plane );
    }
}

void MapInfo::loadJSONActors( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    CCASSERT( doc.HasMember( KEY_ACTORS ), "" );
    const rapidjson::Value& array = doc[KEY_ACTORS];
    
    actors.reserve( array.Size() );
    
    bool definesPlayerObject = false;
    
    for( rapidjson::SizeType i = 0; i < array.Size(); ++i )
    {
        const rapidjson::Value& obj = array[i];
        
        CCASSERT( obj.IsObject(), "" );
        Actor actor;
        
        CCASSERT( obj.HasMember( "type" ), "" );
        CCASSERT( obj["type"].IsString(), "" );
        actor.type = obj["type"].GetString();
        
        CCASSERT( obj.HasMember( "x" ), "" );
        CCASSERT( obj["x"].IsInt(), "" );
        actor.x = obj["x"].GetInt();
        
        CCASSERT( obj.HasMember( "y" ), "" );
        CCASSERT( obj["y"].IsInt(), "" );
        actor.y = obj["y"].GetInt();
        
        CCASSERT( obj.HasMember( "z" ), "" );
        CCASSERT( obj["z"].IsInt(), "" );
        actor.z = obj["z"].GetInt();
        
        CCASSERT( obj.HasMember( "yaw" ), "" );
        CCASSERT( obj["yaw"].IsDouble(), "" );
        actor.yaw = obj["yaw"].GetDouble();
        
        if( actor.type == PLAYER_ACTOR_STR )
        {
            definesPlayerObject = true;
        }
        
        actors.push_back( actor );
    }
    
    CCASSERT( definesPlayerObject, PLAYER_ACTOR_UNDEFINED_ERR_MSG );
}

void MapInfo::loadJSONBehaviors( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    if( !doc.HasMember( KEY_BEHAVIORS ) )
    {
        return;
    }
    const rapidjson::Value& array = doc[KEY_BEHAVIORS];
    
    behaviors.reserve( array.Size() );
    
    for( rapidjson::SizeType i = 0; i < array.Size(); ++i )
    {
        const rapidjson::Value& obj = array[i];
        
        CCASSERT( obj.IsObject(), "" );
        Behavior behavior;
        
        if( obj.HasMember( "onEnter" ) )
        {
            CCASSERT( obj["onEnter"].IsString(), "" );
            behavior.onEnter = obj["onEnter"].GetString();
        }
        
        if( obj.HasMember( "onExit" ) )
        {
            CCASSERT( obj["onExit"].IsString(), "" );
            behavior.onEnter = obj["onExit"].GetString();
        }
        
        if( obj.HasMember( "onCreate" ) )
        {
            CCASSERT( obj["onCreate"].IsString(), "" );
            behavior.onEnter = obj["onCreate"].GetString();
        }
        
        behaviors.push_back( behavior );
    }
}

/*
 
    TODO: Implement triggers for this map.
 
 */
void MapInfo::loadJSONTriggers( const rapidjson::Document& doc )
{
    CCASSERT( doc.IsObject(), ASSERT_FAILED_NANO );
    CCASSERT( doc.HasMember( KEY_TRIGGERS ), TRIGGERS_UNDEFINED_ERR_MSG );
    const rapidjson::Value& array = doc[KEY_TRIGGERS];
    
    triggers.reserve( array.Size() );
    
    for( rapidjson::SizeType i = 0; i < array.Size(); ++i )
    {
        const rapidjson::Value& obj = array[i];
        
        CCASSERT( obj.IsObject(), "" );
        Trigger trigger;
        
        if( obj.HasMember( "continueRaycast" ) )
        {
            CCASSERT( obj["continueRaycast"].IsBool(), "" );
            trigger.continueRaycast = obj["continueRaycast"].GetBool();
        }
        
        triggers.push_back( trigger );
    }
}

int MapInfo::getBehaviorIndex( int tileResourceIndex )
{
    return tiles[tileResourceIndex].tag;
}

//==============================================================================
//
// CONSTRUCTORS / DESTRUCTOR
//
//==============================================================================

MapInfo::MapInfo( std::string fullPath )
{
    loadMapInfo( fullPath );
}

MapInfo::MapInfo()
{
}

MapInfo::~MapInfo()
{
    if( planes.size() > 0 )
    {
        for( int i = 0; i < planes.size(); ++i )
        {
            delete planes[i].map;
        }
    }
    spritesheets.clear();
    tiles.clear();
    planes.clear();
    actors.clear();
}
