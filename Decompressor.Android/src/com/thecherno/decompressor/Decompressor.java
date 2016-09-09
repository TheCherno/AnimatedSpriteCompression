package com.thecherno.decompressor;

import android.app.Activity;
import android.widget.TextView;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.content.Intent;
import android.util.Log;
import java.net.*;
import java.io.*;

public class Decompressor extends Activity
{
    static {
        System.loadLibrary("Decompressor.Android");
    }
	
	private final static String TAG = "Decompressor";
	private TextView[] m_TextViews;

	@Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
		setContentView(R.layout.main_layout);

		m_TextViews = new TextView[] {
			(TextView)findViewById(R.id.labelTitle01),
			(TextView)findViewById(R.id.labelSubtitle01),
			(TextView)findViewById(R.id.labelResult01),
			(TextView)findViewById(R.id.labelTitle02),
			(TextView)findViewById(R.id.labelSubtitle02),
			(TextView)findViewById(R.id.labelResult02)
		};

		Run();	
    }

	private String FormatData(float[] data)
	{
		String result = Build.MODEL + "\n";
		for (int i = 0; i < data.length; i += 2)
		{
			result += i + ": " + String.format("%.4f", data[i + 1]) + "," + BytesToString((long)data[i + 0]) + "\n";
		}
		return result;
	}

	private void SendData(final String destination, final float[] data)
	{
		new Thread(new Runnable()
		{
			public void run()
			{
				String string = FormatData(data);
				byte[] buffer = string.getBytes();
				try
				{
					URL url	= new URL(destination);
					HttpURLConnection connection = (HttpURLConnection)url.openConnection();
					connection.setConnectTimeout(30000);
					connection.setReadTimeout(60000);
					connection.setUseCaches(false);
					connection.setDoInput(true);

					connection.setRequestMethod("POST");
					connection.setRequestProperty("Content-Type", "text/plain"); 
					connection.setRequestProperty("Content-Length", Integer.toString(buffer.length));

					OutputStream out = connection.getOutputStream();
					out.write(buffer);
					out.flush();
					out.close();

					InputStream in = connection.getInputStream();
					int status = connection.getResponseCode();
					Log.w(TAG, "HTTP status = " + status);
				}
				catch (IOException e)
				{
					Log.e(TAG, Log.getStackTraceString(e)); 
				}
			}
		}).start();
	}

	private void Run()
	{
		// Structure:
		//   - filesize
		//   - time
        float[] data = RunDecompression();
		SendData("http://10.88.40.104/dump.php", data);

		m_TextViews[0].setText("animation.bin");
		m_TextViews[1].setText((int)data[0] + " bytes");
		m_TextViews[2].setText(data[1] + " ms");

		m_TextViews[3].setText("animation2.bin");
		m_TextViews[4].setText((int)data[2] + " bytes");
		String result = "";
		for (int i = 0; i < data.length && i < 12; i += 2)
			result += String.format("%.4f", data[i + 1]) + " ms (" + BytesToString((long)data[i + 0]) + ")\n";
		m_TextViews[5].setText(result);
	}

	public void OnRefreshButtonClick(View v)
	{
		Run();
	}

	public void OnViewButtonClick(View v)
	{
		Intent display = new Intent(getApplicationContext(), DisplayActivity.class);
		startActivity(display);
	}

    public native float[] RunDecompression();

	private String BytesToString(long bytes)
	{
	    final float gb = 1024 * 1024 * 1024;
	    final float mb = 1024 * 1024;
	    final float kb = 1024;
	    
	    String result;
	    if (bytes > gb)
	        result = String.format("%.2f GB", bytes / gb);
	    else if (bytes > mb)
	        result = String.format("%.2f MB", bytes / mb);
	    else if (bytes > kb)
	        result = String.format("%.2f KB", bytes / kb);
	    else
	        result = String.format("%d bytes", (int)bytes);
	    
	    return result;
	}
  
}