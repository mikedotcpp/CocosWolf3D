//
//  BatchedGLProgram.hpp
//  CocosWolf3D
//
//  Created by Michael Edwards on 2/20/17.
//
//

#ifndef BatchedGLProgram_hpp
#define BatchedGLProgram_hpp

#include "cocos2d.h"

namespace mikedotcpp
{
    /**
     * It is annoying, but I have to write my own GL Program class just so that I can put a simple line of text
     * at the beginning of a vertex shader. This is all in support of instancing.
     */
    class BatchedGLProgram : public cocos2d::GLProgram
    {
    public:
        BatchedGLProgram();
        static BatchedGLProgram* createWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);
        bool initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename);
        bool initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray );
        bool compileShader(GLuint * shader, GLenum type, const GLchar* source, const std::string& convertedDefines);
    };
}

#endif /* BatchedGLProgram_hpp */
