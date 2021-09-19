package dataProcess;

public class condState2JudgeDB {

    public static int judgeDB(double[][] dbData, double[] featureData){

        int andbType = 0;

        // 条件语句判断Waving
        if (featureData[2] > 3 && featureData[4] > 2 && featureData[0] > 8 && Math.abs(featureData[6]) < 0.2 && Math.abs(featureData[8]) < 8 && featureData[15]> 1000){
            andbType = 1;
            return andbType;

        }

        //条件语句判断Swerving
        if (featureData[15] < 1500 && featureData[12] > 15 && featureData[2] > 3.5 && featureData[4] > 1.8 && featureData[0] > 13 ){
            andbType = 2;
            return andbType;}

        // 条件语句判断sideslipping
        if (featureData[14]  < 0 && featureData[7] < 0 && featureData[1] > 7 && Math.abs(featureData[6]) > 0.7 && featureData[15] < 800 && featureData[4] < 8 && featureData[5] < 8){
            andbType = 3;
            return andbType;
        }

        // 条件语句判断Fast U-turn
        double priOrixSum = 0, postOriSum = 0;
        for (int z1 = 0; z1 < dbData[2].length; z1++){
            if (z1 < Math.floor(dbData[2].length/2)){
                priOrixSum += dbData[2][z1];
            }else{
                postOriSum += dbData[2][z1];
            }
            priOrixSum = priOrixSum/(Math.floor(dbData[2].length/2));
            postOriSum = priOrixSum/(dbData[2].length - Math.floor(dbData[2].length/2));
        }
        if (Math.abs(featureData[6]) > 1 && featureData[0] > 8 && featureData[3] < 1.5 && featureData[4] > 7 && (postOriSum * postOriSum) < 0 && featureData[15] > 1500){
            andbType = 4;
            return andbType;
        }

        // 条件语句判断 Turning with a wide radius
        if (Math.abs(featureData[6]) > 0.5 && featureData[2] > 1 && featureData[4] > 8 && Math.abs(featureData[8]) > 20 && featureData[15] > 800){
            andbType = 5;
            return andbType;
        }

        // 条件语句判断sudden starking
        if (featureData[2] < 1 && featureData[0] < 5){
            andbType = 6;
            return andbType;
        }

        return andbType;



    }


}


