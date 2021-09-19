package cn.uprogrammer.sensordatacollect;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.Typeface;
import android.hardware.Sensor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import cn.uprogrammer.sensordatacollect.Gps.GPSView;
import cn.uprogrammer.sensordatacollect.Sensor.showSensor;
import cn.uprogrammer.sensordatacollect.classifyUi.stateUI;
import cn.uprogrammer.sensordatacollect.IPSService;
import cn.uprogrammer.sensordatacollect.R;

public class UI extends AppCompatActivity {
    private static String TAG = "CANAPP";

    private ListView listView;

    Handler handler;
    private Map statemap;
    int OBDSTATE = 2;
    private Toast warnToast;
    private Toast filterToast;


    private String[] name;
    private String[] state;

    SimpleAdapter simpleAdapter;

    //OBD State
    private String[] obd_name = {"Straight Driving", "Special Vehicle Status", "Speed", "RPM", "Coolant temperature", "Inlet temperature", "Intake flow rate", "Manifold absolute pressure", "Throttle position"};
    private String[] obd_state = {"Straight Driving", "Special Vehicle Status", "Vehicle_Speed", "RPM", "Coolant temperature", "Inlet temperature", "Intake flow rate", "Manifold absolute pressure", "Throttle position"};

    private String[] uds_name = {"Straight Driving", "Special Vehicle Status", "SteerAngle", "Speed", "RPM", "Acceleration", "BrakePressure", "LateralAcceleration", "LongitudinalAcceleration", "Tilt_Angular_Acc_Raw"};
    private String[] uds_state = {"Straight Driving", "Special Vehicle Status", "SteerAngle", "Vehicle_Speed", "RPM", "Acceleration", "BrakePressure", "LateralAcceleration", "LongitudinalAcceleration", "Tilt_Angular_Acc_Raw"};


