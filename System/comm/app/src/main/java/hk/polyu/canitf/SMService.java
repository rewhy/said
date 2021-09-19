package hk.polyu.canitf;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.OutputStream;

public class SMService implements SMInterface {
	// private static final String Version = "1.16s";
	private static String TAG = "canitfservice";
	private static boolean DEBUG = false;
	private static final int CMD_SEND_MSG = 10;
	private LocalSocket socket = null;
	private Handler mPublicHandler;
	private OutputStream ou = null;
	private BufferedInputStream mIn = null;
	private boolean isSocketConnected = false;
	private boolean heartBeatOn = false;
	private HeartBeatThread hbt = null;
	private ReceiveThread rt = null;
	private Handler mPrivateHandler = null;
	private Token token = null;
	private static boolean isSocketReady = false;

	private volatile static SMService mService = null;

	public static SMService getSMService(Handler handler) {
		if (mService == null) {
			synchronized (SMService.class) {
				if (mService == null) {
					mService = new SMService(handler);
				}
			}
		}
		isSocketReady = false;
		return mService;
	}

	public Handler getHandler() {
		return mPublicHandler;
	}

	public void setHandler(Handler mHandler) {

		mPublicHandler = mHandler;
	}

	public SMService(Handler handler) {
		mService = this;
		doInit(handler);
		isSocketReady = false;
	}

	private void doInit(Handler handler) {

		mPublicHandler = handler;


		mPrivateHandler = runSendThread();

		hbt = new HeartBeatThread(SMCmd.PORT_NUM);
		new Thread(hbt).start();

		token = new Token();
	}

	Handler runSendThread() {

		HandlerThread hThread = new HandlerThread("SendThread");

		hThread.start();

		Looper looper = hThread.getLooper();

		Handler h = new Handler(looper) {
			@Override
			public void handleMessage(Message msg) {
				// TODO Auto-generated method stub
				if (isSocketConnected) {
					switch (msg.what) {
					case CMD_SEND_MSG: {
						try {
							ou.write((byte[]) msg.obj);
							ou.flush();

							if (!isSocketReady) {
								// notify main handler.
								mPublicHandler.sendEmptyMessage(SMCmd.SOCKET_CONNECT_SUCCESS);
								isSocketReady = true;
							}

							if (DEBUG) {
								Log.d(TAG, "CMD_SEND_MSG write success!");
							}

						} catch (IOException e) {
							// TODO Auto-generated catch block
							Log.d(TAG, "CMD_SEND_MSG IOException");
							isSocketConnected = false;
							try {
								socket.close();
								socket = null;
							} catch (IOException e1) {
								// TODO Auto-generated catch block
								e1.printStackTrace();
							}
							e.printStackTrace();
						}

					}
						break;

					default:
						break;
					}

				} else {
					removeMessages(0);
				}

				super.handleMessage(msg);
			}
		};

		return h;
	}

	class HeartBeatThread implements Runnable {
		private int port;
		private int retries = 0;
		private String SOCKET_NAME = "canitfsocket";

		public HeartBeatThread(int port) {
			this.port = port;
		}

