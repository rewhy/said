package cn.uprogrammer.sensordatacollect_udp2;


import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import jxl.Sheet;
import jxl.Workbook;
import jxl.write.WritableSheet;
import jxl.write.WritableWorkbook;

import static java.lang.Thread.sleep;


public class MainActivity extends Activity //implements SensorEventListener 最初是SensorListener
{
    private SensorManager mSensorManager;
    private String excelPath;//文件存储路径
    private String sensor_txtPath;
    private String txtPath;
    private FileOutputStream fos = null;

    private LocationManager lm;
    private String tag = "log";
    //private TextView etGps;
    private PowerManager.WakeLock mWakeLock;

    Timer timer = null;
    Timer timer0 = null;
    Timer timer1 = null;
    TextView etGyro;
    TextView etMagnetic;
    TextView etLinearAcc;
    TextView etOrientation; // 方向传感器 -zkf
    TextView etTimeGap;
    // etWave, etSwer, etSidesl, etFastU, etTwr, etSuddenB;


    int andbCount = 1;


    EditText inputfilename;//文件名输入框
    Button start;
    Button stop;
    Button bt_calibration;

    float flag = 0;

    ArrayList<Float> AccList = new ArrayList<Float>();
    ArrayList<Float> GyrList = new ArrayList<Float>();
    ArrayList<Float> MagList = new ArrayList<Float>();
    ArrayList<Float> OriList = new ArrayList<Float>();// 方向传感器 -zkf
    ArrayList<Float> TimeList = new ArrayList<Float>();// 记录时间-z

    ArrayList<Float> AyList = new ArrayList<>();
    ArrayList<Float> AzList = new ArrayList<>();

    ArrayList Sensor0 = new ArrayList();
    ArrayList Sensor1 = new ArrayList();
    ArrayList Sensor2 = new ArrayList();
    ArrayList Sensor3 = new ArrayList();
    ArrayList Sensor4 = new ArrayList();
    ArrayList Sensor5 = new ArrayList();


    private int countWriteTime = 0;
    private float AccData[] = new float[3];
    private float GyrData[] = new float[3];
    private float MagData[] = new float[3];
    private float LAccData[] = new float[3];
    private float OriData[] = new float[3]; // 方向传感器 -zkf
    private float timeGapData; // 用于记录时间——zkf
    private float timeGpsData;
    private float r_ori[] = new float[9];
    private float outR[] = new float[9];

    // 记录时间——zkf
    private Date curDate = new Date(System.currentTimeMillis());
    private Date endDate = new Date(System.currentTimeMillis());
    //private Date gpsDate = new Date(System.currentTimeMillis());

    private Workbook wb;
    private WritableWorkbook wbook;//需要导入jxl工程或者包
    private WritableSheet sh;
    private Sheet sheet;

    CreateXls data_XLS = new CreateXls();//需要导入工程或者jxl包*/
    CreatFile data_txt = new CreatFile();

    //Network
    //private static final String HOST = "10.42.0.1";
    private static final String HOST = "192.168.12.1";
    private static final int PORT = 9999;
    private DatagramSocket socket = null;
    private BufferedReader in = null;
    private PrintWriter out = null;
    private double ltr = 0;
    private int ltr_flag = 0;
    private boolean defence = false;
    private InetAddress address;

    //校准
    private boolean flag_calibration = false;
    public static SensorCalibration SC;
    private ArrayList Sensor_List = new ArrayList();
    private Handler mHandler = null;
    private double [][] sensor_show = new double[7][1];
    //StringBuilder sb = new StringBuilder();
    Thread udp1;
    Thread udp2;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        start = (Button) findViewById(R.id.bn1);
        stop = (Button) findViewById(R.id.bn2);
        bt_calibration = (Button) findViewById(R.id.calibration) ;
        try {
            address = InetAddress.getByName(HOST);
        } catch (Exception e){
            e.printStackTrace();
        }

