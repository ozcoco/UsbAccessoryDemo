package me.oz.demo.usbsample.usb;

/**
 * @ProjectName: UsbSample
 * @Package: me.oz.demo.usbsample.usb
 * @ClassName: AccessoryNative
 * @Description:
 * @Author: oz
 * @CreateDate: 2019/12/31 15:10
 * @UpdateUser:
 * @UpdateDate: 2019/12/31 15:10
 * @UpdateRemark:
 * @Version: 1.0
 */
public class AccessoryNative {

    static {
        System.loadLibrary("usb_accessory");
    }

    public native static void check();

}
