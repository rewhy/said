package cn.uprogrammer.sensordatacollect.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;

import cn.uprogrammer.sensordatacollect.R;

public class SpeedIndicatorView extends View {

    private final Rect CACHE_RECT = new Rect();

    private Paint mFillPaint;
    private Paint mStrokePaint;

    private int mSpeed;
    
    public int getSpeed() {
        return mSpeed;
    }

    public void setSpeed(int speed) {
        mSpeed = speed;
        invalidate();
    }

    public SpeedIndicatorView(Context context) {
        super(context);
        initialize(context);
    }

    public SpeedIndicatorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize(context);
    }

    public SpeedIndicatorView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize(context);
    }

    private void initialize(Context context) {
        Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        DisplayMetrics metrics = new DisplayMetrics();
        display.getMetrics(metrics);
        Point size = new Point();
        display.getSize(size);
        int width = size.x;
        int height = size.y;

        mFillPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        {
            mFillPaint.setStyle(Paint.Style.FILL);
            mFillPaint.setColor(context.getResources().getColor(R.color.colorPrimary));
            mFillPaint.setTextSize(width / 4.2f);
            mFillPaint.setTypeface(Typeface.createFromAsset(context.getAssets(), "fonts/jefferies.otf"));
            mFillPaint.setTextAlign(Paint.Align.CENTER);
        }

        mStrokePaint = new Paint(mFillPaint);
        {
            mStrokePaint.setStyle(Paint.Style.STROKE);
            mStrokePaint.setColor(Color.WHITE);
            mStrokePaint.setStrokeWidth(4f * metrics.density);
        }

        setSpeed(0);
    }

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);
        String text = String.valueOf(mSpeed);
        mFillPaint.getTextBounds(text, 0, text.length(), CACHE_RECT);

        float width = canvas.getWidth() / 2f;
        float height = canvas.getHeight() / 2f + CACHE_RECT.height() / 2;

        canvas.drawText(text, width, height, mStrokePaint);
        canvas.drawText(text, width, height, mFillPaint);
    }
}
