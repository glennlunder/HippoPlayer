#include "../../hippoplayer/plugin_api/HippoPlugin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FlacReplayerData
{
	FLAC__StreamDecoder* decoder;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* flacInfo(void* userData)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* flacTrackInfo(void* userData)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char** flacSupportedExtensions(void* userData)
{
	static const char* supportedFomats[] =
	{
		"flac",
		0,
	};

	return supportedFomats;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* flacCreate()
{
	// TODO: supply custom allocator

	void* replayerData = malloc(sizeof(struct FlacReplayerData));
	memset(replayerData, 0, sizeof(struct FlacReplayerData));

	return replayerData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacDestroy(void* userData)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacOpen(void* userData, const char* buffer)
{
	FlacReplayerData* replayer = (FlacReplayer*)malloc(sizeof(FlacReplayer));

    FLAC__stream_decoder_init_stream(decoder, read_callback, seek_callback, 
    								 tell_callback, length_callback, eof_callback, write_callback,
                                     metadata_callback, error_callback, replayer);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacClose(void* userData)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacReadData(void* userData, void* dest)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacSeek(void* userData, int ms)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int flacFrameSize(void* userData)
{
	return 44100 * sizeof(uint16_t) * 2 / 50;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HippoPlaybackPlugin g_flacPlugin = 
{
	1,
	flacInfo,
	flacTrackInfo,
	flacSupportedExtensions,
	flacCreate,
	flacDestroy,
	flacOpen,
	flacClose,
	flacReadData,
	flacSeek,
	flacFrameSize,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HippoPlaybackPlugin* getPlugin()
{
	return &g_flacPlugin;
}


