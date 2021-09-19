package cn.uprogrammer.sensordatacollect;

import android.hardware.Sensor;
import android.nfc.Tag;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;


import libsvm.svm;
import libsvm.svm_model;
import libsvm.svm_node;
import libsvm.svm_parameter;

import static cn.uprogrammer.sensordatacollect.StateTables.SingleState;
import static cn.uprogrammer.sensordatacollect.readFile.readFile.readToArray;
import static rfPrediction.DESF.DESF_filter;
import static rfPrediction.featureExtraction.*;
import cn.uprogrammer.sensordatacollect.readFile.parseData;



public class Window {

    public static String log_name;

    public static long Window_start = 0;
    public static long Window_size = 1000;//单位ms


    public static List Can0_List = new ArrayList();
    public static List Sensor_List = new ArrayList();
    public static List Gps_List = new ArrayList();
    public static List Vehicle_Speed_List = new ArrayList();

    /**
     * zkf
     **/
    public static List<Double> SteerAngle_List = new ArrayList();
    public static List<Double> Acceleration_List = new ArrayList();
    public static List<Double> LateralAcc_List = new ArrayList();
    /****/


    /**
     * 等待删除
     ***/
    public static int FLAGZKF = 2;
    /*****/

    public static List<Map> StateRules = new ArrayList();
    public static List<Long> rule_time_threshold = new ArrayList();
    public static List<Long> rule_time_first = new ArrayList();

    public static void ResetWindow() {
        Window_start = 0;
        Can0_List.clear();
        Sensor_List.clear();
        Gps_List.clear();
        SteerAngle_List.clear();
        Acceleration_List.clear();
        LateralAcc_List.clear();
        Vehicle_Speed_List.clear();
    }

