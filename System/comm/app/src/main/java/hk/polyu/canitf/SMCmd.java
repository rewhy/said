package hk.polyu.canitf;




public class SMCmd {
	public static final int PORT_NUM = 6001;
	public static final int FRAME_DATA_MAX = 255;
	public static final int PAYLOAD_DATA_MAX = 220;



	public static final byte START1 = (byte) 0xFF;
	public static final byte START2 = (byte) 0xFE;
	public static final byte END1 = START2;
	public static final byte END2 = START1;


	//cmd from source manager:
	public static final byte CMDF_BASE_CTRL = 0x01;
	public static final byte CMDF_RET_DATA = 0x09;


	//sub cmd of CMDF_RET_DATA
	public static final byte SCMD_RET_WORK_MODE = 0x01;


	//cmd to source manager.
	public static final byte CMDT_BASE_STATE = (byte) 0x81;
	public static final byte CMDT_HEART_BEAT = (byte) 0x83;
	public static final byte CMDT_REG_APP = (byte) 0x8E;


	public static final int SOCKET_CONNECT_SUCCESS = 8000;
	public static final int AUTH_FAILED = 8001;
	public static final int TIMER_1_SEC = 8002;
	public static final int RX_MSG = 8888;
}



