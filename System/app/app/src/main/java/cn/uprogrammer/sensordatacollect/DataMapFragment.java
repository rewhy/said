package cn.uprogrammer.sensordatacollect;

import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import org.achartengine.ChartFactory;
import org.achartengine.GraphicalView;
import org.achartengine.chart.PointStyle;
import org.achartengine.model.TimeSeries;
import org.achartengine.model.XYMultipleSeriesDataset;
import org.achartengine.model.XYSeries;
import org.achartengine.renderer.XYMultipleSeriesRenderer;
import org.achartengine.renderer.XYSeriesRenderer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.graphics.Paint.Align;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.RelativeLayout;
import android.widget.TextView;

/**
 * A placeholder fragment containing a simple view.
 */
public class DataMapFragment extends Fragment {

    private String tag = "test";
    private Handler handler;
    private XYSeries series;
    private XYMultipleSeriesDataset mDataset;
    private GraphicalView chart;
    private XYMultipleSeriesRenderer renderer0 = new XYMultipleSeriesRenderer();
    private final int POINT_GENERATE_PERIOD=50; //单位是ms
    RelativeLayout breathWave;
    private List<Double> drawPack = new ArrayList(); // 需要绘制的数据集
    private TextView tv;
    private Context context;
    private static final int MAX_POINT = 100;


    private int addX = -1;
    int[] xv = new int[MAX_POINT];
    int[] yv = new int[MAX_POINT];
    int i = 0;
    //example
    Random random = new Random();

    private static final int SERIES_NR=1;
    private TimeSeries series1;


    private TimerTask task;

    private Timer timer = new Timer();
    private Map statemap;
    Bundle bundle;
    String state;
    String key;
    double YMAX = 1000;
    double YMIN = 0;

    public DataMapFragment() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        statemap = StateTables.SingleState;

        bundle = this.getArguments();
        state = bundle.getString("state");

        switch (state){
            case "SPEED":
                key = "Vehicle_Speed";
                YMAX = 260;
                break;
            case "RPM":
                key = "RPM";
                YMAX = 16000;
                break;
            case "Coolant temperature":
                key = "Coolant temperature";
                YMAX = 230;
                break;
            case "Inlet temperature":
                key = "Inlet temperature";
                YMAX = 230;
                break;
            case "Intake flow rate":
                key = "Intake flow rate";
                YMAX = 680;
                break;
            case "Manifold absolute pressure":
                key = "Manifold absolute pressure";
                YMAX = 260;
                break;
            case "Throttle position":
                key = "Throttle position";
                YMAX = 260;
                break;
            case "Acceleration":
                key = "Acceleration";
                YMAX = 15;
                YMIN = -10;
                break;
            case "BrakePressure":
                key = "BrakePressure";
                YMAX = 100;
                break;
            case "LateralAcceleration":
                key = "LateralAcceleration";
                YMAX = 15;
                YMIN = -10;
                break;
            case "LongitudinalAcceleration":
                key = "LongitudinalAcceleration";
                YMAX = 15;
                YMIN = -10;
                break;
            case "Tilt_Angular_Acc_Raw":
                key = "Tilt_Angular_Acc_Raw";
                YMAX = 10;
                break;
            case "SteerAngle":
                key = "SteerAngle";
                YMAX = 400;
                YMIN = -400;
                break;

        }

        handler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                ArrayList<Integer> datas = new ArrayList();

                double value = (double)statemap.get(key);
                int rValue = (int) Math.round(value);

                datas.add(rValue);

                //刷新图表
                updateCharts(datas);
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
        timer.schedule(task, 0,300);

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        //return inflater.inflate(R.layout.fragment_data_map, container, false);
        View view = inflater.inflate(R.layout.fragment_data_map, container, false);

