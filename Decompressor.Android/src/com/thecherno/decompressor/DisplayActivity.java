package com.thecherno.decompressor;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;
import android.view.View;
import android.content.Context;
import android.opengl.*;

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;

public class DisplayActivity extends Activity implements Runnable
{
	
	public class DisplayRenderer implements GLSurfaceView.Renderer
	{
		private native void OnInit();
		private native void OnSurfaceChanged(int width, int height);
		private native void OnDraw();

		public void onSurfaceCreated(GL10 unused, EGLConfig config)
		{
			OnInit();
		}

		public void onDrawFrame(GL10 unused)
		{
			OnDraw();
		}

		public void onSurfaceChanged(GL10 unused, int width, int height)
		{
			OnSurfaceChanged(width, height);
		}
	}

	public class DecompressorSurface extends GLSurfaceView
	{
		private DisplayRenderer m_Renderer;

		public DecompressorSurface(Context context)
		{
			super(context);

			// Create an OpenGL ES 2.0 context
			setEGLContextClientVersion(2);

			m_Renderer = new DisplayRenderer();
			setRenderer(m_Renderer);
		}
	}


	private GLSurfaceView m_SurfaceView;

	@Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		m_SurfaceView = new DecompressorSurface(this);
		setContentView(m_SurfaceView);

		new Thread(this).start();
    }

	public void run()
	{
		
	}


  
}