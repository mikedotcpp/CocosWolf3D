//
// This shader is used for the mesh rendering path only. It allows for normal mapping and realtime lighting.
//
#define USE_NORMAL_MAPPING 1
#define MAX_DIRECTIONAL_LIGHT_NUM 1
#define MAX_POINT_LIGHT_NUM 2
#define MAX_SPOT_LIGHT_NUM 2

#ifdef USE_NORMAL_MAPPING
#if (MAX_DIRECTIONAL_LIGHT_NUM > 0)
uniform vec3 u_DirLightSourceDirection[MAX_DIRECTIONAL_LIGHT_NUM];
#endif
#endif
#if (MAX_POINT_LIGHT_NUM > 0)
uniform vec3 u_PointLightSourcePosition[MAX_POINT_LIGHT_NUM];
#endif
#if (MAX_SPOT_LIGHT_NUM > 0)
uniform vec3 u_SpotLightSourcePosition[MAX_SPOT_LIGHT_NUM];
#ifdef USE_NORMAL_MAPPING
uniform vec3 u_SpotLightSourceDirection[MAX_SPOT_LIGHT_NUM];
#endif
#endif

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec3 a_normal;
#ifdef USE_NORMAL_MAPPING
attribute vec3 a_tangent;
attribute vec3 a_binormal;
#endif
varying vec2 TextureCoordOut;

// Uniforms
const int MAX_POSITION_COUNT = 600;
uniform vec3 u_posPalette[MAX_POSITION_COUNT];

#ifdef USE_NORMAL_MAPPING
#if MAX_DIRECTIONAL_LIGHT_NUM
varying vec3 v_dirLightDirection[MAX_DIRECTIONAL_LIGHT_NUM];
#endif
#endif
#if MAX_POINT_LIGHT_NUM
varying vec3 v_vertexToPointLightDirection[MAX_POINT_LIGHT_NUM];
#endif
#if MAX_SPOT_LIGHT_NUM
varying vec3 v_vertexToSpotLightDirection[MAX_SPOT_LIGHT_NUM];
#ifdef USE_NORMAL_MAPPING
varying vec3 v_spotLightDirection[MAX_SPOT_LIGHT_NUM];
#endif
#endif

#ifndef USE_NORMAL_MAPPING
#if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
varying vec3 v_normal;
#endif
#endif

void main(void)
{
#ifdef GL_ES
//         CONFIRMED WORKS (iOS)
        vec4 ePosition = CC_MVMatrix * vec4( u_posPalette[ gl_InstanceIDEXT ] + a_position.xyz, 1 );
#else
//        // CONFIRMED WORKS (DESKTOP - MAC)
        vec4 ePosition = CC_MVMatrix * vec4( u_posPalette[ gl_InstanceIDARB ] + a_position.xyz, 1 );
#endif
    
#ifdef USE_NORMAL_MAPPING
    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        vec3 eTangent = normalize(CC_NormalMatrix * a_tangent);
        vec3 eBinormal = normalize(CC_NormalMatrix * a_binormal);
        vec3 eNormal = normalize(CC_NormalMatrix * a_normal);
    #endif
    #if (MAX_DIRECTIONAL_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_DIRECTIONAL_LIGHT_NUM; ++i)
        {
            v_dirLightDirection[i].x = dot(eTangent, u_DirLightSourceDirection[i]);
            v_dirLightDirection[i].y = dot(eBinormal, u_DirLightSourceDirection[i]);
            v_dirLightDirection[i].z = dot(eNormal, u_DirLightSourceDirection[i]);
        }
    #endif
    
    #if (MAX_POINT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_POINT_LIGHT_NUM; ++i)
        {
            vec3 pointLightDir = u_PointLightSourcePosition[i].xyz - ePosition.xyz;
            v_vertexToPointLightDirection[i].x = dot(eTangent, pointLightDir);
            v_vertexToPointLightDirection[i].y = dot(eBinormal, pointLightDir);
            v_vertexToPointLightDirection[i].z = dot(eNormal, pointLightDir);
        }
    #endif
    
    #if (MAX_SPOT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
        {
            vec3 spotLightDir = u_SpotLightSourcePosition[i] - ePosition.xyz;
            v_vertexToSpotLightDirection[i].x = dot(eTangent, spotLightDir);
            v_vertexToSpotLightDirection[i].y = dot(eBinormal, spotLightDir);
            v_vertexToSpotLightDirection[i].z = dot(eNormal, spotLightDir);
            
            v_spotLightDirection[i].x = dot(eTangent, u_SpotLightSourceDirection[i]);
            v_spotLightDirection[i].y = dot(eBinormal, u_SpotLightSourceDirection[i]);
            v_spotLightDirection[i].z = dot(eNormal, u_SpotLightSourceDirection[i]);
        }
    #endif
#else
    #if (MAX_POINT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_POINT_LIGHT_NUM; ++i)
        {
            v_vertexToPointLightDirection[i] = u_PointLightSourcePosition[i].xyz - ePosition.xyz;
        }
    #endif
    
    #if (MAX_SPOT_LIGHT_NUM > 0)
        for (int i = 0; i < MAX_SPOT_LIGHT_NUM; ++i)
        {
            v_vertexToSpotLightDirection[i] = u_SpotLightSourcePosition[i] - ePosition.xyz;
        }
    #endif
    
    #if ((MAX_DIRECTIONAL_LIGHT_NUM > 0) || (MAX_POINT_LIGHT_NUM > 0) || (MAX_SPOT_LIGHT_NUM > 0))
        v_normal = CC_NormalMatrix * a_normal;
    #endif
#endif
    
    TextureCoordOut = a_texCoord;
    TextureCoordOut.y = 1.0 - TextureCoordOut.y;
    gl_Position = CC_PMatrix * ePosition;
}