        breathWave = view.findViewById(R.id.cardiograph);
        tv = view.findViewById(R.id.test);
        initCardiograph();
        return view;
    }

    public void recievedMessage(String action){
        if (action.equals("START")) {
            //handler.postDelayed(runnable, POINT_GENERATE_PERIOD);
        } else if (action.equals("STOP")) {
            Log.d("MAP", "recieved Stop !");
            handler.removeCallbacksAndMessages(null);

        }
    }

    private XYMultipleSeriesDataset getDateDemoDataset() {
        XYMultipleSeriesDataset dataset1 = new XYMultipleSeriesDataset();
        final int nr = 10;
        long value = new Date().getTime();
        Random r = new Random();
        for (int i = 0; i < SERIES_NR; i++) {
            series1 = new TimeSeries("Demo series " + (i + 1));
            for (int k = 0; k < nr; k++) {
                series1.add(new Date(value+k*1000), 20 +r.nextInt() % 10);
            }
            dataset1.addSeries(series1);
        }
        Log.i("DEMO", dataset1.toString());
        return dataset1;
    }
    private XYMultipleSeriesRenderer getDemoRenderer() {
        XYMultipleSeriesRenderer renderer = new XYMultipleSeriesRenderer();
        renderer.setChartTitle("实时曲线");//标题
        renderer.setChartTitleTextSize(20);
        renderer.setXTitle("时间");    //x轴说明
        renderer.setAxisTitleTextSize(16);
        renderer.setAxesColor(Color.BLACK);
        renderer.setLabelsTextSize(15);    //数轴刻度字体大小
        renderer.setLabelsColor(Color.BLACK);
        renderer.setLegendTextSize(15);    //曲线说明
        renderer.setXLabelsColor(Color.BLACK);
        renderer.setYLabelsColor(0,Color.BLACK);
        renderer.setShowLegend(false);
        renderer.setMargins(new int[] {20, 30, 100, 0});
        XYSeriesRenderer r = new XYSeriesRenderer();
        r.setColor(Color.BLUE);
        r.setChartValuesTextSize(15);
        r.setChartValuesSpacing(3);
        r.setPointStyle(PointStyle.CIRCLE);
        r.setFillPoints(true);
        renderer.addSeriesRenderer(r);
        renderer.setMarginsColor(Color.WHITE);
        renderer.setPanEnabled(false,false);
        renderer.setShowGrid(true);
        renderer.setYAxisMax(50);
        renderer.setYAxisMin(-30);
        renderer.setInScroll(true);  //调整大小
        return renderer;
    }


    public void initCardiograph() {
        context = getActivity().getApplicationContext();
        //context = getActivity();
        // 这个类用来放置曲线上的所有点，是一个点的集合，根据这些点画出曲线
        series = new XYSeries(tag);

        // 创建一个数据集的实例，这个数据集将被用来创建图表
        mDataset = new XYMultipleSeriesDataset();
        // 将点集添加到这个数据集中
        mDataset.addSeries(series);

        // 以下都是曲线的样式和属性等等的设置，renderer相当于一个用来给图表做渲染的句柄
        /* int color = Color.parseColor("#08145e"); */
        int color = getResources().getColor(R.color.cardio_color3);
        PointStyle style = PointStyle.CIRCLE;
        //buildRenderer(color, style, true);
        //buildRenderer(getResources().getColor(R.color.cardio_color2), style, true);
        // 设置好图表的样式

        renderer0=setChartSettings( color, style, true,"X", "Y", YMIN, 100, 0, YMAX, color,
                color);
        // 生成图表
        chart = ChartFactory.getLineChartView(context, mDataset, renderer0);
        chart.setBackgroundColor(getResources().getColor(
                R.color.cardio_bg_color));
        breathWave.removeAllViews();
        breathWave.addView(chart);

    }

    protected XYMultipleSeriesRenderer setChartSettings(int color, PointStyle style, boolean fill,String xTitle, String yTitle, double xMin, double xMax,
                                    double yMin, double yMax, int axesColor, int labelsColor) {
        XYMultipleSeriesRenderer renderer = new XYMultipleSeriesRenderer();

        XYSeriesRenderer r = new XYSeriesRenderer();
        // 设置图表中曲线本身的样式，包括颜色、点的大小以及线的粗细等
        r.setColor(color);
        r.setPointStyle(style);
        r.setFillPoints(fill);
        r.setLineWidth(3);
        renderer.addSeriesRenderer(r);

        // 有关对图表的渲染可参看api文档
        renderer.setApplyBackgroundColor(true);
        renderer.setBackgroundColor(getResources().getColor(
                R.color.black));
        renderer.setChartTitle(tag);
        renderer.setChartTitleTextSize(20);
        renderer.setLabelsTextSize(19);// 设置坐标轴标签文字的大小
        renderer.setXTitle(xTitle);
        renderer.setYTitle(yTitle);
        renderer.setXAxisMin(xMin);
        renderer.setXAxisMax(xMax);
        renderer.setYAxisMin(yMin);
        renderer.setYAxisMax(yMax);
        //renderer.setYAxisAlign(Align.RIGHT, 0);//用来调整Y轴放置的位置，表示将第一条Y轴放在右侧
        renderer.setAxesColor(axesColor);
        renderer.setLabelsColor(labelsColor);
        renderer.setShowGrid(true);
        renderer.setGridColor(Color.GRAY);
        renderer.setXLabels(10);//若不想显示X标签刻度，设置为0 即可
        renderer.setYLabels(10);
        renderer.setLabelsTextSize(18);// 设置坐标轴标签文字的大小
        renderer.setXLabelsColor(labelsColor);
        renderer.setYLabelsColor(0, labelsColor);
        renderer.setYLabelsVerticalPadding(-5);
        renderer.setXTitle("");
        renderer.setYTitle("");
        renderer.setYLabelsAlign(Align.RIGHT);
        renderer.setAxisTitleTextSize(20);
        renderer.setPointSize((float) 1);
        renderer.setShowLegend(false);
        renderer.setFitLegend(true);
        renderer.setMargins(new int[] { 30, 45, 10, 20 });// 设置图表的外边框(上/左/下/右)
        //表格边框颜色
        renderer.setMarginsColor(getResources().getColor(
                R.color.cardio_bg_color));

        return renderer;
    }

    /**
     * @Title leftUpdateCharts
     * @Description 新生成的点一直在左侧，产生向右平移的效果， 基于X轴坐标从0开始，然后递减的思想处理
     * @param datas
     * @return void
     */
    protected void  leftUpdateCharts(ArrayList<Integer> datas) {
        for (int addY : datas) {
            series.add(i, addY);
            i--;
        }
        if (Math.abs(i) < MAX_POINT) {
            renderer0.setXAxisMin(-MAX_POINT);
            renderer0.setXAxisMax(0);
        } else {
            renderer0.setXAxisMin(-series.getItemCount());
            renderer0.setXAxisMax(-series.getItemCount() + MAX_POINT);
        }

        chart.repaint();
    }

    /**
     * @Title updateCharts
     * @Description 新生成的点一直在右侧，产生向左平移的效果，基于X轴坐标从0开始，然后递加的思想处理
     * @param datas
     * @return void
     */
    protected void updateCharts(ArrayList<Integer> datas) {
        for (int addY : datas) {
            series.add(i, addY);
            i++;
        }
        if (i < MAX_POINT) {
            renderer0.setXAxisMin(0);
            renderer0.setXAxisMax(MAX_POINT);
        } else {
            renderer0.setXAxisMin(series.getItemCount() - MAX_POINT);
            renderer0.setXAxisMax(series.getItemCount());
        }
        chart.repaint();
    }

    /**
     * @Title updateChart
     * @Description 新生成的点一直在x坐标为0处，因为将所有旧点的x坐标值加1，所以产生向右平移的效果
     * @param addY
     * @return void
     */
    private void updateChart(int addY) {

        // 设置好下一个需要增加的节点
        addX = 0;
        // 移除数据集中旧的点集
        mDataset.removeSeries(series);
        // 判断当前点集中到底有多少点，因为屏幕总共只能容纳MAX_POINT个，所以当点数超过MAX_POINT时，长度永远是MAX_POINT
        int length = series.getItemCount();
        if (length > MAX_POINT) {
            length = MAX_POINT;
        }
        // 将旧的点集中x和y的数值取出来放入backup中，并且将x的值加1，造成曲线向右平移的效果
        for (int i = 0; i < length; i++) {
            xv[i] = (int) series.getX(i) + 1;
            yv[i] = (int) series.getY(i);
        }
        // 点集先清空，为了做成新的点集而准备
        series.clear();
        // 将新产生的点首先加入到点集中，然后在循环体中将坐标变换后的一系列点都重新加入到点集中
        // 这里可以试验一下把顺序颠倒过来是什么效果，即先运行循环体，再添加新产生的点
        series.add(addX, addY);
        for (int k = 0; k < length; k++) {
            series.add(xv[k], yv[k]);
        }

        // 在数据集中添加新的点集
        mDataset.addSeries(series);
        // 视图更新，没有这一步，曲线不会呈现动态
        // 如果在非UI主线程中，需要调用postInvalidate()，具体参考api
        //chart.invalidate();
        chart.repaint();
    }

    /**
     * @Title rightUpdateChart
     * @Description 新生成的点一直在x坐标为MAX_POINT处，因为将所有旧点的x坐标值减1，所以产生向左平移的效果，无法看到历史数据点
     * @param addY
     * @return void
     */
    private void rightUpdateChart(int addY) {

        // 设置好下一个需要增加的节点
        addX =MAX_POINT;
        // 移除数据集中旧的点集
        mDataset.removeSeries(series);
        // 判断当前点集中到底有多少点，因为屏幕总共只能容纳MAX_POINT个，所以当点数超过MAX_POINT时，长度永远是MAX_POINT
        int length = series.getItemCount();
        if (length > MAX_POINT) {
            length = MAX_POINT;
        }
        // 将旧的点集中x和y的数值取出来放入backup中，并且将x的值-1，造成曲线向左平移的效果
        for (int i = 0; i < length; i++) {
            xv[i] = (int) series.getX(i) - 1;
            yv[i] = (int) series.getY(i);
        }
        // 点集先清空，为了做成新的点集而准备
        series.clear();
        // 将新产生的点首先加入到点集中，然后在循环体中将坐标变换后的一系列点都重新加入到点集中
        // 这里可以试验一下把顺序颠倒过来是什么效果，即先运行循环体，再添加新产生的点
        series.add(addX, addY);
        for (int k = 0; k < length; k++) {
            series.add(xv[k], yv[k]);
        }

        // 在数据集中添加新的点集
        mDataset.addSeries(series);
        // 视图更新，没有这一步，曲线不会呈现动态
        // 如果在非UI主线程中，需要调用postInvalidate()，具体参考api
        //chart.invalidate();
        chart.repaint();
    }


}
