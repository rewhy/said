package cn.uprogrammer.sensordatacollect_udp2;

import android.os.Environment;
import android.util.Log;

import java.io.File;

import jxl.Workbook;
import jxl.write.Label;
import jxl.write.WritableSheet;
import jxl.write.WritableWorkbook;


public class CreateXls {

    private WritableWorkbook wwb;
    //创建EXCEL文件
    public WritableWorkbook excelCreate(File file) {
        // 准备设置excel工作表的标题-cwhe修改：
        String[] title = {"ACCx/mg", "ACCy/mg", "ACCz/mg", "OriZ", "OriX", "OriY", "Magx/mT", "Magy/mT", "Magz/mT", "GYROx/m°" , "GYROy/m°", "GYROz/m°","Time/ms"};
        //String[] title = {"ACCx/mg","ACCy/mg","ACCz/mg","GYROx/m°","GYROy/m°","GYROz/m°","Magx/mT","Magy/mT","Magz/mT","Time/ms"};
        WritableSheet ws = null;

        try {
//		OutputStream os = new FileOutputStream(excelPath);

            // 创建Excel工作薄
            wwb = Workbook.createWorkbook(file);

            // 添加第一个工作表并设置第一个Sheet的名字
            ws = wwb.createSheet("mysheet", 0);

            Label label;
            for (int i = 0; i < title.length; i++) {
                // Label(x,y,z)其中x代表单元格的第x+1列，第y+1行, 单元格的内容是y
                // 在Label对象的子对象中指明单元格的位置和内容
                label = new Label(i, 0, title[i]);
                // 将定义好的单元格添加到工作表中
                ws.addCell(label);
            }

            // 写入数据
            wwb.write();

            // 关闭文件
            wwb.close();


        } catch (Exception e) {
            e.printStackTrace();
        }
        return wwb;
    }


    public String getExcelDir() {

        // SD卡指定文件夹
        String sdcardPath = Environment.getExternalStorageDirectory()
                .toString();

        File dir = new File(sdcardPath + File.separator + "MeasureData");

        if (dir.exists()) {
            return dir.toString();
        } else {
            dir.mkdirs();
            Log.d("BAG", "保存路径不存在,");
            return dir.toString();
        }
    }
}