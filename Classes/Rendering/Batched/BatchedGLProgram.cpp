//
//  BatchedGLProgram.cpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/20/17.
//
//

#include "BatchedGLProgram.hpp"

using namespace mikedotcpp;

static const char * COCOS2D_SHADER_UNIFORMS =
"uniform mat4 CC_PMatrix;\n"
"uniform mat4 CC_MVMatrix;\n"
"uniform mat4 CC_MVPMatrix;\n"
"uniform mat3 CC_NormalMatrix;\n"
"uniform vec4 CC_Time;\n"
"uniform vec4 CC_SinTime;\n"
"uniform vec4 CC_CosTime;\n"
"uniform vec4 CC_Random01;\n"
"uniform sampler2D CC_Texture0;\n"
"uniform sampler2D CC_Texture1;\n"
"uniform sampler2D CC_Texture2;\n"
"uniform sampler2D CC_Texture3;\n"
"//CC INCLUDES END\n\n";


BatchedGLProgram::BatchedGLProgram()
{
    _director = cocos2d::Director::getInstance();
    CCASSERT(nullptr != _director, "Director is null when init a GLProgram");
    memset(_builtInUniforms, 0, sizeof(_builtInUniforms));
}

BatchedGLProgram* BatchedGLProgram::createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename)
{
    auto ret = new (std::nothrow) BatchedGLProgram();
    if( ret && ret->initWithFilenames( vShaderFilename, fShaderFilename ) )
    {
        ret->link();
        ret->updateUniforms();
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BatchedGLProgram::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename)
{
    auto fileUtils = cocos2d::FileUtils::getInstance();
    std::string vertexSource = fileUtils->getStringFromFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(vShaderFilename));
    std::string fragmentSource = fileUtils->getStringFromFile(cocos2d::FileUtils::getInstance()->fullPathForFilename(fShaderFilename));
    
    return initWithByteArrays(vertexSource.c_str(), fragmentSource.c_str());
}

bool BatchedGLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray )
{
    _program = glCreateProgram();
    CHECK_GL_ERROR_DEBUG();

    // convert defines here. If we do it in "compileShader" we will do it twice.
    // a cache for the defines could be useful, but seems like overkill at this point
    std::string replacedDefines = "";

    _vertShader = _fragShader = 0;

    if (vShaderByteArray)
    {
        if (!compileShader(&_vertShader, GL_VERTEX_SHADER, vShaderByteArray, replacedDefines))
        {
            CCLOG("cocos2d: ERROR: Failed to compile vertex shader");
            return false;
       }
    }

    // Create and compile fragment shader
    if (fShaderByteArray)
    {
        if (!compileShader(&_fragShader, GL_FRAGMENT_SHADER, fShaderByteArray, replacedDefines))
        {
            CCLOG("cocos2d: ERROR: Failed to compile fragment shader");
            return false;
        }
    }

    if (_vertShader)
    {
        glAttachShader(_program, _vertShader);
    }
    CHECK_GL_ERROR_DEBUG();

    if (_fragShader)
    {
        glAttachShader(_program, _fragShader);
    }

    _hashForUniforms.clear();

    CHECK_GL_ERROR_DEBUG();

    return true;
}

bool BatchedGLProgram::compileShader(GLuint* shader, GLenum type, const GLchar* source, const std::string& convertedDefines)
{
    GLint status;
    
    if (!source)
    {
        return false;
    }
    
    const GLchar *sources[] = {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WINRT
        (type == GL_VERTEX_SHADER ? "precision mediump float;\n precision mediump int;\n" : "precision mediump float;\n precision mediump int;\n"),
#elif (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32 && CC_TARGET_PLATFORM != CC_PLATFORM_LINUX && CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
        (type == GL_VERTEX_SHADER ? "#extension GL_EXT_draw_instanced : enable \n" : "precision highp float;\n precision highp int;\n"),
//        (type == GL_VERTEX_SHADER ? "#extension GL_EXT_draw_instanced : enable \nprecision mediump float;\n precision mediump int;\n" : "precision mediump float;\n precision mediump int;\n"),
//        (type == GL_VERTEX_SHADER ? "#extension GL_EXT_draw_instanced : enable \nprecision highp float;\n precision highp int;\n" : "precision mediump float;\n precision mediump int;\n"),
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
        (type == GL_VERTEX_SHADER ? "#extension GL_ARB_draw_instanced : enable \n" : ""),
#endif
        COCOS2D_SHADER_UNIFORMS,
        convertedDefines.c_str(),
        source};
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, sizeof(sources)/sizeof(*sources), sources, nullptr);
    glCompileShader(*shader);
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    
    if (! status)
    {
        GLsizei length;
        glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
        GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);
        
        glGetShaderSource(*shader, length, nullptr, src);
        CCLOG("cocos2d: ERROR: Failed to compile shader:\n%s", src);
        
        free(src);
        
        return false;
    }
    
    return (status == GL_TRUE);
}
