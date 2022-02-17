package com.lang.y_eesp32_smartglasses_r2;

import androidx.appcompat.app.AppCompatActivity;
//import android.support.v7.app.AppCompatActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.NotificationCompat;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.content.ActivityNotFoundException;

import android.speech.RecognizerIntent;
import android.view.View;

import java.util.ArrayList;

import android.speech.tts.TextToSpeech;

import java.util.Locale;



public class MainActivity extends AppCompatActivity {


    //----------------------------------------------------------(((----_________________-----)))-------------------------------
//    private String deviceName = null;
//    private String deviceAddress;
//    public static Handler handler;
//    public static BluetoothSocket mmSocket;
//    public static ConnectedThread connectedThread;
//    //public static CreateConnectThread createConnectThread;
//
//    private final static int CONNECTING_STATUS = 1; // used in bluetooth handler to identify message status
//    private final static int MESSAGE_READ = 2; // used in bluetooth handler to identify message update

    //----------------------------------------------------------(((----_________________-----)))-------------------------------

    public static MainService.ESP32.ConnectedThread connectedThread;

    static final String sESP32_MAC = "48:26:2C:52:D6:80";
    //   static final String sESP32_MAC = "24:6F:28:A1:87:7A";                                           // test
    static final String sCHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

    static SharedPreferences sharedPreferences;
    final static String SP_TITLE = "SP_TITLE";
    final static String SP_LIST = "SP_LIST";
    public static MainService.ESP32.CreateConnectThread createConnectThread;
    static boolean isConnected = false;

    final static String TAG = "MyInfo";

    private BluetoothAdapter mBluetoothAdapter;

    final int REQUEST_ENABLE_BT = 1;

    Intent iNotificationService = null;
    Intent iMainService = null;



    static boolean shouldRunning = false;
    //;;;;;;
    private static final int REQUEST_CODE = 100;
    public TextView voiceOutput;
    public TextView batteryLevel;     //Naya
    public TextView arduinoMsg; //yeh bhi naya
    //
    private String deviceName = null;
    private String deviceAddress;

    Button bStart;
    Button bSstop;
    Button buttonConnect;
    //Button bupdatebattery;
    Button b_updateInput;
    EditText etCmd;
    Button bSendCmd;
    //Button bthelisten;
    Button bWebUpdate;
    Button bRestart;
    //TextView voiceOutput;
    //voiceOutput= (TextView) findViewById(R.id.voiceOutput);
    Button bTestScreen1, bTestScreen2, bTestScreen3, bTestScreen4, bTestScreen5, bTestScreen6;

  //  Button bOpenList;
    TextToSpeech textToSpeech;

    Button bspeakText;

    NotificationCompat.Builder builder = new NotificationCompat.Builder(this)
            .setSmallIcon(R.drawable.notification_icon)
            .setContentTitle("code code")
            .setContentText("context done (DLA)")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT);
    //   --------------PUTHUSU ------------------- //
    /*
    Button btnStart, btnStop, btnBind, btnUnbind, btnUpby1, btnUpby10;
    TextView textStatus, textIntValue, textStrValue;
    Messenger mService = null;
    boolean mIsBound;
    final Messenger mMessenger = new Messenger(new IncomingHandler());


     class IncomingHandler extends Handler {
         @Override

         public void handleMessage(Message renmsg) {
             switch (renmsg.what) {
                 case MainService.MSG_SET_THE_STRING_VALUE:
                     textIntValue.setText("Int Message: " + renmsg.arg1);
                     break;
                 case MainService.MSG_SET_STRING_VALUE:
                     String str1 = renmsg.getData().getString("str1");
                     textStrValue.setText("Str Message: " + str1);
                     break;
                 default:
                     super.handleMessage(renmsg);
             }
         }
     }
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mService = new Messenger(service);
            textStatus.setText("Attached.");
            try {
                Message msg = Message.obtain(null, MainService.MSG_REGISTER_CLIENT);
                msg.replyTo = mMessenger;
                mService.send(msg);
            }
            catch (RemoteException e) {
                // In this case the service has crashed before we could even do anything with it
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            // This is called when the connection with the service has been unexpectedly disconnected - process crashed.
            mService = null;
            textStatus.setText("Disconnected.");
        }
    };
    public void onServiceDisconnected(ComponentName className) {
        // This is called when the connection with the service has been unexpectedly disconnected - process crashed.
        mService = null;
        textStatus.setText("Disconnected.");
    }*/
     // -----------------munjuch ------------- //


