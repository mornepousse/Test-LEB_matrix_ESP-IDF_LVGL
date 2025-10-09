void SpiDacWriteValue(signed short value);
void SpiDacInit(void);
void InitAudioTimer(void);
int AudioWriteDualBuffer(signed short *buf, int length);
int AudioDualBufferReady(void);

#define SpiDacCsHigh() {LPC_GPIO0->DATA|=(1<<7);}
#define SpiDacCsLow() {LPC_GPIO0->DATA&=~(1<<7);}
