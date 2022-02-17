package com.lang.y_eesp32_smartglasses_r2;

import android.app.Notification;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcel;
import android.os.Parcelable;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.text.TextUtils;
import android.util.Log;
import android.widget.RemoteViews;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/*

https://stackoverflow.com/questions/9293617/retrieve-text-from-a-remoteviews-object
Powiadomiena z Google Maps i innych view

 */

public class NotificationService extends NotificationListenerService {

    private static final String TAG = "MyInfoNS";

    @Override public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

    @Override public void onCreate() {
        super.onCreate();
        Log.i(TAG, "NotificationService started!");
    }

    @Override public void onNotificationPosted(StatusBarNotification sbn) {
        String packageName="", title="", text="";
        Bundle extras;

        try {
            packageName = sbn.getPackageName();
            extras  = sbn.getNotification().extras;
            title   = (extras.getString("android.title") !=null) ? extras.getString("android.title").toString() : "";
            text    = (extras.getString("android.text") !=null) ? extras.getString("android.text").toString() : "";

            if ( (packageName != null && !packageName.isEmpty()) && (title != null && !title.isEmpty()) ) {
                Log.i(TAG, "Received notification: package=" + packageName + ", title=" + title + ", text= " + text);

                // Notification / Msg
                MainService.ESP32.msgNotiScreen("115", title, text);

            } else {
//                Notification notification = (Notification) event.getParcelableData();
//                RemoteViews views = notification.contentView;





                int id = sbn.getNotification().icon;

                Bitmap b1 = getIconD(packageName, id);
                Bitmap b2 = getIconD(packageName,                 extras.getInt(Notification.EXTRA_LARGE_ICON_BIG));
                Bitmap b3 = getIconD(packageName,                 extras.getInt(Notification.EXTRA_LARGE_ICON));
                Bitmap b4 = getIconD(packageName,                 extras.getInt(Notification.EXTRA_SMALL_ICON));



                RemoteViews views = sbn.getNotification().bigContentView;
                if (views == null) views = sbn.getNotification().contentView;



                List<String> aText = new ArrayList<String>();

                List<String> aMethods = new ArrayList<String>();

                try {

                    // Bierzemy pola z powiadomienia
                    Field field = views.getClass().getDeclaredField("mActions");
                    field.setAccessible(true);

                    // Dostep do View z powiadomien - Parcelable - Paczka
                    @SuppressWarnings("unchecked")
                    ArrayList<Parcelable> actions = (ArrayList<Parcelable>) field.get(views);

                    // Find the setText() reflection actions
                    for (Parcelable p : actions) {

                        Parcel parcel = Parcel.obtain();
                        p.writeToParcel(parcel, 0);
                        parcel.setDataPosition(0);

                        // The tag tells which type of action it is (2 is ReflectionAction, from the source)
                        int tag = parcel.readInt();
                        if (tag != 2) continue;

                        // View ID
                        parcel.readInt();

                        String methodName = parcel.readString();
                        if (methodName == null) continue;

                        aMethods.add(methodName);

                        // Save strings
                        if (methodName.equals("setText")) {

                            // Parameter type (10 = Character Sequence)
                            parcel.readInt();

                            // Store the actual string
                            String t = TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(parcel).toString().trim();
                            aText.add(t);

                            Log.i(TAG, "onNotificationPosted: " + t);

                        } else if (methodName.equals("setBackgroundColor")) {
                            parcel.readInt();

//                            Bitmap b = (Bitmap) parcel.readParcelable(getClass().getClassLoader());

//                            aText.add(t);
                        }

                        parcel.recycle();

                    }


                } catch (Exception e) {
                    e.printStackTrace();
                }

                Log.i(TAG, "");

                /*

                Sikorskiego
                57 min · 4.7 km · 11:42 PM ETA
                Exit navigation
                0 m

                 */




            }

        } catch (Exception e) { e.printStackTrace(); }
    }

    Bitmap getIconD(String packageName, int id) {
        Context remotePackageContext=null;
        Bitmap bmp = null;
        try {
            remotePackageContext = getApplicationContext().createPackageContext(packageName, 0);
            Drawable ic= remotePackageContext.getResources().getDrawable(id);
            if (ic != null) bmp = ((BitmapDrawable) ic).getBitmap();
        } catch (Exception e) {
        }
        return bmp;
    }
}