		public void connect() {

			// LocalSocket s =null;
			LocalSocketAddress l;
			socket = new LocalSocket();
			l = new LocalSocketAddress(SOCKET_NAME, LocalSocketAddress.Namespace.RESERVED);

			retries++;
			if (DEBUG) {
				Log.i(TAG, "connect InetSocketAddress:" + port + " --retries:" + retries);
			}

			try {
				socket.connect(l);
			} catch (IOException e) {
				e.printStackTrace();

				try {
					socket.close();
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}

				socket = null;
				e.printStackTrace();
				return;

			}

			try {
				ou = socket.getOutputStream();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			if (DEBUG) {
				Log.i(TAG, " reset getInputStream");
			}
			try {
				mIn = new BufferedInputStream(socket.getInputStream());
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			isSocketConnected = true;

			// register to source manager.
			byte[] wrBuff = getTxHeartBeat();
			Message msg = mPrivateHandler.obtainMessage(CMD_SEND_MSG, wrBuff);
			mPrivateHandler.sendMessageDelayed(msg, 1);

			if (rt == null) {
				rt = new ReceiveThread();
				new Thread(rt).start();
			}

		}

		public void run() {
			try {
				if (DEBUG) {
					Log.i(TAG, "new socket connect :" + port);
				}

				connect();

				do {

					if (isSocketConnected) {

						byte[] wrBuff = getTxHeartBeat();

						while (true) {
							Thread.sleep(10000);
							if (heartBeatOn) {

								if (DEBUG) {
									Log.i(TAG, "heart beat");
								}

								Message msg = mPrivateHandler.obtainMessage(CMD_SEND_MSG, wrBuff);
								mPrivateHandler.sendMessageDelayed(msg, 1);
							}

							if (isSocketConnected == false) {
								break;
							}
						}
					}

					do {
						Thread.sleep(1000);
						if (DEBUG) {
							Log.i(TAG, "reconnect");
						}
						connect();
					} while (isSocketConnected == false);

				} while (true);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	class ReceiveThread implements Runnable {
		private int totalDataLength = 0;

		byte[] buff = new byte[SMCmd.FRAME_DATA_MAX];
		byte offset = 0;
		int rxDataLength = 0;

		void doResetBuffer(byte[] buff) {
			for (int i = 0; i < buff.length; i++) {
				buff[i] = 0;
			}

			totalDataLength = 0;
		}

		private void doReadBuffer() {
			int r = -1;
			try {
				while ((r = mIn.read()) != -1) {

					if (DEBUG) {
						Log.i(TAG, "read byte:" + Integer.toHexString(r));
					}

					if (offset >= buff.length) {
						offset = 0;
						doResetBuffer(buff);
					}

					buff[offset] = Byte.valueOf((byte) r);
					offset++;

					switch (offset) {
					case 1:
						if (buff[0] != SMCmd.START1) {
							offset = 0;
							doResetBuffer(buff);
						}
						break;
					case 2:
						if (buff[1] != SMCmd.START2) {
							if (buff[1] == SMCmd.START1) {
								buff[0] = buff[1];
								buff[1] = 0;
								offset = 1;
							} else {
								offset = 0;
								doResetBuffer(buff);
							}
						}
						break;
					case 3:
						// if(buff[2] != mAppId) {
						// offset = 0;
						// doResetBuffer(buff);
						// }
						break;
					case 4:
						totalDataLength = buff[3] + 2;
						rxDataLength = 0;
						break;

					default: {
						rxDataLength++;

						if (rxDataLength >= totalDataLength) {
							if ((buff[offset - 2] == SMCmd.END1) && (buff[offset - 1] == SMCmd.END2)) {
								byte[] buffer = getDataBuff(buff, 2, (totalDataLength));

								if (buffer != null) {
									// get data buffer success!
									// check rx data. if it is the blocked data.
									// notify.
									byte cmd = buffer[1];
									boolean isSyncData = false;

									if (cmd == SMCmd.CMDF_RET_DATA) {
										Log.d(TAG, " 11 SMCmd.CMDF_RET_DATA " + buffer[2]);
										if (buffer[2] == SMCmd.SCMD_RET_WORK_MODE) {
											isSyncData = true;
										}
									}

									if (isSyncData) {
										synchronized (token) {
											token.setData(buffer[2]);
											token.notifyAll();
											Log.d(TAG, " SMCmd.CMDF_RET_DATA " + buffer[2]);
										}
									} else {
										Message msg = mPublicHandler.obtainMessage(SMCmd.RX_MSG, buffer);
										mPublicHandler.sendMessageDelayed(msg, 1);
									}
								}
							}

							offset = 0;
							doResetBuffer(buff);
						}

					}
						break;
					}

				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				if (DEBUG) {
					Log.i(TAG, "Close in buffer");
				}

				try {
					mIn.close();
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}

				mIn = null;
				e.printStackTrace();
			}

		}

		public void run() {
			while (true) {
				if (mIn != null) {
					doReadBuffer();
				}
			}
		}

		private byte[] getDataBuff(byte[] buff, int offset, int count) {
			byte[] buffer = new byte[count];

			if (offset + count > buff.length) {
				return null;
			}

			for (int i = 0; i < count; i++) {
				buffer[i] = buff[i + offset];
			}

			return buffer;
		}
	}

	public static String bytesToString(byte[] b, int offset, int leng) {
		StringBuffer result = new StringBuffer("");
		if (offset + leng >= b.length) {
			return "";
		}

		for (int i = offset; i < leng; i++) {
			result.append((char) (b[i] & 0xff));
		}
		return result.toString();
	}

	private byte[] getTxHeartBeat() {
		return getTxBuffer(SMCmd.CMDT_HEART_BEAT);
	}

	private byte[] getTxBuffer(byte cmd) {
		byte dataLenth = 1;
		int bufLength = 6 + dataLenth;
		int dataOffset = 4;
		byte[] wrBuff = new byte[bufLength];

		wrBuff[0] = SMCmd.START1;
		wrBuff[1] = SMCmd.START2;
		wrBuff[2] = 0x00;
		wrBuff[3] = dataLenth;
		wrBuff[dataOffset++] = cmd;
		wrBuff[bufLength - 2] = SMCmd.END1;
		wrBuff[bufLength - 1] = SMCmd.END2;

		return wrBuff;
	}

	private byte[] getTxBuffer(byte cmd, byte subCmd) {
		byte dataLenth = 2;
		int bufLength = 6 + dataLenth;
		int dataOffset = 4;
		byte[] wrBuff = new byte[bufLength];

		wrBuff[0] = SMCmd.START1;
		wrBuff[1] = SMCmd.START2;
		wrBuff[2] = 0x00;
		wrBuff[3] = dataLenth;
		wrBuff[dataOffset++] = cmd;
		wrBuff[dataOffset++] = subCmd;
		wrBuff[bufLength - 2] = SMCmd.END1;
		wrBuff[bufLength - 1] = SMCmd.END2;

		return wrBuff;
	}

	private byte[] getTxBuffer(byte cmd, byte[] payload) {
		int dataLenth = 1 + payload.length;
		int bufLength = 6 + dataLenth;
		int dataOffset = 4;
		byte[] wrBuff = new byte[bufLength];

		wrBuff[0] = SMCmd.START1;
		wrBuff[1] = SMCmd.START2;
		wrBuff[2] = 0x00;
		wrBuff[3] = (byte) dataLenth;
		wrBuff[dataOffset++] = cmd;

		System.arraycopy(payload, 0, wrBuff, dataOffset, payload.length);
		dataOffset += payload.length;

		wrBuff[bufLength - 2] = SMCmd.END1;
		wrBuff[bufLength - 1] = SMCmd.END2;

		return wrBuff;
	}

	public void sendBaseCmd(byte subCmd, byte appId) {
		byte[] payload = new byte[2];
		payload[0] = subCmd;
		payload[1] = appId;

		byte[] wrBuff = getTxBuffer(SMCmd.CMDT_BASE_STATE, payload);
		Message msg = mPrivateHandler.obtainMessage(CMD_SEND_MSG, wrBuff);
		mPrivateHandler.sendMessageDelayed(msg, 1);
	}

	public void heartBeatEnable() {
		heartBeatOn = !heartBeatOn;
	}

	public void close() {
	}


	@Override
	public void registerApp(byte appId) {
		// mAppId = appId;
		// byte[] wrBuff = getTxHeartBeat();
		// Message msg = mPrivateHandler.obtainMessage(CMD_SEND_MSG,wrBuff);
		// mPrivateHandler.sendMessageDelayed(msg, 1);
		byte[] wrBuff = getTxBuffer(SMCmd.CMDT_REG_APP, appId);
		Message msg = mPrivateHandler.obtainMessage(CMD_SEND_MSG, wrBuff);
		mPrivateHandler.sendMessageDelayed(msg, 1);
	}



	public void connect() {

	}

	public void disconnect() {

		isSocketConnected = false;
		try {
			socket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		socket = null;
	}


	private class Token {
		private int data;

		public Token() {
			setData(0);
		}

		public void setData(int data) {
			this.data = data;
		}

		public int getData() {
			return data;
		}
	}


}
