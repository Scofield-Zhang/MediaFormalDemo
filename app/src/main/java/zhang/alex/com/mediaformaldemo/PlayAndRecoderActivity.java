package zhang.alex.com.mediaformaldemo;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class PlayAndRecoderActivity extends AppCompatActivity implements View.OnClickListener {

    private Button btnRecoder;
    private boolean isRecording;
    private MySurfaceView mySurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play_and_recoder);
        initView();
    }

    private void initView() {
        btnRecoder = (Button) findViewById(R.id.recoder);
        mySurfaceView = (MySurfaceView) findViewById(R.id.msf);
        btnRecoder.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.recoder:
                if (isRecording) {
                    mySurfaceView.startRecording();
                }else {
                    mySurfaceView.stopRecording();
                }
                updateControl();
                break;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mySurfaceView.startCamera();
        mySurfaceView.setVisibility(View.VISIBLE);
    }



    @Override
    protected void onPause() {
        mySurfaceView.releaseCamera();
        mySurfaceView.setVisibility(View.GONE);
        mySurfaceView.stopRecording();
        super.onPause();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        mySurfaceView.releaseCamera();
    }

    private void updateControl() {
        btnRecoder.setText(isRecording?R.string.startRecoder:R.string.stopRecoder);
        isRecording = !isRecording;
        Log.d("isRecoding", "updateControl: "+isRecording);
    }

//    private void startRecording() {
//        isRecording = false;
//        try {
//            // turn red
//            mMuxer = new MediaMuxerWrapper(".mp4");	// if you record audio only, ".m4a" is also OK.
//            if (true) {
//                new MediaVideoEncoder(mMuxer, mMediaEncoderListener, MySurfaceView.VIDEO_WIDTH, MySurfaceView.VIDEO_HEIGHT);
//            }
//            if (true) {
//                new MediaAudioEncoder(mMuxer, mMediaEncoderListener);
//            }
//            mMuxer.prepare();
//            mMuxer.startRecording();
//        } catch (final IOException e) {
//
//        }
//    }
//    private void stopRecording() {
//        isRecording = true;
//        if (mMuxer != null) {
//            mMuxer.stopRecording();
//            mMuxer = null;
//            // you should not wait here
//        }
//    }
//    private final MediaEncoder.MediaEncoderListener mMediaEncoderListener = new MediaEncoder.MediaEncoderListener() {
//        @Override
//        public void onPrepared(final MediaEncoder encoder) {
//            if (encoder instanceof MediaVideoEncoder){}
//               // mCameraView.setVideoEncoder((MediaVideoEncoder)encoder);
//            //mySurfaceView.setVideoRecoder((MediaVideoEncoder)encoder);
//    }
//
//        @Override
//        public void onStopped(final MediaEncoder encoder) {
//            if (encoder instanceof MediaVideoEncoder){}
//                // mCameraView.setVideoEncoder(null);
//        }
//    };


}
