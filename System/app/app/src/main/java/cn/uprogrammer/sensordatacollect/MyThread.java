package cn.uprogrammer.sensordatacollect;

import android.content.res.AssetManager;
import android.util.Log;

import java.io.InputStreamReader;
import java.util.List;
import java.util.concurrent.TimeUnit;

import cn.uprogrammer.sensordatacollect.IPSService;
import cn.uprogrammer.sensordatacollect.SMCmd;
import cn.uprogrammer.sensordatacollect.SensorUtil;
import weka.core.pmml.jaxbbindings.True;

import static cn.uprogrammer.sensordatacollect.IPSService.assetManager;


public class MyThread extends Thread {

    public List gpsData;

    // 通过构造方法给线程名字赋值
    public MyThread(String name, String filename, AssetManager assetManager) {
        super(name);// 给线程名字赋值
        IPSService ipzkf = new IPSService();
        gpsData = ipzkf.getLocalGPSData(assetManager, filename);
    }



    @Override
    public void run(){
        long sensortime = SensorUtil.time4gps;
        Log.d("gps", "run: " + sensortime);

        int i = 0;
        String [] data_temp = (String[])gpsData.get(i);
        Log.d("gps", "run: time"+data_temp[1]+ " speed: "+data_temp[0]);

        if (Long.valueOf(data_temp[1]) > sensortime){

            long sleepTime = Long.valueOf(data_temp[1]) - sensortime;
            Log.d("gps", "sleep time: "+sleepTime);
            try {
                TimeUnit.MILLISECONDS.sleep(sleepTime);//毫秒
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }else if (Long.valueOf(data_temp[1]) < sensortime){
            while ((Long.valueOf(data_temp[1]) - sensortime) > 1000L){
                data_temp = (String[])gpsData.get(i++);
            }
            Log.d("gps", "getData"+i);


        }
        SMCmd.latestGPSSpeed = Double.valueOf(data_temp[0]);
        long ptime = System.currentTimeMillis();

        while(i <= gpsData.size()){
            long ctime = System.currentTimeMillis();
            if ((ctime - ptime) < 1000){

            }else{
                ptime = ctime;
                data_temp = (String [])gpsData.get(i++);
                Log.d("gps", "run: "+data_temp[0]);

                SMCmd.latestGPSSpeed = Double.valueOf(data_temp[0]);
            }

        }
    }


}

