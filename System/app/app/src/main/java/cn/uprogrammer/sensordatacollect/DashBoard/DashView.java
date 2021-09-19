package cn.uprogrammer.sensordatacollect.DashBoard;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

import cn.uprogrammer.sensordatacollect.R;
import cn.uprogrammer.sensordatacollect.StateTables;

public class DashView extends AppCompatActivity {

    DashboardView dashboardView1;
    DashboardView dashboardView3;
    DashboardView dashboardView4;

    Handler handler;
    private Map statemap;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_dash_view);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);


        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });

        //初始化表盘
        dashboardView1 = findViewById(R.id.dashboard_view_2);
        dashboardView3 = findViewById(R.id.dashboard_view_3);
        dashboardView4 = findViewById(R.id.dashboard_view_4);



        initDash2();
        initDash3();
        initDash4();

        dashboardView1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dashboardView1.setRealTimeValue(150.f, true, 0);
                //dashboardView1.setRealTimeValue(150.f,true);
            }
        });

        dashboardView4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dashboardView4.setRealTimeValue(180.f, true, 0);
                //dashboardView1.setRealTimeValue(150.f,true);
            }
        });


        handler = new Handler(){
            @Override
            public void handleMessage(Message msg){

                //double speed = (double)statemap.get("Vehicle_Speed");
                float speed = (float) ((200.0) * new Random().nextFloat());
                Log.d("Dash",""+ speed);

                dashboardView4.setRealTimeValue( speed, true, 0);
            }
        };
        listenState();


    }

    public void initDash2(){
        List<HighlightCR> highlight1 = new ArrayList<>();
        highlight1.add(new HighlightCR(210, 60, Color.parseColor("#03A9F4")));
        highlight1.add(new HighlightCR(270, 60, Color.parseColor("#FFA000")));
        dashboardView1.setStripeHighlightColorAndRange(highlight1);
    }

    public void initDash3(){
        List<HighlightCR> highlight2 = new ArrayList<>();
        highlight2.add(new HighlightCR(170, 140, Color.parseColor("#607D8B")));
        highlight2.add(new HighlightCR(310, 60, Color.parseColor("#795548")));
        dashboardView3.setStripeHighlightColorAndRange(highlight2);
    }

    public void initDash4(){
        dashboardView4.setRadius(110);
        dashboardView4.setArcColor(getResources().getColor(android.R.color.black));
        dashboardView4.setTextColor(Color.parseColor("#212121"));
        dashboardView4.setBgColor(getResources().getColor(android.R.color.white));
        dashboardView4.setStartAngle(150);
        dashboardView4.setPointerRadius(80);
        dashboardView4.setCircleRadius(8);
        dashboardView4.setSweepAngle(240);
        dashboardView4.setBigSliceCount(12);
        dashboardView4.setMaxValue(260);
        dashboardView4.setRealTimeValue(0);
        dashboardView4.setMeasureTextSize(14);
        dashboardView4.setHeaderRadius(50);
        dashboardView4.setHeaderTitle("km/h");
        dashboardView4.setHeaderTextSize(16);
        dashboardView4.setStripeWidth(20);
        dashboardView4.setStripeMode(DashboardView.StripeMode.OUTER);
        List<HighlightCR> highlight3 = new ArrayList<>();
        highlight3.add(new HighlightCR(150, 100, Color.parseColor("#4CAF50")));
        highlight3.add(new HighlightCR(250, 80, Color.parseColor("#FFEB3B")));
        highlight3.add(new HighlightCR(330, 60, Color.parseColor("#F44336")));
        dashboardView4.setStripeHighlightColorAndRange(highlight3);
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
                        Thread.sleep(1000);
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
