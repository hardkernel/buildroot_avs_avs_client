#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DISPCARD_TEMPLATE_METHOD "cmd_template"
#define DISPCARD_PLAYER_METHOD "cmd_player"
#define DISPCARD_STATE_METHOD "cmd_state"

#define DISPLAYCARD_NONE   0
#define DISPLAYCARD_SERVER 1
#define DISPLAYCARD_CLIENT 2

int disp_connection(int dis_type);
int disp_send(char** s_str, const char* method);

int disp_state(void);
int disp_recv(char** s_str);

#ifdef __cplusplus
}
#endif
#endif //__COMMUNICATION_H
