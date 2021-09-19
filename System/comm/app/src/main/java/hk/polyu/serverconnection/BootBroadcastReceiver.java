package hk.polyu.serverconnection;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;


public class BootBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		// TODO Auto-generated method stub
		String action = intent.getAction();

		if (action.equals("android.intent.action.BOOT_COMPLETED")) {
			Intent service = new Intent(context, hk.polyu.canitf.CanItfService.class);
			context.startService(service);

		}

	}
}
