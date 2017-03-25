
#include "MFRC522Debug.h"

/**
 * Returns a __FlashStringHelper pointer to the PICC type name.
 * 
 * @return const __FlashStringHelper *
 */
const __FlashStringHelper *MFRC522Debug::PICC_GetTypeName(MFRC522::PICC_Type piccType	///< One of the PICC_Type enums.
) {
	switch (piccType) {
		case MFRC522::PICC_TYPE_ISO_14443_4:		return F("PICC compliant with ISO/IEC 14443-4");
		case MFRC522::PICC_TYPE_ISO_18092:		return F("PICC compliant with ISO/IEC 18092 (NFC)");
		case MFRC522::PICC_TYPE_MIFARE_MINI:		return F("MIFARE Mini, 320 bytes");
		case MFRC522::PICC_TYPE_MIFARE_1K:		return F("MIFARE 1KB");
		case MFRC522::PICC_TYPE_MIFARE_4K:		return F("MIFARE 4KB");
		case MFRC522::PICC_TYPE_MIFARE_UL:		return F("MIFARE Ultralight or Ultralight C");
		case MFRC522::PICC_TYPE_MIFARE_PLUS:		return F("MIFARE Plus");
		case MFRC522::PICC_TYPE_MIFARE_DESFIRE:	return F("MIFARE DESFire");
		case MFRC522::PICC_TYPE_TNP3XXX:			return F("MIFARE TNP3XXX");
		case MFRC522::PICC_TYPE_NOT_COMPLETE:	return F("SAK indicates UID is not complete.");
		case MFRC522::PICC_TYPE_UNKNOWN:
		default:						return F("Unknown type");
	}
} // End PICC_GetTypeName()

/**
 * Dumps debug info about the connected PCD to Serial.
 * Shows all known firmware versions
 */
void MFRC522Debug::PCD_DumpVersionToSerial(MFRC522 *_device) {
	// Get the MFRC522Debug firmware version
	byte v = _device->PCD_ReadRegister(MFRC522::VersionReg);
	getOutput()->print(F("Firmware Version: 0x"));
	getOutput()->print(v, HEX);
	// Lookup which version
	switch (v) {
		case 0x88:
			getOutput()->println(F(" = (clone)"));
			break;
		case 0x90:
			getOutput()->println(F(" = v0.0"));
			break;
		case 0x91:
			getOutput()->println(F(" = v1.0"));
			break;
		case 0x92:
			getOutput()->println(F(" = v2.0"));
			break;
		default:
			getOutput()->println(F(" = (unknown)"));
	}
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF))
		getOutput()->println(F("WARNING: Communication failure, is the MFRC522Debug properly connected?"));
} // End PCD_DumpVersionToSerial()

/**
 * Returns a __FlashStringHelper pointer to a status code name.
 * 
 * @return const __FlashStringHelper *
 */
const __FlashStringHelper *MFRC522Debug::GetStatusCodeName(MFRC522::StatusCode code	///< One of the StatusCode enums.
) {
	switch (code) {
		case MFRC522::STATUS_OK:				return F("Success.");
		case MFRC522::STATUS_ERROR:			return F("Error in communication.");
		case MFRC522::STATUS_COLLISION:		return F("Collission detected.");
		case MFRC522::STATUS_TIMEOUT:		return F("Timeout in communication.");
		case MFRC522::STATUS_NO_ROOM:		return F("A buffer is not big enough.");
		case MFRC522::STATUS_INTERNAL_ERROR:	return F("Internal error in the code. Should not happen.");
		case MFRC522::STATUS_INVALID:		return F("Invalid argument.");
		case MFRC522::STATUS_CRC_WRONG:		return F("The CRC_A does not match.");
		case MFRC522::STATUS_MIFARE_NACK:	return F("A MIFARE PICC responded with NAK.");
		default:					return F("Unknown error");
	}
} // End GetStatusCodeName()

