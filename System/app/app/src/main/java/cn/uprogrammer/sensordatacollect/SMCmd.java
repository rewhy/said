package cn.uprogrammer.sensordatacollect;


import android.content.Context;
import android.content.res.AssetManager;
import android.location.Location;
import android.location.LocationManager;
import android.util.Log;

import org.apache.commons.math3.linear.Array2DRowRealMatrix;
import org.apache.commons.math3.linear.RealMatrix;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;

import cn.uprogrammer.sensordatacollect.view.SpeedIndicatorView;
import rfPrediction.SensorCalibration;
import weka.classifiers.Classifier;

import static rfPrediction.featureExtraction.*;
import static rfPrediction.getCurrentStatus.*;


public class SMCmd {

    public static FileOutputStream fos = null;


    //cmd from canitf:
    public static final byte CMDF_RET_DATA = 0x09;


    //sub cmd of CMDF_RET_DATA

    //cmd to canitf.
    public static final byte CMDT_HEART_BEAT = (byte) 0x81;
    public static final byte CMDT_CAN_FRAME_DATA = (byte) 0x91;
    public static final byte CMDT_GET_CAN_FILTER_ID = (byte) 0x92;
    public static final byte CMDT_SET_CAN_FILTER_ID = (byte) 0x93;
    public static final byte CMDT_ACTION_ID = (byte) 0x94;

    public static final int PORT_NUM = 6001;
    public static final int FRAME_DATA_MAX = 255;
    public static final int PAYLOAD_DATA_MAX = 220;
    public static final byte START1 = (byte) 0xFF;
    public static final byte START2 = (byte) 0xFE;
    public static final byte END1 = START2;
    public static final byte END2 = START1;
    // Message/Intent type inside the app
    public static final int APP_INTENT_TYPE_TX = 10;
    public static final int APP_MSG_TYPE_TX = 10;
    //cmd from canitf:
    public static final byte CMD_TC_HEARBEAT = 0x00;
    public static final byte CMD_TC_CAN_FRAME = 0x01;
    public static final byte CMD_TC_ONBOARD_INFO = 0x02;
    public static final byte CMD_TC_CONTEXT_INFO = 0x03;
    public static final byte CMD_TC_FILTER_INFO = 0x04;
    //sub cmd of CMDF_RET_DATA
    public static final byte SCMD_RET_WORK_MODE = 0x01;
    //cmd to canitf.
    public static final byte CMD_FC_HEARTBEAT = (byte) 0x11;
    public static final byte CMD_FC_CAN_FRAME_DATA = (byte) 0x12;
    public static final byte CMD_FC_GET_FILTER = (byte) 0x13;
    public static final byte CMD_FC_SET_FILTER = (byte) 0x14;
    public static final byte CMD_FC_ACTION_ID = (byte) 0x15;
    public static final byte CMD_ACTION_HORN = (byte) 0x21;
    public static final byte CMD_ACTION_UNLOCK = (byte) 0x22;
    public static final byte CMD_ACTION_LOCK = (byte) 0x23;
    public static final byte CMD_ACTION_OPEN_WINDOW = (byte) 0x24;
    public static final byte CMD_ACTION_CLOSE_WINDOW = (byte) 0x25;
    public static final byte CMD_ACTION_CLOSE_RVM = (byte) 0x26;
    public static final byte CMD_ACTION_OPEN_RVM = (byte) 0x27;
    public static final byte CMD_ACTION_OPEN_LIGNHG = (byte) 0x28;
    public static final byte CMD_ACTION_CLOSE_LIGNHG = (byte) 0x29;
    public static final int SOCKET_CONNECT_SUCCESS = 8000;
    public static final int TIMER_1_SEC = 8002;
    public static final int RX_MSG = 8888;
    private static final String TAG = "CANAPP";

    // parameter for status detection
    public static List sensor4Cal;
    public static int timeInd;
    public static List eventList;
    public static SensorCalibration SC;
    public static double [] threshold;
    public static int currentEvent;
    public static int eventCount;
    public static int eventLength;
    public static int straightStatus;
    public static int flag_straight;
    public static List gpsSpeed;

    public static int countHighway;
    public static double [] averageSpeedHw;
    public static double [] averageSpeedIn1min;
    public static int highwayFlag;
    public static int stopFlag;
    public static int bumpFlag;
    public static double [] recordBump;

    public static double [] recordBrak;
    public static int countBrak;

    public static int leftSignal;
    public static int rightSingal;
    public static double latestGPSSpeed;


