//
// Created by 张涛 on 2017/5/2.
//

#include "EGLCore.h"
#include <android/log.h>
#include <EGL/eglext.h>
#include <android/native_window_jni.h>

#define DePrint(...) do{__android_log_print(ANDROID_LOG_ERROR, "Demon_jni", __VA_ARGS__);}while(0)

EGLCore::EGLCore(JNIEnv* env, jobject surface, EGLContext sharedContext, bool recordable):
        mSecondEGLSurface(EGL_NO_SURFACE)
{
    createEGLContext(sharedContext, recordable);
    mEGLSurface = createWindowSurface(env, surface);
}

EGLCore::EGLCore(int width, int height, EGLContext sharedContext , bool recordable):
        mSecondEGLSurface(EGL_NO_SURFACE)
{
    createEGLContext(sharedContext, recordable);
    mEGLSurface = createOffscreenSurface(width, height);
}

void EGLCore::createEGLContext(EGLContext sharedContext, bool recordable)
{
    mEGLDisplay = EGL_NO_DISPLAY;
    mEGLContext = EGL_NO_CONTEXT;
    mEGLConfig = nullptr;
    mGlVersion = -1;


    if (mEGLDisplay != EGL_NO_DISPLAY)
    {
        DePrint("EGL already set up");
        return;
    }

    if (sharedContext == nullptr)
    {
        sharedContext = EGL_NO_CONTEXT;
    }

    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        DePrint("unable to get EGL14 display");
        return;
    }
    int version[2];

    // EGLDisplay dpy, EGLint *major, EGLint *minor
    if (!eglInitialize(mEGLDisplay, version, 0))
    {
        mEGLDisplay = nullptr;
        DePrint("unable to initialize EGL14");
    }

    if (mEGLContext == EGL_NO_CONTEXT)
    {
        EGLConfig config = getConfig(recordable, 2);
        if (config == nullptr) {
            DePrint("Unable to find a suitable EGLConfig");
            return;
        }
        int attrib2_list[] =
                {
                        EGL_CONTEXT_CLIENT_VERSION, 2,
                        EGL_NONE
                };
        // EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list
        EGLContext context = eglCreateContext(mEGLDisplay, config, sharedContext, attrib2_list);
        checkEglError("eglCreateContext");
        mEGLConfig = config;
        mEGLContext = context;
        mGlVersion = 2;

    }

    // Confirm with query.
    int values[1];
    // EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value
    eglQueryContext(mEGLDisplay, mEGLContext, EGL_CONTEXT_CLIENT_VERSION, values);
    DePrint("EGLContext created, client version %d",values[0]);
}

EGLConfig EGLCore::getConfig(bool recordable, int version) {
    int renderableType = EGL_OPENGL_ES2_BIT;
    if (version >= 3) {
        renderableType |= EGL_OPENGL_ES3_BIT_KHR;
    }

    // The actual surface is generally RGBA or RGBX, so situationally omitting alpha
    // doesn't really help.  It can also lead to a huge performance hit on glReadPixels()
    // when reading into a GL_RGBA buffer.
    int attribList[] =
    {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, renderableType,
            EGL_NONE, 0,      // placeholder for recordable [@-3]
            EGL_NONE
    };
    if (recordable) {
        attribList[sizeof(attribList) - 3] = EGL_RECORDABLE_ANDROID;
        attribList[sizeof(attribList) - 2] = 1;
    }
    EGLConfig configs[1];
    int numConfigs[1];
    // EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config
    if (!eglChooseConfig(mEGLDisplay, attribList, configs, sizeof(configs), numConfigs)) {
        DePrint("unable to find RGB8888 / %d EGLConfig", version);
        return nullptr;
    }
    return configs[0];
}

