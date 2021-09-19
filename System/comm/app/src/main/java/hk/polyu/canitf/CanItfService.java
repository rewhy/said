package hk.polyu.canitf;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import java.util.Timer;
import java.util.TimerTask;

import hk.polyu.serverconnection.AsynNetUtils;
import hk.polyu.util.SHAEncoding;


public class CanItfService extends Service implements SensorEventListener {
    private static final String VERSION = "1.0.0";
    private static final String TAG = "canitfservice";
    private SMService mSmService;
    private static CanItfService mInstance;
    private static Context context;

    private Timer mTimer;

    private SensorManager mSensorManager;
    private Sensor gsensor;
    private float mLastX, mLastY, mLastZ;
    private String sX, sY, sZ;
    private String sensorString = "02,";
    private String ip = "https://158.132.11.160:25001";
    private float sensorReportValue = (float) 0.2;

    @Override
    public IBinder onBind(Intent arg0) {
        Log.d(TAG, "onBind: ");
        return mBinder;
    }

    public static CanItfService getInstance() {
        return mInstance;
    }

    public void onCreate() {
        super.onCreate();
        mInstance = this;

        Log.d(TAG, "onCreate: ");
        Log.d(TAG, VERSION);

        context = getApplicationContext();


        mSmService = new SMService(mHandler);
        mSmService.heartBeatEnable();

        mTimer = new Timer();
        setTimerTask();


        //获取Sensor服务
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        //获取gsensor的对象
        gsensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        //注册数据监听器，当有数据时会回调onSensorChanged方法
        mSensorManager.registerListener((SensorEventListener) this, gsensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    public void onDestroy() {
        mHandler.removeMessages(0);

        mSmService.close();
        super.onDestroy();

        mTimer.cancel();

        if (mSensorManager != null) {
            mSensorManager.unregisterListener(this);
            mSensorManager = null;
        }
    }


    private ICanItfService.Stub mBinder = new ICanItfService.Stub() {

        @Override
        public void setServerAdd(String add) throws RemoteException {
            ip = add;
            Log.d(TAG, "setServerAdd: " + add);
        }
    };


    void processRxData(byte[] buff) {
        int validLength = buff.length - 2;
        byte[] validData = new byte[validLength];
        System.arraycopy(buff, 3, validData, 1, validLength - 1);
        validData[0] = 0x01;    //0x01 is can data.

        String content = Utils.getByteStringHex(validData);
        postContent(content);

        reportGsensorData();
    }

    void postContent(String content) {
        Log.d(TAG, "Post: " + content);


        final String SHA_CONENT = SHAEncoding.encodingwithSHA256(content);
        System.out.println("Send SHA256:" + SHA_CONENT);
        AsynNetUtils.post(ip, content, new AsynNetUtils.Callback() {
            @Override
            public void onResponse(String response) {
                System.out.println("Receive SHA256:" + response);
                if (SHA_CONENT.equals(response)) {
                    Log.d(TAG, "Message received and verified!");
                } else {
                    Log.d(TAG, "Message received but not verified!");
                }
            }
        });
    }

    void reportGsensorData() {
        //0x02 is sensor data.
        postContent(sensorString);
    }

    @SuppressLint("HandlerLeak")
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case SMCmd.TIMER_1_SEC:
                    //reportGsensorData();
                    break;
                case SMCmd.RX_MSG: {
                    processRxData((byte[]) msg.obj);
                }
                break;
                case SMCmd.SOCKET_CONNECT_SUCCESS: {

                    Log.d(TAG, "SOCKET_CONNECT_SUCCESS");
                }
                break;
                case SMCmd.AUTH_FAILED: {
                }
                break;
                default:
                    break;
            }

            super.handleMessage(msg);
        }
    };

    private void setTimerTask() {
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                Message message = new Message();
                mHandler.sendEmptyMessage(SMCmd.TIMER_1_SEC);
            }
        }, 2000, 1000);
    }


    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor == null)
            return;

        //Log.d(TAG, "onSensorChanged" + event.sensor.getName());

        //判断获取的数据类型是不是gsensor
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            //获得数据为float类型的数据
            float x, y, z;
            boolean report = false;

            x = event.values[0];
            y = event.values[1];
            z = event.values[2];

            if ((Math.abs(x - mLastX) > sensorReportValue ||
                    Math.abs(y - mLastY) > sensorReportValue ||
                    Math.abs(z - mLastZ) > sensorReportValue)) {
                report = true;
            }

            mLastX = x;
            mLastY = y;
            mLastZ = z;


            //将float类型的数据转为字符型供textView显示
            sX = String.valueOf(mLastX);
            sY = String.valueOf(mLastY);
            sZ = String.valueOf(mLastZ);

            sensorString = "02," + sX + "," + sY + "," + sZ;

            //Log.d(TAG, sensorString);

            if(report){
                reportGsensorData();
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {
        //Log.d(TAG, "onAccuracyChanged" + sensor.getName());
    }

}
