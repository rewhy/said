package dataProcess;

import java.util.ArrayList;
import java.util.List;

import queueImplement.QueueLinked;
import queueImplement.QueueNode;

public class processDESF {

    public static double[][] DESF_filter(double[][] data, double alph){
        // 数据为d行 n列，按特征（行）过滤
        int dataDim = data.length;
        int dataSize = data[0].length;
        if (dataDim == 7){
            dataDim  = 6;
        }

        for (int z1 = 0; z1 < dataDim; z1++){
            double xi_1 = data[z1][0];
            for (int z2 = 1; z2 < dataSize; z2++){
                if (data[z1][z2] >= xi_1){
                    data[z1][z2] = (1 - alph) * xi_1 + alph * data[z1][z2];
                }
                else {
                    data[z1][z2] = alph * xi_1 + (1 - alph) * data[z1][z2];
                }
                xi_1 = data[z1][z2];

            }
        }
        return data;
    }

    public static  double[] get1_value(double data[][]) {
        double a = 0.76;//指数平滑系数
        ArrayList<Double> S_1 = new ArrayList<Double>();
        double[] pre_values = new double[data.length];
        double s1 = 0.0;
        double st=0.0;
        for (int i = 0; i < data.length; i++) {
            for (int j = 0; j < data[0].length; j++) {
                if (j < 4) {
                    s1 += data[i][j];
                    if (data[0].length < 20)
                        st=s1/3;
                    else
                        st=data[i][0];
                }
            }
            S_1.add(st);
            for (int k = 0; k <data[0].length ; k++) {
                S_1.add(a*data[i][k]+(1-a)*S_1.get(k));
            }
            pre_values[i]=S_1.get(S_1.size()-1);
        }
        return pre_values;
    }

    public static double[][] get2_valueQue(QueueLinked qWin, int t) {

        QueueNode myQue = qWin.front;

        int queLen = qWin.queueSize();
        double[][] data_all = new double[5][queLen];
//        for (int z = 0; z < queLen; z++){
//            data_all[z] = myQue.dequeue();
//        }
        int z11 = 0;
        double[] temp;
        while(myQue != null){
//             temp = qWin.dequeue();
             temp = myQue.data;
             myQue = myQue.next;
            data_all[0][z11] = temp[0];
            data_all[1][z11] = temp[1];
            data_all[2][z11] = temp[2];
            data_all[3][z11] = temp[3];
            data_all[4][z11] = temp[4];
            z11++;
        }
        double[][] pre_values = data_all;

        for (int z = 0; z < data_all.length -1; z++){

            double[] data = data_all[z];
            double a = 0.3;//指数平滑系数
            ArrayList<Double> S_1 = new ArrayList<Double>();
            ArrayList<Double> S2_1_new = new ArrayList<Double>();
            ArrayList<Double> S2_2_new = new ArrayList<Double>();


            double s1 = 0.0;
            double st = 0.0;
            double at = 0.0;
            double bt = 0.0;
            double xt = 0.0;

            for (int j = 1; j < data.length; j++) {

                if (j < 4) {
                    s1 += data[j - 1];
                    if (data.length < 20)
                        st = s1 / 3;
                    else
                        st = data[0];
                }
            }
            S_1.add(st);//初始值
            for (int k = 0; k < data.length ; k++) {
                if(k==0)
                    S2_1_new.add(a * data[k] + (1 - a) * S_1.get(k));
                else
                    S2_1_new.add(a*data[k]+(1-a)*S2_1_new.get(k-1));
            }
            for (int l = 0; l <data.length ; l++) {
                if(l==0){
                    double tmp = a * S2_1_new.get(l) + (1 - a) * S_1.get(l);
                    S2_2_new.add(tmp);
                    pre_values[z][l] = tmp;
                }
                else{
                    double tmp = a*S2_1_new.get(l)+(1-a)*S2_2_new.get(l-1);
                    S2_2_new.add(a*S2_1_new.get(l)+(1-a)*S2_2_new.get(l-1));
                    pre_values[z][l] = tmp;

                }
            }
        }
        return pre_values;
    }

