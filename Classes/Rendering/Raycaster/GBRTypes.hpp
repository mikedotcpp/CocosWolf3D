//
//  GBRTypes.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 3/14/17.
//
//

#ifndef GBRTypes_hpp
#define GBRTypes_hpp

namespace mikedotcpp
{
    /**
     * A container for 3 floats.
     */
    struct Point3f
    {
    public:
        float x;
        float y;
        float z;
        
        Point3f(){ x = 0.0f, y = 0.0f, z = 0.0f; };
        
        Point3f( float newx, float newy, float newz )
        {
            x = newx, y = newy, z = newz;
        }
        
        float getSquaredDistance( Point3f otherPoint )
        {
            float deltaX = otherPoint.x - x;
            float deltaY = otherPoint.y - y;
            float deltaZ = otherPoint.z - z;
            return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
        }
        
        float getDistance( Point3f otherPoint )
        {
            float sqDist = getSquaredDistance( otherPoint );
            return sqrtf( sqDist );
        }
    };
    
    /**
     * A container for 2 int positions and a float that represents the height. This is a very specific format, I should
     * consider renaming to something less general.
     */
    struct Point2i
    {
    public:
        int x;
        int y;
        float height;
        Point3f hit;
        
        Point2i(){ x = 0, y = 0, height = 0; };
        
        Point2i( int newx, int newy )
        {
            x = newx;
            y = newy;
            height = 0;
        }
    };
}

#endif /* GBRTypes_hpp */
