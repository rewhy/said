package cn.uprogrammer.sensordatacollect;

import android.app.Service;
import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.location.Criteria;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import cn.uprogrammer.sensordatacollect.view.SpeedIndicatorView;
import weka.classifiers.Classifier;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static cn.uprogrammer.sensordatacollect.SMCmd.*;

public class IPSService extends Service {

    static {
        System.loadLibrary("native-lib");
    }

    //test enc
    public static native String enc(String plain);

    String a = "1234567890";
    String content = enc(a);

    static final String TAG = "CANAPP";
    static final String SOCKET_NAME = "canitfsocket";

    static int retries = 0;

    private LocalSocket local_socket = null;
    private OutputStream out_stream = null;
    private InputStream in_stream = null;
    private boolean is_connected = false;
    //private boolean         is_tx_enable = false;
    //private boolean         is_hearbeat_on = false;
    private volatile boolean is_to_exit;


    //private Handler         tx_msg_handler = null;
    private Looper looper;
    private Handler mTxNativeHandler;
    private HeartBeatThread hb_thread = null;
    private MsgReceiveThread rx_msg_thread = null;
    // private MsgTransmitThread   tx_msg_thread  = null;

    private SMCmd cmd = null;

    // read io stream from assets
    public static AssetManager assetManager;

    // parameter for rf vehicle status estimation
    public static Classifier multiClassifier = null;

    //GPS ABOUT
    private String gps_name;
    private FileOutputStream fos = null;
    private LocationManager lm;
    private long gpsTime;

//    public MyThread mt;


