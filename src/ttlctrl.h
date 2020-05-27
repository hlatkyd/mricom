/*
 * ttlctrl.h
 *
 */

#include "common.h"
#include "socketcomm.h"

#define N_USER_BITS 3

int wait_user_bits(comedi_t *dev, int subdev, int chan[N_USER_BITS], int num);
int wait_console_ttl(comedi_t *dev, int subdev, int chan);
int wait_console_handshake(comedi_t *dev, int subdev, int chan, int outchan);
int wait_console_end_signal(comedi_t *dev, int subdev, int inchan);
int send_console_end_signal(comedi_t *dev, int subdev, int outchan);
