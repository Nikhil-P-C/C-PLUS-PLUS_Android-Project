package org.myapp;

import androidx.appcompat.app.AppCompatActivity;

import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import org.libsdl.app.SDLActivity;

import java.lang.reflect.Field;


public class mySDLActivity extends SDLActivity {

    static {
        // Force SDLActivity to load first

        // Load your native library
        System.loadLibrary("SDL3");
        System.loadLibrary("main");
    }


    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        // Force SDL to resize window if orientation changes
        onNativeResize();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        try {
            Field surfaceField = SDLActivity.class.getDeclaredField("mSurface"); // internal SurfaceView
            surfaceField.setAccessible(true);
            SurfaceView sdlView = (SurfaceView) surfaceField.get(this);
            if (sdlView != null) {
                sdlView.setRotation(0);  // force landscape
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


}