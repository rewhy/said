package hk.polyu.canitf;

import android.os.Handler;

public interface SMInterface {
	public void heartBeatEnable();

	public Handler getHandler();
	public void setHandler(Handler mHandler);

	public void registerApp(byte appId);
}
