package com.lang.y_eesp32_smartglasses_r2;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import android.view.KeyEvent;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Message;
import android.view.View;
import android.widget.Toast;

public class MainService extends Service {
    private String deviceName = null;
    private String deviceAddress;
    public static Handler handler;
    public static BluetoothSocket mmSocket;
    public static ESP32.ConnectedThread connectedThread;
    public static ESP32.CreateConnectThread createConnectThread;


    final static String TAG = "MyInfoService";
    static Context context;


    Message msg;

    private static BluetoothAdapter mBluetoothAdapter;

    static boolean isConnected = false;
    static BluetoothGattCharacteristic foundCharacteristic = null;
    static BluetoothGatt bluetoothGattC = null;

    private final static int CONNECTING_STATUS = 1; // used in bluetooth handler to identify message status
    private final static int MESSAGE_READ = 2; // used in bluetooth handler to identify message update

//    private static final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
//
//
//
//        @Override
//        public void onCharacteristicChanged(BluetoothGatt gatt, final BluetoothGattCharacteristic characteristic) {
//            String value_status = characteristic.getStringValue(0);
//            Log.i(TAG, "onCharacteristicChanged: " + value_status);
//
//            if (value_status != null && value_status.equals("1")) {
//                ESP32.ScreenState.TIME_LAST_INVOKE = System.currentTimeMillis();
//                ESP32.STATUS = ESP32Status.OLED_OFF;
//            } else if (value_status != null) ESP32.onInterrupt(value_status);
//        }
//
//        @Override public void onConnectionStateChange(final BluetoothGatt gatt, final int status, final int newState) {
//            if (newState == BluetoothProfile.STATE_CONNECTED) {
//                Log.i(TAG, "Attempting to start service discovery:" + gatt.discoverServices());
//                isConnected = true;
//            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
//                Log.i(TAG, "Disconnected from GATT server.");
//                isConnected = false;
//            }
//        }
//
//        @Override public void onServicesDiscovered(final BluetoothGatt gatt, final int status) {
//
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                Log.i(TAG, "Discovered services");
//                //tvStatus.setText("Connected");
//
//                bluetoothGattC = gatt;
//
//                List<BluetoothGattService> lServices = gatt.getServices();
//                for (BluetoothGattService service : lServices) {
//
//                    UUID uuid = UUID.fromString(sCHARACTERISTIC_UUID);
//                    BluetoothGattCharacteristic localCharacteristic = service.getCharacteristic(uuid);
//                    if (localCharacteristic != null) {
//                        foundCharacteristic = localCharacteristic;
//                        break;
//                    }
//                }
//
//                gatt.setCharacteristicNotification(foundCharacteristic, true);
//
//                // *** Startup commands ***
//                ESP32.startupSection();
//
//            } else {
//                Log.i(TAG, "Discovering services failed");
//                // tvStatus.setText("Disconnected");
//            }
//        }
//    };

    private interface MessageConstants {
        public static final int MESSAGE_READ = 0;
        public static final int MESSAGE_WRITE = 1;
        public static final int MESSAGE_TOAST = 2;

        // ... (Add other message types here as needed.)
    }

    @Nullable @Override public IBinder onBind(Intent intent) {
        return null;//return mMessenger.getBinder();
    }

    @Override public void onCreate() {
        super.onCreate();

        context = this;



//       final BluetoothManager mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
//       mBluetoothAdapter = mBluetoothManager.getAdapter();

    }


    static class ESP32Status {
        static final int OLED_ON = 0;
        static final int OLED_OFF = 1;
    }

    static class ESP32 {
        static int STATUS = MainService.ESP32Status.OLED_ON;

        // Saved last evoked screen type (when user go to one of the screens)
        static class ScreenState {
            static final long TIME_IDLE_GO_HOME = 90000; // After this time with OLED off go back to mainscreen
            static long TIME_LAST_INVOKE = 0;

            static final int SCREEN_MAIN = 0;
            static final int SCREEN_MUSI = 1;
            static final int SCREEN_MSGN = 2;
            static final int SCREEN_CALL = 3;
            static final int SCREEN_LIST = 4;

            static final int SCREEN_NAVI = 5;                                                       //disabled

            static int CURRENT_SCREEN = SCREEN_MAIN;

            static String
                    prev_MsgScreen = "#1|280|None|No recent msg",
                    prev_CallScreen = "#2|No recent calls",
                    prev_NaviScreen = "#3|0|0|0|280",
                    prev_ListScreen = "#4|257|No List|280|No list",
                    prev_MusicScreen = "#5|182|No music playing|210|214";
        }