    private BroadcastReceiver mMessageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "Received broadcast message");
            if (is_connected == false)
                return;
            int msgId = intent.getIntExtra("ACTION_CONTROL_TYPE", -1);
            switch (msgId) {
                case SMCmd.APP_INTENT_TYPE_TX:
                    byte actionId = intent.getByteExtra("DO_ACTION", (byte) 0);
                    byte[] wrBuff = cmd.getTxBuffer(SMCmd.CMD_FC_ACTION_ID, actionId);
                    Message msg = mTxNativeHandler.obtainMessage(SMCmd.APP_MSG_TYPE_TX, wrBuff);
                    Log.d(TAG, String.format("To do action 0x%x", actionId));
                    if (actionId != 0)
                        mTxNativeHandler.sendMessage(msg);
                    break;
                default:
                    break;
            }
        }
    };

    public IPSService() {
    }

    public void startNativeComService() {

        Log.d(TAG, "Starting CAN-IPS service");
        cmd = new SMCmd();
        is_to_exit = false;

        HandlerThread handlerThread = new HandlerThread("MsgTxHandler", Process.THREAD_PRIORITY_BACKGROUND);
        handlerThread.start();
        looper = handlerThread.getLooper();
        mTxNativeHandler = new MsgTxServiceHandler(looper);

        /* Create and start a new thread for keepping the connection */
        hb_thread = new HeartBeatThread(PORT_NUM);
        new Thread(hb_thread).start();

        /* Create and start a new thread for receiving message */
        rx_msg_thread = new MsgReceiveThread();
        new Thread(rx_msg_thread).start();

        LocalBroadcastManager.getInstance(this).registerReceiver(mMessageReceiver,
                new IntentFilter("MSG_ACTION_CONTROL"));


        //GPS
        lm = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
        onGPS();
    }


    private void onGPS() {
        Criteria criteria = new Criteria();
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(false);
        criteria.setBearingRequired(false);
        criteria.setCostAllowed(true);
        criteria.setPowerRequirement(Criteria.POWER_LOW);

        String bestProvider = lm.getBestProvider(criteria, true);
        //Location lc = lm.getLastKnownLocation(bestProvider);
        Location lc = getBestLocation(lm);

        if (lc == null) {

        }
        updateLocation(lc);
        try {

            lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 500, 0, locationListener);

        } catch (SecurityException e) {
            e.printStackTrace();
        }

    }

    private void closeGPS() {
        lm.removeUpdates(locationListener);
    }

    private Location getBestLocation(LocationManager locationManager) {
        Location result = null;
        if (locationManager != null) {
            if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                // TODO: Consider calling
                //    ActivityCompat#requestPermissions


                return null;
            }
            result = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
            if (result != null) {
                return result;
            } else {
                result = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);

            }

            if (result != null) {
                return result;
            } else {
                result = locationManager.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
            }
        }
        return result;


    }

    //GPS listener
    public LocationListener locationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {

            updateLocation(location);
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {

        }

        @Override
        public void onProviderEnabled(String provider) {
            updateLocation(getBestLocation(lm));
        }

        @Override
        public void onProviderDisabled(String provider) {
            updateLocation(null);
        }
    };

    public void updateLocation(Location location) {
        if (location != null) {
            gpsTime = System.currentTimeMillis();
            String content;
            content = "[GPS@" + gpsTime + "]: " + location.toString();
            Write(content, gps_name);
            Log.d("gpsupdate", "getSpeed: "+location.getSpeed());

            latestGPSSpeed = location.getSpeed();
            Log.d("gpsupdate", "latestGPSSpeed: "+latestGPSSpeed+ " , sucess updated");
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, content);
        Log.d(TAG, "Create canips service");
        StateTables.InitStateTables();
        Window.InitStateRules();
        SMCmd.initiateSMCmd();

        long thisTime = System.currentTimeMillis();
        gps_name = CreatFile.getFileDir() + File.separator + thisTime + "_gps.log";
        CreatFile.createTxt(gps_name);



        assetManager = getAssets();
        loadModel(assetManager);

        /***
         * remeber to delete
         * ***/
//        mt = new MyThread("gpsUpdata", "1571456530647_gps.log", assetManager);
//        mt.start();
//        mt.start();

        if (multiClassifier != null) {
            Log.d("classifier", "c1 successu load");
        }

        LocationManager lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);


        startNativeComService();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "To start CanipsService");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        is_to_exit = true;
        Window.ResetWindow();
        Window.StateRules.clear();
        closeGPS();
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    /**
     * connecto the local socket
     *
     * @param port
     */
    public void connectNative(int port) {
        // LocalSocket s =null;

        LocalSocketAddress lAddr;
        local_socket = new LocalSocket();
        lAddr = new LocalSocketAddress(SOCKET_NAME, LocalSocketAddress.Namespace.RESERVED);

        retries++;
        Log.d(TAG, "Connect InetSocketAddress:" + port + " --retries:" + retries);

        try {
            local_socket.connect(lAddr);
        } catch (IOException e) {
            e.printStackTrace();
            try {
                local_socket.close();
            } catch (IOException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }

            local_socket = null;
            e.printStackTrace();
            return;

        }

        Log.d(TAG, " Try to reset getOutputStream");
        try {
            out_stream = local_socket.getOutputStream();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        Log.d(TAG, " Try to reset getInputStream");
        try {
            in_stream = new BufferedInputStream(local_socket.getInputStream());
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        is_connected = true;
    }

    /**
     * Disconnect from the local socket
     */
    public void disconnect() {

        is_connected = false;

        Log.d(TAG, "1 Disconnection.");

        try {

            if (in_stream != null)
                in_stream.close();

            if (out_stream != null)
                out_stream.close();

            if (local_socket != null)
                local_socket.close();

        } catch (IOException e) {
            // TODO Auto-generated catch block

            e.printStackTrace();
        }

        local_socket = null;
        in_stream = null;
        out_stream = null;

        Log.d(TAG, "2 Disconnection.");
    }

    /**
     * Tx the message to the server
     *
     * @param buff
     * @return
     */
    boolean txNativeMessage(byte[] buff) {
        if (is_connected && (out_stream != null)) {
            try {
                out_stream.write(buff);
                out_stream.flush();
                return true;
            } catch (IOException e) {
                Log.w(TAG, "Transmit message error!!");
                is_connected = false;
            }
        }
        Log.d(TAG, "Tx message: " + buff);
        return false;
    }

    private final class MsgTxServiceHandler extends Handler {

        public MsgTxServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            synchronized (this) {
                if (is_connected == false) {
                    removeMessages(0);
                    return;
                }
                // Processing incoming messages
                switch (msg.what) {
                    case SMCmd.APP_MSG_TYPE_TX:
                        //Log.d(TAG, "Received message (what: APP_MSG_TYPE_TX)");
                        if (txNativeMessage((byte[]) msg.obj) == false) {// Tx Error
                            // TODO: message transmitted error
                            is_connected = false;
                        } else {
                            // TODO
                            // Log.d(TAG, "Tx messagess");
                        }
                        break;
                    default:
                        break;
                }
            }
            //stops the service for the start id.
            stopSelfResult(msg.arg1);
        }
    }

    public class MsgBinder extends Binder {
        public IPSService getService() {
            return IPSService.this;
        }
    }

    /* Thread for transmitting the heartbeat messages */
    class HeartBeatThread implements Runnable {
        private int port;

        public HeartBeatThread(int port) {
            this.port = port;
        }

        public void run() {
            try {
                Log.d(TAG, "Try new socket connect: " + port);

                connectNative(port);

                while (is_to_exit == false) {

                    /* If the socket connection stopped, try to connect again */
                    while ((is_connected == false) && (is_to_exit == false)) {
                        disconnect();
                        Thread.sleep(1000);
                        Log.d(TAG, "Reconnect");
                        connectNative(port);
                    }

                    if ((is_connected == true) && (is_to_exit == false)) {
                        /* Get the heartbeat packet data buffer */
                        byte[] wrBuff = cmd.getTxHeartbeat();

                        /* Transmit heartbeat packet periodically */
                        // Log.d(TAG, "Heart beat");

                        Message msg = mTxNativeHandler.obtainMessage(SMCmd.APP_MSG_TYPE_TX, wrBuff);
                        mTxNativeHandler.sendMessage(msg);

                        Thread.sleep(1000);
                    }
                }

                disconnect();

            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }

    /* Thread for receiving the messages from CANIPS server */
    class MsgReceiveThread implements Runnable {

        byte[] buff = new byte[FRAME_DATA_MAX];
        byte offset = 0;
        private int totalDataLength = 0;

        /* Reset the Rx data buffer and its length */
        void doResetBuffer(byte[] buff) {
            for (int i = 0; i < buff.length; i++) {
                buff[i] = 0;
            }

            totalDataLength = 0;
        }

        /* To read data from Rx buffer */
        private void doReadBuffer() {
            int r = -1;

            try {
                /* Iteratively read data from the input stream */
                while (((r = in_stream.read())) != -1) {
                    // Log.d(TAG, "Read byte: " + Integer.toHexString(r));

                    if (offset >= buff.length) { // Ring buffer?
                        offset = 0;
                        doResetBuffer(buff);
                    }

                    buff[offset++] = Byte.valueOf((byte) r);

                    switch (offset) {
                        case 1:
                            if (buff[0] != SMCmd.START1) { // 0xFF Not the first byte
                                offset = 0;
                                doResetBuffer(buff);
                            }
                            break;
                        case 2:
                            if (buff[1] != SMCmd.START2) { // 0xFE not the second buyte
                                if (buff[1] == SMCmd.START1) { // 0xFF the first byte
                                    buff[0] = buff[1];
                                    buff[1] = 0;
                                    offset = 1;
                                } else {
                                    offset = 0;
                                    doResetBuffer(buff);
                                }
                            }
                            break;
                        case 3:
                            break;
                        case 4:
                            totalDataLength = buff[3] + 2; // Message Length
                            break;
                        default:
                            if (offset == totalDataLength + 4) {
                                if ((buff[offset - 2] == SMCmd.END1) && (buff[offset - 1] == SMCmd.END2)) { // 0xFE 0xFF End of the message
                                    byte[] msg_data = cmd.getMsgPayload(buff, 2, (totalDataLength));

                                    if (msg_data != null) {
                                        // Log.d(TAG, String.format("To process %d bytes", msg_data.length));
                                        cmd.processMsg(msg_data, multiClassifier);
                                    }
                                }
                                offset = 0;
                                doResetBuffer(buff);
                            }
                            break;
                    }

                    if (is_to_exit) {
                        break;
                    }
                }
            } catch (IOException e) {
                // TODO Auto-generated catch block
                Log.d(TAG, "Close in buffer");
                is_connected = false;
            }
        }

        @Override
        public void run() {
            while (is_to_exit == false) {
                if (is_connected) {
                    /* Read data received from the native service */
                    doReadBuffer();
                }
            }
            Log.d(TAG, "To leave the Rx thread.");
        }
    }

    public static void loadModel(AssetManager assetManager) {

        try {
//            model_c6_1109.model
            multiClassifier = (Classifier) weka.core.SerializationHelper.read(assetManager.open("model_c7_1216.model"));

//            multiClassifier = (Classifier) weka.core.SerializationHelper.read(assetManager.open("c5_th2.model"));
//            mClassifier1 = (Classifier) weka.core.SerializationHelper.read(assetManager.open( "rf_c2_f35_noaccX.model"));

            if (multiClassifier != null) {
                Log.d("classifier", "c1 success");
            }
//

        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            // Weka "catch'em all!"
            e.printStackTrace();
        }

    }

    public List getLocalGPSData(AssetManager assetManager, String filename) {
        Log.d("gpsFile", ": "+filename);
        List gpsData = new ArrayList();
        InputStream inputStream = null;
        InputStreamReader isr = null;
        BufferedReader br = null;
        StringBuffer sb = new StringBuffer();

        try {
            inputStream = assetManager.open(filename);
            isr = new InputStreamReader(inputStream);
            br = new BufferedReader(isr);


            String line = null;
            while ((line = br.readLine()) != null) {
                sb.append("\n" + line);
                String[] temp1 = line.split(" ");
                if (temp1.length < 8) {
                    continue;
                }

                String[] temp2 = temp1[0].split("@");
                String[] temp3 = temp2[1].split("]");
                String timeStamp = temp3[0];
                String speed = temp1[6].split("=")[1];
                sb.append("\n" + line);
                Log.d("gpsFile", "speed: "+speed + "time:  "+timeStamp);

                String [] t = new String[2];
                t[0] = speed;
                t[1] = timeStamp;
                gpsData.add(t);
                Log.d("gpsFile", "speed: "+speed + "time:  "+timeStamp);


//                System.out.printf("time: %s, speed: %s \n", timeStamp, speed);
            }
            Log.d("gpsFile", "gpsData: "+gpsData.size());


            br.close();
            isr.close();
            inputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
                if (isr != null) {
                    isr.close();
                }
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return gpsData;

    }

    private void Write(String data, String Path) {
        String content = data;
        File file = new File(Path);


        try {
            fos = new FileOutputStream(file, true);
            fos.write(content.getBytes());
            fos.write("\r\n".getBytes());
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

    }


}
