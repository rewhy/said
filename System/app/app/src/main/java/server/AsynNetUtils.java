package server;

import android.os.Handler;


public class AsynNetUtils {

    public interface Callback{
        void onResponse(String response);
    }



    public static void post(final String url, final String content, final Callback callback){
        final Handler handler = new Handler();
        new Thread(new Runnable() {
            @Override
            public void run() {
                final String response = OkHttpUtil.post(url,content);
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        callback.onResponse(response);
                    }
                });
            }
        }).start();
    }

}