/**
 * Dumps debug info about the selected PICC to getOutput()->
 * On success the PICC is halted after dumping the data.
 * For MIFARE Classic the factory default key of 0xFFFFFFFFFFFF is tried.  
 *
 * @DEPRECATED Kept for bakward compatibility
 */
void MFRC522Debug::PICC_DumpToSerial(
		MFRC522 *_device,
		MFRC522::Uid *uid    ///< Pointer to Uid struct returned from a successful PICC_Select().
) {
	MFRC522::MIFARE_Key key;
	
	// Dump UID, SAK and Type
	PICC_DumpDetailsToSerial(_device, uid);
	
	// Dump contents
	MFRC522::PICC_Type piccType = MFRC522::PICC_GetType(uid->sak);
	switch (piccType) {
		case MFRC522::PICC_TYPE_MIFARE_MINI:
		case MFRC522::PICC_TYPE_MIFARE_1K:
		case MFRC522::PICC_TYPE_MIFARE_4K:
			// All keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
			for (byte i = 0; i < 6; i++) {
				key.keyByte[i] = 0xFF;
			}
			PICC_DumpMifareClassicToSerial(_device, uid, piccType, &key);
			break;
		
		case MFRC522::PICC_TYPE_MIFARE_UL:
			PICC_DumpMifareUltralightToSerial(_device);
			break;
		
		case MFRC522::PICC_TYPE_ISO_14443_4:
		case MFRC522::PICC_TYPE_MIFARE_DESFIRE:
		case MFRC522::PICC_TYPE_ISO_18092:
		case MFRC522::PICC_TYPE_MIFARE_PLUS:
		case MFRC522::PICC_TYPE_TNP3XXX:
			getOutput()->println(F("Dumping memory contents not implemented for that PICC type."));
			break;
		
		case MFRC522::PICC_TYPE_UNKNOWN:
		case MFRC522::PICC_TYPE_NOT_COMPLETE:
		default:
			break; // No memory dump here
	}
	
	getOutput()->println();
	_device->PICC_HaltA(); // Already done if it was a MIFARE Classic PICC.
} // End PICC_DumpToSerial()

/**
 * Dumps card info (UID,SAK,Type) about the selected PICC to getOutput()->
 *
 * @DEPRECATED kept for backward compatibility
 */
void MFRC522Debug::PICC_DumpDetailsToSerial(
		MFRC522 *_device,
		MFRC522::Uid *uid    ///< Pointer to Uid struct returned from a successful PICC_Select().
) {
	// UID
	getOutput()->print(F("Card UID:"));
	for (byte i = 0; i < uid->size; i++) {
		if (uid->uidByte[i] < 0x10)
			getOutput()->print(F(" 0"));
		else
			getOutput()->print(F(" "));
		getOutput()->print(uid->uidByte[i], HEX);
	}
	getOutput()->println();
	
	// SAK
	getOutput()->print(F("Card SAK: "));
	if (uid->sak < 0x10)
		getOutput()->print(F("0"));
	getOutput()->println(uid->sak, HEX);
	
	// (suggested) PICC type
	MFRC522::PICC_Type piccType = _device->PICC_GetType(uid->sak);
	getOutput()->print(F("PICC type: "));
	getOutput()->println(PICC_GetTypeName(piccType));
} // End PICC_DumpDetailsToSerial()

/**
 * Dumps memory contents of a MIFARE Classic PICC.
 * On success the PICC is halted after dumping the data.
 */
