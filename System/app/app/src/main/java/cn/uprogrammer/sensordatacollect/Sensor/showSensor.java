package cn.uprogrammer.sensordatacollect.Sensor;

import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MenuItem;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

import cn.uprogrammer.sensordatacollect.DashBoard.DashboardView;
import cn.uprogrammer.sensordatacollect.DashBoard.HighlightCR;
import cn.uprogrammer.sensordatacollect.R;
import cn.uprogrammer.sensordatacollect.StateTables;


public class showSensor extends AppCompatActivity {

    Handler handler;
    private Map statemap;
    static String[] sensor_name = {"ACCX","ACCY","ACCZ","GYROX","GYROY","GYROZ"};
/*
    DashboardView dashboardView1;
    DashboardView dashboardView2;
    DashboardView dashboardView3;
    DashboardView dashboardView4;
    DashboardView dashboardView5;
    DashboardView dashboardView6;
*/
    DashboardView[] dashList = new DashboardView[6];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_show_sensor);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);


        dashList[0] = findViewById(R.id.dashboard_view_1);
        dashList[1] = findViewById(R.id.dashboard_view_2);
        dashList[2] = findViewById(R.id.dashboard_view_3);
        dashList[3] = findViewById(R.id.dashboard_view_4);
        dashList[4] = findViewById(R.id.dashboard_view_5);
        dashList[5]= findViewById(R.id.dashboard_view_6);

        initDash();

        statemap = StateTables.SingleState;
        handler = new Handler(){
            @Override
            public void handleMessage(Message msg){

                //double speed = (double)statemap.get("Vehicle_Speed");
                //Log.d("Dash",""+ speed);
                for(int j=0;j<6;j++){
                    double value =(double)statemap.get(sensor_name[j]);
                    dashList[j].setRealTimeDValue( value, true, 100);
                }


            }
        };
        listenState();
    }

    public void initDash(){
        List<HighlightCR> highlight1 = new ArrayList<>();
        highlight1.add(new HighlightCR(210, 60, Color.parseColor("#03A9F4")));
        highlight1.add(new HighlightCR(270, 60, Color.parseColor("#FFA000")));
        for(int i=0;i<6;i++){
            dashList[i].setStripeHighlightColorAndRange(highlight1);
        }
        /*
        dashboardView1.setStripeHighlightColorAndRange(highlight1);
        dashboardView2.setStripeHighlightColorAndRange(highlight1);
        dashboardView3.setStripeHighlightColorAndRange(highlight1);
        dashboardView4.setStripeHighlightColorAndRange(highlight1);
        dashboardView5.setStripeHighlightColorAndRange(highlight1);
        dashboardView6.setStripeHighlightColorAndRange(highlight1);*/


    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public volatile boolean exit = false;
    public void listenState(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                while(!exit){

                    handler.sendEmptyMessage(0);
                    try{
                        Thread.sleep(100);
                    }catch (InterruptedException e){
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        exit = true;

    }

}
