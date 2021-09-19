package cn.uprogrammer.sensordatacollect;

import android.util.Log;

import org.apache.commons.math3.analysis.function.Sin;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;

public class StateTables {

    public static int State_number = 0;
    public String TAG = "ST";

    public static Map<String,Double> SingleState = new HashMap();
    public static int[] stateNumber=new int[13];
    // "Left_Turn"0 , "Right_Turn"1, "Left_Lane_Change"2, "Right_Lane_Change" 3, "Left_U_Turn" 4, "Right_U_Turn" 5, "STOP" 6
    // Rules: "brake": 7, "highWay Driving":8, "bump": 9, "L1 Acceleration" 10, "L2 Acceleration" 11, "L3 Acceleration" 12
    public static int laneSignal = 0;

    public static String Action = "";
    public static int Decison = -1;

    public static void InitStateTables(){

        for(int i=0;i<7;i++){
            stateNumber[i]=0;
        }
        //UDS_STATE
        SingleState.put("Vehicle_Speed",(double)0);
        SingleState.put("Acceleration",(double)0);
        SingleState.put("Auto_Trans_Acceleration",(double)0);
        SingleState.put("RPM",(double)0);
        SingleState.put("Gear",(double)0);
        SingleState.put("LongitudinalAcceleration",(double)0);
        SingleState.put("LongitudinalAcceleration_Raw",(double)0);
        SingleState.put("Tilt_Angular_Acc_Raw",(double)0);
        SingleState.put("BrakePressure",(double)0);
        SingleState.put("Auto_Trans_BrakePressure",(double)0);
        SingleState.put("SteerAngle",(double)0);
        SingleState.put("LateralAcceleration", (double)0);

        //OBD_STATE
        SingleState.put("Coolant temperature", (double)0);
        SingleState.put("Inlet temperature", (double)0);
        SingleState.put("Intake flow rate", (double)0);
        SingleState.put("Manifold absolute pressure", (double)0);
        SingleState.put("Throttle position", (double)0);

        SingleState.put("Orientation",StateValue.Go_Straight);
        SingleState.put("Gradient",StateValue.Horizontal);
        SingleState.put("Window",StateValue.Window_Close);
        SingleState.put("Door",StateValue.Door_Close);

        //Sensors
        SingleState.put("ACCX",(double)0);
        SingleState.put("ACCY",(double)0);
        SingleState.put("ACCZ",(double)0);
        SingleState.put("GYROX",(double)0);
        SingleState.put("GYROY",(double)0);
        SingleState.put("GYROZ",(double)0);

        //Classfy State;
        SingleState.put("Straight Driving",StateValue.Driving_Event);
        SingleState.put("Special Vehicle Status",StateValue.DEFAULT);

        //Warning
        SingleState.put("Warning Message",(double)-1);

        //TODO
        SingleState.put("Turn_Signal_Lamp",(double)0); //left:1,right:2,both3

        SingleState.put("Frontdriverside_Window",(double)0);
        SingleState.put("Reardriverside_Window",(double)0);
        SingleState.put("Frontpassenger_Window",(double)0);
        SingleState.put("Rearpassenger_Window",(double)0);

        SingleState.put("Frontdriverside_Door",(double)0);
        SingleState.put("Reardriverside_Door",(double)0);
        SingleState.put("Frontpassenger_Door",(double)0);
        SingleState.put("Rearpassenger_Door",(double)0);

        SingleState.put("Rearview_Mirrors",(double)1);
    }


    public void InsertState(String s_name,double s_value){

        SingleState.put(s_name,s_value);
        State_number++;

    }

    public boolean ChangeState(String s_name,double s_value){
        if(SingleState.containsKey(s_name))
        {
            SingleState.put(s_name,s_value);
            return true;
        }
        else {
            Log.d(TAG,"No such State");
            return false;
        }

    }
/*
    public static void UpataOBD(String id,byte[] data){
        switch (id) {
            case "07E8":
                if(data[0]<=0x07&&data[1]==0x41){
                    if(data[2]==(byte) 0x0D){
                        int speed = (data[3] & 0xFF);
                        double v_speed = (double)speed;
                        Log.d("OBDINFO",""+speed);
                        SingleState.put("Vehicle_Speed",v_speed);
                    }

                    if(data[2]==(byte) 0x0C){ //发动机转速
                        int rpm = (data[3])<<8 |(data[4]);
                        double v_rpm = ((double)rpm)/4;
                        Log.d("OBDINFO",""+rpm);
                        SingleState.put("RPM",v_rpm);
                    }
                }


        }
    }
*/
    public static void UpdataUDS(byte[] buf){
        Float res ;
        int pid = 0;
        int sid = 0;

        int[] as = new int[5];
        ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer().get(as);
        sid = as[0];
        if(sid!=0x22){
            return ;
        }
        pid = as[1];
        res = new Float((float) as[2] / 1000.0f);
        double value = Double.parseDouble(res.floatValue()+"");
        //Log.d("LYY00",""+ pid);
        switch (pid){
            case 0x01://Speed
                SingleState.put("Vehicle_Speed",value);
                Window.Vehicle_Speed_List.add(value);

                break;
            case 2://ACC
                //Log.d("LYY02","ACC:"+value);
                SingleState.put("Acceleration",value);
                break;
            case 3:
                SingleState.put("RPM",value);
                break;
            case 4:
                SingleState.put("BrakePressure",value);
                break;
//            case 3:
//                SingleState.put("BrakePressure",value);
//                break;
//            case 4:
//                SingleState.put("RPM",value);
//                break;
            case 5:
                SingleState.put("LateralAcceleration",value);
                break;
            case 6:
                SingleState.put("LongitudinalAcceleration",value);
                break;
            case 7:
                SingleState.put("LongitudinalAcceleration_Raw",value);
                break;
            case 8:
                SingleState.put("Tilt_Angular_Acc_Raw",value);
                break;
            case 9:
                SingleState.put("Auto_Trans_BrakePressure",value);
                break;
            case 0x0A:
                SingleState.put("Auto_Trans_Acceleration",value);
                break;
            case 0x0B:
                //Log.d("LYY3","Steer"+res+"//"+buf[8]+buf[9]+buf[10]+buf[11]);
                SingleState.put("SteerAngle",value);
                break;

        }




    }

