package cn.uprogrammer.sensordatacollect;

public class StateValue {
    //orientation
    public static final double Go_Straight = 0;
    public static final double Turn_Left = 1;
    public static final double Turn_Right = 2;

    //Gradient
    public static final double Horizontal = 0;
    public static final double Upslope = 1;
    public static final double Downslope = 2;

    //Window
    public static final double Window_Open = 1;
    public static final double Window_Close = 0;

    //Door
    public static final double Door_Close = 0;
    public static final double Door_Open = 1;

    public static final int NOT_IN_CAR = -1;
    public static final int NOT_MALICIOUS = 0;
    public static final int IS_MALICIOUS = 1;

    //Special status
    public static final double DEFAULT = -1;
    public static final double Driving_Event = 0;
    public static final double Straight_Forward_Driving = 1;


    public static final double Left_Turn = 2;
    public static final double Right_Turn = 3;
    public static final double Left_Lane_Change = 4;
    public static final double Right_Lane_Change = 5;
    public static final double Left_U_Turn = 6;
    public static final double Right_U_Turn = 7;
    public static final double Braking = 8;
    public static final double STOP = 9;
    public static final double Brake = 10;
    public static final double Highway_Driving = 11;
    public static final double Bump = 12;

    public static final double L1_Acc = 13;
    public static final double L2_Acc = 14;
    public static final double L3_Acc = 15;






    public static final double Under_Abnormal_Event = -3;

    public static final double NORMAL = -1;


}
