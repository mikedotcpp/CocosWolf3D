//
//  GBRaycaster.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 10/3/16.
//
//

#include "GBRaycaster.hpp"

using namespace mikedotcpp;

//==============================================================================
//
// ASSERT MESSAGES
//
//==============================================================================

#define NO_DELEGATE_MSG "GBRaycaster::castRays(...) requires a _delegate that implements the GBRaycasterInterface."

//==============================================================================
//
// INITIALIZATION
//
//==============================================================================

GBRaycaster::GBRaycaster( const MapInfo& mapInfo, GBRaycasterInterface* delegate )
{
    setTileWidth( mapInfo.tileSize );
    setTileHeight( mapInfo.tileSize );
    setMapWidth( mapInfo.width );
    setMapHeight( mapInfo.height );
    setDelegate( delegate );
    _planes = mapInfo.planes;
    preComputeRayAngles();
}

GBRaycaster::~GBRaycaster()
{
    _rayAngles.clear();
    memset( &_rayAngles, 0, sizeof( _rayAngles ) );
    printf( "GBRaycaster deleted, release resources.\n" );
}

//==============================================================================
//
// RAYCASTING
//
//==============================================================================

Point2i GBRaycaster::tileCoordForPosition( Point3f pos )
{
    return tileCoordForPosition( pos.x, pos.y );
}

Point2i GBRaycaster::tileCoordForPosition( float x, float y )
{
    Point2i result;
    static float mapHeightxTileHeight = _mapHeight * _tileHeight;
    static float widthDivisor = 1 / _tileWidth;
    static float heightDivisor = 1 / _tileHeight;
    result.x = floorf( x * widthDivisor );
    result.y = floorf( ( mapHeightxTileHeight - y ) * heightDivisor );
    return result;
}

Point3f GBRaycaster::tilePositionForCoord( Point2i coord )
{
    return tilePositionForCoord( coord.x, coord.y );
}

Point3f GBRaycaster::tilePositionForCoord( int x, int y )
{
    Point3f result;
    result.x = ( x * _tileWidth ) + ( _tileWidth / 2 );
    result.y = ( ( _mapHeight - y - 1 ) * _tileHeight ) + ( _tileHeight / 2 );
    return result;
}

void GBRaycaster::preComputeRayAngles()
{
    float fovRadians = _fov * MATH_PI / 180.0f;
    float _viewDist = ( _rayCount/2.0f ) / tan( fovRadians/2.0f );
    _rayAngles.reserve( _rayCount );
    for ( int i = 0; i < _rayCount; i++)
    {
        float rayScreenPos = ( -_rayCount/2.0f + i );
        float rayViewDist = sqrt( rayScreenPos*rayScreenPos + _viewDist*_viewDist );
        float rayAngle = asin( rayScreenPos / rayViewDist );
        _rayAngles.push_back( rayAngle );
    }
}

void GBRaycaster::transposeAboutY( Point3f& vector )
{
    vector.y = vector.x;
    vector.x = vector.z;
}

void GBRaycaster::castRays( Point3f playerPosition, float rotation )
{
    assert( _delegate != nullptr && NO_DELEGATE_MSG );
    
    transposeAboutY( playerPosition );
    Point2i playerTileCoord = tileCoordForPosition( playerPosition );
    Point3f playerTilePosition = tilePositionForCoord( playerTileCoord );
    
    setPlayerTile( playerTileCoord, playerTilePosition );
    
    for( int rayIndex = 0; rayIndex < _rayCount; rayIndex++ )
    {
        float rayAngle = rotation + _rayAngles.at( rayIndex );
        rayAngle = fmodf( rayAngle, TWO_PI );
        rayAngle = ( rayAngle < 0 ) ? rayAngle + TWO_PI : rayAngle;
        castRay( rayIndex, rayAngle, playerTileCoord, playerTilePosition, playerPosition );
    }
}

void GBRaycaster::setPlayerTile( Point2i playerTile, Point3f playerPosition )
{
    int index = getIndexFromMapCoord( playerTile );
    for( int i = 0; i < _planes.size(); ++i )
    {
        Plane plane = _planes[i];
        int tileIndex = plane.map[index] - 1;
        if( tileIndex >= 0 )
        {
            Point3f tilePos = Point3f( playerPosition.y, plane.height, playerPosition.x );
            _delegate->processHit( index, 0.0f, tilePos, tileIndex, i );
        }
    }
}

