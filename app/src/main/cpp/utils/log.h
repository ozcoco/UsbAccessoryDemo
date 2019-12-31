//
// Created by Administrator on 2019/12/4.
//

#ifndef SHAREDEMO_LOG_H
#define SHAREDEMO_LOG_H

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "^_^", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "^_*", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "*_*", __VA_ARGS__))

#endif //SHAREDEMO_LOG_H
