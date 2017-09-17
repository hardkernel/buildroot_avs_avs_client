/*
* This file is part of avs.
* Copyright (c) amlogic 2017
* All rights reserved.
* author:renjun.xu@amlogic.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software.
*
*/
#ifndef EVENTS_PROCESS_H
#define EVENTS_PROCESS_H

#include <linux/input.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

class EventsProcess {
  public:
    struct KeyMapItem_t {
        const char* type;
        int value;
    };
    EventsProcess();
    virtual ~EventsProcess() { }
    virtual void Init();
    virtual const char* WaitKey();
    virtual void KeyLongPress(int key);
protected:
    void EnqueueKey(int key_code);
private:
    pthread_mutex_t key_queue_mutex;
    pthread_cond_t key_queue_cond;
    int key_queue[256], key_queue_len;
    struct timeval last_queue_time;
    char key_pressed[KEY_MAX + 1];
    int key_last_down;
    int key_down_count;
    bool enable_reboot;
    int rel_sum;

    int last_key;
    bool report_longpress_flag;
    struct key_timer_t {
        EventsProcess* ep;
        int key_code;
        int count;
    };

    int num_keys;
    KeyMapItem_t* keys_map;

    #define DEFAULT_KEY_NUM 1

    pthread_t input_thread_;

    static int InputCallback(int fd, uint32_t epevents, void* data);
    int OnInputEvent(int fd, uint32_t epevents);
    void ProcessKey(int key_code, int updown);

    static void* time_key_helper(void* cookie);
    void time_key(int key_code, int count);
    const char* getKeyType(int key);
    void load_key_map();
    int getMapKey(int key);
};

#endif  // RECOVERY_UI_H