int GBRaycaster::getTileResourceIndex( Point3f position )
{
    Plane plane;
    Point3f transposedPosition = position;
    transposeAboutY( transposedPosition );
    int tileResourceIndex = -1;
    for( int i = 0; i < _planes.size(); ++i )
    {
        plane = _planes[i];
//        CCLOG( "PLANE_HEIGHT: %i  PLAYER_HEIGHT: %f", plane.height, position.y );
        if( plane.height == position.y )
        {
            Point2i positionTile = tileCoordForPosition( transposedPosition );
            int tileIndex = getIndexFromMapCoord( positionTile );
            tileResourceIndex = plane.map[tileIndex] - 1;
            if( tileResourceIndex >= 0 )
            {
                break;
            }
        }
    }
    return tileResourceIndex;
}

float GBRaycaster::getTileResourceHeight( Point3f position )
{
    Plane plane;
    Point3f transposedPosition = position;
    transposeAboutY( transposedPosition );
    float tileResourceHeight = 0.0f;
    int tileResourceIndex = -1;
    for( int i = 0; i < _planes.size(); ++i )
    {
        plane = _planes[i];
        if( plane.height < position.y )
        {
            Point2i positionTile = tileCoordForPosition( transposedPosition );
            int tileIndex = getIndexFromMapCoord( positionTile );
            tileResourceIndex = plane.map[tileIndex] - 1;
            if( tileResourceIndex >= 0 )
            {
                tileResourceHeight = plane.height;
                break;
            }
        }
    }
    return tileResourceHeight;
}

void GBRaycaster::clearTileResourceAt( Point3f position )
{
    Plane plane;
    Point3f transposedPosition = position;
    transposeAboutY( transposedPosition );
    int tileResourceIndex = -1;
    for( int i = 0; i < _planes.size(); ++i )
    {
        plane = _planes[i];
        if( plane.height == position.y )
        {
            Point2i positionTile = tileCoordForPosition( transposedPosition );
            int tileIndex = getIndexFromMapCoord( positionTile );
            tileResourceIndex = plane.map[tileIndex] - 1;
            if( tileResourceIndex >= 0 )
            {
                plane.map[tileIndex] = 0;
                break;
            }
        }
    }
}

void GBRaycaster::castRay( int stripIdx, float rayAngle, Point2i playerTileCoords, Point3f playerTilePosition, Point3f playerPosition )
{
    Point3f raySinCos( sinf( rayAngle ), cosf( rayAngle ), 0.0f );
    traceRayVertically( rayAngle, raySinCos, playerTilePosition, playerPosition );
    traceRayHorizontally( rayAngle, raySinCos, playerTilePosition, playerPosition );
}

void GBRaycaster::traceRayVertically( float rayAngle, Point3f raySinCos, Point3f playerTilePosition, Point3f playerPosition )
{
    bool right = ( rayAngle > THREE_QUARTERS || rayAngle < ONE_QUARTER );
    float slope = raySinCos.x / raySinCos.y;
    float dX = right ? _tileWidth : -_tileWidth;
    Point3f rayPointChange( dX, dX * slope, 0.0f );
    float x = right ? ( playerTilePosition.x + _tileWidth/2 ) : ( playerTilePosition.x - _tileWidth/2 );
    Point3f rayPoint( x, playerPosition.y + ( x - playerPosition.x ) * slope, 0.0f );
    float horizontalIncrement = right ? 0 : -_tileWidth;
    Point3f increment( horizontalIncrement, 0.0f, 0.0f );
    traceRay( rayPoint, rayPointChange, increment, rayAngle );
}

void GBRaycaster::traceRayHorizontally( float rayAngle, Point3f raySinCos, Point3f playerTilePosition, Point3f playerPosition )
{
    bool up = ( rayAngle < MATH_PI && rayAngle > 0 );
    float slope = raySinCos.y / raySinCos.x;
    float dY = up ? _tileHeight : -_tileHeight;
    Point3f rayPointChange( dY *  slope, dY, 0.0f );
    float y = up ? (playerTilePosition.y + _tileHeight/2) : (playerTilePosition.y - _tileHeight/2);
    Point3f rayPoint( playerPosition.x + (y - playerPosition.y) * slope, y, 0.0f );
    float verticalIncrement = up ? _tileHeight : 0;
    Point3f increment( 0, verticalIncrement, 0.0f );
    traceRay( rayPoint, rayPointChange, increment, rayAngle );
}

