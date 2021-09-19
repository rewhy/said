package cn.uprogrammer.sensordatacollect_udp2;

import org.apache.commons.math3.linear.Array2DRowRealMatrix;
import org.apache.commons.math3.linear.EigenDecomposition;
import org.apache.commons.math3.linear.RealMatrix;
import org.apache.commons.math3.linear.RealVector;

import java.util.List;

import static dataProcess.processDESF.DESF_filter;

public class SensorCalibration {
    public double [][] R = new double [3][3];
    public double meanAccX = 0;
    public double meanAccY = 0;
    public double meanAccZ = 0;
    public double meanGyroX = 0;
    public double meanGyroY = 0;
    public double meanGyroZ = 0;

    public RealMatrix RMatrix;

    public double getSum(double[] inputData) {
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

    public double getAverage(double[] inputData) {
        if (inputData == null || inputData.length == 0) {
            return -1;
        }
        int len = inputData.length;
        double result;
        result = getSum(inputData) / len;

        return result;
    }

    public static double[][] convertSensorData(List sensorData) {

        double[][] currentSensorData = new double[7][sensorData.size()];
        for (int z2 = 0; z2 < sensorData.size(); z2++) {
            double[] temp = (double[]) sensorData.get(z2);
            for (int z3 = 0; z3 < 7; z3++) {
                currentSensorData[z3][z2] = temp[z3];        // sensor data 7 * n, saved by time sequence
            }
        }
        return currentSensorData;
    }
    //获取旋转矩阵
    public void getRotationMatrix(double [][] sensorData){

        double [][] temp1 = new double[3][sensorData[0].length];
        for (int z1 = 0; z1 < 3; z1++){
            temp1[z1] = sensorData[z1];
        }
        double [][] gyro = new double[3][sensorData[0].length];
        for (int z1 = 3; z1 < 6; z1++){
            gyro[z1-3] = sensorData[z1];
        }


        temp1 = DESF_filter(temp1, 0.6);  //d 行 n 列

        RealMatrix data4R = new Array2DRowRealMatrix(temp1);
        RMatrix = data4R.multiply(data4R.transpose());

        EigenDecomposition e = new EigenDecomposition(RMatrix);
        RealVector arrayEigenVector0 = e.getEigenvector(0);
        RealVector arrayEigenVector1 = e.getEigenvector(1);
        RealVector arrayEigenVector2 = e.getEigenvector(2);
        R[0] = arrayEigenVector0.toArray();
        R[1] = arrayEigenVector1.toArray();
        R[2] = arrayEigenVector2.toArray();

        double [][] temp = new double[3][3];
        for (int z1 = 0; z1 < 3; z1++){
            for (int z2 = 0; z2 < 3; z2++){
                temp[z1][z2] = R[2-z2][z1];
                temp[z1][z2] = temp[z1][z2] * Math.pow(-1, z2+1);
            }
        }
        R = temp;

        if (Math.abs(R[0][0]) < Math.abs(R[1][0])){
            for (int z1 = 0; z1 < 3; z1++){
                double a = R[z1][0];
                R[z1][0] = R[z1][1];
                R[z1][1] = a;
            }
        }
        if (R[0][0] > 0){
            for (int z1 = 0; z1 < 3; z1++){
                R[z1][0] = -R[z1][0];
            }
        }
        if (R[1][1] < 0){
            for (int z1 = 0; z1 < 3; z1++){
                R[z1][1] = -R[z1][1];
            }
        }
        if (R[2][2] < 0){
            for (int z1 = 0; z1 < 3; z1++){
                R[z1][2] = -R[z1][2];
            }
        }


        RMatrix = new Array2DRowRealMatrix(R);

        RealMatrix caliberatedData = data4R.transpose().multiply(RMatrix);  // n行d列
        caliberatedData = caliberatedData.transpose();  //  d行，n列
        double [][] data2CalMean = caliberatedData.getData();
        meanAccX = getAverage(data2CalMean[0]);  // 第 0 行，即 Acc X 的均值
        meanAccY = getAverage(data2CalMean[1]);  // 第 1 行，即 Acc Y 的均值
        meanAccZ = getAverage(data2CalMean[2]);  // 第 1 行，即 Acc Z 的均值

        RealMatrix gyroMat = new Array2DRowRealMatrix(gyro);
        RealMatrix gyroCal = gyroMat.transpose().multiply(RMatrix);
        gyroCal = gyroCal.transpose();
        double [][] gyro2CalMean = gyroCal.getData();
        meanGyroX = getAverage(gyro2CalMean[0]);
        meanGyroY = getAverage(gyro2CalMean[1]);
        meanGyroZ = getAverage(gyro2CalMean[2]);
    }


    public double[][] calibrateSensor(double [][] sensorData){
// 传入数据 7 * n 的sensor数据
        int num = sensorData[0].length;

        double [][] currentSensorData = new double [7][num];
        double [] sensorTime = new double [num];
        // 提取5s的sensor acc 数据,d 行 n 列 (7 行，100列)
        //

        sensorData = DESF_filter(sensorData, 0.6);
        currentSensorData = DESF_filter(sensorData, 0.6);

        // 将Acc 数据校准
        double [][]sensorAcc = new double[3][];
        sensorAcc[0] = currentSensorData[0];
        sensorAcc[1] = currentSensorData[1];
        sensorAcc[2] = currentSensorData[2];
        RealMatrix accMatrix = new Array2DRowRealMatrix(sensorAcc); // accMatrix 3 行 n 列， acc数据
        accMatrix = accMatrix.transpose().multiply(RMatrix);
        sensorAcc = accMatrix.transpose().getData();

        double [][] sensorGyro = new double[3][];
        sensorGyro[0] = currentSensorData[3];
        sensorGyro[1] = currentSensorData[4];
        sensorGyro[2] = currentSensorData[5];
        RealMatrix gyroMatrix = new Array2DRowRealMatrix(sensorGyro);
        gyroMatrix = gyroMatrix.transpose().multiply(RMatrix);
        sensorGyro = gyroMatrix.transpose().getData();

        for (int z1 = 0; z1 < sensorAcc[0].length; z1++){
            currentSensorData[0][z1] = sensorAcc[0][z1] - meanAccX; // 左右加速度
            if (Math.abs(currentSensorData[0][z1]) < 0.01){
                currentSensorData[0][z1] = 0;
            }
            currentSensorData[1][z1] = sensorAcc[1][z1] - meanAccY;
//            currentSensorData[2][z1] = sensorAcc[2][z1] - meanAccZ;


            currentSensorData[3][z1] = sensorGyro[0][z1] - meanGyroX;
            currentSensorData[4][z1] = sensorGyro[1][z1] - meanGyroY;
            currentSensorData[5][z1] = sensorGyro[2][z1] - meanGyroZ;
        }

        currentSensorData[6] = sensorTime;

//        return  sensorData; // 7 * n ; 0-5 sensor, 6 time
        return currentSensorData;
    }


}
