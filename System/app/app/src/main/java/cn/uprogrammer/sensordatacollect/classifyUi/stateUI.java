package cn.uprogrammer.sensordatacollect.classifyUi;

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

import cn.uprogrammer.sensordatacollect.R;
import cn.uprogrammer.sensordatacollect.StateTables;
import cn.uprogrammer.sensordatacollect.StateValue;

public class stateUI extends AppCompatActivity {

    private ListView listView;

    Handler handler;
    private Map statemap;
    private int[] stateNum;
    private int size = 14;
    SimpleAdapter simpleAdapter;

    private String[] name = {"Straight", "Left_Turn", "Right_Turn", "Left_Lane_Change", "Right_Lane_Change", "Left_U_Turn", "Right_U_Turn", "STOP",
            "Brake", "Highway Driving", "Bump", "L1 Acceleration", "L2 Acceleration", "L3 Acceleration"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_state_ui);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        listView = findViewById(R.id.state_list_view);
        final Map<String, Object>[] item_list = new Map[size];
        List<Map<String, Object>> data = new ArrayList<Map<String, Object>>();

        for (int no = 0; no < size; no++) {
            item_list[no] = new HashMap<String, Object>();
            item_list[no].put("image", R.drawable.arrow);
            if (no == 0) {
                item_list[no].put("name", name[no]);
            } else if (no == 9) {
                item_list[no].put("name", name[no]);
            } else {
                item_list[no].put("name", name[no] + ":" + "0");
            }
            data.add(item_list[no]);
        }

        simpleAdapter = new SimpleAdapter(this, data,
                R.layout.state_item, new String[]{"image", "name"},
                new int[]{R.id.Icon2, R.id.Name2});
        //ArrayAdapter<String> adapter = new ArrayAdapter<String>(UI.this, android.R.layout.simple_list_item_1, data);

        listView.setAdapter(simpleAdapter);

        stateNum = StateTables.stateNumber;
        statemap = StateTables.SingleState;

        handler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                //SHOW ITEM LIST
                for (int i = 0; i < size; i++) {

                    String show = "";

                    //state
                    if (i == 0) {

//                        double val = (double)statemap.get("Straight Driving");
//                        Double d_value = new Double(val);
//                        int i_value = d_value.intValue();
//                        switch (i_value){
//                            case (int)StateValue.Driving_Event:
//                                show = "Not Straight";
//                                break;
//                            case (int)StateValue.Straight_Forward_Driving:
//                                show = "Straight";
//                                break;
//                            default:
//                                show = "MODEL OUT OF CONTROL";
//                        }

                        int val = StateTables.laneSignal;
                        switch (val) {
                            case (0):
                                show = "Straight";
                                break;
                            case (1):
                                show = "Left Action";
                                break;
                            case (2):
                                show = "Right Action";
                                break;
                            default:
                                show = "MODEL OUT OF CONTROL";
                        }


                    } else if (i == 9) {
                        int i_value = StateTables.stateNumber[8];
                        Log.d("Rule HW", "handleMessage: " + StateTables.stateNumber[8]);
                        switch (i_value) {
                            case 0:
                                show = "Not Driving on Highway";
                                break;
                            case 1:
                                show = "Driving on Highway";
                                break;
                        }
                    } else if (i == 7) {
                        int i_value = StateTables.stateNumber[7];
                        switch (i_value) {
                            case 0:
                                show = "Vehicle is STOP";
                                break;
                            case 1:
                                show = "Vehicle is Not STOP";
                                break;
                            default:
                                show = "System is initializing.";

                        }

                    } else {
                        int value = stateNum[i - 1];
                        show = name[i] + ":" + value;

                    }

                    item_list[i].put("name", show);
                }


                simpleAdapter.notifyDataSetChanged();
            }

        };
        listenState();

    }

    public volatile boolean exit = false;

    public void listenState() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (!exit) {

                    handler.sendEmptyMessage(0);
                    try {
                        Thread.sleep(500);
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

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }


}