void GBRaycaster::traceRay( Point3f rayPoint, Point3f rayPointChange, Point3f increment, float rayAngle )
{
    int expectedX = 0, expectedY = 0;
    while( rayPoint.x >= 0 && rayPoint.x < _mapWidth * _tileWidth && rayPoint.y >= 0 && rayPoint.y < _mapHeight * _tileHeight )
    {
        int wallSub1 = MAX( 0, ( ( rayPoint.x + increment.x ) * _tileWidthDivisor ) );
        int wallSub2 = MIN( _mapHeight - 1, ( ( _mapHeight * _tileHeight - ( rayPoint.y + increment.y ) ) * _tileHeightDivisor ) );
        int index = getIndexFromMapCoord( Point2i( wallSub1, wallSub2 ) );
        int returnCount = 0;
        
        // Check to see if the next tile encountered is outside the expected bounds.
        if( expectedX != 0 || expectedY != 0 )
        {
            if( abs( wallSub1 - expectedX ) > 1 || abs( wallSub2 - expectedY ) > 1 )
            {
                return;
            }
        }
        expectedX = wallSub1;
        expectedY = wallSub2;
        
        // Draw sprites/meshes for each plane at this tile location.
        for( int i = 0; i < _planes.size(); ++i )
        {
            Plane plane = _planes[i];
            int tileIndex = plane.map[index] - 1;
            if( tileIndex >= 0 )
            {
                Point3f tilePos = tilePositionForCoord( wallSub1, wallSub2 );
                
                bool continueProcessing = _delegate->processHit( index,
                                                                rayAngle,
                                                                Point3f( tilePos.y, plane.height, tilePos.x ),
                                                                tileIndex,
                                                                i );
                if( !continueProcessing )
                {
                    returnCount++;
                }
            }
        }
        
        if( returnCount > 0 )
        {
            return;
        }
        
        rayPoint.x += rayPointChange.x;
        rayPoint.y += rayPointChange.y;
    }
}

/**
 * DEPRECATED
 */
Point2i GBRaycaster::getShortestMapCoord( Point3f playerPosition, Point2i vCoord, Point2i hCoord )
{
    Point3f vPoint = tilePositionForCoord( vCoord );
    Point3f hPoint = tilePositionForCoord( hCoord );
    float vDist = vPoint.getSquaredDistance( playerPosition );
    float hDist = hPoint.getSquaredDistance( playerPosition );
    if( !vDist || hDist < vDist )
    {
        return hCoord;
    }
    return vCoord;
}

int GBRaycaster::getIndexFromMapCoord( Point2i coord )
{
    int index = _mapWidth * coord.y + coord.x;
    return index;
}

//==============================================================================
//
// ACCESSORS AND MUTATORS
//
//==============================================================================

void GBRaycaster::setRayCount( int count )
{
    if( count > 0 )
    {
        _rayCount = count;
    }
}

int GBRaycaster::getRayCount()
{
    return _rayCount;
}

void GBRaycaster::setTileWidth( float width )
{
    _tileWidth = width;
    _tileWidthDivisor = 1 / _tileWidth;
}

float GBRaycaster::getTileWidth()
{
    return _tileWidth;
}

void GBRaycaster::setTileHeight( float height )
{
    _tileHeight = height;
    _tileHeightDivisor = 1 / _tileHeight;
}

float GBRaycaster::getTileHeight()
{
    return _tileHeight;
}

void GBRaycaster::setMapWidth( float width )
{
    _mapWidth = width;
}

float GBRaycaster::getMapWidth()
{
    return _mapWidth;
}

void GBRaycaster::setMapHeight( float height )
{
    _mapHeight = height;
}

float GBRaycaster::getMapHeight()
{
    return _mapHeight;
}

int GBRaycaster::getMapSize()
{
    return _mapWidth * _mapHeight;
}

void GBRaycaster::setDelegate( GBRaycasterInterface* delegate )
{
    _delegate = delegate;
}
