package com.lang.y_eesp32_smartglasses_r2;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.widget.EditText;

import androidx.annotation.Nullable;

public class ListActivity extends Activity {

    EditText etTitle;
    EditText etList;

    @Override protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);

      //  etTitle = findViewById(R.id.l_et_Title);
        // etList = findViewById(R.id.l_et_Text);

        etTitle.setText(MainActivity.readSharedPreferencesTitle());
        etList.setText(MainActivity.readSharedPreferencesList());
    }


    @Override public void onBackPressed() {
        super.onBackPressed();
        MainActivity.saveSharedPreferencesTitle(etTitle.getText().toString());
        MainActivity.saveSharedPreferencesList(etList.getText().toString());
    }

    @Override protected void onDestroy() {
        super.onDestroy();
        MainActivity.saveSharedPreferencesTitle(etTitle.getText().toString());
        MainActivity.saveSharedPreferencesList(etList.getText().toString());
    }
}
