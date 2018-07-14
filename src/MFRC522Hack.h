#ifndef MFRC522HACK_H
#define MFRC522HACK_H

#include <Arduino.h>
#include "MFRC522.h"
#include "MFRC522Debug.h"

class MFRC522Hack {
private:
	MFRC522 &_device;
public:
	MFRC522Hack(MFRC522 &device) : _device(device) {};
	
	const bool MIFARE_OpenUidBackdoor(const bool logErrors) const;
	
	const bool MIFARE_SetUid(const byte *const newUid, const byte uidSize, const bool logErrors) const;
	
	const bool MIFARE_UnbrickUidSector(const bool logErrors) const;
};


#endif //MFRC522HACK_H
