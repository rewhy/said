package cn.uprogrammer.sensordatacollect.helper;

import android.content.Context;
import android.content.DialogInterface;
import android.os.CountDownTimer;
import android.support.annotation.NonNull;
import android.support.annotation.StyleRes;
import android.support.v7.app.AlertDialog;
import android.widget.Button;

import java.util.Locale;
import java.util.concurrent.TimeUnit;

public class AlertTimerDialog extends AlertDialog {

    protected AlertTimerDialog(@NonNull Context context) {
        super(context);
    }

    private static class Params {

        boolean isInitialized = false;
        int whichButton = -9;
        int delay = -9;
    }

    public static class Builder extends AlertDialog.Builder {

        private final Params mParams = new Params();

        private CountDownTimer mTimer;

        public Builder(@NonNull Context context) {
            super(context);
        }

        public Builder(@NonNull Context context, @StyleRes int themeResId) {
            super(context, themeResId);
        }

        @Override
        public Builder setCancelable(boolean cancelable) {
            super.setCancelable(cancelable);
            return this;
        }

        @Override
        public Builder setMessage(CharSequence message) {
            super.setMessage(message);
            return this;
        }

        @Override
        public Builder setNegativeButton(CharSequence text, OnClickListener listener) {
            super.setNegativeButton(text, listener);
            return this;
        }

        @Override
        public Builder setNeutralButton(CharSequence text, OnClickListener listener) {
            super.setNeutralButton(text, listener);
            return this;
        }

        @Override
        public Builder setOnCancelListener(OnCancelListener onCancelListener) {
            super.setOnCancelListener(onCancelListener);
            return this;
        }

        @Override
        public Builder setOnDismissListener(OnDismissListener onDismissListener) {
            super.setOnDismissListener(onDismissListener);
            return this;
        }

        @Override
        public Builder setPositiveButton(CharSequence text, OnClickListener listener) {
            super.setPositiveButton(text, listener);
            return this;
        }

        @Override
        public Builder setTitle(CharSequence title) {
            super.setTitle(title);
            return this;
        }

        public Builder setTimer(int whichButton, int delay) {
            mParams.whichButton = whichButton;
            mParams.delay = delay;
            mParams.isInitialized = true;

            return this;
        }

        @Override
        public AlertDialog create() {
            final AlertDialog dialog = super.create();

            if (mParams.isInitialized) {
                setTimer(dialog, mParams.whichButton, mParams.delay);

                dialog.setOnCancelListener(new OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        mTimer.cancel();
                    }
                });
            }

            return dialog;
        }

        @Override
        public AlertDialog show() {
            final AlertDialog dialog = create();
            dialog.show();
            return dialog;
        }

        private void setTimer(final AlertDialog dialog, final int whichButton, final int delay) {
            dialog.setOnShowListener(new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialogInterface) {
                    final Button button = dialog.getButton(whichButton);
                    final CharSequence text = button.getText();

                    mTimer = new CountDownTimer(delay, 100) {
                        @Override
                        public void onTick(long millisUntilFinished) {
                            button.setText(String.format(
                                    Locale.getDefault(),
                                    "%s (%d)",
                                    text,
                                    TimeUnit.MILLISECONDS.toSeconds(millisUntilFinished) + 1
                            ));
                        }

                        @Override
                        public void onFinish() {
                            try {
                                dialog.dismiss();
                            } catch (Exception e) {
                                e.printStackTrace();
                            } finally {
                                button.performClick();
                                dialog.cancel();
                            }
                        }
                    }.start();
                }
            });
        }
    }
}