    public boolean isValidMsage(byte[] buf) {

        return false;
    }

    /**
     * Create the encoded Tx data buffer based on the specific command
     *
     * @param cmd is type/command of the Tx message
     * @return the data buffer storing the Tx message
     */
    private byte[] getTxBuffer(byte cmd) {
        byte dataLenth = 1;
        int bufLength = 6 + dataLenth;
        int dataOffset = 4;
        byte[] wrBuff = new byte[bufLength];

        wrBuff[0] = START1;
        wrBuff[1] = START2;
        wrBuff[2] = 0x00;
        wrBuff[3] = dataLenth;
        wrBuff[dataOffset++] = cmd;
        wrBuff[bufLength - 2] = END1;
        wrBuff[bufLength - 1] = END2;

        return wrBuff;
    }

    /**
     * Create the encoded Tx data buffer based on both command and sub-command
     *
     * @param cmd
     * @param subCmd
     * @return
     */
    public byte[] getTxBuffer(byte cmd, byte subCmd) {
        byte dataLenth = 2;
        int bufLength = 6 + dataLenth;
        int dataOffset = 4;
        byte[] wrBuff = new byte[bufLength];

        wrBuff[0] = START1;
        wrBuff[1] = START2;
        wrBuff[2] = 0x00;
        wrBuff[3] = dataLenth;
        wrBuff[dataOffset++] = cmd;
        wrBuff[dataOffset++] = subCmd;
        wrBuff[bufLength - 2] = END1;
        wrBuff[bufLength - 1] = END2;

        return wrBuff;
    }

    /**
     * Create the encoded Tx data buffer based on both command and  payload
     *
     * @param cmd
     * @param payload
     * @return
     */
    public byte[] getTxBuffer(byte cmd, byte[] payload) {
        int dataLenth = 1 + payload.length;
        int bufLength = 6 + dataLenth;
        int dataOffset = 4;
        byte[] wrBuff = new byte[bufLength];

        wrBuff[0] = START1;
        wrBuff[1] = START2;
        wrBuff[2] = 0x00;
        wrBuff[3] = (byte) dataLenth;
        wrBuff[dataOffset++] = cmd;

        System.arraycopy(payload, 0, wrBuff, dataOffset, payload.length);
        dataOffset += payload.length;

        wrBuff[bufLength - 2] = END1;
        wrBuff[bufLength - 1] = END2;

        return wrBuff;
    }

    /**
     * GEt the heartbeat packet bytes
     *
     * @return
     */
    public byte[] getTxHeartbeat() {
        return getTxBuffer(CMD_FC_HEARTBEAT);
    }

    /**
     * Get the bytes stored at the specified offset of the data buffer
     */
    public byte[] getMsgPayload(byte[] buff, int offset, int count) {
        byte[] buffer = new byte[count];

        if (offset + count > buff.length) {
            return null;
        }

        for (int i = 0; i < count; i++) {
            buffer[i] = buff[i + offset];
        }
        return buffer;
    }

