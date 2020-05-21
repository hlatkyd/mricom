/*
 * ttlctrl.h
 *
 */

#include "common.h"
#include "socketcomm.h"

#define N_USER_BITS 3

int wait_user_bits(comedi_t *dev, int subdev, int chan[N_USER_BITS], int num);