        //TODO  ********************************** Startup section - write cmds which will be sent after successful connection ***************************************8
        static void startupSection() {
            new Thread() {
                public void run() {
                    try {
                        MainActivity.tempowrite("#MX=20");
                        sleep(1000);
                        MainActivity.tempowrite("#TST=100");
                        sleep(1000);


                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }.start();
        }
        //TODO ******************************************************************************************


        static boolean oldconnect(String address) {
            Log.i(TAG, "Connecting to: " + address);
            if (mBluetoothAdapter == null || address == null) {
                Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");
                return false;

            }

            BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);

           // BluetoothGatt bluetoothGatt = device.connectGatt(context, false, mGattCallback);
            Log.d(TAG, "Trying to create a new connection.");

            return true;
        }

        static void disconnect() {
            if (bluetoothGattC != null) {
                bluetoothGattC.disconnect();
                new Handler().postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        bluetoothGattC.close();
                        bluetoothGattC = null;
                        isConnected = false;
                    }
                }, 1000);
            }
        }

        static boolean isConnected() {
            return foundCharacteristic != null && bluetoothGattC != null && isConnected;
        }


        //        static void writetoESP(String input) {
//            byte[] bytes = input.getBytes(); //converts entered String into bytes
//            try {
//                mmOutStream.write(bytes);
//            } catch (IOException e) {
//                Log.e("Send Error", "Unable to send message", e);
//            }
//        }
        static void showLastScreen(int screenId) {
            switch (screenId) {
                case ScreenState.SCREEN_MAIN:
                    onHome();
                    break;
//                case ScreenState.SCREEN_NAVI: writetoESP(ScreenState.prev_NaviScreen) ; break;    // For now disabled maybe upgrade in future? :) Or YOU gonna do it? Are you? .. Do it :P

                case ScreenState.SCREEN_MSGN:
                    MainActivity.tempowrite(ScreenState.prev_MsgScreen);
                    break;
                case ScreenState.SCREEN_CALL:
                    MainActivity.tempowrite(ScreenState.prev_CallScreen);
                    break;
                case ScreenState.SCREEN_LIST:
                    onList();
                    break;
                case ScreenState.SCREEN_MUSI:
                    MainActivity.tempowrite(ScreenState.prev_MusicScreen);
                    break;
            }
        }
        static void onHome() {
//            SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.getDefault());
            String HH = new SimpleDateFormat("HH", Locale.getDefault()).format(new Date());
            String mm = new SimpleDateFormat("mm", Locale.getDefault()).format(new Date());

//            sdf = new SimpleDateFormat("dd MMM", Locale.getDefault());
            String date = new SimpleDateFormat("dd MMM", Locale.getDefault()).format(new Date());

            mainScreen(HH, mm, date, "-1", "");
        }
        static int currentListPos=0;
        static void onList() {
            String list = MainActivity.readSharedPreferencesList();

            ArrayList<String> aList = new ArrayList<>();

            Pattern p = Pattern.compile("-(.*)");
            Matcher m = p.matcher(list);

            while (m.find()) {
                aList.add(m.group(1));
            }

            if (currentListPos >= aList.size()) currentListPos = 0;

            listScreen("257", MainActivity.readSharedPreferencesTitle(), "221", aList.get(currentListPos).trim());
        }


        /* --- Screens --- */
        static void mainScreen(String HH, String mm, String date, String symbol, String degrees) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_MAIN;
            String t = "#0|" + HH + "|" + mm + "|" + date + "|" + symbol + "|" + degrees;
            MainActivity.tempowrite(t);
        }

        static void msgNotiScreen(String symbol, String from, String text) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_MSGN;
            String t = "#1|" + symbol + "|" + from + "|" + text;
            ScreenState.prev_MsgScreen = t;
            MainActivity.tempowrite(t);
        }

        //contains t_solresult
        static void callScreen(String from) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_CALL;
            String t = "#2|" + Globals.t_solresult;
            ScreenState.prev_CallScreen = t;
            MainActivity.tempowrite(t);
        }

        static void navScreen(String maxSpeed, String distance, String distanceToDes, String symbol) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_NAVI;
            String t = "#3|" + maxSpeed + "|" + distance + "|" + distanceToDes + "|" + symbol;
            ScreenState.prev_NaviScreen = t;
            MainActivity.tempowrite(t);
        }

        static void listScreen(String symbolMain, String title, String symbolSub, String text) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_LIST;
            String t = "#4|" + symbolMain + "|" + title + "|" + symbolSub + "|" + text;
            ScreenState.prev_ListScreen = t;
            MainActivity.tempowrite(t);
        }

        static void musicScreen(String musicIcon, String title, String symbolPlayStop, String symbolNext) {
            ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_MUSI;
            String t = "#5|" + musicIcon + "|" + title + "|" + symbolPlayStop + "|" + symbolNext;
            ScreenState.prev_MusicScreen = t;
            MainActivity.tempowrite(t);
        }

        static void sendToast (String msg) {
//          if (Looper.myLooper() == Looper.getMainLooper()) {
//              Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
//
//          } else {
//              new Handler(Looper.getMainLooper()).post(new Runnable() {
//                  @Override
//                  public void run() {
//                      Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
//                  }
//              });
//          }



            Handler handler = new Handler(Looper.getMainLooper());
            handler.post(new Runnable() {
                public void run() {
                    Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
                }
            });
        }



        /* --- On Touch Click on ESP32 --- */
        static void onInterrupt(String interruptName) {
            // @Override
            // public void handleMessage(Message msg){
//                                                                                static void onInterrupt(String interruptName) {
            Log.i(TAG, "Interrupt: " + interruptName);


            /* --- All Actions --- */

            // After time of idle go back to home
            if ((System.currentTimeMillis() - ScreenState.TIME_LAST_INVOKE) >= ScreenState.TIME_IDLE_GO_HOME)
                ScreenState.CURRENT_SCREEN = ScreenState.SCREEN_MAIN;

            // If oled off 1 click turn on last viewed screen
            if (interruptName.equals("#TS1") && STATUS == 1) {
                showLastScreen(ScreenState.CURRENT_SCREEN);

            } else if (interruptName.equals("#TS0")) {      // Long click changes the screens
                if (ScreenState.CURRENT_SCREEN >= 4) ScreenState.CURRENT_SCREEN = -1;
                showLastScreen(++ScreenState.CURRENT_SCREEN);
            }

            // Screen Actions
            else if (ScreenState.CURRENT_SCREEN == ScreenState.SCREEN_MUSI) {
//                if (interruptName.equals("#TS1")) {
//                    boolean isplaying = MainService.Music.musicToggle();
//
////                    "#5|182|No music playing|210|214"
//                    if (isplaying) musicScreen("182", "Music", "210", "214");
//                    else musicScreen("182", "Music", "211", "214");
//                } else if (interruptName.equals("#TS2")) {
//                    MainService.Music.musicNext();
//
//                }
            } else if (ScreenState.CURRENT_SCREEN == ScreenState.SCREEN_LIST) {
                if (interruptName.equals("#TS1")) {
                    currentListPos++;
                    onList();
                }
            }

            //Sense Word
            else if (interruptName.length() == 1) {
                if (interruptName.equals("]")) {
                    Globals.t_typewordResultFINAL = Globals.r_typewordResult;
                    Globals.r_typewordResult = "";

                } else {
                    Globals.r_typewordResult = Globals.r_typewordResult + interruptName;
                }
            }


            //Battery Status
            else if (interruptName.equals("#b")) {
                Globals.espBattery = interruptName.substring(2);
                MainActivity.tempowrite("o");

            }


            // Awoken - nvm which key pattern
            STATUS = 0;



//        static void onHome() {
////            SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.getDefault());
//            String HH = new SimpleDateFormat("HH", Locale.getDefault()).format(new Date());
//            String mm = new SimpleDateFormat("mm", Locale.getDefault()).format(new Date());
//
////            sdf = new SimpleDateFormat("dd MMM", Locale.getDefault());
//            String date = new SimpleDateFormat("dd MMM", Locale.getDefault()).format(new Date());
//
//            mainScreen(HH, mm, date, "-1", "");
//        }




//        static void showLastScreen(int screenId) {
//            switch (screenId) {
//                case ScreenState.SCREEN_MAIN:
//                    onHome();
//                    break;
////                case ScreenState.SCREEN_NAVI: writetoESP(ScreenState.prev_NaviScreen) ; break;    // For now disabled maybe upgrade in future? :) Or YOU gonna do it? Are you? .. Do it :P
//
//                case ScreenState.SCREEN_MSGN:
//                    MainActivity.tempowrite(ScreenState.prev_MsgScreen);
//                    break;
//                case ScreenState.SCREEN_CALL:
//                    MainActivity.tempowrite(ScreenState.prev_CallScreen);
//                    break;
//                case ScreenState.SCREEN_LIST:
//                    onList();
//                    break;
//                case ScreenState.SCREEN_MUSI:
//                    MainActivity.tempowrite(ScreenState.prev_MusicScreen);
//                    break;
//            }
//        }



        }

        /* ============================ Thread to Create Bluetooth Connection =================================== */
        public static class CreateConnectThread extends Thread {

            public CreateConnectThread(BluetoothAdapter bluetoothAdapter, String address) {
            /*
            Use a temporary object that is later assigned to mmSocket
            because mmSocket is final.
             */
                BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(address);
                BluetoothSocket tmp = null;
                UUID uuid = bluetoothDevice.getUuids()[0].getUuid();

                try {
                /*
                Get a BluetoothSocket to oldconnect with the given BluetoothDevice.
                Due to Android device varieties,the method below may not work fo different devices.
                You should try using other methods i.e. :
                 */
                    tmp = bluetoothDevice.createInsecureRfcommSocketToServiceRecord(uuid);
                    //tmp = bluetoothDevice.createRfcommSocketToServiceRecord(uuid);

                } catch (IOException e) {
                    Globals.system="cannot connect to device";
                    Globals.bleOn=0;
                    Log.e(TAG, "Socket's create() method failed", e);

                }
                mmSocket = tmp;
            }

            @RequiresApi(api = Build.VERSION_CODES.N)
            public void run() {
                // Cancel discovery because it otherwise slows down the connection.
                BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                bluetoothAdapter.cancelDiscovery();
                try {
                    // Connect to the remote device through the socket. This call blocks
                    // until it succeeds or throws an exception.
                    mmSocket.connect();
                    Log.e("Status", "Device connected");
                    // handler.obtainMessage(CONNECTING_STATUS, 1, -1).sendToTarget();
                    sendToast("Device Connected");
                    Globals.bleOn=1;
                } catch (IOException connectException) {
                    // Unable to oldconnect; close the socket and return.
                    try {
                        mmSocket.close();
                        Log.e("Status", "Cannot connect to device");
                        Globals.bleOn=0;
                        // handler.obtainMessage(CONNECTING_STATUS, -1, -1).sendToTarget();
                        Globals.system="cannot connect to device";
                        sendToast("Cannot connect to device");
                    } catch (IOException closeException) {
                        Log.e(TAG, "Could not close the client socket", closeException);
                    }
                    return;
                }
                ESP32.startupSection();
                // The connection attempt succeeded. Perform work associated with
                // the connection in a separate thread.
                connectedThread = new ConnectedThread(mmSocket);
                //IMPORTANT! REQUIRES HIGHER API LEVEL
                connectedThread.run();
            }

            // Closes the client socket and causes the thread to finish.
            public void cancel() {
                try {
                    mmSocket.close();
                } catch (IOException e) {
                    Log.e(TAG, "Could not close the client socket", e);
                }
            }
        }


        //data transfer thread



        public static class ConnectedThread extends Thread {
            private final BluetoothSocket mmSocket;
            private final InputStream mmInStream;
            public final OutputStream mmOutStream;
            private byte[] mmBuffer; // mmBuffer store for the stream


            public ConnectedThread(BluetoothSocket socket) {
                mmSocket = socket;
                InputStream tmpIn = null;
                OutputStream tmpOut = null;

                // Get the input and output streams, using temp objects because
                // member streams are final
                try {
                    tmpIn = socket.getInputStream();
                    tmpOut = socket.getOutputStream();
                } catch (IOException e) {
                }

                mmInStream = tmpIn;
                mmOutStream = tmpOut;
            }


            @RequiresApi(api = Build.VERSION_CODES.N)
            public void run() {
                mmBuffer = new byte[1024];
                int numBytes; // secondary backup for bytes returned from read()

                byte[] buffer = new byte[1024];  // buffer store for the stream
                int bytes = 0; // bytes returned from read()
                // Keep listening to the InputStream until an exception occurs
                while (true) {
                    try {
                    /*
                    Read from the InputStream from Arduino until termination character is reached.
                    Then send the whole String message to GUI Handler.
                     */


                        //numBytes = mmInStream.read(mmBuffer); // secondary backup
                        buffer[bytes] = (byte) mmInStream.read();
                        String readMessage;
                        //String SecondReadMessage = mmInStream;

//                    String probmessage = new BufferedReader(new InputStreamReader(mmInStream))
//                            .lines().collect(Collectors.joining("\n"));

//                    if (probmessage != null){
//                      Globals.r_arduinoMsgvar= SecondReadMessage;
//                              mmInStream.removeAll(mmInStream);
//
//
//
//                    }
//                    if (buffer[bytes] == '\n') {
//                        readMessage = new String(buffer, 0, bytes);
//                                                        Log.e("Arduino Message", readMessage);
//                                                        Message readMsg = handler.obtainMessage(
//                                                                MessageConstants.MESSAGE_READ, numBytes, -1,
//                                                                mmBuffer);
//                                                        readMsg.sendToTarget();
//                        handler.obtainMessage(MESSAGE_READ,readMessage).sendToTarget();
//                        ESP32.onInterrupt(readMessage);
//                        Globals.r_arduinoMsgvar= readMessage;
//
//                        bytes = 0;
//                    } else {
//                        bytes++;
//                    }

                        if (buffer[bytes] == '\n') {
                            readMessage = new String(buffer, 0, bytes);
                            Log.e("Arduino Message", readMessage);
                            // handler.obtainMessage(MESSAGE_READ, readMessage).sendToTarget();
                            sendToast("in- "+ readMessage);
                            Globals.system=readMessage;
                            onInterrupt(readMessage);
                            bytes = 0;
                        } else {
                            bytes++;
                        }

                        if (Globals.t_sendnosend ==1){
                            connectedThread.write(Globals.t_gowrite);
                            Globals.t_sendnosend =0;
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                        break;
                    }
                }
            }

            /* Call this from the main activity to send data to the remote device */



            public void write(String input) {
                byte[] bytes = input.getBytes(); //converts entered String into bytes
                try {
                    mmOutStream.write(bytes);
                    // Share the sent message with the UI activity.
//                Message writtenMsg = handler.obtainMessage(
//                        MessageConstants.MESSAGE_WRITE, -1, -1, mmBuffer);
//                writtenMsg.sendToTarget();
                    sendToast("out-"+input);
                } catch (IOException e) {
                    Log.e("Send Error", "Unable to send message", e);
                    Globals.system="unable to send";
                    // Send a failure message back to the activity.
//                Message writeErrorMsg =
//                        handler.obtainMessage(MessageConstants.MESSAGE_TOAST);
//                Bundle bundle = new Bundle();
//                bundle.putString("toast",
//                        "Couldn't send data to the other device");
//                writeErrorMsg.setData(bundle);
//                handler.sendMessage(writeErrorMsg);
                    sendToast("couldn't send");
                }
            }

//        public void write(byte[] bytes) {
//            try {
//                mmOutStream.write(bytes);
//
//
//
//            } catch (IOException e) {
//                Log.e(TAG, "Error occurred when sending data", e);
//
//
//            }
//        }



            /* Call this from the main activity to shutdown the connection */
            public void cancel() {
                try {
                    mmSocket.close();
                } catch (IOException e) {
                }
            }
        }




        /* --- Music Player --- */
        static class Music {
            public static final String CMDTOGGLEPAUSE = "togglepause";
            public static final String CMDPAUSE = "pause";
            public static final String CMDPREVIOUS = "previous";
            public static final String CMDNEXT = "next";
            public static final String CMDSTOP = "stop";

            public static final String Service = "com.android.music.musicservicecommand";
            public static final String Command = "command";

            static boolean isMusicPlaying = false;

            static boolean musicToggle() {
                if (isMusicPlaying) musicPause();
                else musicPlay();

                isMusicPlaying = !isMusicPlaying;

                return isMusicPlaying;
            }

            static void musicPlay() {
                Intent i = new Intent(Intent.ACTION_MEDIA_BUTTON);
                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PLAY));
                MainService.context.sendOrderedBroadcast(i, null);

                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_MEDIA_PLAY));
                MainService.context.sendOrderedBroadcast(i, null);
            }

            static void musicPause() {
                Intent i = new Intent(Intent.ACTION_MEDIA_BUTTON);
                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_PAUSE));
                MainService.context.sendOrderedBroadcast(i, null);

                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_MEDIA_PAUSE));
                MainService.context.sendOrderedBroadcast(i, null);
            }

            static void musicNext() {
                Intent i = new Intent(Intent.ACTION_MEDIA_BUTTON);
                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_MEDIA_NEXT));
                MainService.context.sendOrderedBroadcast(i, null);

                i.putExtra(Intent.EXTRA_KEY_EVENT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_MEDIA_NEXT));
                MainService.context.sendOrderedBroadcast(i, null);
            }
        }




    }}