    public static double[] get2_value(double data[][],int t) {

        double a = 0.3;//指数平滑系数
        List<Double> S_1 = new ArrayList<Double>();
        List<Double> S2_1_new = new ArrayList<Double>();
        List<Double> S2_2_new = new ArrayList<Double>();
        double[] pre_values = new double[data.length];
        double s1 = 0.0;
        double st = 0.0;
        double at =0.0;
        double bt = 0.0;
        double xt = 0.0;
        for (int i = 0; i < data.length; i++) {
            for (int j = 1; j < data[0].length; j++) {

                if (j < 4) {
                    s1 += data[i][j - 1];
                    if (data[0].length < 20)
                        st = s1 / 3;
                    else
                        st = data[i][0];
                }
            }
            S_1.add(st);//初始值
            for (int k = 0; k < data[0].length; k++) {
                if(k==0)
                    S2_1_new.add(a * data[i][k] + (1 - a) * S_1.get(k));
                else
                    S2_1_new.add(a*data[i][k]+(1-a)*S2_1_new.get(k-1));
            }
            for (int l = 0; l <data[0].length ; l++) {
                if(l==0)
                    S2_2_new.add(a * S2_1_new.get(l) + (1 - a) * S_1.get(l));
                else
                    S2_2_new.add(a*S2_1_new.get(l)+(1-a)*S2_2_new.get(l-1));
            }
            at=S2_1_new.get(S2_1_new.size()-1)*2-S2_2_new.get(S2_2_new.size()-1);
            bt=a/(1-a)*(S2_1_new.get(S2_1_new.size()-1)-S2_2_new.get(S2_2_new.size()-1));
            xt=at+bt*t;
            pre_values[i] = xt;
        }


        return pre_values;
    }

    public static double[] get3_value(double data[][] ,int t ) {
        double a = 0.25;//指数平滑系数
        List<Double> S_1 = new ArrayList<Double>();
        List<Double> S3_1_new = new ArrayList<Double>();
        List<Double> S3_2_new = new ArrayList<Double>();
        List<Double> S3_3_new = new ArrayList<Double>();
        double[] pre_values = new double[data.length];
        double s1 = 0.0;
        double st = 0.0;
        double at =0.0;
        double bt = 0.0;
        double xt = 0.0;
        double ct=0.0;
        for (int i = 0; i < data.length; i++) {
            for (int j = 1; j < data[0].length; j++) {

                if (j < 4) {
                    s1 += data[i][j - 1];
                    if (data[0].length < 20)
                        st = s1 / 3;//小于20个数据，取前3个的平均值
                    else
                        st = data[i][0];//否则取第一个
                }
            }
            S_1.add(st);//初始值
            for (int k = 0; k < data[0].length; k++) {
                if(k==0)
                    S3_1_new.add(a * data[i][k] + (1 - a) * S_1.get(k));
                else
                    S3_1_new.add(a*data[i][k]+(1-a)*S3_1_new.get(k-1));
            }
            for (int l = 0; l <data[0].length ; l++) {
                if(l==0)
                    S3_2_new.add(a * S3_1_new.get(l) + (1 - a) * S_1.get(l));
                else
                    S3_2_new.add(a*S3_1_new.get(l)+(1-a)*S3_2_new.get(l-1));
            }
            for (int j = 0; j <data[0].length ; j++) {
                if(j==0)
                    S3_3_new.add(a * S3_2_new.get(j) + (1 - a) * S_1.get(j));
                else
                    S3_3_new.add(a*S3_2_new.get(j)+(1-a)*S3_3_new.get(j-1));
            }
            at=S3_1_new.get(S3_1_new.size()-1)*3-S3_2_new.get(S3_2_new.size()-1)*3+S3_3_new.get(S3_3_new.size()-1);
            bt=(a/(2*Math.pow((1-a),2)))*((6-5*a)*S3_1_new.get(S3_1_new.size()-1)-2*(5-4*a)*S3_2_new.get(S3_2_new.size()-1)+(4-3*a)*S3_3_new.get(S3_3_new.size()-1));
            ct=(Math.pow(a,2)/(2*Math.pow((1-a),2)))*(S3_1_new.get(S3_1_new.size()-1)-2*S3_2_new.get(S3_2_new.size()-1)+S3_3_new.get(S3_3_new.size()-1));
            xt=at+bt*t+ct*Math.pow(t,2);
            pre_values[i] = xt;
        }
        return  pre_values;
    }

}