//    public static class ConnectedThread extends Thread {
//        private final BluetoothSocket mmSocket;
//        private final InputStream mmInStream;
//        private final OutputStream mmOutStream;
//
//        public ConnectedThread(BluetoothSocket socket) {
//            mmSocket = socket;
//            InputStream tmpIn = null;
//            OutputStream tmpOut = null;
//
//            // Get the input and output streams, using temp objects because
//            // member streams are final
//            try {
//                tmpIn = socket.getInputStream();
//                tmpOut = socket.getOutputStream();
//            } catch (IOException e) { }
//
//            mmInStream = tmpIn;
//            mmOutStream = tmpOut;
//        }
//
//        public void run() {
//            byte[] buffer = new byte[1024];  // buffer store for the stream
//            int bytes = 0; // bytes returned from read()
//            // Keep listening to the InputStream until an exception occurs
//            while (true) {
//                try {
//                    /*
//                    Read from the InputStream from Arduino until termination character is reached.
//                    Then send the whole String message to GUI Handler.
//                     */
//                    buffer[bytes] = (byte) mmInStream.read();
//                    String readMessage;
//                    if (buffer[bytes] == '\n'){
//                        readMessage = new String(buffer,0,bytes);
//                        Log.e("Arduino Message",readMessage);
//                        handler.obtainMessage(MESSAGE_READ,readMessage).sendToTarget();
//                        bytes = 0;
//                    } else {
//                        bytes++;
//                    }
//                } catch (IOException e) {
//                    e.printStackTrace();
//                    break;
//                }
//            }
//        }
//
//        /* Call this from the main activity to send data to the remote device */
//        public void write(String input) {
//            byte[] bytes = input.getBytes(); //converts entered String into bytes
//            try {
//                mmOutStream.write(bytes);
//            } catch (IOException e) {
//                Log.e("Send Error","Unable to send message",e);
//            }
//        }
//
//        /* Call this from the main activity to shutdown the connection */
//        public void cancel() {
//            try {
//                mmSocket.close();
//            } catch (IOException e) { }
//        }
//    }


    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        arduinoMsg = findViewById(R.id.arduinoMsg);
        voiceOutput = findViewById(R.id.voiceOutput);
        batteryLevel = findViewById(R.id.tv_Status);

        bspeakText = findViewById(R.id.btnText);

        //instantiating a object of the ClassB and passing tv
        // MainService obj = new MainService(voiceOutput);

        final Toolbar toolbar = findViewById(R.id.toolbar);
        final ProgressBar progressBar = findViewById(R.id.progressBar);
        progressBar.setVisibility(View.GONE);

        // If a bluetooth device has been selected from SelectDeviceActivity
        deviceName = getIntent().getStringExtra("deviceName");
        if (deviceName != null) {
            // Get the device address to make BT Connection
            deviceAddress = getIntent().getStringExtra("deviceAddress");
            // Show progree and connection status
            toolbar.setSubtitle("Connecting to " + deviceName + "...");
            progressBar.setVisibility(View.VISIBLE);
            //buttonConnect.setEnabled(false);

            /*
            This is the most important piece of code. When "deviceName" is found
            the code will call a new thread to create a bluetooth connection to the
            selected device (see the thread code below)
             */
           BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            createConnectThread = new MainService.ESP32.CreateConnectThread(bluetoothAdapter, deviceAddress);
            //createConnectThread.start();
        }

        textToSpeech = new TextToSpeech(getApplicationContext(), new TextToSpeech.OnInitListener() {
            @Override
            public void onInit(int i) {

                // if No error is found then only it will run
                if (i != TextToSpeech.ERROR) {
                    textToSpeech.setLanguage(Locale.UK);

                    Toast.makeText(getApplicationContext(), "உரை முதல் பேச்சு செயல்பாட்டில் பிழைகள் இல்லை", Toast.LENGTH_SHORT).show();


                }
            }
        });


            sharedPreferences = getPreferences(MODE_PRIVATE);

            bStart = findViewById(R.id.b_Start);
            bSstop = findViewById(R.id.b_Sstop);

            buttonConnect = findViewById(R.id.buttonConnect);
            b_updateInput = findViewById(R.id.b_updateInput);
            batteryLevel = findViewById(R.id.tv_Status);
            //bthelisten = findViewById(R.id.b_thelisten); //T\d\f\0\0\0\0\0\0\0\0\0\o-_O_DDDOooo-O_O_O_O_O_O_ssssssssss
            etCmd = findViewById(R.id.et_Cmd);
            //bupdatebattery = findViewById(R.id.updateBattery);
            bSendCmd = findViewById(R.id.b_SendCmd);

            bWebUpdate = findViewById(R.id.b_WebUpdate);
            bRestart = findViewById(R.id.b_ESPRestart);

            bTestScreen1 = findViewById(R.id.b_TestScreen1);
            bTestScreen2 = findViewById(R.id.b_TestScreen2);
            bTestScreen3 = findViewById(R.id.b_TestScreen3);
            bTestScreen4 = findViewById(R.id.b_TestScreen4);
            bTestScreen5 = findViewById(R.id.b_TestScreen5);
            bTestScreen6 = findViewById(R.id.b_TestScreen6);

        //    bOpenList = findViewById(R.id.b_OpenList);


            // If bluetooth supported (Rage BLE not supported)
