package hk.polyu.serverconnection;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.google.gson.Gson;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;

import hk.polyu.canitf.ICanItfService;
import hk.polyu.util.SHAEncoding;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private TextView httpAddress;
    private TextView contentData;
    private Button sendButton;
    private TextView resultView;
    private String ip;

    private Button setAddButton;

    private ICanItfService mService;

    Context mContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mContext = getApplicationContext();
        bindService();

        // initialize
        httpAddress = findViewById(R.id.httpAdd);
        contentData = findViewById(R.id.data);
        sendButton = findViewById(R.id.send);
        resultView =  findViewById(R.id.result);

        setAddButton = findViewById(R.id.setAdd);
        initCallBacks();

    }

    protected void initCallBacks(){
        // get server ip and client

        ip = httpAddress.getText().toString();// by default https://175.159.18.211:25001


        // add send data listener
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String content = contentData.getText().toString().trim();
                final String SHA_CONENT = SHAEncoding.encodingwithSHA256(content);
                System.out.println("Send SHA256:"+SHA_CONENT);
                AsynNetUtils.post(ip,content,new AsynNetUtils.Callback(){
                    @Override
                    public void onResponse(String response) {
                        System.out.println("Receive SHA256:"+response);
                        if(SHA_CONENT.equals(response)) {
                            resultView.setText("Message received and verified!");

                        }else{
                            resultView.setText("Message received but not verified!");
                        }
                    }
                });
            }
        });

        setAddButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                System.out.println("setAddButton setOnClickListener ");
                ip = httpAddress.getText().toString();

                if(mService != null){
                    try{
                        mService.setServerAdd(ip);
                        Log.d(TAG, "setAddButton" + ip);
                    }catch (Exception e) {
                        // TODO: handle exception
                    }
                }
            }
        });
    }

    @Override
    protected void onStop() {
        super.onStop();
        unBindService();
    }

    private void bindService() {
        Intent intent = new Intent(ICanItfService.class.getName());
        intent.setPackage("hk.polyu.canitf");
        mContext.bindService(intent, mServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    private void unBindService() {
        if (mService != null) {
            mContext.unbindService(mServiceConnection);
        }
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            mService = ICanItfService.Stub.asInterface(arg1);
            Log.d(TAG, "onServiceConnected");


        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            Log.d(TAG, "onServiceDisconnected");
            mService = null;
        }

    };
}
