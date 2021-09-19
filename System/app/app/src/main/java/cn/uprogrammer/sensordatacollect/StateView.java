package cn.uprogrammer.sensordatacollect;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;

import cn.uprogrammer.sensordatacollect.view.InformationView;
import cn.uprogrammer.sensordatacollect.view.RpmIndicatorView;
import cn.uprogrammer.sensordatacollect.view.SpeedIndicatorView;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

public class StateView extends AppCompatActivity {

    private InformationView mInformationView;
    private RpmIndicatorView mRpmIndicator;
    private SpeedIndicatorView mSpeedIndicator;

    private TimerTask task;
    private Timer timer = new Timer();
    private Handler handler;
    private Map statemap;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_state_view);

        //setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);


        Intent intent = new Intent(StateView.this, IPSService.class);
        startService(intent);
        Log.d("CANAPP", "Entering CANIPS");



        statemap = StateTables.SingleState;

        handler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                double speed = (double)statemap.get("Vehicle_Speed");
                double rpm = (double)statemap.get("RPM");
                int rSpeed = (int) Math.round(speed);
                int rRPM = (int) Math.round(rpm);
                //Log.d("Speed:",""+rSpeed);
                //刷新图表
                //Log.d("RPM:",""+rRPM);
                updateRpm(rRPM);
                updateSpeed(rSpeed);
                super.handleMessage(msg);
            }
        };

        task = new TimerTask() {
            @Override
            public void run() {
                Message message = new Message();
                message.what = 200;
                handler.sendMessage(message);
            }
        };
        timer.schedule(task, 500,300);

        mInformationView = findViewById(R.id.main_information_view);
        mRpmIndicator = findViewById(R.id.main_riv);
        mSpeedIndicator = findViewById(R.id.main_siv);

        final ImageView bottom = findViewById(R.id.click);

        bottom.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                Intent data_intent = new Intent();
                data_intent.setClass(StateView.this,UI.class);
                data_intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                startActivity(data_intent);
                return true;
            }
        });


        mInformationView.startTimer();
    }



    @Override
    protected void onDestroy() {

        mInformationView.stopTimer();
        timer.cancel();
        Intent intent = new Intent(StateView.this, IPSService.class);
        stopService(intent);

        super.onDestroy();

    }

    public void updateSpeed(int speed) {
        mSpeedIndicator.setSpeed(speed);
    }

    public void updateRpm(int rpm) {
        mRpmIndicator.setValue(rpm);
        mInformationView.setRpm(rpm);
    }




}


