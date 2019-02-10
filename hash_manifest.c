// code by ntu ael

#include <string.h>

#include "Velox_meta.h"
#include "Velox_uptime.h"

#include "sdcard_redundant.h"

#include "mccif_spy.h"
#include "mccif_xdata.h"

#include "shared_obdh.h"

#include "hash_blob.h"
#include "hash_frame.h"
#include "hash_util.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_trxuv.h"

#define MANIFEST_PEEK(ptr, num) \
  memcpy(ptr, xdata_txbuffer + MCCIF_DATA_OFFSET + size, num); \
  size += num

extern short g_s_BinaryId;

volatile char manifest_reset_incr = 1;

//unsigned long Hash_addressManifest() {
//  unsigned long time = uptime.tai_time;
//  time /= g_manifest.divider; // safety measure to prevent out of bounds/"pages" access, however does not alter value of frameIndex->page (because no immediate need)
//  return g_manifest.offset + time % 32;
//}

// failure is not an option
void Hash_readManifest() {
	char success;
	char capture;
	short size = 0;
	capture = g_sdcard.read.failures;
	//TOSHOW the right way to do this is we should record the manifest. If it is the same offset then we don't do anything.
	//Else, if it is not the same offset, we should re-copy the value, because the manifest.offset would have been changed!
	success = SDred_read(g_manifest.offset, xdata_txbuffer, sizeof xdata_txbuffer, g_manifest.copies); //TOSHOW there is a bug here!! It should be g_manifest.copies instead of g_hashFrame.copies!!
	g_manifest.failures = g_sdcard.read.failures - capture;
	if (success) {
		MANIFEST_PEEK(&uptime.total, 6);
		MANIFEST_PEEK(&chip_meta.totalResets, 4);
		MANIFEST_PEEK(&g_hashFrame, sizeof g_hashFrame);
		MANIFEST_PEEK(&g_hashBlob, sizeof g_hashBlob);
		MANIFEST_PEEK(&g_putFrame.frameIndex.page, sizeof g_putFrame.frameIndex.page);
		// --- 5
		MANIFEST_PEEK(&g_putFrame.task.delay, sizeof g_putFrame.task.delay); //to store all the important data!! ian: updated on 20140207
		MANIFEST_PEEK(&g_putFrame.mask, sizeof g_putFrame.mask); //to store all the important data!! ian: updated on 20140207
		MANIFEST_PEEK(&g_getFrame.mask, sizeof g_getFrame.mask); //to store all the important data!! ian: updated on 20140207
		MANIFEST_PEEK(&g_sendAck.uplinkId, sizeof g_sendAck.uplinkId);
		MANIFEST_PEEK(&g_rtHk.template, sizeof g_rtHk.template);
		// --- 10
		MANIFEST_PEEK(&g_s_BinaryId, sizeof g_s_BinaryId);
		MANIFEST_PEEK(&g_trxuv.rateCW, sizeof g_trxuv.rateCW);
		MANIFEST_PEEK(&g_trxuv.runtime.delay, sizeof g_trxuv.runtime.delay);
		MANIFEST_PEEK(&g_sdcard.write, sizeof g_sdcard.write);
		MANIFEST_PEEK(&g_sdcard.read, sizeof g_sdcard.read);
		// --- 15
		MANIFEST_PEEK(&g_slow_beacon.mode, sizeof g_slow_beacon.mode);
		// --- 16
		// ---
		if (manifest_reset_incr) {
			chip_meta.totalResets += manifest_reset_incr;
			uptime.total += uptime.session;
		}
		manifest_reset_incr = 0;
		g_rtHk.template &= XDATSEG_FULLMASK;
	} else {
		// in case of failure, default values are used
	}
}

#define MANIFEST_POKE(ptr, num) \
    memcpy(g_c_BinaryContent + size, ptr, num); \
    size += num

// harmless reuse of memory, since cqt always load total chunk and immediately processes the data subsequently
extern char g_c_BinaryContent[CQT_RAWDATA];

void Hash_writeManifest() {
	short length;
	short size = 0;
	// ---
	MANIFEST_POKE(&uptime.total, 6);
	MANIFEST_POKE(&chip_meta.totalResets, 4);
	MANIFEST_POKE(&g_hashFrame, sizeof g_hashFrame);
	MANIFEST_POKE(&g_hashBlob, sizeof g_hashBlob);
	MANIFEST_POKE(&g_putFrame.frameIndex.page, sizeof g_putFrame.frameIndex.page);
	// --- 5
	MANIFEST_POKE(&g_putFrame.task.delay, sizeof g_putFrame.task.delay); //to store all the important data!! ian: updated on 20140207
	MANIFEST_POKE(&g_putFrame.mask, sizeof g_putFrame.mask); //to store all the important data!! ian: updated on 20140207
	MANIFEST_POKE(&g_getFrame.mask, sizeof g_getFrame.mask); //to store all the important data!! ian: updated on 20140207
	MANIFEST_POKE(&g_sendAck.uplinkId, sizeof g_sendAck.uplinkId);
	MANIFEST_POKE(&g_rtHk.template, sizeof g_rtHk.template);
	// --- 10
	MANIFEST_POKE(&g_s_BinaryId, sizeof g_s_BinaryId);
	MANIFEST_POKE(&g_trxuv.rateCW, sizeof g_trxuv.rateCW);
	MANIFEST_POKE(&g_trxuv.runtime.delay, sizeof g_trxuv.runtime.delay);
	MANIFEST_POKE(&g_sdcard.write, sizeof g_sdcard.write);
	MANIFEST_POKE(&g_sdcard.read, sizeof g_sdcard.read);
	// --- 15
	MANIFEST_POKE(&g_slow_beacon.mode, sizeof g_slow_beacon.mode);
	// --- 16
	// ---
	length = Mccif_formatContent(XDATA_OBDH_CQTDATA_OFFSET, size, xdata_txbuffer);
	SDred_write(g_manifest.offset, xdata_txbuffer, length, g_manifest.copies);
}

