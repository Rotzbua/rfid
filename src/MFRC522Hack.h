#ifndef RFID_MFRC522HACKS_H
#define RFID_MFRC522HACKS_H

#include <Arduino.h>
#include "MFRC522.h"
#include "MFRC522Debug.h"

class MFRC522Hack {
private:
	MFRC522 *const _device;
public:
	MFRC522Hack(MFRC522 *device) : _device(device) {};
	
	bool MIFARE_OpenUidBackdoor(bool logErrors);
	
	bool MIFARE_SetUid(byte *newUid, byte uidSize, bool logErrors);
	
	bool MIFARE_UnbrickUidSector(bool logErrors);
};


#endif //RFID_MFRC522HACKS_H
