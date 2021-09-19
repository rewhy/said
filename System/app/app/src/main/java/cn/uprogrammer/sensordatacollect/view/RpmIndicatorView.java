package cn.uprogrammer.sensordatacollect.view;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.support.v7.widget.AppCompatImageView;
import android.util.AttributeSet;
import android.view.Display;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.LinearInterpolator;
import android.widget.ImageView;

import cn.uprogrammer.sensordatacollect.helper.BitmapHelper;


public class RpmIndicatorView extends AppCompatImageView {

    private final static int ANGLE_OFFSET = 30;
    private final static int MAX_ANGLE = 180 - ANGLE_OFFSET;

    private final static float POS_OFFSET = 125f;

    private final static float MAX_VALUE = 10000f;
    private final static float INCOMING_MAX_VALUE = 8000f;
    private final static float VALUE_OFFSET = MAX_VALUE - INCOMING_MAX_VALUE;

    private RectF mRectF;
    private Path mPath;
    private Paint mPaint;

    private float mAngle = 0;

    private ValueAnimator mAnimator = new ValueAnimator();

    private int mValue = 0;

    public int getValue() {
        return mValue;
    }
    
    public void setValue(int value) {
        mValue = Math.max(0, Math.min(value, (int) MAX_VALUE));

        float previousAngle = mAngle;
        mAngle = ((mValue + VALUE_OFFSET) * MAX_ANGLE / MAX_VALUE);

        mAnimator.cancel();
        mAnimator.setFloatValues(previousAngle, mAngle);
        mAnimator.start();
    }

    public RpmIndicatorView(Context context) {
        super(context);
        initialize(context);
    }

    public RpmIndicatorView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize(context);
    }

    public RpmIndicatorView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize(context);
    }

    private void initialize(Context context) {
        Bitmap bitmap = ((BitmapDrawable) getDrawable()).getBitmap();
        Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

        Rect rect = BitmapHelper.getBitmapOpaqueRect(bitmap);
        BitmapHelper.ScaleFactor scale = BitmapHelper.getScaleFactor(bitmap, display.getWidth(), display.getHeight());

        mRectF = new RectF(
                scale.x * (rect.left - POS_OFFSET),
                scale.y * rect.top,
                scale.x * (rect.right  + POS_OFFSET),
                scale.y * (rect.bottom + POS_OFFSET) * 2f
        );
        mPath = new Path();
        mPaint = new Paint();
        {
            mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
            mPaint.setAntiAlias(true);
            mPaint.setStyle(Paint.Style.FILL_AND_STROKE);
        }

        mAnimator.setDuration(250);
        mAnimator.setInterpolator(new LinearInterpolator());
        mAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                updatePath((float) animation.getAnimatedValue());
            }
        });

        setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        setValue(0);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        canvas.drawPath(mPath, mPaint);
    }

    private void updatePath(float angle) {
        mPath.reset();
        mPath.moveTo(mRectF.centerX(), mRectF.centerY());
        mPath.arcTo(mRectF, 180, ANGLE_OFFSET);
        mPath.addArc(mRectF, 360 - ANGLE_OFFSET, angle - MAX_ANGLE);
        mPath.lineTo(mRectF.centerX(), mRectF.centerY());

        invalidate();
    }
}
