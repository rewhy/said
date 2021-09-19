package cn.uprogrammer.sensordatacollect_udp2;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.IOException;

public class CreatFile {
    public void createTxt(String FileName){

        File mFile = new File(FileName);
        if(mFile.exists()){

        }
        else{
            try{
                mFile.createNewFile();
            }catch (IOException e){
                e.printStackTrace();
            }

        }

    }

    public String getFileDir(){
        String sdcardPath = Environment.getExternalStorageDirectory().toString();

        File dir = new File(sdcardPath + File.separator + "Sensor_Data");

        if(dir.exists()){
            return dir.toString();
        }
        else{
            dir.mkdirs();
            Log.d("BAG", "保存路径不存在,");
            return dir.toString();
        }
    }
}