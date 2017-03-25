#include <stdint.h>
#include <Arduino.h>
#include <HardwareSerial.h>
#include "MFRC522.h"

#ifndef MFRC522Debug_h
#define MFRC522Debug_h

class MFRC522Debug {
private:
	
public:
	//constexpr MFRC522Debug(Print* print): _print(print) {};
	//static void setOutput(Print *print ) { _print = print; };
	static Print *getOutput(Print *print = nullptr) {
		static Print *__print;
		if (print != nullptr)
			__print = print;
		if (__print != nullptr)
			return __print;
		return &Serial;
	}
	
	// Get human readable code and type
	static const __FlashStringHelper *PICC_GetTypeName(MFRC522::PICC_Type type);
	
	static const __FlashStringHelper *GetStatusCodeName(MFRC522::StatusCode code);
	
	// Support functions for debuging
	static void PCD_DumpVersionToSerial(MFRC522 *const _device);
	
	static void PICC_DumpToSerial(MFRC522 *const _device, MFRC522::Uid *uid);
	
	static void PICC_DumpDetailsToSerial(MFRC522 *const _device, MFRC522::Uid *uid);
	
	static void PICC_DumpMifareClassicToSerial(MFRC522 *const _device, MFRC522::Uid *uid, MFRC522::PICC_Type piccType, MFRC522::MIFARE_Key *key);
	
	static void PICC_DumpMifareClassicSectorToSerial(MFRC522 *const _device, MFRC522::Uid *uid, MFRC522::MIFARE_Key *key, byte sector);
	
	static void PICC_DumpMifareUltralightToSerial(MFRC522 *const _device);
};
#endif // MFRC522Debug_h
