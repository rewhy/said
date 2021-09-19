package cn.uprogrammer.sensordatacollect;

import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.TypedValue;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.util.Map;

public class DataMap extends AppCompatActivity {


    private TextView Data;

    private FragmentManager manager;
    FragmentTransaction transaction;

    Handler handler;
    private Map statemap;
    private String state;
    private String key;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_data_map);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        Intent intent = getIntent();
        state = intent.getStringExtra("state");


        manager = this.getSupportFragmentManager();

        //Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG).setAction("Action", null).show();
        DataMapFragment fragment = new DataMapFragment();

        Bundle bundle = new Bundle();
        bundle.putString("state", state);
        fragment.setArguments(bundle);

        transaction = manager.beginTransaction();
        transaction.add(R.id.fragments,fragment);
        transaction.commit();


/*
        button=(Button)this.findViewById(R.id.button2);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DataMapFragment fragment = new DataMapFragment();
                Intent startIntent=new Intent();
                startIntent.setAction("START");
                //sendBroadcast(startIntent);//通过广播的方式向fragment发送消息
                fragment.recievedMessage("START");//通过直接调用public方法，向fragment发送消息
            }
        });*/



        Data = this.findViewById(R.id.Data);

        Typeface typeface = Typeface.createFromAsset(getAssets(), "fonts/jefferies.otf");
        Data.setTypeface(typeface);
        Data.setTextSize(TypedValue.COMPLEX_UNIT_SP, 40);
        Data.setText("");


        switch (state){
            case "SPEED":
                key = "Vehicle_Speed";
                break;
            case "RPM":
                key = "RPM";
                break;
                default:
                key = state;
        }

        handler = new Handler(){
            @Override
            public void handleMessage(Message msg){
                statemap = StateTables.SingleState;
                StringBuilder sb = new StringBuilder();

                sb.append(state + ":    " + statemap.get(key) + "\n");
                Data.setText(sb.toString());

            }
        };
        listenState();



    }

    public static void getType(){

    }

    public volatile boolean exit = false;
    public void listenState(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                while(!exit){

                    handler.sendEmptyMessage(0);
                    try{
                        Thread.sleep(300);
                    }catch (InterruptedException e){
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        exit = true;

    }


}
