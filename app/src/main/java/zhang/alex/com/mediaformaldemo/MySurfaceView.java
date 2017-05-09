package zhang.alex.com.mediaformaldemo;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

import zhang.alex.com.mediaformaldemo.encoder.MediaAudioEncoder;
import zhang.alex.com.mediaformaldemo.encoder.MediaEncoder;
import zhang.alex.com.mediaformaldemo.encoder.MediaMuxerWrapper;
import zhang.alex.com.mediaformaldemo.encoder.MediaVideoEncoder;
import zhang.alex.com.mediaformaldemo.utils.CameraUtils;

import static android.content.ContentValues.TAG;

/**
 *
 * Created by 张涛 on 2017/5/9.
 */

public class MySurfaceView extends SurfaceView implements SurfaceHolder.Callback,SurfaceTexture.OnFrameAvailableListener,MediaEncoder.MediaEncoderListener{
    static{
        System.loadLibrary("native-lib");
    }

    public static final int VIDEO_WIDTH = 720;
    public static final int VIDEO_HEIGHT = 1280;

    private SurfaceTexture cameraSurfaceTexture;
    private Camera mCamera;
    private int mCameraPreviewThousandFps;
    private MediaVideoEncoder videoEncoder;
    private MediaMuxerWrapper mMuxer ;

    public native void createEGLContextFromJNI(Surface surface) ;
    public native int createTextureIdFromJNI();
    private native void createEGLSurfaceFromJNI(Surface surface) ;
    public native void setGLViewPort(int width, int height) ;
    public native void makeCurrentSurfaceView();
    public native void drawTexture();
    public native void swapBuffer();
    public native void setPresentationTime(long presentationTime);
    public native void makeCurrentGLSurface();
    public native void swapBufferGLSurface();

    public MySurfaceView(Context context) {
        this(context,null);
    }

    public native void createWindowsSurfaceFromJNI(Surface surface);


    public MySurfaceView(Context context, AttributeSet attrs) {this(context, attrs,0);}


    public MySurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        getHolder().addCallback(this);

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        createEGLContextFromJNI(holder.getSurface());
        int textureId = createTextureIdFromJNI();
        Log.d("Demon_jni", "surfaceCreated: "+textureId);
        cameraSurfaceTexture = new SurfaceTexture(textureId);
        Log.d("Demon_jni", "cameraSurfaceTexture: ");
        cameraSurfaceTexture.setOnFrameAvailableListener(this);
        try {
            mCamera.setPreviewTexture(cameraSurfaceTexture);
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            mMuxer = new MediaMuxerWrapper(".mp4");
        } catch (IOException e) {
            e.printStackTrace();
        }
        // TODO 这里 为null
        videoEncoder = new MediaVideoEncoder(mMuxer,this,VIDEO_WIDTH,VIDEO_HEIGHT);
       // createWindowsSurfaceFromJNI(videoEncoder.getSurface());
        Log.d("Demon_jni", "videoEncoder: ");
        mCamera.startPreview();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setGLViewPort(width, height);
    }


    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        makeCurrentSurfaceView();
        cameraSurfaceTexture.updateTexImage();
        Log.d("Demon_jni", "onFrameAvailable: 更新");
        drawTexture();
        swapBuffer();
        makeCurrentGLSurface();
        // TODO
         videoEncoder.frameAvailableSoon();
        setPresentationTime(cameraSurfaceTexture.getTimestamp());
        swapBufferGLSurface();
        Log.d("onFrameAvailable", "onFrameAvailable: ");
    }

    public void openCamera(int desiredWidth, int desiredHeight, int desiredFps) {
        if (mCamera != null) {
            throw new RuntimeException("camera already initialized");
        }
        Camera.CameraInfo info = new Camera.CameraInfo();
        int numCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numCameras; i++) {
            Camera.getCameraInfo(i, info);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                mCamera = Camera.open(i);
                break;
            }
        }
        if (mCamera == null) {
            mCamera = Camera.open();
        }
        if (mCamera == null) {
            throw new RuntimeException("Unable to open camera");
        }
        Camera.Parameters parms = mCamera.getParameters();
        CameraUtils.choosePreviewSize(parms, desiredWidth, desiredHeight);
        mCameraPreviewThousandFps = CameraUtils.chooseFixedPreviewFps(parms, desiredFps * 1000);
        parms.setRecordingHint(true);
        mCamera.setParameters(parms);
        Camera.Size cameraPreviewSize = parms.getPreviewSize();
        String previewFacts = cameraPreviewSize.width + "x" + cameraPreviewSize.height +
                " @" + (mCameraPreviewThousandFps / 1000.0f) + "fps";
        Log.i(TAG, "Camera config: " + previewFacts);
    }

    /**
     * 释放相机
     */
    public void releaseCamera() {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.release();
            mCamera = null;
            Log.d(TAG, "releaseCamera -- done");
        }
    }

    public void startCamera() {
        openCamera(VIDEO_WIDTH, VIDEO_HEIGHT, 30);
    }

    @Override
    public void onPrepared(MediaEncoder encoder) {
        if (encoder instanceof MediaVideoEncoder){
            videoEncoder = (MediaVideoEncoder) encoder;
            Log.d("Demon_jni", "执行到这里吗: ");

            Log.d("Demon_jni", "执行到这里: ");

        }
    }

    @Override
    public void onStopped(MediaEncoder encoder) {

    }
    public void startRecording() {
        try {
//            mMuxer = new MediaMuxerWrapper(".mp4");
//            if (true) {
//                new MediaVideoEncoder(mMuxer, this, MySurfaceView.VIDEO_WIDTH, MySurfaceView.VIDEO_HEIGHT);
//            }
            if (true) {
                new MediaAudioEncoder(mMuxer, this);
            }
            mMuxer.prepare();
            mMuxer.startRecording();
        } catch (final IOException e) {

        }
    }
    public void stopRecording() {
        if (mMuxer != null) {
            mMuxer.stopRecording();
            mMuxer = null;
            // you should not wait here
        }
    }
}
