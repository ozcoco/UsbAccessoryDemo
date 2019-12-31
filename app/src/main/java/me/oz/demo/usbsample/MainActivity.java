package me.oz.demo.usbsample;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import me.oz.demo.usbsample.ui.main.MainFragment;

public class MainActivity extends AppCompatActivity {

    private final String TAG = MainActivity.class.getSimpleName();

    private AccessoryCommunicator mAccessoryCommunicator;

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        setContentView(R.layout.main_activity);

        requestPermissions(new String[]{
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS,
                        Manifest.permission.ACCESS_NETWORK_STATE,
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.BATTERY_STATS},
                123);

        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.container, MainFragment.newInstance())
                    .commitNow();
        }

        init();
    }

    private void init() {

        mAccessoryCommunicator = new AccessoryCommunicator(this) {

            @Override
            public void onReceive(byte[] payload, int length) {
                Log.i(TAG, "onReceive: " + new String(payload, 0, length));

                Toast.makeText(getApplicationContext(), "---> onReceive", Toast.LENGTH_SHORT).show();

            }

            @Override
            public void onError(String msg) {
                Log.i(TAG, "onError: " + msg);

                Toast.makeText(getApplicationContext(), "---> onError:" + msg, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onConnected() {
                Log.i(TAG, "onConnected: ");

                Toast.makeText(getApplicationContext(), "---> onConnected", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onDisconnected() {
                Log.i(TAG, "onDisconnected: ");

                Toast.makeText(getApplicationContext(), "---> onDisconnected", Toast.LENGTH_SHORT).show();
            }
        };

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mAccessoryCommunicator != null)
            mAccessoryCommunicator.closeAccessory();

        mAccessoryCommunicator = null;
    }
}
