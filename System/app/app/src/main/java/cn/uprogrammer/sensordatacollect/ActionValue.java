package cn.uprogrammer.sensordatacollect;

public class ActionValue {
    //Part State condition analysis
    public static final String EQUAL = "11";
    public static final String GREATER = "20";
    public static final String LESS = "00";
    public static final String GREATEREQUAL = "21";
    public static final String LESSEQUAL = "01";
    public static final String UNEQUAL ="10";

    //Decision
    public static final int MISMATCHING = 0;
    public static final int ACCEPT = 1;
    public static final int REJECT = 2;

    public static final int ENQUIRY = 3;

    public static final String[] Warning={
            "RPM is too high!!!",
            "Stopped without flameout!",
            "Turn signal lamp on while straight driving",
            "Not turn left signal lamp while turning left",
            "Not turn left signal lamp while Left_Lane_Change",
            "Not turn left signal lamp while left-U-Turn",
            "Not turn right signal lamp while turning right",
            "Not turn right signal lamp while Right_Lane_Change",
            "Not turn right signal lamp while right-U-Turn",
            "Rapid accelerate while making a turn",
            "Open the window with high speed",
            "Open the window with high speed",
            "Open the window with high speed",
            "Open the window with high speed",
            "Open the door when car is moving",
            "Open the door when car is moving",
            "Open the door when car is moving",
            "Open the door when car is moving",
            "Not set the rearviewer mirrors when car is moving",
            "Gear 1 when speed is too high",
            "Gear 2 when speed is too high",
            "Gear 3 when speed is too low",
            "Gear 3 when speed is too high",
            "Gear 4 when speed is too low",
            "Gear 4 when speed is too high",
            "Gear 5 when speed is too low",
    };

    public static final String[] DECISION={
            "passed!",
            "filtered out!"
    };

    public static final String[] ACTION={
            "skeda",
            "get-vin",
            "unlock",
            "lock",
            "openwindow",
            "closewindow",
            "closervm",
            "openrvm",
            "dipped_headlight",
            "high_beam",
            "turn_signal_lamp",
            "foglight",
            "rearfoglamp",
            "brakelamp",
            "centrallockindicatorlight",
            "reversing_lamp",
            "dome_colck",
            "windscreen_washers",
            "signal_horn",
            "frontdriverside_closewindow",
            "frontdriverside_openwindow",
            "reardriverside_closewindow",
            "reardriverside_openwindow",
            "frontpassenger_closewindow",
            "frontpassenger_openwindow",
            "rearpassenger_closewindow",
            "rearpassenger_openwindow",
            "lighting",
            "indicatorlight"
    };

}