//            if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
//                Toast.makeText(this, R.string.ble_not_supported, Toast.LENGTH_SHORT).show();
//                finish();
//            }

            final BluetoothManager mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            mBluetoothAdapter = mBluetoothManager.getAdapter();

            // Request enabling? Bluetooth if turned off
            if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }

            // ------------- Notification Listener Permission - Msg/Notification screen - MSG from notifications --------------------
        /*if (ContextCompat.checkSelfPermission(this, Manifest.permission.BIND_NOTIFICATION_LISTENER_SERVICE) != PackageManager.PERMISSION_GRANTED) {
            Intent intent = new Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS");
            startActivity(intent);
        }*/

            // --- MSG/Notifications - MSG from notifications - Service Start
            iNotificationService = new Intent(MainActivity.this, NotificationService.class);
            startService(iNotificationService);

            // --- MainService - Start
            iMainService = new Intent(MainActivity.this, MainService.class);
            startService(iMainService);


            // Start
            bStart.setOnClickListener(view -> MainService.ESP32.oldconnect(sESP32_MAC));
            bSstop.setOnClickListener(view -> MainService.ESP32.oldconnect(sESP32_MAC));
        buttonConnect.setOnClickListener(view -> {
            // Move to adapter list
            Intent intent = new Intent(MainActivity.this, SelectDeviceActivity.class);
            startActivity(intent);
        });
            bSendCmd.setOnClickListener(view -> {
                if (etCmd.getText() != null && !etCmd.getText().toString().isEmpty()) {
                   // MainService.ConnectedThread.write(etCmd.getText().toString());

                    tempowrite(etCmd.getText().toString());

                }
            });
            bWebUpdate.setOnClickListener(view ->   tempowrite("OM=1"));
            bRestart.setOnClickListener(view -> tempowrite("#RESTART"));


            // --- Test Buttons ---
            bTestScreen1.setOnClickListener(view -> {
                MainService.ESP32.mainScreen("14", "35", "13 sty", "125", "23C");
            });

            bTestScreen2.setOnClickListener(view -> {
                MainService.ESP32.msgNotiScreen("156", "Vanakkam!", "This is the example text screen. Lorem Impsum Dolor");
            });

            bTestScreen3.setOnClickListener(view -> {
                MainService.ESP32.callScreen("Tata");
            });

            bTestScreen4.setOnClickListener(view -> {
                MainService.ESP32.navScreen("120 km/h", "500 m", "54 km", "76");
            });

            bTestScreen5.setOnClickListener(view -> {
                MainService.ESP32.listScreen("257", "Shopping list", "221", "Masala Dosa Naan");
            });

            bTestScreen6.setOnClickListener(view -> {
                MainService.ESP32.musicScreen("182", "Kalyana Vayasu - Anirudh Rav.", "210", "214");

            });

       //     bOpenList.setOnClickListener(view -> startActivity(new Intent(MainActivity.this, ListActivity.class)));
           // bupdatebattery.setOnClickListener(view -> batteryLevel.setText(Globals.espBattery));

        b_updateInput.setOnClickListener(view -> {
            // Move to adapter list
            arduinoMsg.setText(Globals.r_incoming);
            Toast.makeText(getApplicationContext(), Globals.system, Toast.LENGTH_SHORT).show();
            if (Globals.bleOn == 1) {
                toolbar.setSubtitle("Device connected");
            } else {
                toolbar.setSubtitle("Device fails to connect");
            }
        });

        bspeakText.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    textToSpeech.speak(Globals.r_typewordResult, TextToSpeech.QUEUE_FLUSH, null);
                    Toast.makeText(getApplicationContext(), Globals.r_typewordResult, Toast.LENGTH_SHORT).show();

                }
            });
        }
    public void onClick (View v)
    {
        Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);

        try {
            startActivityForResult(intent, REQUEST_CODE);
        } catch (ActivityNotFoundException a) {


        }
    }
    public static void wait ( int ms)
    {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
    }
    @Override

    //Define an OnActivityResult method in our intent caller Activity//


    protected void onActivityResult ( int requestCode, int resultCode, Intent data){
        super.onActivityResult(requestCode, resultCode, data);
        int ms;

        switch (requestCode) {
            case REQUEST_CODE: {

//If RESULT_OK is returned...//

                if (resultCode == RESULT_OK && null != data) {

//...then retrieve the ArrayList//

                    ArrayList<String> result = data.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);

//Update our TextView//


                    voiceOutput.setText(result.get(0));
                    Globals.t_solresult = result.get(0);
                    wait(1000);
                    Toast.makeText(getApplicationContext(), Globals.t_solresult, Toast.LENGTH_SHORT).show();

                    //sendMessageToService(result.get(0));


                }

            }

        }




    }
    static void saveSharedPreferencesTitle(String s) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(SP_TITLE, s);
        editor.apply();
    }

    static String readSharedPreferencesTitle() {
        return sharedPreferences.getString(SP_TITLE, "");
    }

    static void saveSharedPreferencesList(String s) {
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(SP_LIST, s);
        editor.apply();
    }

    static String readSharedPreferencesList() {
        return sharedPreferences.getString(SP_LIST, "");
    }

    static void tempowrite (String input) {
        Globals.t_gowrite = input;
        Globals.t_sendnosend = 1;
    }

     void sendToast(String msg) {

        Toast.makeText(getApplicationContext(), "உரை முதல் பேச்சு செயல்பாட்டில் பிழைகள் இல்லை", Toast.LENGTH_SHORT).show();


    }
    public void showToast(final String toast)
    {
        runOnUiThread(() -> Toast.makeText(MainActivity.this, toast, Toast.LENGTH_SHORT).show());
    }
    @Override protected void onPause() {
        super.onPause();
    }

    @Override protected void onDestroy() {
        super.onDestroy();
        MainService.ESP32.disconnect();
        if (iNotificationService    != null) stopService(iNotificationService);
        if (iMainService            != null) stopService(iMainService);
    }


    @Override protected void onStop() {
        super.onStop();
    }
}
