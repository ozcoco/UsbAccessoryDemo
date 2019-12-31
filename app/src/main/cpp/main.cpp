//
// Created by Administrator on 2019/12/31.
//

#include "main.h"

/**
 *
 * filename: usbacc.c
 * brief: Use libusb to emulate usb android open accesory
 *
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <jni.h>
#include <utils/log.h>

extern "C" {
#include <libusb.h>
}


#define EP_IN 0x81
#define EP_OUT 0x02

#define LG_VID 0x1004
#define PID 0x633e

#define GOOGLE_VID 0x18d1
#define ACCESSORY_PID 0x2d01
#define ACCESSORY_PID_ALT 0x2d00

#define AOA_BUFF_MAX 16384
#define LEN 2

int init(void);

int deInit(void);

int setupAccessory(void);

int usbSendCtrl(char *buff, int req, int index);

void error(int code);

void status(int code);

void *usbRWHdlr(void *threadarg);

struct libusb_device_handle *handle;
char stop;
char success = 0;

struct usbAccessory {
    char *manufacturer;
    char *modelName;
    char *description;
    char *version;
    char *uri;
    char *serialNumber;
};

struct usbAccessory gadgetAccessory = {
        const_cast<char *>("GitHub"),
        const_cast<char *>("RacquetScience"),
        const_cast<char *>("Description"),
        const_cast<char *>("1.0"),
        const_cast<char *>("https://github.com"),
        const_cast<char *>("01linuxSerialNo.")
};

int main(int argc, char **argv) {
    pthread_t tid;
    if (init() < 0)
        return -1;

    if (setupAccessory() < 0) {
        fprintf(stdout, "Error setting up accessory\n");
        deInit();
        return -1;
    }
    pthread_create(&tid, NULL, usbRWHdlr, NULL);
    pthread_join(tid, NULL);

    deInit();
    fprintf(stdout, "Done, no errors\n");
    return 0;
}

JavaVM *gVm;

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    LOGI("%s", "---------------->  JNI_OnLoad");

    gVm = vm;

    return JNI_VERSION_1_6;

}

extern "C" JNIEXPORT void JNICALL
Java_me_oz_demo_usbsample_usb_AccessoryNative_check(JNIEnv *env, jclass type) {

    pthread_t tid;

    if (init() < 0)
        return;

    if (setupAccessory() < 0) {
        fprintf(stdout, "Error setting up accessory\n");
        LOGE("%s \n", "Error setting up accessory");
        deInit();
        return;
    }
    pthread_create(&tid, NULL, usbRWHdlr, NULL);
    pthread_join(tid, NULL);

    deInit();
    fprintf(stdout, "Done, no errors\n");
    LOGI("%s \n", "Done, no errors");

}


void *usbRWHdlr(void *threadarg) {

    JNIEnv *env;

    const int &check = gVm->GetEnv((void **) &env, JNI_VERSION_1_6);

    //检查Native与Java分离
    if (check != JNI_EDETACHED) {

        gVm->AttachCurrentThread(&env, nullptr);
    }

    int response, transferred;
    unsigned int index = 0;
    unsigned char inBuff[AOA_BUFF_MAX] = {0};
    unsigned char outBuff[AOA_BUFF_MAX] = {0};

    for (;;) {
        response =
                libusb_bulk_transfer(handle, EP_IN, inBuff, sizeof(inBuff), &transferred, 0);
        if (response < 0) {
            error(response);

            JNIEnv *pEnv;

            const int &check = gVm->GetEnv((void **) &pEnv, JNI_VERSION_1_6);

            //检查Native与Java分离
            if (check != JNI_EDETACHED) {

                gVm->DetachCurrentThread();
            }

            return nullptr;
        }

        fprintf(stdout, "msg: %s\n", inBuff);
        LOGI("msg: %s\n", inBuff);

        sprintf(reinterpret_cast<char *>(outBuff), "ACK: %07d", index++);
        LOGI("ACK: %07d", index++);

        response =
                libusb_bulk_transfer(handle, EP_OUT, outBuff, strlen(
                        reinterpret_cast<const char *>(outBuff)), &transferred, 0);
        if (response < 0) {
            error(response);

            JNIEnv *pEnv;

            const int &check = gVm->GetEnv((void **) &pEnv, JNI_VERSION_1_6);

            //检查Native与Java分离
            if (check != JNI_EDETACHED) {

                gVm->DetachCurrentThread();
            }

            return nullptr;
        }

        sleep(1);
    }
}

int init() {
    libusb_init(NULL);
    if ((handle = libusb_open_device_with_vid_pid(NULL, LG_VID, PID)) == NULL) {
        fprintf(stdout, "Problem acquireing handle\n");
        LOGI("Problem acquireing handle\n");
        return -1;
    }
    libusb_claim_interface(handle, 0);
    return 0;
}

int deInit() {
    if (handle != NULL) {
        libusb_release_interface(handle, 0);
        libusb_close(handle);
    }
    libusb_exit(NULL);
    return 0;
}

int usbSendCtrl(char *buff, int req, int index) {
    int response = 0;

    if (NULL != buff) {
        response =
                libusb_control_transfer(handle, 0x40, req, 0, index,
                                        reinterpret_cast<unsigned char *>(buff), strlen(buff) + 1,
                                        0);
    } else {
        response =
                libusb_control_transfer(handle, 0x40, req, 0, index,
                                        reinterpret_cast<unsigned char *>(buff), 0, 0);
    }

    if (response < 0) {
        error(response);
        return -1;
    }

    return 0;
}

int setupAccessory() {
    unsigned char ioBuffer[2];
    int devVersion;
    int response;
    int tries = 5;

    response = libusb_control_transfer(
            handle, //handle
            0xC0, //bmRequestType
            51, //bRequest
            0, //wValue
            0, //wIndex
            ioBuffer, //data
            2, //wLength
            0 //timeout
    );

    if (response < 0) {
        error(response);
        return -1;
    }

    devVersion = ioBuffer[1] << 8 | ioBuffer[0];
    fprintf(stdout, "Verion Code Device: %d \n", devVersion);
    LOGI("Verion Code Device: %d \n", devVersion);

    usleep(1000);//sometimes hangs on the next transfer :(

    if (usbSendCtrl(gadgetAccessory.manufacturer, 52, 0) < 0) {
        return -1;
    }
    if (usbSendCtrl(gadgetAccessory.modelName, 52, 1) < 0) {
        return -1;
    }
    if (usbSendCtrl(gadgetAccessory.description, 52, 2) < 0) {
        return -1;
    }
    if (usbSendCtrl(gadgetAccessory.version, 52, 3) < 0) {
        return -1;
    }
    if (usbSendCtrl(gadgetAccessory.uri, 52, 4) < 0) {
        return -1;
    }
    if (usbSendCtrl(gadgetAccessory.serialNumber, 52, 5) < 0) {
        return -1;
    }

    fprintf(stdout, "Accessory Identification sent\n");
    LOGI("Accessory Identification sent\n");

    if (usbSendCtrl(NULL, 53, 0) < 0) {
        return -1;
    }

    fprintf(stdout, "Attempted to put device into accessory mode\n");
    LOGI("Attempted to put device into accessory mode\n");

    if (handle != NULL) {
        libusb_release_interface(handle, 0);
    }

    for (;;) {
        tries--;
        if ((handle = libusb_open_device_with_vid_pid(NULL,
                                                      GOOGLE_VID, ACCESSORY_PID)) == NULL) {
            if (tries < 0) {
                return -1;
            }
        } else {
            break;
        }
        sleep(1);
    }

    libusb_claim_interface(handle, 0);
    fprintf(stdout, "Interface claimed, ready to transfer data\n");
    LOGI("Interface claimed, ready to transfer data\n");

    return 0;
}

void error(int code) {
    fprintf(stdout, "\n");
    switch (code) {
        case LIBUSB_ERROR_IO:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_IO\n"
                    "Input/output error.\n");

            LOGI("Error: LIBUSB_ERROR_IO\n %s",
                 "Input/output error.\n");
            break;
        case LIBUSB_ERROR_INVALID_PARAM:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_INVALID_PARAM\n"
                    "Invalid parameter.\n");

            LOGI("Error: LIBUSB_ERROR_INVALID_PARAM\n %s",
                 "Invalid parameter.\n");

            break;
        case LIBUSB_ERROR_ACCESS:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_ACCESS\n"
                    "Access denied (insufficient permissions).\n");

            LOGI("Error: LIBUSB_ERROR_ACCESS\n %s",
                 "Access denied (insufficient permissions).\n");
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_NO_DEVICE\n"
                    "No such device (it may have been disconnected).\n");

            LOGI("Error: LIBUSB_ERROR_NO_DEVICE\n %s",
                 "No such device (it may have been disconnected).\n");
            break;
        case LIBUSB_ERROR_NOT_FOUND:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_NOT_FOUND\n"
                    "Entity not found.\n");

            LOGI("Error: LIBUSB_ERROR_NOT_FOUND\n %s",
                 "Entity not found.\n");
            break;
        case LIBUSB_ERROR_BUSY:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_BUSY\n"
                    "Resource busy.\n");
            LOGI("Error: LIBUSB_ERROR_BUSY\n %s",
                 "Resource busy.\n");
            break;
        case LIBUSB_ERROR_TIMEOUT:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_TIMEOUT\n"
                    "Operation timed out.\n");
            LOGI("Error: LIBUSB_ERROR_TIMEOUT\n %s",
                 "Operation timed out.\n");
            break;
        case LIBUSB_ERROR_OVERFLOW:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_OVERFLOW\n"
                    "Overflow.\n");
            LOGI("Error: LIBUSB_ERROR_TIMEOUT\n %s",
                 "Overflow.\n");
            break;
        case LIBUSB_ERROR_PIPE:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_PIPE\n"
                    "Pipe error.\n");
            LOGI("Error: LIBUSB_ERROR_PIPE\n %s",
                 "Pipe error.\n");
            break;
        case LIBUSB_ERROR_INTERRUPTED:
            fprintf(stdout,
                    "Error:LIBUSB_ERROR_INTERRUPTED\n"
                    "System call interrupted (perhaps due to signal).\n");
            LOGI("Error: LIBUSB_ERROR_INTERRUPTED\n %s",
                 "System call interrupted (perhaps due to signal).\n");
            break;
        case LIBUSB_ERROR_NO_MEM:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_NO_MEM\n"
                    "Insufficient memory.\n");
            LOGI("Error: LIBUSB_ERROR_NO_MEM\n %s",
                 "Insufficient memory.\n");
            break;
        case LIBUSB_ERROR_NOT_SUPPORTED:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_NOT_SUPPORTED\n"
                    "Operation not supported or unimplemented on this platform.\n");

            LOGI("Error: LIBUSB_ERROR_NOT_SUPPORTED\n %s",
                 "Operation not supported or unimplemented on this platform.\n");
            break;
        case LIBUSB_ERROR_OTHER:
            fprintf(stdout,
                    "Error: LIBUSB_ERROR_OTHER\n"
                    "Other error.\n");
            LOGI("Error: LIBUSB_ERROR_OTHER\n %s",
                 "Other error.\n");
            break;
        default:
            fprintf(stdout,
                    "Error: unkown error\n");
            LOGI("Error: unkown error\n");
    }
}

void status(int code) {
    fprintf(stdout, "\n");
    switch (code) {
        case LIBUSB_TRANSFER_COMPLETED:
            fprintf(stdout,
                    "Success: LIBUSB_TRANSFER_COMPLETED\n"
                    "Transfer completed.\n");
            break;
        case LIBUSB_TRANSFER_ERROR:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_ERROR\n"
                    "Transfer failed.\n");
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_TIMED_OUT\n"
                    "Transfer timed out.\n");
            break;
        case LIBUSB_TRANSFER_CANCELLED:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_CANCELLED\n"
                    "Transfer was cancelled.\n");
            break;
        case LIBUSB_TRANSFER_STALL:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_STALL\n"
                    "For bulk/interrupt endpoints: halt condition detected.\n"
                    "For control endpoints: control request not supported.\n");
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_NO_DEVICE\n"
                    "Device was disconnected.\n");
            break;
        case LIBUSB_TRANSFER_OVERFLOW:
            fprintf(stdout,
                    "Error: LIBUSB_TRANSFER_OVERFLOW\n"
                    "Device sent more data than requested.\n");
            break;
        default:
            fprintf(stdout,
                    "Error: unknown error\nTry again(?)\n");
            break;
    }
}