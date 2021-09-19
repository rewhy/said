package rfPrediction;


import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class featureExtraction {

    public static double [] get_meaningful_features(double [][] sensorData){    // input 0-6 * n sensor 7 * n gpsspeed
        double [] feature = new double[23];
        double [] temp = extract_mf(sensorData[0]);

        for (int z1 = 0; z1 < 11; z1++){
            feature[z1] = temp[z1];
            Log.d("DataSize", "feature["+z1+"]: "+ feature[z1]);
        }

        temp = extract_mf(sensorData[5]);

        for (int z1 = 11; z1 < 22; z1++){
            feature[z1] = temp[z1-11];
            Log.d("DataSize", "feature["+z1+"]: "+ feature[z1]);
        }
        feature[22] = 0;
        for (int z1 = 0; z1 < sensorData[0].length; z1++){
            feature[22] += sensorData[5][z1] * sensorData[7][z1];
        }

        return feature;

    }

    public static double [] extract_mf(double [] sensorData){
        double [] feature = new double[11];
        feature[0] = getSum(sensorData);
        int data_size = sensorData.length;
        int ind1_4 = (int)Math.floor(data_size/4) - 1;
        int ind2_4 = 2 * (int)Math.floor(data_size/4) - 1;
        int ind3_4 = 3 * (int)Math.floor(data_size/4) - 1;
        feature[1] = sensorData[ind1_4] - sensorData[0];
        feature[2] = sensorData[ind2_4] - sensorData[ind1_4];
        feature[3] = sensorData[ind3_4] - sensorData[ind2_4];
        feature[4] = sensorData[data_size-1] - sensorData[ind3_4];
        feature[5] = sensorData[data_size-1] - sensorData[ind2_4];
        feature[6] = sensorData[ind2_4] - sensorData[0];
        feature[7] = getMax(sensorData);
        feature[8] = getMin(sensorData);
        feature[9] = getAverage(sensorData);
        feature[10] = getStandardDiviation(sensorData);
        return  feature;
    }


    public static double [] get_35_Feature(double [][] sensorData, double [] steerAngleData){
        // sensorData d行n列，已校准； steerAngleData 1行n列
        List <Double> featureList = new ArrayList();
        /*** 提取sensor特征 ***/
        featureList = get5Features(sensorData, featureList);
        /*** 提取速度特征 ***/

        featureList.add(getMax(steerAngleData));
        featureList.add(getMin(steerAngleData));
        double temp = getMax(steerAngleData) - getMin(steerAngleData);
        featureList.add(temp);
        featureList.add(getAverage(steerAngleData));
        featureList.add(getStandardDiviation(steerAngleData));

        double [] featureData = new double [featureList.size()];
        for (int z1 = 0; z1 < featureList.size(); z1++){
            featureData[z1] = featureList.get(z1);
//            Log.d("feature:", "feature "+ z1+":"+featureData[z1]);
        }
        return featureData;
    }

    public static double[] get_50_Feature(double [][] sensorData, double [] steerAngleData){
        // sensorData d行n列， steerAngleData 1行n列

        List <Double> featureList = new ArrayList();
        /*** 提取sensor特征 ***/

        featureList = get7Features(sensorData, featureList);
        /*** 提取速度特征 ***/
        featureList.add(getMax(steerAngleData));
        featureList.add(getMin(steerAngleData));
        double temp = getMax(steerAngleData) - getMin(steerAngleData);
        featureList.add(temp);
        featureList.add(getAverage(steerAngleData));
        featureList.add(getStandardDiviation(steerAngleData));
        int arrayLength = steerAngleData.length;
//        Log.d("speedfeature","speed size"+steerAngleData.length);
        if (arrayLength == 0){
            featureList.add(0.0);
            featureList.add(0.0);
        }else {
            double[] temp1 = Arrays.copyOfRange(steerAngleData, 0, (int) Math.floor(arrayLength / 2));
            featureList.add(getAverage(temp1));
            temp1 = Arrays.copyOfRange(steerAngleData, (int) (Math.floor(arrayLength / 2) + 1), arrayLength);
            featureList.add(getAverage(temp1));
        }
        /*** 提取持续时间 ***/
        double time_gap = (getMax(sensorData[6]) - getMin(sensorData[6]))/1000;
        featureList.add(time_gap);

        double [] featureData = new double [featureList.size()];
        for (int z1 = 0; z1 < featureList.size(); z1++){
            featureData[z1] = featureList.get(z1);
//            Log.d("feature:", "feature50 "+ z1+":"+featureData[z1]);
        }
        return featureData;
    }

    public static List get7Features(double[][] data, List featureList) {
        // data d行，n列
        int dataDim = data.length;
        for (int z1 = 0; z1 < dataDim - 1; z1++) { //最后一维为时间
            int z2 = z1 * (data[z1].length);

            featureList.add(getMax(data[z1]));
            featureList.add(getMin(data[z1]));
            double temp = getMax(data[z1]) - getMin(data[z1]);
            featureList.add(temp);
            featureList.add(getAverage(data[z1]));
            featureList.add(getStandardDiviation(data[z1]));
            int arrayLength = data[z1].length;
            double[] temp1 = Arrays.copyOfRange(data[z1], 0, (int) Math.floor(arrayLength / 2));
            featureList.add(getAverage(temp1));
            if (data[z1].length > 3) {
                double [] temp2 = Arrays.copyOfRange(data[z1], (int) (Math.floor(arrayLength / 2)+1 ), (data[z1].length - 1));
                featureList.add(getAverage(temp2));

            }else{
                double [] temp2 = new double[]{0,0};
                featureList.add(getAverage(temp2));

            }

        }
        return featureList;
    }

    public static List get5Features(double[][] data, List featureList) {
        // data d行，n列
        int dataDim = data.length;
        for (int z1 = 0; z1 < dataDim - 1; z1++) { //最后一维为时间
            int z2 = z1 * (data[z1].length);
            featureList.add(getMax(data[z1]));
            featureList.add(getMin(data[z1]));
            double temp = getMax(data[z1]) - getMin(data[z1]);
            featureList.add(temp);
            featureList.add(getAverage(data[z1]));
            featureList.add(getStandardDiviation(data[z1]));
        }
        return featureList;
    }


    /**
     * 求给定双精度数组中值的最大值
     *
     * @param inputData 输入数据数组
     * @return 运算结果, 如果输入值不合法，返回为-1
     */
    public static double getMax(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }

        int len = inputData.length;
        double max = inputData[0];

        for (int i = 0; i < len; i++) {
            if (max < inputData[i]) {
                max = inputData[i];
            }
        }
        return max;
    }

    /**
     * 求求给定双精度数组中值的最小值
     *
     * @param inputData 输入数据数组
     * @return 运算结果, 如果输入值不合法，返回为-1
     */
    public static double getMin(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double min = inputData[0];
        for (int i = 0; i < len; i++) {
            if (min > inputData[i]) {
                min = inputData[i];
            }
        }
        return min;
    }

    /**
     * 求给定双精度数组中值的和
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static double getSum(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double sum = 0;
        for (int i = 0; i < len; i++) {
            sum = sum + inputData[i];
        }

        return sum;
    }


    public static double getAbsSum(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double sum = 0;
        for (int i = 0; i < len; i++) {
            sum = sum + Math.abs(inputData[i]);
        }

        return sum;
    }

    /**
     * 求给定双精度数组中值的数目
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static int getCount(double[] inputData) {
        if (inputData == null) {
            return -1;
        }

        return inputData.length;
    }

    /**
     * 求给定双精度数组中值的平均值
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static double getAverage(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double result;
        result = getSum(inputData) / len;

        return result;
    }

    /**
     * 求给定双精度数组中值的平方和
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static double getSquareSum(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double sqrsum = 0.0;
        for (int i = 0; i < len; i++) {
            sqrsum = sqrsum + inputData[i] * inputData[i];
        }


        return sqrsum;
    }

    /**
     * 求给定双精度数组中值的方差
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static double getVariance(double[] inputData) {
        int count = getCount(inputData);
        double sqrsum = getSquareSum(inputData);
        double average = getAverage(inputData);
        double result;
        result = (sqrsum - count * average * average) / count;

        return result;
    }

    /**
     * 求给定双精度数组中值的标准差
     *
     * @param inputData 输入数据数组
     * @return 运算结果
     */
    public static double getStandardDiviation(double[] inputData) {
        if (inputData.length == 0){
            return 0;
        }
        double result;
        //绝对值化很重要
        result = Math.sqrt(Math.abs(getVariance(inputData)));
        return result;
    }


}