void MFRC522Debug::PICC_DumpMifareClassicToSerial(
		MFRC522 *_device,
		MFRC522::Uid *uid,            ///< Pointer to Uid struct returned from a successful PICC_Select().
		MFRC522::PICC_Type piccType,    ///< One of the PICC_Type enums.
		MFRC522::MIFARE_Key *key        ///< Key A used for all sectors.
) {
	byte no_of_sectors = 0;
	switch (piccType) {
		case MFRC522::PICC_TYPE_MIFARE_MINI:
			// Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
			no_of_sectors = 5;
			break;
		
		case MFRC522::PICC_TYPE_MIFARE_1K:
			// Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes.
			no_of_sectors = 16;
			break;
		
		case MFRC522::PICC_TYPE_MIFARE_4K:
			// Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
			no_of_sectors = 40;
			break;
		
		default: // Should not happen. Ignore.
			break;
	}
	
	// Dump sectors, highest address first.
	if (no_of_sectors) {
		getOutput()->println(F("Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15  AccessBits"));
		for (int8_t i = no_of_sectors - 1; i >= 0; i--) {
			PICC_DumpMifareClassicSectorToSerial(_device, uid, key, i);
		}
	}
	_device->PICC_HaltA(); // Halt the PICC before stopping the encrypted session.
	_device->PCD_StopCrypto1();
} // End PICC_DumpMifareClassicToSerial()

/**
 * Dumps memory contents of a sector of a MIFARE Classic PICC.
 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
 */
void MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(
		MFRC522 *_device,
		MFRC522::Uid *uid,            ///< Pointer to Uid struct returned from a successful PICC_Select().
		MFRC522::MIFARE_Key *key,    ///< Key A for the sector.
		byte sector            ///< The sector to dump, 0..39.
) {
	MFRC522::StatusCode status;
	byte firstBlock;        // Address of lowest address to dump actually last block dumped)
	byte no_of_blocks;        // Number of blocks in sector
	bool isSectorTrailer;    // Set to true while handling the "last" (ie highest address) in the sector.
	
	// The access bits are stored in a peculiar fashion.
	// There are four groups:
	//		g[3]	Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
	//		g[2]	Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
	//		g[1]	Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
	//		g[0]	Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
	// Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB.
	// The four CX bits are stored together in a nible cx and an inverted nible cx_.
	byte c1, c2, c3;        // Nibbles
	byte c1_, c2_, c3_;        // Inverted nibbles
	bool invertedError;        // True if one of the inverted nibbles did not match
	byte g[4];                // Access bits for each of the four groups.
	byte group;                // 0-3 - active group for access bits
	bool firstInGroup;        // True for the first block dumped in the group
	
	// Determine position and size of sector.
	if (sector < 32) { // Sectors 0..31 has 4 blocks each
		no_of_blocks = 4;
		firstBlock = sector * no_of_blocks;
	} else if (sector < 40) { // Sectors 32-39 has 16 blocks each
		no_of_blocks = 16;
		firstBlock = 128 + (sector - 32) * no_of_blocks;
	} else { // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
		return;
	}
	
	// Dump blocks, highest address first.
	byte byteCount;
	byte buffer[18];
	byte blockAddr;
	isSectorTrailer = true;
	invertedError = false;    // Avoid "unused variable" warning.
	for (int8_t blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--) {
		blockAddr = firstBlock + blockOffset;
		// Sector number - only on first line
		if (isSectorTrailer) {
			if (sector < 10)
				getOutput()->print(F("   ")); // Pad with spaces
			else
				getOutput()->print(F("  ")); // Pad with spaces
			getOutput()->print(sector);
			getOutput()->print(F("   "));
		} else {
			getOutput()->print(F("       "));
		}
		// Block number
		if (blockAddr < 10)
			getOutput()->print(F("   ")); // Pad with spaces
		else {
			if (blockAddr < 100)
				getOutput()->print(F("  ")); // Pad with spaces
			else
				getOutput()->print(F(" ")); // Pad with spaces
		}
		getOutput()->print(blockAddr);
		getOutput()->print(F("  "));
		// Establish encrypted communications before reading the first block
		if (isSectorTrailer) {
			status = _device->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
			if (status != MFRC522::STATUS_OK) {
				getOutput()->print(F("PCD_Authenticate() failed: "));
				getOutput()->println(GetStatusCodeName(status));
				return;
			}
		}
		// Read block
		byteCount = sizeof(buffer);
		status = _device->MIFARE_Read(blockAddr, buffer, &byteCount);
		if (status != MFRC522::STATUS_OK) {
			getOutput()->print(F("MIFARE_Read() failed: "));
			getOutput()->println(GetStatusCodeName(status));
			continue;
		}
		// Dump data
		for (byte index = 0; index < 16; index++) {
			if (buffer[index] < 0x10)
				getOutput()->print(F(" 0"));
			else
				getOutput()->print(F(" "));
			getOutput()->print(buffer[index], HEX);
			if ((index % 4) == 3) {
				getOutput()->print(F(" "));
			}
		}
		// Parse sector trailer data
		if (isSectorTrailer) {
			c1 = buffer[7] >> 4;
			c2 = buffer[8] & 0xF;
			c3 = buffer[8] >> 4;
			c1_ = buffer[6] & 0xF;
			c2_ = buffer[6] >> 4;
			c3_ = buffer[7] & 0xF;
			invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
			g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
			g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
			g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
			g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
			isSectorTrailer = false;
		}
		
		// Which access group is this block in?
		if (no_of_blocks == 4) {
			group = blockOffset;
			firstInGroup = true;
		} else {
			group = blockOffset / 5;
			firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
		}
		
		if (firstInGroup) {
			// Print access bits
			getOutput()->print(F(" [ "));
			getOutput()->print((g[group] >> 2) & 1, DEC);
			getOutput()->print(F(" "));
			getOutput()->print((g[group] >> 1) & 1, DEC);
			getOutput()->print(F(" "));
			getOutput()->print((g[group] >> 0) & 1, DEC);
			getOutput()->print(F(" ] "));
			if (invertedError) {
				getOutput()->print(F(" Inverted access bits did not match! "));
			}
		}
		
		if (group != 3 && (g[group] == 1 || g[group] == 6)) { // Not a sector trailer, a value block
			int32_t value = (int32_t(buffer[3]) << 24) | (int32_t(buffer[2]) << 16) | (int32_t(buffer[1]) << 8) |
							int32_t(buffer[0]);
			getOutput()->print(F(" Value=0x"));
			getOutput()->print(value, HEX);
			getOutput()->print(F(" Adr=0x"));
			getOutput()->print(buffer[12], HEX);
		}
		getOutput()->println();
	}
	
	return;
} // End PICC_DumpMifareClassicSectorToSerial()


