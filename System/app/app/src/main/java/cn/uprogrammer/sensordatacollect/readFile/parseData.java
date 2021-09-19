package cn.uprogrammer.sensordatacollect.readFile;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import cn.uprogrammer.sensordatacollect.ivRule;

public class parseData {



    public static double[] parseGPS(String data){
        if(data==null){
            return null;
        }
        double[] res = new double[2];
        String regex = "gps (.*),";
        Pattern p= Pattern.compile(regex);
        Matcher m = p.matcher(data);
        if(m.find()){
            res[0] = Double.parseDouble(m.group(1));
        }else {
            return null;
        }

        regex = ",(.*) acc";
        p = Pattern.compile(regex);
        m = p.matcher(data);
        if(m.find()){
            res[1] = Double.parseDouble(m.group(1));
        }else {
            return null;
        }
        return res;
    }

    public static double[] parseString(String data){
        if(data==null){
            return null;
        }
        double[] res = new double[2];

        String startStr = "gps ";
        String midStr = ",";
        String endStr = " acc";

        int startIndex1 = data.indexOf(startStr)+ startStr.length();
        int endIndex1 = data.indexOf(midStr);
        int startIndex2 = endIndex1+ midStr.length();
        int endIndex2 = data.indexOf(endStr);
        res[0] = Double.parseDouble(data.substring(startIndex1,endIndex1));
        res[1] = Double.parseDouble(data.substring(startIndex2,endIndex2));

        return res;
    }


    public static ivRule parseRule(String data){
        ivRule res = new ivRule();
        if(data==null){
            return null;
        }

        int startIndex = 0;

        long time;
        String regex = "Time:(.*) Number";
        Pattern p= Pattern.compile(regex);
        Matcher m = p.matcher(data);
        if(m.find()){
            time = Long.parseLong(m.group(1));
        }else {
            return null;
        }
        res.time = time;

        int number;
        regex = "Number:(.*?) \\[Rule";
        p = Pattern.compile(regex);
        m = p.matcher(data);
        if(m.find()){
            number = Integer.parseInt(m.group(1));
        }else {
            return null;
        }

        res.numbers = number;
        String condition="0";
        String[] tmp;
        int count = 0;

        regex = "Rule:(.*?)\\]";
        p = Pattern.compile(regex);
        m = p.matcher(data);

        while (m.find()){
            count++;
            condition = m.group(1);
            tmp = condition.split(",");
            res.Condition.put(tmp[0],tmp[1]);
        }

        if(count!=number){
            return null;
        }

        return res;

    }



}
