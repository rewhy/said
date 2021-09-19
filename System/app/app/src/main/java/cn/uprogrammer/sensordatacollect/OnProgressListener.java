package cn.uprogrammer.sensordatacollect;

import java.util.List;

public interface OnProgressListener {
    void onProgress(int progress);
    void onWarn(List warnlist);
}
