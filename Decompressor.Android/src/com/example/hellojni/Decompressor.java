package com.thecherno.decompressor;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;
import android.view.View;

public class Decompressor extends Activity
{
    static {
        System.loadLibrary("Decompressor.Android");
    }
	
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

	private void Run()
	{
		// Structure:
		//   - filesize
		//   - time
        float[] data = RunDecompression();

		m_TextViews[0].setText("animation.bin");
		m_TextViews[1].setText((int)data[0] + " bytes");
		m_TextViews[2].setText(data[1] + " ms");

		m_TextViews[3].setText("animation2.bin");
		m_TextViews[4].setText((int)data[2] + " bytes");
		String result = "";
		for (int i = 0; i < data.length; i += 2)
			result += String.format("%.4f", data[i + 1]) + " ms (" + BytesToString((long)data[i + 0]) + ")\n";
		m_TextViews[5].setText(result);

	}

	public void OnRefreshButtonClick(View v)
	{
		Run();
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