    public static void initiateSMCmd() {
        sensor4Cal = new ArrayList();
        SC = new SensorCalibration();
        timeInd = 0;
        currentEvent = -1;
        latestGPSSpeed = 0;


        straightStatus = 0;
        leftSignal = 0; // 0 直行；1左
        rightSingal = 0;    // 0 直行； 1 右

        eventCount = 0;
        eventLength = 3;
        flag_straight = 0;
        highwayFlag = 0;
        stopFlag = 0;

        eventList = new ArrayList();
        gpsSpeed = new ArrayList();

        threshold = new double [6];
        threshold[0] = 10; // integration upper
        threshold[1] = -10; // integration bounder
        threshold[2] = 0.5; // ori gyro z upper
        threshold[3] = -0.5; // ori gyro z bounder
        threshold[4] = 0.5; // slop upper
        threshold[5] = -0.5; // slop bounder



        // brake
        recordBrak = new double [2];
        recordBrak[0] = 0;
        recordBrak[1] = 0;
        countBrak = 0;


        averageSpeedHw = new double [5]; // record average speed in 5 min
        for (int z1 = 0; z1 < averageSpeedHw.length;z1++){
            averageSpeedHw[z1] = 0;
        }
        countHighway = 0;
        averageSpeedIn1min = new double [60]; // record average speed in 1 min
        for (int z1 = 0; z1 < averageSpeedIn1min.length;z1++){
            averageSpeedIn1min[z1] = 0;
        }
        recordBump = new double[60];
        for (int z1 = 0; z1 < recordBump.length;z1++){
            recordBump[z1] = 0;
        }


        StateTables.SingleState.put("Straight Driving", (double) straightStatus);
        StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent);


    }


    /* *
    Process the message from the CANIPS server
     */
    public void processMsg(byte[] buff, Classifier multiClassClassifier) {
        long currentTime = System.currentTimeMillis();
        if (Window.Window_start == 0) {
            Window.Window_start = currentTime;
        }

        // Log.d(TAG, "processRxData: ");
        int validLength = buff.length - 2;
        int dataLength = buff.length - 11;
        byte command = buff[0];
        byte length = buff[1];

        /* Data (CAN frame) read from OBD port */
        if (command == CMD_TC_HEARBEAT) {

            // Log.d(TAG, "TO_CLIENT_HEARTBEAT");
        } else if (command == CMD_TC_CAN_FRAME) {/* Data read from on-board sensors */
            // Log.d(TAG, "TO_CLIENT_CAN_FRAME");

        } else if (command == CMD_TC_ONBOARD_INFO) {
            // Log.d(TAG, "TO_CLIENT_ONBOARD_INFO");
            byte[] data = new byte[24];
            for (int i = 0; i < 24; i++) {
                data[i] = buff[i + 2];
            }
            SensorUtil.parsetoString(data);
            if (currentTime - Window.Window_start <= Window.Window_size) {
                // 数据未满1s，继续添加，
            } else {
                vehicleStatusEstimation(multiClassClassifier);
                Window.ResetWindow();
            }
        }


        // Log.d(TAG, "TO_CLIENT_CONTEXT_INFO");
        else if (command == CMD_TC_CONTEXT_INFO) {
            byte[] data = new byte[20];
            // Log.d(TAG, String.format("TO_CLIENT_CONTEXT_INFO %d bytes", buff.length));
            for (int i = 0; i < 20; i++) {
                data[i] = buff[i + 2];
            }
            Utils.parsetoString(data);
            StateTables.UpdataOBD(data);
            StateTables.UpdataUDS(data);
            Window.SendWarning(currentTime);

            if (currentTime - Window.Window_start <= Window.Window_size) {
                // 数据未满1s，继续添加，
                //StateWindow.Can0_List.add(content);
                //StateTables.UpdateUDS(canID, payload);
            } else {
                vehicleStatusEstimation(multiClassClassifier);
                Window.ResetWindow();
            }


        } else if(command == CMD_TC_FILTER_INFO){
            byte[] data = new byte[2];
            for (int i = 0;i < 2 ; i++){
                data[i] = buff[i+2];
            }
            int num = data[0];
            int decision = data[1];
            StateTables.Action = ActionValue.ACTION[num];
            StateTables.Decison = decision;
            Log.d("LYYREAD",StateTables.Action+":"+decision);

        }
        else {
        }

    }

    /**
     * zkf 1030
     **/
    public void vehicleStatusEstimation(Classifier multiClassClassifier) {
        if (timeInd < 4){
            timeInd++;
            for (int z1 = 0; z1 < Window.Sensor_List.size(); z1++){
                sensor4Cal.add(Window.Sensor_List.get(z1));
            }
        }else if (timeInd == 4) {
            for (int z1 = 0; z1 < Window.Sensor_List.size(); z1++){
                sensor4Cal.add(Window.Sensor_List.get(z1));
            }
            double [][] currentSensorData = convertSensorData(sensor4Cal);
            sensor4Cal.clear();

            SC.getRotationMatrix(currentSensorData); // 计算校准矩阵
            Log.d("DataSize", "SC ACCX:" + SC.meanAccX + ", AccY: " + SC.meanAccY +", Gyro Z: "+SC.meanGyroZ);

            double [] currentSpeedData = converSpeedData(Window.Vehicle_Speed_List);
            if (getAverage(currentSpeedData) < 2 && getStandardDiviation(currentSpeedData) < 1) {
//                if (stopFlag == 0){
//                    StateTables.stateNumber[6]++;
//                }
//                stopFlag = 1;

                StateTables.stateNumber[6] = 0;
                currentEvent = 9;
                StateTables.SingleState.put("Straight Driving", (double) straightStatus);
                StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent);
            }
            StateTables.stateNumber[6] = 1;
            stopFlag = 0;
            timeInd++;
        }else{
            double [][] currentSensorData1 = convertSensorData(Window.Sensor_List);
            currentSensorData1 = SC.calibrateSensor(currentSensorData1);  // 7 * n
            for (int z1 = 0; z1 < currentSensorData1[5].length; z1++){
                Log.d("mySensor", "cal sensor[5]: "+ currentSensorData1[5][z1]);
            }

            double [] currentSpeedData = converSpeedData(Window.Vehicle_Speed_List);    // 7 * n
            /** rules to identify action **/
            averageSpeedIn1min[timeInd % 60] = getAverage(currentSpeedData);
            //highway driving
            if ((timeInd % 60) == 0){
                averageSpeedHw[countHighway%5] = getAverage(averageSpeedIn1min);
                countHighway++;
            }

            if (countHighway > 4){
                if (getAverage(averageSpeedHw) > 60){
                    Log.d("Rule Identify", "driving on high way" + getAverage(averageSpeedHw));
                    highwayFlag = 1;
                    StateTables.stateNumber[8] = 1;

                }else{
                    highwayFlag = 0;
                    StateTables.stateNumber[8] = 0;

                }
            }

            // brake
            // level 3: acc > 1.67 last 2s; level 2: acc > 2.22 last 2s; level 1: acc > 2.78, last 2s
            if (currentSpeedData.length > 1 ){
                recordBrak[timeInd%2] = currentSpeedData[currentSpeedData.length-1]-currentSpeedData[0];
            }
            double accTemp = getAbsSum(recordBrak)/2;
            if (accTemp > 1.67 && accTemp < 2.22){
                StateTables.stateNumber[10] ++;

                Log.d("Rule Identify", "braking happened");

            }
            else if (accTemp > 2.22 && accTemp < 2.78 ){
                StateTables.stateNumber[11] ++;

                Log.d("Rule Identify", "braking happened");

            }else if (accTemp > 2.78 ){
                StateTables.stateNumber[12] ++;
                Log.d("Rule Identify", "braking happened");
            }
            if (accTemp < -3){
                StateTables.stateNumber[7] ++;
            }

            
            double [][] currentSensorData = convertSensorData(Window.Sensor_List);
            currentSensorData = SC.calibrateSensor(currentSensorData);  // 7 * n

            // identify bump
            double tempAccZ = getAbsSum(currentSensorData[2]);
            for (int z111 = 0; z111 < currentSensorData[2].length;z111++){
                Log.d("Rule Identify", "driving on bump road: "+currentSensorData[2][z111]);
            }
            if (tempAccZ >= 7.5){
                bumpFlag = 1;
                recordBump[timeInd % 60] = 1;
                StateTables.stateNumber[9]++;

                Log.d("Rule Identify","driving on bump road" + tempAccZ);
            }
            else{
                recordBump[timeInd % 60] = 0;
            }
            if (getSum(recordBump) > 30) {   // bump happens a lot in 1min
                Log.d("Rule Identify", "give a warning to driver?");
            }

            /***/

            if (getAverage(currentSpeedData) < 2 && getStandardDiviation(currentSpeedData) < 1) {
//                if (stopFlag == 0) {
//                    StateTables.stateNumber[6]++;
//                }
                StateTables.stateNumber[6] = 0;
                straightStatus = 0;
                flag_straight = 1;
                stopFlag = 1;
                gpsSpeed.add(latestGPSSpeed);
                Log.d("gpsupdate", "gpsSpeed: "+latestGPSSpeed);



                currentEvent = 9;
                StateTables.SingleState.put("Straight Driving", (double) straightStatus);
                StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent);
                Log.d("Speed C In","current speed: " + getAverage(currentSpeedData) + "std: "+getStandardDiviation(currentSpeedData));


                Log.d("1028", "time: " + timeInd + ", straight: " + straightStatus + ", event: " + currentEvent +
                        ", angle:" + StateTables.SingleState.get("SteerAngle") + ", speed: " + StateTables.SingleState.get("Vehicle_Speed"));
            }else{
//                double [][] currentSensorData = convertSensorData(Window.Sensor_List);
//                currentSensorData = SC.calibrateSensor(currentSensorData);  // 7 * n
                stopFlag = 0;
                StateTables.stateNumber[6] = 1;
                double f_int = getSum(currentSensorData[5]);    // integration
                double [] f_ori = currentSensorData[5]; // oridata
                double f_slop = currentSensorData[5][currentSensorData[5].length-1] - currentSensorData[5][0]; //slop

                if (f_int < threshold[0] && f_int > threshold[1] && getMax(f_ori) < threshold[2] &&
                        getMin(f_ori) > threshold[3] && f_slop < threshold[4] && f_slop > threshold[5]){
                    flag_straight = 1;
                    straightStatus = 1;
                    currentEvent = -1;
                    leftSignal = 0;
                    rightSingal = 1;
                    StateTables.laneSignal = 0;
                    StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent);
                    Log.d("1028", "time: " + timeInd + ", straight: " + straightStatus + ", event: " + currentEvent +
                            ", angle:" + StateTables.SingleState.get("SteerAngle") + ", speed: "
                            + + getAverage(currentSpeedData));
                }else{
                    // left  +  right -
                    /** not straight refined to : left action/right action **/
                    if (f_int > threshold[0] || getMax(f_ori) > threshold[2] || f_slop > threshold[4]){  //left action
                        StateTables.laneSignal = 1;
                    }else if (f_int < threshold[1] || getMin(f_ori) < threshold[3] || f_slop < threshold[5]){ // right action
                        StateTables.laneSignal = 2;
                    }

                    gpsSpeed.add(latestGPSSpeed);
                    Log.d("gpsupdate", "gpsSpeed: "+latestGPSSpeed);

                    Log.d("GPS_data", "Current GPS: "+latestGPSSpeed);


                    straightStatus = 0;
                    flag_straight = 0;
                    eventCount += 1;
                    currentEvent = -3;
                    eventList.add(currentSensorData);
                    StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent);

                    Log.d("1028", "time: " + timeInd + ", straight: " + straightStatus + ", event: " + currentEvent +
                            ", angle:" + StateTables.SingleState.get("SteerAngle") + ", speed: "
                            + getAverage(currentSpeedData));
                }
            }

            if (flag_straight == 1 && eventCount >= eventLength){
                int dataNum = 0;
                for (int z1 = 0; z1 < eventCount; z1++){
                    double [][] temp = (double[][]) eventList.get(z1);
                    dataNum += temp[0].length;
                }
                double [][] data4identify = new double [8][dataNum];
                dataNum = 0;
                for (int z1 = 0; z1 < eventCount; z1++){
                    double [][] temp = (double [][]) eventList.get(z1);
                    for (int z2 = 0; z2 < temp[0].length; z2++, dataNum++){
                        for (int z3 = 0; z3 < 7; z3++){
                            data4identify[z3][dataNum] = temp[z3][z2];
                        }
                        data4identify[7][dataNum] = (double)gpsSpeed.get(z1);
                    }
                }
                Log.d("DataSize", "EventList: " + eventList.size() + ", dataSize: "+ data4identify[0].length );

                double [] feature = get_meaningful_features(data4identify);

                Log.d("DataSize", "featureSize: " + feature.length );

                currentEvent = identifyDrivingEvent(feature, multiClassClassifier);
                StateTables.stateNumber[currentEvent]++;
                StateTables.SingleState.put("Special Vehicle Status", (double) currentEvent+2);

                eventList = new ArrayList();
                gpsSpeed = new ArrayList();
                eventCount = 0;
            }else if (flag_straight == 1 && eventCount < eventLength){
                eventList = new ArrayList();
                gpsSpeed = new ArrayList();

                eventCount = 0;
                currentEvent = -1;
            }
            StateTables.SingleState.put("Straight Driving", (double) straightStatus);



            Log.d("1028", "time: " + timeInd + ", straight: " + straightStatus + ", event: " + currentEvent +
                    ", angle:" + StateTables.SingleState.get("SteerAngle") + ", speed: " +
                    StateTables.SingleState.get("Vehicle_Speed"));

            timeInd++;
        }
    }





    public double[][] convertSensorData(List sensorData) {

        double[][] currentSensorData = new double[7][sensorData.size()];
        for (int z2 = 0; z2 < sensorData.size(); z2++) {
            double[] temp = (double[]) sensorData.get(z2);
            for (int z3 = 0; z3 < 7; z3++) {
                currentSensorData[z3][z2] = temp[z3];        // sensor data 7 * n, saved by time sequence
            }
        }
        return currentSensorData;
    }

    public double[] converSpeedData(List speedData) {


        double[] currentSpeedData = new double[speedData.size()];

        for (int z1 = 0; z1 < speedData.size(); z1++){
            currentSpeedData[z1] = (double) speedData.get(z1);
        }

        return currentSpeedData;
    }


}