EGLCore::~EGLCore()
{
    if (mEGLDisplay != EGL_NO_DISPLAY)
    {
        // Android is unusual in that it uses a reference-counted EGLDisplay.  So for
        // every eglInitialize() we need an eglTerminate().
        eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mEGLDisplay, mEGLContext);
        eglReleaseThread();
        eglTerminate(mEGLDisplay);
    }
    if (mEGLSurface != EGL_NO_SURFACE)
    {
        releaseSurface(mEGLSurface);
    }
    if (mSecondEGLSurface != EGL_NO_SURFACE)
    {
        releaseSurface(mSecondEGLSurface);
    }
    mEGLDisplay = EGL_NO_DISPLAY;
    mEGLContext = EGL_NO_CONTEXT;
    mEGLConfig = nullptr;
}

void EGLCore::releaseSurface(EGLSurface eglSurface) {
    eglDestroySurface(mEGLDisplay, eglSurface);
}

EGLSurface EGLCore::createOffscreenSurface(int width, int height)
{
    int surfaceAttribs[] =
            {
                    EGL_WIDTH, width,
                    EGL_HEIGHT, height,
                    EGL_NONE
            };
    EGLSurface eglSurface = eglCreatePbufferSurface(mEGLDisplay, mEGLConfig, surfaceAttribs);
    checkEglError("eglCreatePbufferSurface");
    if (eglSurface == nullptr)
    {
        DePrint("surface was null");
        return nullptr;
    }
    return eglSurface;
}

void EGLCore::makeCurrent(EGLCoreSurfaceOrder order)
{
    if (mEGLDisplay == EGL_NO_DISPLAY)
    {
        DePrint("NOTE: makeCurrent w/o display");
    }

    EGLSurface surface = order == EGLCoreSurfaceOrder ::EGLCoreSurfaceOrderFirst ? mEGLSurface : mSecondEGLSurface;

    if (!eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext))
    {
        DePrint("eglMakeCurrent failed");
        return;
    }
    DePrint("绑定成功");
}
void EGLCore::bindCurrent(EGLSurface surface)
{
    if (mEGLDisplay == EGL_NO_DISPLAY) {
        // called makeCurrent() before create?
        DePrint("NOTE: makeCurrent w/o display");
    }
    if (!eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext)) {
        DePrint("eglMakeCurrent failed");
    }
}

void EGLCore::makeNothingCurrent()
{
    if (!eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
    {
        DePrint("eglMakeCurrent failed");
        return;
    }
}

EGLBoolean EGLCore::swapBuffers(EGLCoreSurfaceOrder order)
{
    EGLSurface surface = order == EGLCoreSurfaceOrder::EGLCoreSurfaceOrderFirst ? mEGLSurface : mSecondEGLSurface;
    EGLBoolean swapBufferResult = eglSwapBuffers(mEGLDisplay, surface);
    DePrint("swapBufferResult:成功");
    return swapBufferResult;
}

bool EGLCore::isCurrent(EGLSurface eglSurface)
{
    return (eglGetCurrentContext()) && (eglGetCurrentSurface(EGL_DRAW));
}

int EGLCore::querySurface(EGLSurface eglSurface, int what)
{
    int value[1];
    // EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value
    eglQuerySurface(mEGLDisplay, eglSurface, what, value);
    return value[0];
}

char EGLCore::queryString(int what)
{
    //EGLDisplay dpy, EGLint name
    return  *(eglQueryString(mEGLDisplay, what));
}

void EGLCore::checkEglError(std::string msg)
{
    int error;
    if ((error = eglGetError()) != EGL_SUCCESS)
    {
       // DePrint("%s: EGL error: 0x %d", msg, error);
        return;
    }
}

EGLSurface EGLCore::createWindowSurface(JNIEnv *env, jobject surface)
{
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    DePrint("is here ?");
    int surfaceAttribs[] = {EGL_NONE};
    EGLSurface eglSurface = eglCreateWindowSurface(mEGLDisplay, mEGLConfig, window, surfaceAttribs);
    DePrint("is here ? 2");
    checkEglError("eglCreateWindowSurface");
    DePrint("is here ? 3");
    if (eglSurface == nullptr)
    {
        DePrint("surface was null");
        return nullptr;
    }
    DePrint("is here ? 4 success");
    return eglSurface;
}

void EGLCore::setupSecondSurface(JNIEnv *env, jobject surface) {
    mSecondEGLSurface = createWindowSurface(env, surface);
}



