package cn.uprogrammer.sensordatacollect;

import android.widget.TextView;

public class GetView {
    static TextView time;
    static TextView rpm;
    public static void saveTimeView(TextView time0){
        time = time0;
    }

    public static void saveRPMView(TextView rpm0){
        rpm = rpm0;
    }

    public static TextView getTimeView(){
        return time;
    }

    public static TextView getRPMView(){
        return rpm;
    }
}
