/* comedifunc.h
 *
 *
 */

int comedi_device_setup();
int comedi_device_close();
int comedi_digital_trig(char *eventfile);
int comedi_start_analog_acq();
int comedi_setup_digital_sequence(char *filename);
int comedi_execute_digital_sequence();
