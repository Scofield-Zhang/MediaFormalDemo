//
// Created by 张涛 on 2017/5/2.
//

#ifndef MULTIMEDIADEMO_EGLCORE_H
#define MULTIMEDIADEMO_EGLCORE_H

#include <EGL/egl.h>
#include <jni.h>
#include <string>

enum class EGLCoreSurfaceOrder{
    EGLCoreSurfaceOrderFirst,
    EGLCoreSurfaceOrderSecond
};

class EGLCore {
public:
    EGLCore(JNIEnv* env, jobject surface, EGLContext sharedContext = nullptr, bool recordable = true);
    EGLCore(int width, int height, EGLContext sharedContext = nullptr, bool recordable = true);
    virtual ~EGLCore();
    void makeCurrent(EGLCoreSurfaceOrder order = EGLCoreSurfaceOrder::EGLCoreSurfaceOrderFirst);
    void makeNothingCurrent();
    EGLBoolean swapBuffers(EGLCoreSurfaceOrder order = EGLCoreSurfaceOrder::EGLCoreSurfaceOrderFirst);
    void setupSecondSurface(JNIEnv* env,jobject surface);

    EGLSurface createWindowSurface(JNIEnv* env,jobject surface);
    void bindCurrent(EGLSurface surface);
    EGLDisplay mEGLDisplay ;
private:

    EGLContext mEGLContext ;
    EGLConfig mEGLConfig ;
    EGLSurface mEGLSurface;
    EGLSurface mSecondEGLSurface;
    int mGlVersion ;
    EGLConfig getConfig(bool recordable, int version);
    void createEGLContext(EGLContext sharedContext, bool recordable);

    bool isCurrent(EGLSurface eglSurface);
    void checkEglError(std::string msg);
    int querySurface(EGLSurface eglSurface, int what);
    char queryString(int what);

    void releaseSurface(EGLSurface eglSurface);

    EGLSurface createOffscreenSurface(int width, int height);
protected:

};


#endif //MULTIMEDIADEMO_EGLCORE_H