/**
 * Dumps memory contents of a MIFARE Ultralight PICC.
 */
void MFRC522Debug::PICC_DumpMifareUltralightToSerial(MFRC522 *_device) {
	MFRC522::StatusCode status;
	byte byteCount;
	byte buffer[18];
	byte i;
	
	getOutput()->println(F("Page  0  1  2  3"));
	// Try the mpages of the original Ultralight. Ultralight C has more pages.
	for (byte page = 0; page < 16; page += 4) { // Read returns data for 4 pages at a time.
		// Read pages
		byteCount = sizeof(buffer);
		status = _device->MIFARE_Read(page, buffer, &byteCount);
		if (status != MFRC522::STATUS_OK) {
			getOutput()->print(F("MIFARE_Read() failed: "));
			getOutput()->println(GetStatusCodeName(status));
			break;
		}
		// Dump data
		for (byte offset = 0; offset < 4; offset++) {
			i = page + offset;
			if (i < 10)
				getOutput()->print(F("  ")); // Pad with spaces
			else
				getOutput()->print(F(" ")); // Pad with spaces
			getOutput()->print(i);
			getOutput()->print(F("  "));
			for (byte index = 0; index < 4; index++) {
				i = 4 * offset + index;
				if (buffer[i] < 0x10)
					getOutput()->print(F(" 0"));
				else
					getOutput()->print(F(" "));
				getOutput()->print(buffer[i], HEX);
			}
			getOutput()->println();
		}
	}
} // End PICC_DumpMifareUltralightToSerial()
