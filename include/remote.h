#ifndef REMOTE_H
#define REMOTE_H

void initIRSensor(); 
void decodeIR();
void handleMenuNavigation(unsigned long irCode);
void handleMenuAction(int selectedItem);

#endif // REMOTE_H