    private int size;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.activity_ui);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        //getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        //Intent intent = new Intent(UI.this, IPSService.class);
        //startService(intent);
        //Log.d(TAG, "Entering CANIPS");
        if (OBDSTATE == 1) {
            name = obd_name;
            state = obd_state;
            size = 9;

        } else {
            name = uds_name;
            state = uds_state;
            size = 10;
        }


        listView = findViewById(R.id.list_view);


        final Map<String, Object>[] item_list = new Map[size];
        List<Map<String, Object>> data = new ArrayList<Map<String, Object>>();

        for (int no = 0; no < size; no++) {
            item_list[no] = new HashMap<String, Object>();
            item_list[no].put("image", R.drawable.arrow);
            item_list[no].put("name", name[no]);
            item_list[no].put("value", "0");
            data.add(item_list[no]);
        }


        simpleAdapter = new SimpleAdapter(this, data,
                R.layout.ui_item, new String[]{"image", "name", "value"},
                new int[]{R.id.Icon1, R.id.Name, R.id.Value});
        //ArrayAdapter<String> adapter = new ArrayAdapter<String>(UI.this, android.R.layout.simple_list_item_1, data);

        listView.setAdapter(simpleAdapter);

        statemap = StateTables.SingleState;

        handler = new Handler() {
            @Override
            public void handleMessage(Message msg) {

                //SHOW ITEM LIST
                for (int i = 0; i < size; i++) {
                    double value = (double) statemap.get(state[i]);
                    //Log.d("LYY01",""+statemap.get("Acceleration"));
                    String show = "";

                    if (i == 0) {
                        Double d_value = new Double(value);
                        int i_value = d_value.intValue();
                        switch (i_value) {
                            case (int) StateValue.Driving_Event:
                                show = "Not Straight";
                                break;
                            case (int) StateValue.Straight_Forward_Driving:
                                show = "Straight";
                                break;
                            default:
                                show = "MODEL OUT OF CONTROL";
                        }
                    } else if (i == 1) {
                        Double d_value = new Double(value);
                        //Log.d("LYY11",""+d_value);
                        int i_value = d_value.intValue();
                        //Log.d("LYY11",""+i_value+"/"+(int)value);
                        switch (i_value) {
                            case (int) StateValue.DEFAULT:
                                show = "Not Abnormal Status";
                                break;
                            case (int) StateValue.Left_Turn:
                                show = "Left_Turn";
                                break;
                            case (int) StateValue.Right_Turn:
                                show = "Right_Turn";
                                break;
                            case (int) StateValue.Left_Lane_Change:
                                show = "Left_Lane_Change";
                                break;
                            case (int) StateValue.Right_Lane_Change:
                                show = "Right_Lane_Change";
                                break;
                            case (int) StateValue.Left_U_Turn:
                                show = "Left_U_Turn";
                                break;
                            case (int) StateValue.Right_U_Turn:
                                show = "Right_U_Turn";
                                break;
                            case (int) StateValue.Braking:
                                show = "Braking";
                                break;
                            case (int) StateValue.STOP:
                                show = "STOP";
                                break;
                            case (int) StateValue.Brake:
                                show = "Brake";
                                break;
                            case (int) StateValue.Highway_Driving:
                                show = "Driving on HighWay";
                                break;
                            case (int) StateValue.Bump:
                                show = "Bump";
                                break;
                            case (int) StateValue.L1_Acc:
                                show = "L1 Acceleration";
                                break;
                            case (int) StateValue.L2_Acc:
                                show = "L2 Acceleration";
                                break;
                            case (int) StateValue.L3_Acc:
                                show = "L3 Acceleration";
                                break;


                            case (int) StateValue.Under_Abnormal_Event:
                                show = "Under Abnormal Status";
                                break;
                            default:
                                show = "MODEL OUT OF CONTROL";
                        }

                    } else {
                        show = String.valueOf(value);

                    }

                    item_list[i].put("value", show);
                }


                simpleAdapter.notifyDataSetChanged();

                //

                showWarnToast();
                showFilterToast();
            }
        };
        listenState();


        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            //ListView的列表项的单击事件
            @Override
            //第一个参数：指的是这个ListView；第二个参数：当前单击的那个item
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                System.out.println("position=" + position);
                System.out.println("id=" + id);

                //既然当前点击的那个item是一个TextView，那我们可以将其强制转型为TextView类型，然后通过getText()方法取出它的内容,紧接着以吐司的方式显示出来

                if (OBDSTATE == 1) {
                    switch (position) {
                        case 0:
                            Intent data_intent0 = new Intent();
                            data_intent0.setClass(UI.this, stateUI.class);
                            data_intent0.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent0);
                            break;
                        case 1:
                            Intent data_intent00 = new Intent();
                            data_intent00.setClass(UI.this, stateUI.class);
                            data_intent00.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent00);
                            break;

                        case 2:
                            Intent data_intent = new Intent();
                            data_intent.setClass(UI.this, DataMap.class);
                            data_intent.putExtra("state", "SPEED");
                            data_intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent);
                            break;
                        case 3:
                            Intent data_intent1 = new Intent();
                            data_intent1.setClass(UI.this, DataMap.class);
                            data_intent1.putExtra("state", "RPM");
                            data_intent1.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent1);
                            break;
                        case 4:
                            Intent data_intent2 = new Intent();
                            data_intent2.setClass(UI.this, DataMap.class);
                            data_intent2.putExtra("state", "Coolant temperature");
                            data_intent2.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent2);
                            break;
                        case 5:
                            Intent data_intent3 = new Intent();
                            data_intent3.setClass(UI.this, DataMap.class);
                            data_intent3.putExtra("state", "Inlet temperature");
                            data_intent3.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent3);
                            break;
                        case 6:
                            Intent data_intent4 = new Intent();
                            data_intent4.setClass(UI.this, DataMap.class);
                            data_intent4.putExtra("state", "Intake flow rate");
                            data_intent4.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent4);
                            break;
                        case 7:
                            Intent data_intent5 = new Intent();
                            data_intent5.setClass(UI.this, DataMap.class);
                            data_intent5.putExtra("state", "Manifold absolute pressure");
                            data_intent5.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent5);
                            break;
                        case 8:
                            Intent data_intent6 = new Intent();
                            data_intent6.setClass(UI.this, DataMap.class);
                            data_intent6.putExtra("state", "Throttle position");
                            data_intent6.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent6);
                            break;

                    }
                } else {
                    switch (position) {
                        case 0:
                            Intent data_intent0 = new Intent();
                            data_intent0.setClass(UI.this, stateUI.class);
                            data_intent0.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent0);
                            break;
                        case 1:
                            Intent data_intent00 = new Intent();
                            data_intent00.setClass(UI.this, stateUI.class);
                            data_intent00.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent00);
                            break;
                        case 2:
                            Intent data_intent7 = new Intent();
                            data_intent7.setClass(UI.this, DataMap.class);
                            data_intent7.putExtra("state", "SteerAngle");
                            data_intent7.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent7);
                            break;
                        case 3:
                            Intent data_intent = new Intent();
                            data_intent.setClass(UI.this, DataMap.class);
                            data_intent.putExtra("state", "SPEED");
                            data_intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent);
                            break;
                        case 4:
                            Intent data_intent1 = new Intent();
                            data_intent1.setClass(UI.this, DataMap.class);
                            data_intent1.putExtra("state", "RPM");
                            data_intent1.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent1);
                            break;
                        case 5:
                            Intent data_intent2 = new Intent();
                            data_intent2.setClass(UI.this, DataMap.class);
                            data_intent2.putExtra("state", "Acceleration");
                            data_intent2.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent2);
                            break;
                        case 6:
                            Intent data_intent3 = new Intent();
                            data_intent3.setClass(UI.this, DataMap.class);
                            data_intent3.putExtra("state", "BrakePressure");
                            data_intent3.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent3);
                            break;
                        case 7:
                            Intent data_intent4 = new Intent();
                            data_intent4.setClass(UI.this, DataMap.class);
                            data_intent4.putExtra("state", "LateralAcceleration");
                            data_intent4.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent4);
                            break;
                        case 8:
                            Intent data_intent5 = new Intent();
                            data_intent5.setClass(UI.this, DataMap.class);
                            data_intent5.putExtra("state", "LongitudinalAcceleration");
                            data_intent5.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent5);
                            break;
                        case 9:
                            Intent data_intent6 = new Intent();
                            data_intent6.setClass(UI.this, DataMap.class);
                            data_intent6.putExtra("state", "Tilt_Angular_Acc_Raw");
                            data_intent6.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                            startActivity(data_intent6);
                            break;
                    }

                }


            }

        });
    }

    private void showFilterToast() {
        int value = StateTables.Decison;
        if (value < 0) {
            //if(filterToast!=null){
            //filterToast.cancel();
            //}
            return;
        }

        String warn = "The action " + StateTables.Action + " is " + ActionValue.DECISION[value];

        if (filterToast == null) {


            filterToast = new Toast(getApplicationContext());
            filterToast.setGravity(Gravity.CENTER, 0, 0);
            ImageView image = new ImageView(getApplicationContext());
            image.setImageResource(R.drawable.filter);
            // 创建一个LinearLayout容器
            LinearLayout ll = new LinearLayout(getApplicationContext());
            // 向LinearLayout中添加图片、原有的View
            ll.addView(image);
            // 创建一个ImageView
            final TextView textView = new TextView(getApplicationContext());
            textView.setText(warn);
            // 设置文本框内字体的大小和颜色
            textView.setTextSize(32);
            textView.setTextColor(Color.RED);
            ll.addView(textView);
            // 设置Toast显示自定义View	，Toast里面可以设置View
            filterToast.setView(ll);
            // 设置Toast的显示时间
            filterToast.setDuration(Toast.LENGTH_LONG);
        } else {
            filterToast.cancel();

            filterToast = new Toast(getApplicationContext());
            filterToast.setGravity(Gravity.CENTER, 0, 0);
            ImageView image = new ImageView(getApplicationContext());
            image.setImageResource(R.drawable.filter);
            // 创建一个LinearLayout容器
            LinearLayout ll = new LinearLayout(getApplicationContext());
            // 向LinearLayout中添加图片、原有的View
            ll.addView(image);
            // 创建一个ImageView
            final TextView textView = new TextView(getApplicationContext());
            textView.setText(warn);
            // 设置文本框内字体的大小和颜色
            textView.setTextSize(32);
            textView.setTextColor(Color.RED);
            ll.addView(textView);
            // 设置Toast显示自定义View	，Toast里面可以设置View
            filterToast.setView(ll);
            // 设置Toast的显示时间
            filterToast.setDuration(Toast.LENGTH_LONG);
        }
        filterToast.show();
        StateTables.Decison = -1;

    }

    private void showWarnToast() {
        double value = StateTables.SingleState.get("Warning Message");
        if (value < 0) {
            if (warnToast != null) {
                warnToast.cancel();
            }
            return;
        }
        int rValue = (int) Math.round(value);
        if(rValue>26){
            return;
        }
        String warn = ActionValue.Warning[rValue];
        //Log.d("LYY444",""+rValue);
        if (warnToast == null) {


            warnToast = new Toast(getApplicationContext());
            warnToast.setGravity(Gravity.BOTTOM, 0, 0);
            ImageView image = new ImageView(getApplicationContext());
            image.setImageResource(R.drawable.warn);
            // 创建一个LinearLayout容器
            LinearLayout ll = new LinearLayout(getApplicationContext());
            // 向LinearLayout中添加图片、原有的View
            ll.addView(image);
            // 创建一个ImageView
            final TextView textView = new TextView(getApplicationContext());
            textView.setText(warn);
            // 设置文本框内字体的大小和颜色
            textView.setTextSize(40);
            textView.setTextColor(Color.RED);
            ll.addView(textView);
            // 设置Toast显示自定义View	，Toast里面可以设置View
            warnToast.setView(ll);
            // 设置Toast的显示时间
            warnToast.setDuration(Toast.LENGTH_SHORT);
        } else {
            warnToast.cancel();

            //Log.d("LYY444",""+rValue);

            warnToast = new Toast(getApplicationContext());
            warnToast.setGravity(Gravity.BOTTOM, 0, 0);
            ImageView image = new ImageView(getApplicationContext());
            image.setImageResource(R.drawable.warn);
            // 创建一个LinearLayout容器
            LinearLayout ll = new LinearLayout(getApplicationContext());
            // 向LinearLayout中添加图片、原有的View
            ll.addView(image);
            // 创建一个ImageView
            final TextView textView = new TextView(getApplicationContext());
            textView.setText(warn);
            // 设置文本框内字体的大小和颜色
            textView.setTextSize(40);
            textView.setTextColor(Color.RED);
            ll.addView(textView);
            // 设置Toast显示自定义View	，Toast里面可以设置View
            warnToast.setView(ll);
            // 设置Toast的显示时间
            warnToast.setDuration(Toast.LENGTH_SHORT);
        }
        warnToast.show();

    }

    public volatile boolean exit = false;

    public void listenState() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (!exit) {

                    handler.sendEmptyMessage(0);
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }


    @Override
    protected void onDestroy() {
        exit = true;
        //Intent intent = new Intent(UI.this, IPSService.class);
        //stopService(intent);

        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_scrolling, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.Sensor_SHOW) {
            Intent data_intent2 = new Intent();
            data_intent2.setClass(UI.this, showSensor.class);
            data_intent2.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            startActivity(data_intent2);
            overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);
            return true;
        }

        //noinspection SimplifiableIfStatement

        if (id == R.id.GPS_SHOW) {

            Intent data_intent3 = new Intent();
            data_intent3.setClass(UI.this, GPSView.class);
            data_intent3.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            startActivity(data_intent3);
            overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);
            return true;
        }


        return super.onOptionsItemSelected(item);
    }


}