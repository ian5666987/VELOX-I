// code by ntu ael

#ifndef C8051_PCA0_H
#define C8051_PCA0_H

#define PCA0_8bitPWM_ON       0x42 // for 8 bit mode and turn on immediately
#define PCA0_8bitPWM_OFF      0x40 // for 8 bit mode without turning on
#define PCA0_CAPTURE_CCFFLAG  0x21

void PCA0_init(char p_PCA0MD, char (*pca_cpm_config)(void));
void PCA0_setCPM(char channel, char config);
void PCA0_setDutyCycle(char channel, unsigned char l_uc_duty_cycle);

#endif
