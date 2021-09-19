package cn.uprogrammer.sensordatacollect_udp2;

import android.content.Context;
import android.graphics.Bitmap;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class FileUtils {
    /**
     * 从assets目录中复制整个文件夹内容
     *
     * @param context Context 使用CopyFiles类的Activity
     * @param oldPath String  原文件路径  如：/aa
     * @param newPath String  复制后路径  如：xx:/bb/cc
     */
    public static boolean copyFilesFassets(Context context, String oldPath, String newPath) {
        try {
            String fileNames[] = context.getAssets().list(oldPath);//获取assets目录下的所有文件及目录名
            if (fileNames.length > 0) {//如果是目录
                File file = new File(newPath);
                file.mkdirs();//如果文件夹不存在，则递归
                for (String fileName : fileNames) {
                    copyFilesFassets(context, oldPath + "/" + fileName, newPath + "/" + fileName);
                }
            } else {//如果是文件
                InputStream is = context.getAssets().open(oldPath);
                FileOutputStream fos = new FileOutputStream(new File(newPath));
                byte[] buffer = new byte[1024];
                int byteCount = 0;
                while ((byteCount = is.read(buffer)) != -1) {//循环从输入流读取 buffer字节
                    fos.write(buffer, 0, byteCount);//将读取的输入流写入到输出流
                }
                fos.flush();//刷新缓冲区
                is.close();
                fos.close();
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
            //如果捕捉到错误则通知UI线程
            return false;
        } finally {
            return false;
        }
    }

    /**
     * 复制单个文件路径
     *
     * @param oldPath String  原文件路径  如：/aa
     * @param newPath String  复制后路径  如：xx:/bb/cc
     */
    public static boolean copyFilesFassets(String oldPath, String newPath) throws IOException {

        FileInputStream fis = new FileInputStream(new File(oldPath));
        if (fis == null) return false;
        File newFile = new File(newPath);
        if(!newFile.getParentFile().exists())
        {
            newFile.getParentFile().mkdirs();
        }
        FileOutputStream fos = new FileOutputStream(newFile);
        byte[] buffer = new byte[1024];
        int byteCount = 0;
        while ((byteCount = fis.read(buffer)) != -1) {//循环从输入流读取 buffer字节
            fos.write(buffer, 0, byteCount);//将读取的输入流写入到输出流
        }
        fos.flush();//刷新缓冲区
        fis.close();
        fos.close();
        return true;
    }

    /**
     * 删除给定的路径
     *
     * @param dir 需要删除的路径
     */
    public static boolean deleteDirectory(String dir) {
        // 如果dir不以文件分隔符结尾，自动添加文件分隔符
        if (!dir.endsWith(File.separator))
            dir = dir + File.separator;
        File dirFile = new File(dir);
        // 如果dir对应的文件不存在，或者不是一个目录，则退出
        if ((!dirFile.exists()) || (!dirFile.isDirectory())) {
//            DLog.e("删除目录失败：" + dir + "不存在！");
            return false;
        }
        // 用于标识是否删除成功
        boolean flag = true;
        // 删除文件夹中的所有文件包括子目录
        File[] files = dirFile.listFiles();
        for (int i = 0; i < files.length; i++) {
            // 删除子文件
            if (files[i].isFile()) {
                flag = delFile(files[i].getAbsolutePath());
                if (!flag)
                    break;
            }
            // 删除子目录
            else if (files[i].isDirectory()) {
                flag = deleteDirectory(files[i]
                        .getAbsolutePath());
                if (!flag)
                    break;
            }
        }
        if (!flag) {
//            DLog.e("删除目录失败！");
            return false;
        }
        // 删除当前目录
        if (dirFile.delete()) {
//            DLog.i("删除目录" + dir + "成功！");
            return true;
        } else {
            return false;
        }
    }


    /**
     * 删除给定路径的文件
     *
     * @param path 文件路径
     */
    public static boolean delFile(String path) {
        if (path != null) {
            File file = new File(path);
            if (file.exists()) {
                if (file.delete()) {
                    return true;
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * 按照给定的路径和格式保存图片
     *
     * @param path   图片要保存的路径
     * @param bitmap bitmap形式的图片文件
     * @param format 图片要保存的格式
     *               {@link Bitmap.CompressFormat#JPEG}
     *               {@link Bitmap.CompressFormat#PNG}
     *               {@link Bitmap.CompressFormat#WEBP}
     */
    public static boolean saveBitmap(Bitmap bitmap, String path, Bitmap.CompressFormat format) throws IOException {
        File file = new File(path);//将要保存图片的路径
        BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(file));
        bitmap.compress(format, 100, bos);

        bos.flush();
        bos.close();
        return true;
    }

    /**
     * 按照给定的路径默认保存JPEG格式的图片
     *
     * @param path   图片要保存的路径
     * @param bitmap bitmap形式的图片文件
     */
    public static boolean saveBitmapAsJPEG(Bitmap bitmap, String path) throws IOException {
        saveBitmap(bitmap, path, Bitmap.CompressFormat.JPEG);
        return true;
    }


    /**
     * 读取某个文件夹下的所有文件
     *
     * @param dirPath 文件夹目录（绝对路径）
     */
    public static String[] readFilesInDirectory(String dirPath) {
        File file = new File(dirPath);
        if (null == file) {
//            DLog.e("名为“" + dirPath + "”的文件夹不存在!");
        } else if (!file.isDirectory()) {
//            DLog.e("filePath 不是文件夹：dirPath = " + dirPath);
        } else if (file.isDirectory()) {
            String[] filelist = file.list();
            if (null == filelist) return null;
            String[] filePaths = new String[filelist.length];
            for (int i = 0; i < filelist.length; i++) {
                filePaths[i] = dirPath + "/" + filelist[i];
            }
            return filePaths;
        }
        return null;
    }

}
