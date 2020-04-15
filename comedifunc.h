/* comedifunc.h
 *
 *
 */

int comedi_device_setup();
int comedi_device_close();
int comedi_digital_trig(char *eventfile);
comedi_cmd* comedi_setup_analog_acq();
int comedi_start_analog_acq(comedi_cmd *cmd);
int comedi_setup_digital_sequence(char *filename);
int comedi_execute_digital_sequence();