    public static void InitStateRules() {
        StateRules.clear();
        rule_time_threshold.clear();
        rule_time_first.clear();

        long thisTime = System.currentTimeMillis();

        log_name = "/sdcard/can1" + File.separator + thisTime + "_detect.log";
        CreatFile.createTxt(log_name);

        /**
         * Read file to load rules
         * **/
        int readLen = 100;
        String[] rules = new String[0];
        try {
            rules = readToArray("/sdcard/ivRule",readLen);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int ruleLen = rules.length;
        ivRule tmp;
        for(int i = 0 ;i<ruleLen;i++){
            tmp = parseData.parseRule(rules[i]);
            if(tmp==null){
                break;
            }
            StateRules.add(tmp.Condition);
            //Log.d("LYY111",""+tmp.Condition);
            rule_time_threshold.add(tmp.time);
            rule_time_first.add((long)0);
        }

/*
        Map<String, String> Condition1 = new HashMap();
        Condition1.put("RPM", "203000");
        StateRules.add(Condition1);
        rule_time_threshold.add((long) 5 * 1000);//5s
        rule_time_first.add((long) 0);
        //No flameout while stop
        Map<String, String> Condition2 = new HashMap();
        Condition2.put("RPM", "200");
        Condition2.put("Vehicle_Speed", "110");
        StateRules.add(Condition2);
        rule_time_threshold.add((long) 30
         * 1000);
        rule_time_first.add((long) 0);
*/
    }

    /**
     * 违反的规则
     *
     * @return
     */
    public static List CheckAnomaly(long thistime) {
        List result = new ArrayList();
        boolean flag;
        int NumRule = StateRules.size();
        int ruleno;
        Log.d("lyy", "Rules:" + NumRule);
        for (ruleno = 0; ruleno < NumRule; ruleno++) {

            Map<String, String> temp = StateRules.get(ruleno);
            flag = true;
            for (String key : temp.keySet()) {
                String condition = temp.get(key);
                //Log.d("LYY111",condition);
                double ST_value = SingleState.get(key);
                if (CanTables.STMatch(condition, ST_value) == false) {
                    flag = false;
                    break;
                }
            }
            if (flag == true) {
                //violate the rule
                if (rule_time_first.get(ruleno) == 0) {
                    rule_time_first.set(ruleno, thistime);
                }
                result.add(ruleno);
            } else {
                rule_time_first.set(ruleno, (long) 0);
            }
        }
        if (result.size() == 0) {
            return null;
        }
        return result;
    }


    public static void SendWarning(long thistime) {
        List anomaly = CheckAnomaly(thistime);
        String res = "";
        //Log.d("LYY444", "" + anomaly);
        if (anomaly == null) {
            SingleState.put("Warning Message", (double) -1);
            return;
        }
        int num = anomaly.size();
        int rule_no ;


        for (int key = 0; key < num; key++) {
            rule_no = (int) anomaly.get(key);
            if (thistime - rule_time_first.get(rule_no) >= rule_time_threshold.get(rule_no)) {
                //Log.d("LYY444",""+rule_no);
                SingleState.put("Warning Message", (double) rule_no);
                break;
            }
        }

        //write the res to txt
        for (int key = 0; key < num; key++) {
            rule_no = (int) anomaly.get(key);
            if (thistime - rule_time_first.get(rule_no) >= rule_time_threshold.get(rule_no)) {
                res = res + rule_no + ",";
            }
        }

        if(res!=""){
            res = res + thistime;
            Write(res,log_name);
        }

    }

    public static void Write(String data, String Path) {
        String content = data;
        FileOutputStream fos = null;
        File file = new File(Path);


        try {
            fos = new FileOutputStream(file, true);
            fos.write(content.getBytes());
            fos.write("\r\n".getBytes());
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

    }


    /**
     * zkf 0923
     **/
    // 获取1s窗口内的数据，用于判断是否处于no-event状态


    // 可以判断当前1s win窗内数据是否为event状态


    private static void exit_with_help() {
        System.err.print("usage: svm_predict [options] test_file model_file output_file\n"
                + "options:\n"
                + "-b probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); one-class SVM not supported yet\n"
                + "-q : quiet mode (no outputs)\n");
        System.exit(1);
    }

//    private static double predict(BufferedReader input, DataOutputStream output, svm_model model, int predict_probability) throws IOException {
//        int correct = 0;
//        int total = 0;
//        double error = 0;
//        double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
//
//        int svm_type = svm.svm_get_svm_type(model);
//        int nr_class = svm.svm_get_nr_class(model);
//        double[] prob_estimates = null;
//
//        if (predict_probability == 1) {
//            if (svm_type == svm_parameter.EPSILON_SVR ||
//                    svm_type == svm_parameter.NU_SVR) {
//                svm_predict.info("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=" + svm.svm_get_svr_probability(model) + "\n");
//            } else {
//                int[] labels = new int[nr_class];
//                svm.svm_get_labels(model, labels);
//                prob_estimates = new double[nr_class];
//                output.writeBytes("labels");
//                for (int j = 0; j < nr_class; j++)
//                    output.writeBytes(" " + labels[j]);
//                output.writeBytes("\n");
//            }
//        }
//        double v = 0;
//        while (true) {
//            String line = input.readLine();
//            if (line == null) break;
//
//            StringTokenizer st = new StringTokenizer(line, " \t\n\r\f:");
//
//            double target = atof(st.nextToken());
//            int m = st.countTokens() / 2;
//            svm_node[] x = new svm_node[m];
//            for (int j = 0; j < m; j++) {
//                x[j] = new svm_node();
//                x[j].index = atoi(st.nextToken());
//                x[j].value = atof(st.nextToken());
//            }
//
//
//            if (predict_probability == 1 && (svm_type == svm_parameter.C_SVC || svm_type == svm_parameter.NU_SVC)) {
//                v = svm.svm_predict_probability(model, x, prob_estimates);
//                output.writeBytes(v + " ");
//                for (int j = 0; j < nr_class; j++)
//                    output.writeBytes(prob_estimates[j] + " ");
//                output.writeBytes("\n");
//            } else {
//                v = svm.svm_predict(model, x);
//                output.writeBytes(v + "\n");
//            }
//
//
//            if (v == target)
//                ++correct;
//            error += (v - target) * (v - target);
//            sumv += v;
//            sumy += target;
//            sumvv += v * v;
//            sumyy += target * target;
//            sumvy += v * target;
//            ++total;
//        }
//        return v;
////        if(svm_type == svm_parameter.EPSILON_SVR ||
////                svm_type == svm_parameter.NU_SVR)
////        {
////            svm_predict.info("Mean squared error = "+error/total+" (regression)\n");
////            svm_predict.info("Squared correlation coefficient = "+
////                    ((total*sumvy-sumv*sumy)*(total*sumvy-sumv*sumy))/
////                            ((total*sumvv-sumv*sumv)*(total*sumyy-sumy*sumy))+
////                    " (regression)\n");
////        }
////        else
////            svm_predict.info("Accuracy = "+(double)correct/total*100+
////                    "% ("+correct+"/"+total+") (classification)\n");
//
//
//    }

    private static double atof(String s) {
        return Double.valueOf(s).doubleValue();
    }

    private static int atoi(String s) {
        return Integer.parseInt(s);
    }

    public void GetOrientation() {

    }

    public void GetGradient() {

    }


}