    public static void UpdataOBD(byte[] buf){

        float res = 0.0f;
        int pid = 0;
        int sid = 0;

        int[] as = new int[5];
        ByteBuffer.wrap(buf).order(ByteOrder.LITTLE_ENDIAN).asIntBuffer().get(as);
        sid = as[0];
        if(sid!=0x01){
            return;
        }

        pid = as[1];
        res = (float) as[2] / 1000.0f;
        double value = (double)res;
        switch (pid){
            case 0x0D:
                SingleState.put("Vehicle_Speed",value);
                break;
            case 0x0C:
                SingleState.put("RPM",value);
                break;
            case 0x05:
                SingleState.put("Coolant temperature",value);
                break;
            case 0x0F:
                SingleState.put("Inlet temperature",value);
                break;
            case 0x10:
                SingleState.put("Intake flow rate",value);
                break;
            case 0x0B:
                SingleState.put("Manifold absolute pressure",value);
                break;
            case 0x11:
                SingleState.put("Throttle position",value);
                break;
        }
    }

    public static void UpdateUDS(String id,byte[] data){
        //System.out.println("000");
        switch (id) {
            case "07E8":
                if(data[0]<=0x07&&data[1]==0x62)
                {
                    if(data[2]==(byte) 0xF4 && data[3]==(byte) 0x0D){ // 速度
                        int speed = (data[4] & 0xFF);
                        double v_speed = (double)speed;

                        SingleState.put("Vehicle_Speed",v_speed);
                    }

                    if(data[2]==(byte) 0x12 && data[3]==(byte) 0xB0){   // 加速度
                        int Acceleration = (data[4])<<8 |(data[5]);
                        double v_Acceleration = ((double)Acceleration)*0.001;
                        //v_Acceleration = (Math.round(v_Acceleration * 1000000));
                        //v_Acceleration = v_Acceleration/1000000;
                        Window.Acceleration_List.add(v_Acceleration);
                        SingleState.put("Acceleration",v_Acceleration);
                        Log.d("Acceleration", "Acceleration:"+v_Acceleration);

                    }

                    if(data[2]==(byte) 0x16 && data[3]==(byte) 0x33){ // 纵向加速度
                        int LAcceleration = (data[4])<<8 |(data[5]);
                        double v_LAcceleration = ((double)LAcceleration)*0.002;
                        SingleState.put("LongitudinalAcceleration",v_LAcceleration);
                    }

                    if(data[2]==(byte) 0x15 && data[3]==(byte) 0xD1){     // 横向加速度
                        int lateralAcc = (data[4])<<8 |(data[5]);
                        double v_lateralAcc = ((double)lateralAcc) * 0.01;
                        v_lateralAcc = (Math.round(v_lateralAcc * 1000000));
                        v_lateralAcc = v_lateralAcc/1000000;
                        Window.LateralAcc_List.add(v_lateralAcc);
                        SingleState.put("LateralAcceleration", v_lateralAcc);
                        Log.d("LateralAcceleration", "LateralAcceleration:"+v_lateralAcc);
                    }

                    if(data[2]==(byte) 0xF4 && data[3]==(byte) 0x0C){ //发动机转速
                        int rpm = (data[4])<<8 |(data[5]);
                        double v_rpm = ((double)rpm)/4;
                        SingleState.put("RPM",v_rpm);
                    }

                }

                break;
            case "0778":
                break;

            case "07E9":
                if(data[0]<=0x07&&data[1]==0x62)
                {
                    if(data[2]==(byte) 0x38&&data[3]==(byte) 0x03){
                        int BrakePressure = (data[4])<<8 |(data[5]);
                        double v_BrakePressure = ((double)BrakePressure)/30000.0;
                        SingleState.put("BrakePressure",v_BrakePressure);
                        //Log.d("ST","BP:"+v_BrakePressure);
                    }

                }
                break;

            case "077C":
                if(data[0]<=0x07 && data[1]==0x62)
                {
                    if(data[2]==(byte) 0x18 && data[3]==(byte) 0x12){
                        int SteerAngle = (data[4] ) << 8 | (data[5]  );
                        double num_SA = ((double)SteerAngle) * 0.1;
                        num_SA = (Math.round(num_SA * 1000000));
                        num_SA = num_SA/1000000;
                        Window.SteerAngle_List.add(num_SA);
                        SingleState.put("SteerAngle",num_SA);
                        Log.d("SteerAngle", "Steer Angle:"+num_SA);

                    }

                }
                break;

        }
    }

    public boolean DeleteState(String s_name){
        if(SingleState.containsKey(s_name)){
            SingleState.remove(s_name);
            return true;
        }
        else {
            Log.d(TAG,"No such State");
            return false;
        }
    }



}
