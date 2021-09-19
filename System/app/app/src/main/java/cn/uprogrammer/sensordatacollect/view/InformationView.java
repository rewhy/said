package cn.uprogrammer.sensordatacollect.view;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.os.Handler;
import android.os.Message;
import android.support.v7.widget.AppCompatImageView;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import cn.uprogrammer.sensordatacollect.GetView;
import cn.uprogrammer.sensordatacollect.R;
import cn.uprogrammer.sensordatacollect.StateView;
import cn.uprogrammer.sensordatacollect.helper.BitmapHelper;

public class InformationView extends AppCompatImageView {

    private final Rect CACHE_RECT = new Rect();
    private final float lowPad = 250;

    private BitmapHelper.ScaleFactor mScale;
    private Rect mRect;

    private FrameLayout mLayout;
    private FrameLayout mLayout2;
    private TextView mTvTime;
    private TextView mTvRpm;

    private Date mTime;
    private DateFormat mDateFormat = new SimpleDateFormat("HH:mm");
    private Timer mTimer = new Timer();
    Calendar cal = Calendar.getInstance();

    private int mRpm = 0;

    public Date getTime() {
        return mTime;
    }

    public void increaseTime() {

        int minute=cal.get(Calendar.MINUTE);
        cal.set(Calendar.MINUTE,minute+1);
        //mTime.setMinutes(minute + 1);

        mTvTime.setText(mDateFormat.format(mTime));
        mTvTime.append(" ");
        invalidate();
    }


    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            increaseTime();
        }
    };


    private TimerTask mTimerTask = new TimerTask() {
        @Override
        public void run() {
            mHandler.obtainMessage().sendToTarget();
        }
    };

    public void startTimer() {
        cal = Calendar.getInstance();
        mTime = cal.getTime();
        mTimer.schedule(mTimerTask, 0, 60000);
    }

    public void stopTimer() {
        mTimer.cancel();
    }

    public int getRpm() {
        return mRpm;
    }

    public void setRpm(int rpm) {
        mRpm = rpm;

        String text = String.valueOf(rpm);
        mTvRpm.setText(text);

        mTvRpm.getPaint().getTextBounds(text, 0, text.length(), CACHE_RECT);
        //mTvRpm.setPadding((int) ((mRect.right - 15f) * mScale.x - CACHE_RECT.width()), (int) ((mRect.bottom - 110f) * mScale.y), 0, 0);
        mTvRpm.setPadding((int) ((mRect.right - 15f) * mScale.x - CACHE_RECT.width()), (int) ((mRect.bottom - lowPad) * mScale.y), 0, 0);
        //Log.d("INFORPM","x:"+(int) ((mRect.right - 15f) * mScale.x - CACHE_RECT.width())+"y:"+(int) ((mRect.bottom - 110f) * mScale.y));

        mTvRpm.append(" ");
        invalidate();
    }

    public InformationView(Context context) {
        super(context);
        initialize(context);
        Log.d("INFO","1");
    }

    public InformationView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize(context);
        Log.d("INFO","2");
    }

    public InformationView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize(context);
        Log.d("INFO","3");
    }

    private void initialize(Context context) {
        Display display = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();

        Bitmap bitmap = ((BitmapDrawable) getDrawable()).getBitmap();

        Point size0 = new Point();
        display.getSize(size0);
        int width = size0.x;
        int height = size0.y;
        Log.d("INFO","w:"+width);
        Log.d("INFO","h:"+height);

        mScale = BitmapHelper.getScaleFactor(bitmap, width, height);
        mRect = BitmapHelper.getBitmapOpaqueRect(bitmap);

        mLayout = new FrameLayout(context);

        Typeface typeface = Typeface.createFromAsset(context.getAssets(), "fonts/jefferies.otf");


        float size = width / 20f;

        int x = 33; //20
        int y = 395; //658
        mTvTime = new TextView(context);
        {
            mTvTime.setTypeface(typeface);
            mTvTime.setTextColor(Color.WHITE);
            mTvTime.setTextSize(TypedValue.COMPLEX_UNIT_SP, size);
//            mTvTime.setPadding((int) (80f * mScale.x), (int) ((mRect.bottom - 110f) * mScale.y), 0, 0);
            mTvTime.setPadding((int) (80f * mScale.x), (int) ((mRect.bottom - lowPad) * mScale.y), 0, 0);
            mTvTime.setText("00:00 ");
        }

        Log.d("INFO","x:"+ mRect.bottom +"y:"+  mScale.y) ;

        mTvRpm = new TextView(context);
        {
            mTvRpm.setTypeface(typeface);
            mTvRpm.setTextColor(Color.WHITE);
            mTvRpm.setTextSize(TypedValue.COMPLEX_UNIT_SP, size);
        }



        mLayout.addView(mTvTime);
        mLayout.addView(mTvRpm);

        

        setRpm(0);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        mLayout.measure(canvas.getWidth(), canvas.getHeight());
        mLayout.layout(0, 0, canvas.getWidth(), canvas.getHeight());
        mLayout.draw(canvas);
    }
}
