package me.oz.demo.usbsample.ui.main;

import android.app.Application;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;

import me.oz.demo.usbsample.usb.AccessoryNative;

public class MainViewModel extends AndroidViewModel {

    public MainViewModel(@NonNull Application application) {
        super(application);
    }


    @Override
    protected void onCleared() {
        super.onCleared();
    }


    public void onSingle(View v) {

        AccessoryNative.check();

    }

}
