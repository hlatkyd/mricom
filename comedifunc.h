/* comedifunc.h
 *
 *
 */

int comedi_device_setup();
int comedi_device_close();
int comedi_digital_trig();
int comedi_analog_in();
int comedi_setup_digital_sequence(char *filename);
int comedi_execute_digital_sequence();
