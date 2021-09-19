package cn.uprogrammer.sensordatacollect;

import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import jxl.demo.Write;

import static cn.uprogrammer.sensordatacollect.StateTables.SingleState;

public class SensorUtil {
    private static final String TAG = "canitfservice";

//  #define M_PI 3.14159265358979
//  #define GRAVITY_EARTH           (9.80665f)
//  #define ACC_DATA_CONVERSION(value) (float)((float)((int)(value)) * (GRAVITY_EARTH / (0x4000)))
//  #define GYRO_DATA_CONVERSION(value) (float)(value*M_PI/180.0)
    private static final float GRAVITY_EARTH = (9.80665f);
    private static final float M_PI = (3.14159265358979f);
    public static long time4gps;
    static String[] sensor_name = {"ACCX","ACCY","ACCZ","GYROX","GYROY","GYROZ"};

    public static float ACC_DATA_CONVERSION(short value) {
        float data = value;
        data *= GRAVITY_EARTH;
        data /= 0x4000;
        return data;
    }

    public static float GYRO_DATA_CONVERSION(short value) {
        return (float) (value * M_PI / 180.0);
    }

//    private static final byte REPORT_TO_SERVICER_ID_SENSOR = 0x02;


    public static String parsetoString(byte[] buf) {
        if (buf.length != 24) {
            Log.d(TAG, "The byte array is too small!!!");
            return "NULL";
        }

        int[] as = new int[6];
        String str = "02,";
        // double[] ds = new double[6];

        ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer().get(as);
        // ByteBuffer.wrap(buf).order(ByteOrder.BIG_ENDIAN).asDoubleBuffer().get(ds);
        /*
        double x = (double) as[0] / 1000000.0f;
        double y = (double) as[1] / 1000000.0f;
        double z = (double) as[2] / 1000000.0f;
        double ax = (double) as[3] / 1000000.0f;
        double ay = (double) as[4] / 1000000.0f;
        double az = (double) as[5] / 1000000.0f;
        */

        long sensortime = System.currentTimeMillis();
        time4gps = sensortime;
        double [] sensorData = new double[7];

        double value;
        for(int i = 0; i<6; i++){
            value = (double) as[i] / 1000000.0f;
            sensorData[i] = value;
            SingleState.put( sensor_name[i] ,value);
            str += String.valueOf(value);
            str += ",";
            Log.d("mySensor", "sensor["+i+"]: "+value);
        }
        sensorData[6] = sensortime;
        Window.Sensor_List.add(sensorData);
        //Log.d("SensorINFO",str);
        return str;

        // Log.d(TAG, String.format("Onboard: %f,%f,%f,%f,%f,%f", x, y, z, ax, ay, az));
    }

    public static String getString(byte[] data) {
        if (data.length != 24) {
            return "Null";
        }

        short[] short_data = Utils.bytesToShort(data);

        float value;
        String str = "02,"; //REPORT_TO_SERVICER_ID_SENSOR
        for (int i = 0; i < short_data.length; i++) {

            if (i < 3) {
                value = ACC_DATA_CONVERSION(short_data[i]);
                Log.d("ACC", "ACC"+ i + ": " + value);
            } else {
                value = GYRO_DATA_CONVERSION(short_data[i]);
                Log.d("Gyo", "Gyo" + i + ": " + value);
            }


            str += String.valueOf(value);
            str += ",";
        }

        return str;
    }
}
