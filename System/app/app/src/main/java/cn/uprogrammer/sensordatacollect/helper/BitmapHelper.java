package cn.uprogrammer.sensordatacollect.helper;

import android.graphics.Bitmap;
import android.graphics.Rect;

public class BitmapHelper {

    public static Rect getBitmapOpaqueRect(Bitmap bitmap) {
        int minX = bitmap.getWidth(), maxX = -1;
        int minY = bitmap.getHeight(), maxY = -1;

        for (int y = 0; y < bitmap.getHeight(); y++) {
            for (int x = 0; x < bitmap.getWidth(); x++) {
                int alpha = (bitmap.getPixel(x, y) >> 24) & 255;
                if (alpha > 0) {
                    if (x < minX)
                        minX = x;
                    if (x > maxX)
                        maxX = x;
                    if (y < minY)
                        minY = y;
                    if (y > maxY)
                        maxY = y;
                }
            }
        }

        if ((maxX < minX) || (maxY < minY))
            return new Rect();

        return new Rect(minX, minY, maxX, maxY);
    }

    public static ScaleFactor getScaleFactor(Bitmap bitmap, float width, float height) {
        return new ScaleFactor(width / (float) bitmap.getWidth(), height / (float) bitmap.getHeight());
    }

    public static class ScaleFactor {
        public float x;
        public float y;

        public ScaleFactor(float x, float y) {
            this.x = x;
            this.y = y;
        }
    }
}
