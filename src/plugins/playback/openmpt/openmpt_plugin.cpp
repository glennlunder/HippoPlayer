#include <libopenmpt/libopenmpt.hpp>
#include <HippoPlugin.h>
#include <HippoMessages.h>

#include <vector>
#include <string>
#include <string.h>
#include <assert.h>

const int MAX_EXT_COUNT = 16 * 1024;
static char s_supported_extensions[MAX_EXT_COUNT];

const HippoIoAPI* g_io_api = nullptr;
HippoLogAPI* g_hp_log = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct OpenMptData {
    openmpt::module* mod = 0;
    const HippoMessageAPI* message_api;
    float length = 0.0f;
    void* song_data = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* openmpt_supported_extensions() {
	// If we have already populated this we can just return

	if (s_supported_extensions[0] != 0) {
		return s_supported_extensions;
	}

	std::vector<std::string> ext_list = openmpt::get_supported_extensions();
	size_t count = ext_list.size();

	for (size_t i = 0; i < count; ++i) {
		strcat(s_supported_extensions, ext_list[i].c_str());
		if (i != count - 1) {
			strcat(s_supported_extensions, ",");
		}
	}

	return s_supported_extensions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* openmpt_create(const HippoServiceAPI* service_api) {
    OpenMptData* user_data = new OpenMptData;

    g_io_api = HippoServiceAPI_get_io_api(service_api, 1);
    user_data->message_api = HippoServiceAPI_get_message_api(service_api, 1);

	return (void*)user_data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_destroy(void* user_data) {
    OpenMptData* data = (OpenMptData*)user_data;
    delete data;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HippoProbeResult openmpt_probe_can_play(const uint8_t* data, uint32_t data_size, const char* filename, uint64_t total_size) {
    int res = openmpt::probe_file_header(openmpt::probe_file_header_flags_default, data, data_size, total_size);

    switch (res) {
        case openmpt::probe_file_header_result_success :
        {
            hp_info("Supported: %s", filename);
            return HippoProbeResult_Supported;
        }
        case openmpt::probe_file_header_result_failure :
        {
            hp_debug("Unsupported: %s", filename);
            return HippoProbeResult_Unsupported;
        }

        case openmpt::probe_file_header_result_wantmoredata :
        {
            hp_warn("openmpt: Unable to probe because not enough data\n");
            break;
        }
    }

    hp_warn("openmpt: case %d not handled in switch. Assuming unsupported file\n", res);

    return HippoProbeResult_Unsupported;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
static void send_pattern_data(struct OpenMptData* replayer_data) {
    flatbuffers::FlatBufferBuilder builder(16 * 1024);

    const int current_pattern = replayer_data->mod->get_current_pattern();
    const int current_row = replayer_data->mod->get_current_row();
    const int channel_count = replayer_data->mod->get_num_channels();
    const int num_rows = replayer_data->mod->get_pattern_num_rows(current_pattern);

    assert(channel_count < 2048);
    assert(num_rows < 512);

    flatbuffers::Offset<HippoTrackerChannel> channels[2048];

	for (int channel = 0; channel < channel_count; ++channel) {
        flatbuffers::Offset<HippoRowData> rows[512];

        for (int row = 0; row < num_rows; ++row) {
	        std::string note = replayer_data->mod->format_pattern_row_channel_command(current_pattern, row, channel, openmpt::module::command_note);
	        std::string instrument = replayer_data->mod->format_pattern_row_channel_command(current_pattern, row, channel, openmpt::module::command_instrument);
	        std::string effect = replayer_data->mod->format_pattern_row_channel_command(current_pattern, row, channel, openmpt::module::command_effect);
	        std::string vol_effect = replayer_data->mod->format_pattern_row_channel_command(current_pattern, row, channel, openmpt::module::command_parameter);

            auto note_name = builder.CreateString(note.c_str());
            auto inst_name = builder.CreateString(instrument.c_str());
            auto effect_name = builder.CreateString(effect.c_str());
            auto voleffect_name = builder.CreateString(vol_effect.c_str());

            HippoRowDataBuilder row_builder(builder);
            row_builder.add_note(note_name);
            row_builder.add_instrument(inst_name);
            row_builder.add_effect(effect_name);
            row_builder.add_volumeffect(voleffect_name);
	        rows[row] = row_builder.Finish();
        }

        auto row_data = builder.CreateVector(rows, num_rows);
        channels[channel] = CreateHippoTrackerChannel(builder, row_data);
	}

	auto channel_data = builder.CreateVector(channels, channel_count);
    auto tracker_data = CreateHippoTrackerData(builder, HippoTrackerType_Regular, current_pattern, current_row, channel_data);

    builder.Finish(CreateHippoMessageDirect(builder, MessageType_tracker_data, tracker_data.Union()));

    HippoMessageAPI_send(replayer_data->message_api, builder.GetBufferPointer(), builder.GetSize());

    /*
	for (int i = 0; i < 64; ++i) {
	    std::string t = replayer_data->mod->highlight_pattern_row_channel(pattern, i, 0);
	    std::string note = replayer_data->mod->format_pattern_row_channel_command(pattern, i, 0, openmpt::module::command_note);
	    printf("%04x %s - %s\n", i, t.c_str(), note.c_str());
	}
	*/
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_open(void* user_data, const char* filename, int subsong) {
    uint64_t size = 0;
	struct OpenMptData* replayer_data = (struct OpenMptData*)user_data;

    HippoIoErrorCode res = g_io_api->read_file_to_memory(g_io_api->priv_data,
        filename, &replayer_data->song_data, &size);

    if (res < 0) {
        hp_error("Failed to load %s to memory", filename);
        return -1;
    }

    try {
        replayer_data->mod = new openmpt::module(replayer_data->song_data, size);
    } catch(...) {
        hp_error("Failed to open %s even if is as supported format", filename);
        return -1;
    }

    hp_info("Started to play %s (subsong %d)", filename, subsong);

    replayer_data->length = replayer_data->mod->get_duration_seconds();
    replayer_data->mod->select_subsong(subsong);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_close(void* user_data) {
	struct OpenMptData* replayer_data = (struct OpenMptData*)user_data;

	if (g_io_api) {
	    g_io_api->free_file_to_memory(g_io_api->priv_data, replayer_data->song_data);
	}

	delete replayer_data->mod;
	delete replayer_data;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_read_data(void* user_data, void* dest, uint32_t max_samples) {
	struct OpenMptData* replayer_data = (struct OpenMptData*)user_data;

	// count is number of frames per channel and div by 2 as we have 2 channels
	const int count = 480;
    int gen_count = replayer_data->mod->read_interleaved_stereo(48000, count, (float*)dest) * 2;

    // Send current positions back to frontend if we have some more data
    if (gen_count > 0) {
        flatbuffers::FlatBufferBuilder builder(1024);
        builder.Finish(CreateHippoMessageDirect(builder, MessageType_current_position,
            CreateHippoCurrentPosition(builder,
                replayer_data->mod->get_position_seconds(),
                replayer_data->mod->get_current_pattern(),
                replayer_data->mod->get_current_row(),
                replayer_data->mod->get_current_speed(),
                replayer_data->length).Union()));
        HippoMessageAPI_send(replayer_data->message_api, builder.GetBufferPointer(), builder.GetSize());
    }

    // TODO: Only send pattern data when we need requsted
    //send_pattern_data(replayer_data);
    return gen_count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_seek(void* user_data, int ms) {
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* filename_from_path(const char* path) {
   for(size_t i = strlen(path) - 1;  i > 0; i--) {
      if (path[i] == '/') {
         return &path[i+1];
      }
   }

   return path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openmpt_metadata(const char* filename, const HippoServiceAPI* service_api) {
    void* data = 0;
    uint64_t size = 0;

    const HippoIoAPI* io_api = HippoServiceAPI_get_io_api(service_api, HIPPO_FILE_API_VERSION);
    const HippoMetadataAPI* metadata_api = HippoServiceAPI_get_metadata_api(service_api, HIPPO_METADATA_API_VERSION);

    HippoIoErrorCode res = HippoIo_read_file_to_memory(io_api, filename, &data, &size);

    if (res < 0) {
        return res;
    }

    openmpt::module* mod = nullptr;

    try {
        mod = new openmpt::module(data, size);
    } catch(...) {
        hp_error("Failed to open %s even if is as supported format", filename);
        return -1;
    }

    auto index = HippoMetadata_create_url(metadata_api, filename);
    char title[512] = { 0 };

    const auto& mod_title = mod->get_metadata("title");

    if (mod_title != "") {
        strcpy(title, mod_title.c_str());
    } else {
        const char* file_title = filename_from_path(filename);
        strcpy(title, file_title);
    }

    hp_info("Updating meta data for %s", filename);

    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_TitleTag, title);
    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_SongTypeTag, mod->get_metadata("type_long").c_str());
    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_AuthoringToolTag, mod->get_metadata("tracker").c_str());
    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_ArtistTag, mod->get_metadata("artist").c_str());
    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_DateTag, mod->get_metadata("date").c_str());
    HippoMetadata_set_tag(metadata_api, index, HippoMetadata_MessageTag, mod->get_metadata("message").c_str());
    HippoMetadata_set_tag_f64(metadata_api, index, HippoMetadata_LengthTag, mod->get_duration_seconds());

	for (const auto& sample : mod->get_sample_names()) {
    	HippoMetadata_add_sample(metadata_api, index, sample.c_str());
	}

	for (const auto& instrument : mod->get_instrument_names()) {
    	HippoMetadata_add_instrument(metadata_api, index, instrument.c_str());
	}

	const int subsong_count = mod->get_num_subsongs();

	if (subsong_count > 1) {
	    int i = 0;
        for (const auto& name : mod->get_subsong_names()) {
            char subsong_name[1024] = { 0 };

            if (name != "") {
                sprintf(subsong_name, "%s - %s (%d/%d)", title, name.c_str(), i + 1, subsong_count);
            } else {
                sprintf(subsong_name, "%s (%d/%d)", title, i + 1, subsong_count);
            }

            mod->select_subsong(i);
            HippoMetadata_add_subsong(metadata_api, index, i, subsong_name, mod->get_duration_seconds());

            ++i;
        }
    }

    // Make sure to free the buffer before we leave
    HippoIo_free_file_to_memory(io_api, data);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void openmpt_event(void* user_data, const unsigned char* data, int len) {
    (void)len;
    (void)user_data;
    (void)data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void openmpt_set_log(struct HippoLogAPI* log) { g_hp_log = log; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HippoPlaybackPlugin g_openmptPlugin = {
	HIPPO_PLAYBACK_PLUGIN_API_VERSION,
	"openmpt",
	"0.0.1",
	"libopenmpt 0.5.0",
	openmpt_probe_can_play,
	openmpt_supported_extensions,
	openmpt_create,
	openmpt_destroy,
	openmpt_event,
	openmpt_open,
	openmpt_close,
	openmpt_read_data,
	openmpt_seek,
	openmpt_metadata,
    openmpt_set_log,
	NULL,
	NULL,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" HIPPO_EXPORT HippoPlaybackPlugin* hippo_playback_plugin() {
	return &g_openmptPlugin;
}


