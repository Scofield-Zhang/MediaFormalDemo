#include <jni.h>
#include <string>
#include <stdlib.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include "EGLCore.h"

#define DePrint(...) do{__android_log_print(ANDROID_LOG_ERROR, "Demon_jni", __VA_ARGS__);}while(0)


std::string glVertexShader =
        "attribute vec4 vPosition;"
                "attribute vec2 aTextureCoord;\n"
                "varying highp vec2 vTextureCoord;\n"
                "void main(){"
                "   vTextureCoord = aTextureCoord;"
                "   gl_Position = vPosition;"
                "}";

std::string glFragmentShader=
        "#extension GL_OES_EGL_image_external : require\n"
                "varying highp vec2 vTextureCoord;\n"
                "uniform samplerExternalOES tex;"
                "void main(){"
                "   highp vec4 color = vec4(vTextureCoord.x, vTextureCoord.y, 0, 1.0);"
                "   gl_FragColor=texture2D(tex, vTextureCoord);\n"
                "}";
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc((size_t) infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    DePrint("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    } else{
        DePrint("create shader error:%x", glGetError());
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);

    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        //checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        // checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc((size_t) bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    DePrint("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}


EGLCore *eglCore;

GLuint  texture;
GLuint gProgram;
GLuint  vertexBuffer;
GLuint indexBuffer;
GLuint gvPositionHandle;
GLuint gTextureCoordHandle;

const GLfloat gTriangleVertices[] =
        {
                -1.0f, -1.0f, 0.0f, 0.0f,   // 0 bottom left
                1.0f, -1.0f,  1.0f, 0.0f,  // 1 bottom right
                -1.0f,  1.0f, 0.0f, 1.0f,   // 2 top left
                1.0f,  1.0f,  1.0f, 1.0f   // 3 top right
        };

const GLubyte gIndexData[] = {
        0, 1, 2,
        2, 3, 1
};

bool setupGraphics()
{
    DePrint("setupGraphics, thread:%ld", pthread_self());
    gProgram = createProgram(glVertexShader.c_str(), glFragmentShader.c_str());
    if (!gProgram)
    {
        return false;
    }
    glUseProgram(gProgram);
    gvPositionHandle = (GLuint) glGetAttribLocation(gProgram, "vPosition");
    gTextureCoordHandle = (GLuint) glGetAttribLocation(gProgram, "aTextureCoord");
    GLuint texSlot = (GLuint)glGetUniformLocation(gProgram, "tex");
    glUniform1i(texSlot, 0);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gTriangleVertices), gTriangleVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndexData), gIndexData, GL_STATIC_DRAW);

    return true;
}


void draw()
{
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glEnableVertexAttribArray(gTextureCoordHandle);
    glVertexAttribPointer(gTextureCoordHandle, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void *)(sizeof(float) * 2));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    DePrint("绘制成功");
}


extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_createEGLContextFromJNI(JNIEnv *env,jobject instance,jobject surface)
{
    eglCore = new EGLCore(env,surface, nullptr,true);
    eglCore->makeCurrent();
    setupGraphics();
    DePrint("create success");
}

extern "C"
JNIEXPORT jint JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_createTextureIdFromJNI(JNIEnv *env,jobject instance)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

EGLSurface  eglSurface;

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_createEGLSurfaceFromJNI(JNIEnv *env,jobject instance,jobject surface)
{
    eglSurface = eglCore->createWindowSurface(env,surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_setGLViewPort(JNIEnv *env, jobject instance,jint width, jint height)
{
    glViewport(0, 0, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_makeCurrentSurfaceView(JNIEnv *env,jobject instance)
{
    eglCore->makeCurrent();
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_drawTexture(JNIEnv *env, jobject instance)
{
    draw();
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_swapBuffer(JNIEnv *env, jobject instance)
{
    eglCore->swapBuffers();
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_makeCurrentGLSurface(JNIEnv *env,jobject instance)
{
    eglCore->bindCurrent(eglSurface);
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_setPresentationTime(JNIEnv *env, jobject instance,jlong presentationTime)
{
    eglPresentationTimeANDROID(eglCore->mEGLDisplay, eglSurface, presentationTime);
}
extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_swapBufferGLSurface(JNIEnv *env, jobject instance)
{
    eglSwapBuffers(eglCore->mEGLDisplay , eglSurface);
}

extern "C"
JNIEXPORT void JNICALL
Java_zhang_alex_com_mediaformaldemo_MySurfaceView_createWindowsSurfaceFromJNI(JNIEnv *env, jobject instance, jobject surface)
{
    eglSurface = eglCore->createWindowSurface(env,surface);
}