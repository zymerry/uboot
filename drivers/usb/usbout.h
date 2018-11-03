
#ifndef __USBOUT_H__
#define __USBOUT_H__

void Ep3Handler(void);
void Ep3HandlerOptimized(void);

void IsrDma2(void);
void ClearEp3OutPktReady(void);

#endif /*__USBOUT_H__*/
