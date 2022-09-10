#ifndef MACRO_HPP
#define MACRO_HPP

#include "../common/mmo.hpp"
#include "map.hpp"

#define CAPTCHA_BMP_SIZE (2 + 52 + (3 * 220 * 90)) // sizeof("BM") + sizeof(BITMAPV2INFOHEADER) + 24bits 220x90 BMP
#define MACRO_DB_MAX_SIZE 0xFFF
#define MAX_CAPTCHA_CHUNK_SIZE 1024

struct captcha_data {
	int id;
	int upload_size;
	int image_size;
	char image_data[CAPTCHA_BMP_SIZE];
	char captcha_answer[16];
	char captcha_key[4];
};

struct macro_detect {
	const struct captcha_data *cd;
	int reporter_aid;
	int retry;
	int timer;
};

enum macro_detect_status : uint8 {
	MCD_TIMEOUT = 0,
	MCD_INCORRECT = 1,
	MCD_GOOD = 2,
};

enum macro_report_status : uint8 {
	MCR_MONITORING = 0,
	MCR_NO_DATA = 1,
	MCR_INPROGRESS = 2,
};

class MacroDatabase : public TypesafeYamlDatabase<std::string, captcha_data> {
public:
	MacroDatabase() : TypesafeYamlDatabase("MACRO_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node);

	// Additional
	std::shared_ptr<captcha_data> captcha_byid(int captcha_id);
};

extern MacroDatabase captcha_db;

int do_init_macro();
void do_final_macro();
void macro_captcha_register(struct map_session_data *sd, const int image_size, const char *captcha_answer);
void macro_captcha_register_upload(struct map_session_data *sd, const char *captcha_key, const int upload_size, const char *upload_data);
void macro_captcha_preview(struct map_session_data *sd, const int captcha_idx);
void macro_detector_request(struct map_session_data *sd);
TIMER_FUNC(macro_detector_timeout);
void macro_detector_process_answer(struct map_session_data *sd, const char *captcha_answer);
void macro_detector_disconnect(struct map_session_data *sd);
void macro_reporter_area_select(struct map_session_data *sd, const int16 x, const int16 y, const int8 radius);
int macro_reporter_area_select_sub(struct block_list *bl, va_list ap);
void macro_reporter_process(struct map_session_data *ssd, struct map_session_data *tsd);
bool macro_read_captcha_db_loadbmp (std::string filename, std::shared_ptr<captcha_data> cd);

#endif /* MACRO_HPP */