        // 输出到界面
        etGyro = (TextView) findViewById(R.id.etGyro);
        etLinearAcc = (TextView) findViewById(R.id.etLinearAcc);
        etTimeGap = (TextView) findViewById(R.id.etTimeGap); //记录时间
        inputfilename = (EditText) findViewById(R.id.filename);

        //校准
        SC = new SensorCalibration();


        //根据返回的结果，判断对应的权限是否有。
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},0);
        }

        // 启动sensor
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        start.setOnClickListener(myListner);//为开始按钮和停止按钮添加监听器
        stop.setOnClickListener(myListner);
        bt_calibration.setOnClickListener(myListner);
        stop.setEnabled(false);

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MyGps:tagforLock");
        mWakeLock.acquire();

        mHandler = new Handler(){

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case 0:
                        double [][] currentSensorData = SensorCalibration.convertSensorData(Sensor_List);
                        SC.getRotationMatrix(currentSensorData);
                        flag_calibration = true;
                        Toast.makeText(MainActivity.this, "校准成功！", Toast.LENGTH_SHORT).show();
                        bt_calibration.setEnabled(true);
                        inputfilename.setEnabled(true);
                        start.setEnabled(true);
                        stop.setEnabled(false);
                        StopSensorListening();//停止传感器采集


                        try {
                            socket.close();
                            //socket = null;
                        } catch (Exception e) {
                            e.printStackTrace();
                        }

                        timer1 = null;
                        break;
                    default:
                        break;
                }
            }
        };



    }

    private float average(ArrayList list0){
        float sum = 0;
        for(int i=0;i < list0.size(); i++){
            sum = sum + (float)list0.get(i);
        }
        float avg = sum / list0.size();
        return avg;
    }


    private void acceptServer(){
        try {
            socket = new DatagramSocket();
        }catch (Exception e){
            e.printStackTrace();
        }
        /*
        in = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
        out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())), true);*/
    }

    private void addAverageList(ArrayList list0, float data,int size){
        if (list0.size() < size){
            list0.add(data);
        }else{
            list0.remove(0);
            list0.add(data);
        }
    }
    //开始和结束按钮监听器的外部实现
    private Button.OnClickListener myListner = new Button.OnClickListener() {

        @Override
        public void onClick(View v) {

            timer0 = new Timer();
            timer1 = new Timer();
            TimerTask task0 = new TimerTask() {
                @Override
                public void run() {
                    byte[] data0 = "3".getBytes();
                    DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                    try {
                        if(socket.isClosed()) {
                            Log.d("LYY1", "The socket is closed");
                        }else {
                            socket.send(packet);
                        }
                        //ATTACK
                        Log.d("LYY1", "33333333333333");
                    }catch (Exception e) {
                        e.printStackTrace();
                    }
                    defence = true;

                }
            };

            TimerTask task1 = new TimerTask() {
                @Override
                public void run() {
                    byte[] data0 = "5".getBytes();
                    DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                    try {
                        if(socket.isClosed()) {
                            Log.d("LYY1", "The socket is closed");
                        }else {
                            socket.send(packet);
                        }
                        //ATTACK
                        Log.d("LYY1", "55555555555555");
                    }catch (Exception e) {
                        e.printStackTrace();
                    }
                    //cancel();
                }
            };

            switch (v.getId()) {
                case R.id.bn1:  //开始按钮
                    if(flag_calibration == false){
                        Toast.makeText(MainActivity.this, "没有进行校准！", Toast.LENGTH_SHORT).show();
                        break;
                    }
                    acceptServer();
                    //初始
                    /*
                    udp1 = new Thread(){
                        @Override
                        public void run() {
                            byte[] data0 = "2".getBytes();
                            DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                            try {
                                socket.send(packet);
                                //Start
                                Log.d("LYY1", "2222222222");
                            }catch (Exception e) {
                                e.printStackTrace();
                            }
                        }
                    };*/


                    ltr = 0;
                    ltr_flag = 0;
                    defence = false;

                    curDate = new Date(System.currentTimeMillis());
                    endDate = new Date(System.currentTimeMillis());
                    timeGapData = 0;
                    StartSensorListening();//启动传感数据采集(注册三个传感器）
                    if (inputfilename.getText().toString().equals("")) {
                        StopSensorListening();//传感器失效
                        break;
                    }


                    //Sensors TXT创建
                    Date date = new Date();
                    SimpleDateFormat sdformat = new SimpleDateFormat("MMdd_hhmmss_SSS");
                    Log.d("LYY", sdformat.format(date));
                    sensor_txtPath = data_txt.getFileDir() + File.separator + inputfilename.getText().toString() + "_sensor";
                    sensor_txtPath =sensor_txtPath + "_" + sdformat.format(date) + ".txt";
                    Log.d(tag, sensor_txtPath);
                    data_txt.createTxt(sensor_txtPath);


                    if (timer == null) {
                        timer = new Timer();
                    }
                    //开启GPS
                    //onGPS();

                    // 调整坐标系
                    outR = getRotattionMatrix(GyrData, LAccData);
                    SensorManager.getRotationMatrix(r_ori, null, AccData, MagData);
                    SensorManager.remapCoordinateSystem(r_ori, SensorManager.AXIS_Y, SensorManager.AXIS_Z, outR);

                    //udp1.start();
                    //定时器
                    //timer0.schedule(task0, 3000);
                    ltr_flag = 0;
                    timer.schedule(new TimerTask() {
                        double [][] sensor_data = new double[7][1];

                        @Override
                        public void run() {
                            if (0 == flag) {

                                sensor_data[0][0] = AccData[0];
                                sensor_data[1][0] = AccData[1];
                                sensor_data[2][0] = AccData[2];
                                sensor_data[3][0] = GyrData[0];
                                sensor_data[4][0] = GyrData[1];
                                sensor_data[5][0] = GyrData[2];
                                sensor_data[6][0] = timeGapData;
                                sensor_data = SC.calibrateSensor(sensor_data);

                                sensor_show = sensor_data.clone();

                                AccList.add((float)sensor_data[0][0]);
                                AccList.add((float)sensor_data[1][0]);
                                AccList.add((float)sensor_data[2][0]);
                                GyrList.add((float)sensor_data[3][0]);
                                GyrList.add((float)sensor_data[4][0]);
                                GyrList.add((float)sensor_data[5][0]);
                                MagList.add(MagData[0]);
                                MagList.add(MagData[1]);
                                MagList.add(MagData[2]);                                // 方向传感器 -zkf
                                OriList.add(OriData[0]);
                                OriList.add(OriData[1]);
                                OriList.add(OriData[2]);                                // 记录时间
                                TimeList.add(timeGapData);

                                //AVG FILTER
                                if (AyList.size() < 50){
                                    AyList.add((float)sensor_data[0][0]);
                                }else{
                                    AyList.remove(0);
                                    AyList.add((float)sensor_data[0][0]);
                                }
                                if (AzList.size() < 50){
                                    AzList.add((float)sensor_data[2][0]);
                                }else{
                                    AzList.remove(0);
                                    AzList.add((float)sensor_data[2][0]);
                                }
                                //SetFilter

                                //float Ay = AccData[0];
                                //float Az = AccData[2];
                                float Ay = 0;
                                float Az = 0;
                                if(AyList.size()>0){
                                    Ay = average(AyList);
                                }
                                if(AzList.size()>0){
                                    Az = average(AzList);
                                }


                                //ltr = (-2 * Ay * 0.59) / (Az * 0.175);
                                if(Ay!=0 && Az!=0) {
                                    float res = Ay / Az;
                                    String res_string = String.format("%.6f", res);
                                    Log.d("LYYLTR", res_string);

                                    byte[] data0 = res_string.getBytes();
                                    DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                                    try {
                                        socket.send(packet);
                                        Log.d("LYY1", "222222222");
                                    } catch (IOException e) {
                                        e.printStackTrace();
                                    }
                                }

                                if (AccList.size() >= 1000){ // 若内存依然无法存储10000个数据，请调整
                                    ArrayList<Float> AccTemp = (ArrayList<Float>)AccList.clone();
                                    ArrayList<Float> GyrTemp = (ArrayList<Float>)GyrList.clone();
                                    ArrayList<Float> MagTemp = (ArrayList<Float>)MagList.clone();
                                    ArrayList<Float> OriTemp = (ArrayList<Float>)OriList.clone();// 方向传感器 -zkf
                                    ArrayList<Float> TimeTemp = (ArrayList<Float>)TimeList.clone();// 记录时间-z
                                    AccList.clear();//清除链表数据
                                    GyrList.clear();
                                    MagList.clear();
                                    OriList.clear();
                                    TimeList.clear();
                                    //WriteXls(AccTemp, GyrTemp, MagTemp, OriTemp, TimeTemp, countWriteTime);
                                    //countWriteTime = countWriteTime + AccTemp.size();
                                    WriteSensorTxt(AccTemp, GyrTemp, MagTemp, OriTemp, TimeTemp);
                                    AccTemp.clear();//清除链表数据
                                    GyrTemp.clear();
                                    MagTemp.clear();
                                    OriTemp.clear();
                                    TimeTemp.clear();

                                }

                            }
                        }
                    }, 0, 20);   //10ms后开始采集，每隔20ms采集一次


                    inputfilename.setEnabled(false);//数据一旦开始采集，不允许输入文件名
                    stop.setEnabled(true);//关闭按钮启用
                    start.setEnabled(false);//开始按钮失效
                    bt_calibration.setEnabled(false);
                    break;

                case R.id.bn2:
                    if(flag_calibration == false){
                        Toast.makeText(MainActivity.this, "没有进行校准！", Toast.LENGTH_SHORT).show();
                        break;
                    }
                    StopSensorListening();//停止传感器采集

                    timer.cancel();//退出采集任务
                    timer = null;
                    inputfilename.setEnabled(true);
                    start.setEnabled(true);
                    stop.setEnabled(false);
                    bt_calibration.setEnabled(true);


                    //WriteXls(AccList, GyrList, MagList, OriList, TimeList, countWriteTime);//核心代码：将采集的数据写入文件中
                    WriteSensorTxt(AccList, GyrList, MagList, OriList, TimeList);
                    AccList.clear();//清除链表数据
                    GyrList.clear();
                    MagList.clear();
                    OriList.clear();
                    TimeList.clear();

                    try {
                        socket.close();
                        socket = null;
                        timer1.cancel();
                        timer1 = null;
                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                    break;

                case R.id.calibration:  //校准按钮
                    acceptServer();

                    Log.d("LYYTEST", "000000000");
                    bt_calibration.setEnabled(false);
                    inputfilename.setEnabled(false);
                    start.setEnabled(false);
                    stop.setEnabled(false);
                    //初始化
                    curDate = new Date(System.currentTimeMillis());
                    endDate = new Date(System.currentTimeMillis());
                    timeGapData = 0;
                    StartSensorListening();//启动传感数据采集(注册三个传感器）

                    if (timer1 == null) {
                        timer1 = new Timer();
                    }

                    // 调整坐标系
                    outR = getRotattionMatrix(GyrData, LAccData);
                    SensorManager.getRotationMatrix(r_ori, null, AccData, MagData);
                    SensorManager.remapCoordinateSystem(r_ori, SensorManager.AXIS_Y, SensorManager.AXIS_Z, outR);

                    timer1.schedule(new TimerTask() {
                        double [] sensor_data = new double[7];
                        boolean straight_flag = true;
                        //llllllllllllllllllllllllllllllllllllllllllllll
                        @Override
                        public void run() {

                            if (timeGapData <= 6000) {
                                /*
                                sensor_data[0] = AccData[0];
                                sensor_data[1] = AccData[1];
                                sensor_data[2] = AccData[2];
                                sensor_data[3] = GyrData[0];
                                sensor_data[4] = GyrData[1];
                                sensor_data[5] = GyrData[2];*/


                                addAverageList(Sensor0, AccData[0], 20);
                                addAverageList(Sensor1, AccData[1], 20);
                                addAverageList(Sensor2, AccData[2], 20);
                                addAverageList(Sensor3, GyrData[0], 20);
                                addAverageList(Sensor4, GyrData[1], 20);
                                addAverageList(Sensor5, GyrData[2], 20);
                                sensor_data[0] = average(Sensor0);
                                sensor_data[1] = average(Sensor1);
                                sensor_data[2] = average(Sensor2);
                                sensor_data[3] = average(Sensor3);
                                sensor_data[4] = average(Sensor4);
                                sensor_data[5] = average(Sensor5);
                                sensor_data[6] = timeGapData;

                                Sensor_List.add(sensor_data);

                                if (timeGapData > 5000){
                                    if (straight_flag) {
                                        byte[] data0 = "1".getBytes();
                                        DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                                        try {
                                            socket.send(packet);
                                            //Go straight
                                            Log.d("LYY1", "11111111");
                                        } catch (Exception e) {
                                            e.printStackTrace();
                                        }
                                        straight_flag = false;
                                    }
                                }
                            } else{
                                sendMessage(0);
                                byte[] data0 = "0".getBytes();
                                DatagramPacket packet = new DatagramPacket(data0, data0.length, address, PORT);
                                try {
                                    socket.send(packet);
                                    Log.d("LYY1", "00000000000");
                                }catch (Exception e) {
                                    e.printStackTrace();
                                }

                                cancel();

                            }

                        }
                    }, 0, 10);   //10ms后开始采集，每隔20ms采集一次

                    break;
            }
        }

    };
    public void sendMessage(int id){
        if (mHandler != null) {
            Message message = Message.obtain(mHandler, id);
            mHandler.sendMessage(message);
        }
    }


    private SensorEventListener listener = new SensorEventListener() {

        @Override
        public void onAccuracyChanged(Sensor sensor, int i) {
            //// TODO: 2018-4-8
        }

        public void onSensorChanged(SensorEvent e) {
            StringBuilder sb = null;

            endDate = new Date(System.currentTimeMillis());
            timeGapData = endDate.getTime() - curDate.getTime();
//            curDate = endDate;
            sb = new StringBuilder();
            sb.append("历时：");
            sb.append(timeGapData);
            etTimeGap.setText(sb.toString());

            switch (e.sensor.getType()) {
                case Sensor.TYPE_GYROSCOPE:     //陀螺传感器

                    sb = new StringBuilder();
                    sb.append("绕X轴转过的角速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[3][0]);
                    }else {
                        sb.append(GyrData[0]);
                    }
                    sb.append("\n绕Y轴转过的角速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[4][0]);
                    }else {
                        sb.append(GyrData[1]);
                    }
                    sb.append("\n绕Z轴转过的角速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[4][0]);
                    }else {
                        sb.append(GyrData[2]);
                    }

                    etGyro.setText(sb.toString());
                    GyrData[0] = e.values[0];
                    GyrData[1] = e.values[1];
                    GyrData[2] = e.values[2];
                    break;

                case Sensor.TYPE_LINEAR_ACCELERATION: // 线性传感器，除重力
                    LAccData[0] = e.values[0];
                    LAccData[1] = e.values[1];
                    LAccData[2] = e.values[2];
                    break;

                case Sensor.TYPE_MAGNETIC_FIELD:    //磁场传感器
                    /*
                    sb = new StringBuilder();
                    sb.append("绕X轴-磁场:");
                    sb.append(e.values[0]);
                    sb.append("\n绕Y轴-磁场:");
                    sb.append(e.values[1]);
                    sb.append("\n绕Z轴-磁场:");
                    sb.append(e.values[2]);

                    //etMagnetic.setText(sb.toString());*/
                    MagData[0] = e.values[0];
                    MagData[1] = e.values[1];
                    MagData[2] = e.values[2];
                    break;

                case Sensor.TYPE_ACCELEROMETER:   //加速度传感器

                    sb = new StringBuilder();
                    sb.append("X轴-加速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[0][0]);
                    }else {
                        sb.append(AccData[0]);
                    }
                    sb.append("\nY轴-加速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[1][0]);
                    }else {
                        sb.append(AccData[1]);
                    }
                    sb.append("\nZ轴-加速度:");
                    if(flag_calibration == true){
                        sb.append(sensor_show[2][0]);
                    }else {
                        sb.append(AccData[2]);
                    }
                    etLinearAcc.setText(sb.toString());
                    AccData[0] = e.values[0];
                    AccData[1] = e.values[1];
                    AccData[2] = e.values[2];

                    break;
            }
            // 方向传感器 -zkf
            SensorManager.getRotationMatrix(r_ori, null, AccData, MagData);

            SensorManager.getOrientation(r_ori, OriData);
            OriData[0] = (float) Math.toDegrees(OriData[0]);
            OriData[1] = (float) Math.toDegrees(OriData[1]);
            OriData[2] = (float) Math.toDegrees(OriData[2]);

//            outR = implementAlignment( GyrData, LAccData);
/*
            sb = new StringBuilder();
            sb.append("Z轴-方向:");
            sb.append(OriData[0]);
            sb.append("\nX轴-方向:");
            sb.append(OriData[1]);
            sb.append("\nY轴-方向:");
            sb.append(OriData[2]);
            etOrientation.setText(sb.toString());*/


        }
    };

    @Override
    protected void onStop() {
        //closeGPS();
        super.onStop();

    }
/*
    private void onGPS(){
        Criteria criteria =new Criteria();
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(false);
        criteria.setBearingRequired(false);
        criteria.setCostAllowed(true);
        criteria.setPowerRequirement(Criteria.POWER_LOW);

        String bestProvider = lm.getBestProvider(criteria, true);
        //Location lc = lm.getLastKnownLocation(bestProvider);
        Location lc = getBestLocation(lm);
        //Location lc = getLastLocation(lm);
        if(lc==null){
            Log.d(tag, " no lccccc 111");
        }
        updateShow(lc);

        try {

            lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 500, 0, locationListener);
        } catch (SecurityException e) {
            e.printStackTrace();
        }

    }

    private void closeGPS() {
        lm.removeUpdates(locationListener);
        updateShow(null);
    }*/

    @Override
    public void onRequestPermissionsResult(int requestCode,  String[] permissions,  int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        Log.d( tag, "ssssssssssss");
        if(requestCode==1&&grantResults[0]==PackageManager.PERMISSION_GRANTED){
            if(lm==null){
                Log.d(tag, " no lm ");
            }

        }

    }

    private void WriteSensorTxt(ArrayList<Float> accdata, ArrayList<Float> gyrdata,
                                ArrayList<Float> magdata, ArrayList<Float> oridata, ArrayList<Float> timegapdata){

        File file = new File(sensor_txtPath);
        int length = (int)(accdata.size() / 3);
        String content;

        try{
            fos = new FileOutputStream(file,true);
            for(int i=0;i<length;i++){
                content = "";
                //accdata[i*3], accdata[i*3+1],accdata[i*3+2],
                //gyrdata[i*3],gyrdata[i*3+1],gyrdata[i*3+2]
                content = content + "ACC: ";
                for(int j=0;j<3;j++){
                    content = content + String.valueOf(accdata.get(i*3+j)) + ", ";
                }

                content = content + "GYR: ";
                for(int j=0;j<3;j++){
                    content = content + String.valueOf(gyrdata.get(i*3+j)) + ", ";
                }

                content = content + "Time: ";
                content = content + String.valueOf(timegapdata.get(i));

                content = content + "\n";
                fos.write(content.getBytes());
            }
            //fos.write(content.getBytes());
            //fos.write("\r\n".getBytes());

        }catch (IOException e){
            e.printStackTrace();
        }finally {
            if(fos!=null){
                try{
                    fos.close();
                }catch (IOException e){
                    e.printStackTrace();
                }
            }
        }

    }

    public float[] getRotattionMatrix(float[] gyrData, float[] laccData) {
        float outR[] = new float[9];

        float maxtemp = Float.NEGATIVE_INFINITY, mintemp = Float.POSITIVE_INFINITY;
        for (int z1 = 0; z1 < 3; z1++) {
            if (gyrData[z1] >= maxtemp) {
                maxtemp = gyrData[z1];
            }

            if (gyrData[z1] <= mintemp) {
                mintemp = gyrData[z1];
            }
        }

        for (int z1 = 0; z1 < 3; z1++) {
            outR[z1 + 2 * (z1 + 1)] = (gyrData[z1] - mintemp) / (maxtemp - mintemp);
        }

        for (int z1 = 0; z1 < 3; z1++) {
            outR[z1 * 3 + 1] = laccData[z1];
        }

        for (int z1 = 0; z1 < 3; z1++) {
            outR[z1 * 3] = outR[z1 + 2 * (z1 + 1)] * outR[z1 * 3 + 1];
        }

        return outR;

    }

    public void StartSensorListening() {
        //super.onResume();
        //陀螺传感器注册监听器
        mSensorManager.registerListener(listener, mSensorManager.getDefaultSensor(
                Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_FASTEST);        //使用SensorManager.SENSOR_DELAY_GAME代替SensorManager.SENSOR_DELAY_FASTEST以节约能耗

        //磁场传感器注册监听器
        mSensorManager.registerListener(listener, mSensorManager.getDefaultSensor(
                Sensor.TYPE_MAGNETIC_FIELD), SensorManager.SENSOR_DELAY_FASTEST);

        //加速度传感器注册监听器
        mSensorManager.registerListener(listener, mSensorManager.getDefaultSensor(
                Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_FASTEST);
    }

    public void StopSensorListening() {
        mSensorManager.unregisterListener(listener);
    }

    /*
    private boolean isGpsAble(LocationManager lm) {
        if (lm.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
            return true;
        } else
            return false;
    }

    private void openGPS2() {
        Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
        startActivityForResult(intent, 0);
    }

    private Location getBestLocation(LocationManager locationManager) {
        Location result = null;
        if (locationManager != null) {
            result = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
            if (result != null) {
                return result;
            } else {
                result = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                return result;
            }
        }
        return result;


    }

    private void updateShow(Location location) {
        Log.d(tag, "000000000000");
        gpsDate = new Date(System.currentTimeMillis());
        timeGpsData = gpsDate.getTime() - curDate.getTime();
        if (location != null) {
            Log.d(tag, "111111111111");
            WriteTxt(location,txtPath);
            StringBuilder sb = new StringBuilder();
            sb.append("当前位置信息:\n");
            sb.append("精度:" + location.getLongitude() + "\n");
            sb.append("纬度:" + location.getLatitude() + "\n");
            sb.append("高度:" + location.getAltitude() + "\n");
            sb.append("速度:" + location.getSpeed() + "\n");
            sb.append("方向:" + location.getBearing() + "\n");
            sb.append("定位精度:" + location.getAccuracy() + "\n");
            etGps.setText(sb.toString());
        } else {
            etGps.setText("");
            Log.d(tag, "2222222222222");
        }
    }*/
}