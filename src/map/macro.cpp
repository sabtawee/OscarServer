#include "../common/db.hpp"
#include "../common/ers.hpp"
#include "../common/grfio.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "macro.hpp"
#include "pc.hpp"

using namespace rathena;

MacroDatabase captcha_db;

const char *macro_allowed_answer_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

/// lookup: captcha_index -> captcha_data
std::shared_ptr<captcha_data> MacroDatabase::captcha_byid(int captcha_id) {
	for (const auto &it : captcha_db) {
		if (it.second->id == captcha_id)
			return it.second;
	}
	return nullptr;
}

void macro_captcha_register(struct map_session_data *sd, const int image_size, const char *captcha_answer)
{
	nullpo_retv(sd);

	if (captcha_answer == NULL || strlen(captcha_answer) < 4
		|| (image_size < 0 || image_size > CAPTCHA_BMP_SIZE)
		|| captcha_db.size() >= MACRO_DB_MAX_SIZE) {
		clif_captcha_upload_request(sd, "", 1); // Notify client of failure.
		return;
	}

	std::shared_ptr<captcha_data> cd = std::make_shared<captcha_data>();
	cd->id = captcha_db.size()+1;
	cd->upload_size = 0;
	cd->image_size = image_size;
	safestrncpy(cd->captcha_answer, captcha_answer, sizeof(cd->captcha_answer));
	memset(cd->image_data, 0, CAPTCHA_BMP_SIZE);

	char captcha_key[50];
	safesnprintf(captcha_key, 50, "%X", (unsigned int)captcha_db.size()+1);
	safestrncpy(cd->captcha_key, captcha_key, sizeof(cd->captcha_key));

	captcha_db.put(cd->captcha_key,cd);

	// Request the image data from the client.
	clif_captcha_upload_request(sd, captcha_key, 0);
}

void macro_captcha_register_upload(struct map_session_data *sd, const char *captcha_key, const int upload_size, const char *upload_data)
{
	nullpo_retv(sd);
	nullpo_retv(captcha_key);
	nullpo_retv(upload_data);
	Assert(upload_size > 0 && upload_size <= MAX_CAPTCHA_CHUNK_SIZE);

	const int captcha_idx = (int)strtol(captcha_key, NULL, 16);
	Assert(captcha_idx > 0 && captcha_idx <= captcha_db.size());

	std::shared_ptr<captcha_data> cd = captcha_db.captcha_byid(captcha_idx);
	Assert((cd->upload_size + upload_size) <= cd->image_size);

	memcpy(&cd->image_data[cd->upload_size], upload_data, upload_size);
	cd->upload_size += upload_size;

	// Notify that the image finished uploading.
	if (cd->upload_size == cd->image_size)
		clif_captcha_upload_end(sd);
}

void macro_captcha_preview(struct map_session_data *sd, const int captcha_idx)
{
	nullpo_retv(sd);

	// Send client error if captcha id is out of range.
	const int cr_len = captcha_db.size();
	if (cr_len == 0 || captcha_idx <= 0 || captcha_idx > (cr_len)) {
		clif_captcha_preview_request_init(sd, "", 0, 1);
		return;
	}

	std::shared_ptr<captcha_data> cd = captcha_db.captcha_byid(captcha_idx);

	// Send preview initialization request to the client.
	clif_captcha_preview_request_init(sd, cd->captcha_key, cd->image_size, 0);

	// Send the image data in chunks.
	const int chunks = (cd->image_size / MAX_CAPTCHA_CHUNK_SIZE) +
						(cd->image_size % MAX_CAPTCHA_CHUNK_SIZE != 0);
	for (int i = 0, offset = 0; i < chunks; i++) {
		const int chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
		clif_captcha_preview_request_download(sd, cd->captcha_key, chunk_size, &cd->image_data[offset]);
		offset += chunk_size;
	}
}

void macro_detector_request(struct map_session_data *sd)
{
	nullpo_retv(sd);

	const struct captcha_data *cd = sd->macro_detect.cd;
	nullpo_retv(cd);

	// Send preview initialization request to the client.
	clif_macro_detector_request_init(sd, cd->captcha_key, cd->image_size);

	// Send the image data in chunks.
	const int chunks = (cd->image_size / MAX_CAPTCHA_CHUNK_SIZE) +
						(cd->image_size % MAX_CAPTCHA_CHUNK_SIZE != 0);
	for (int i = 0, offset = 0; i < chunks; i++) {
		const int chunk_size = min(cd->image_size - offset, MAX_CAPTCHA_CHUNK_SIZE);
		clif_macro_detector_request_download(sd, cd->captcha_key, chunk_size, &cd->image_data[offset]);
		offset += chunk_size;
	}
}

TIMER_FUNC (macro_detector_timeout){
	struct map_session_data *sd = map_id2sd(id);
	nullpo_ret(sd);

	if (sd->macro_detect.timer != tid){
		ShowError("macro_detect timer %d != %d\n",sd->macro_detect.timer,tid);
		sd->macro_detect.timer = INVALID_TIMER;
		return 0;
	}

	// Remove the current timer
	sd->macro_detect.timer = INVALID_TIMER;

	// Deduct an answering attempt
	sd->macro_detect.retry -= 1;

	if (sd->macro_detect.retry == 0) {
		// All attempts have been exhausted block the user
		clif_macro_detector_status(sd, MCD_TIMEOUT);
		chrif_req_login_operation(sd->macro_detect.reporter_aid, sd->status.name, CHRIF_OP_LOGIN_BLOCK, 0, 0, 0);
	} else {
		// update the client
		clif_macro_detector_request_show(sd);

		// Start a new timer
		sd->macro_detect.timer = add_timer(gettick() + battle_config.macro_detect_timeout,
			macro_detector_timeout, sd->bl.id, 0);
	}
	return 0;
}

void macro_detector_process_answer(struct map_session_data *sd, const char *captcha_answer)
{
	nullpo_retv(sd);

	const struct captcha_data *cd = sd->macro_detect.cd;

	// Correct answer
	if (captcha_answer != NULL && strcmp(captcha_answer, cd->captcha_answer) == 0) {
		// Delete the timer
		delete_timer(sd->macro_detect.timer, macro_detector_timeout);

		// Clear the macro detect data
		memset(&sd->macro_detect, 0, sizeof(sd->macro_detect));
		sd->macro_detect.timer = INVALID_TIMER;

		// Unblock all actions for the player
		sd->state.block_action &= ~PCBLOCK_ALL;
		sd->state.block_action &= ~PCBLOCK_IMMUNE;

		// Grant a small buff
		sc_start(NULL, &sd->bl, SC_INCREASEAGI, 100, 10, 120000);
		sc_start(NULL, &sd->bl, SC_BLESSING, 100, 10, 120000);

		// Notify the client
		clif_macro_detector_status(sd, MCD_GOOD);
		clif_refresh(sd);
		return;
	}

	// Deduct an answering attempt
	sd->macro_detect.retry -= 1;

	// All attempts have been exhausted block the user
	if (sd->macro_detect.retry == 0) {
		clif_macro_detector_status(sd, MCD_INCORRECT);
		chrif_req_login_operation(sd->macro_detect.reporter_aid, sd->status.name, CHRIF_OP_LOGIN_BLOCK, 0, 0, 0);
		return;
	}

	// Incorrect response, update the client
	clif_macro_detector_request_show(sd);

	// Reset the timer
	delete_timer(sd->macro_detect.timer, macro_detector_timeout);
	sd->macro_detect.timer = INVALID_TIMER;
	sd->macro_detect.timer = add_timer(gettick() + battle_config.macro_detect_timeout,
										macro_detector_timeout, sd->bl.id, 0);
}

void macro_detector_disconnect(struct map_session_data *sd)
{
	nullpo_retv(sd);

	// Delete the timeout timer
	if (sd->macro_detect.timer != INVALID_TIMER) {
		delete_timer(sd->macro_detect.timer, macro_detector_timeout);
		sd->macro_detect.timer = INVALID_TIMER;
	}

	// If the player disconnects before clearing the challenge the account is banned.
	if (sd->macro_detect.retry != 0)
		chrif_req_login_operation(sd->macro_detect.reporter_aid, sd->status.name, CHRIF_OP_LOGIN_BLOCK, 0, 0, 0);
}

void macro_reporter_area_select(struct map_session_data *sd, const int16 x, const int16 y, const int8 radius)
{
	nullpo_retv(sd);

	std::vector<int> aid_list;

	map_foreachinarea(macro_reporter_area_select_sub, sd->bl.m,
					x - radius, y - radius, x + radius, y + radius,
					BL_PC, &aid_list);

	clif_macro_reporter_select(sd, aid_list);
	aid_list.clear();
}

int macro_reporter_area_select_sub(struct block_list *bl, va_list ap)
{
	Assert(bl->type == BL_PC);

	std::vector<int> *aid_list = va_arg(ap, std::vector<int>*);
	nullpo_ret(aid_list);

	aid_list->push_back(bl->id);
	return 0;
}

void macro_reporter_process(struct map_session_data *ssd, struct map_session_data *tsd)
{
	nullpo_retv(ssd);
	nullpo_retv(tsd);
	Assert(captcha_db.size() != 0);

	// pick a random image from the database.
	int captcha_idx = rnd() % (int)captcha_db.size();
	if(captcha_idx == 0)
		captcha_idx = captcha_db.size();

	std::shared_ptr<captcha_data> cd = captcha_db.captcha_byid(captcha_idx);

	// set macro detection data
	memcpy(&tsd->macro_detect.cd, &cd, sizeof(cd));
	tsd->macro_detect.reporter_aid = ssd->status.account_id;
	tsd->macro_detect.retry = battle_config.macro_detect_retry;

	// Block all actions for the target player
	tsd->state.block_action |= (PCBLOCK_ALL | PCBLOCK_IMMUNE);

	// open macro detect client side
	macro_detector_request(tsd);

	// start the timeout timer.
	tsd->macro_detect.timer = add_timer(gettick() + battle_config.macro_detect_timeout,
										macro_detector_timeout, tsd->bl.id, 0);
}

const std::string MacroDatabase::getDefaultLocation() {
	return std::string(db_path) + "/macro_db.yml";
}

uint64 MacroDatabase::parseBodyNode(const ryml::NodeRef& node){

	uint16 id;

	if (!this->asUInt16(node, "Id", id))
		return 0;

	if( !this->nodesExist( node, { "Filename", "Answer" } ) )
		return 0;

	std::shared_ptr<captcha_data> cd = std::make_shared<captcha_data>();
	cd->id = id;

	std::string answer;

	if (!this->asString(node, "Answer", answer))
		return 0;

	std::string filename;

	if (!this->asString(node, "Filename", filename))
		return 0;

	safestrncpy( cd->captcha_answer, answer.c_str(), answer.length()+1 );

	if(!macro_read_captcha_db_loadbmp (filename, cd))
		return 0;
	cd->upload_size = cd->image_size;

	char captcha_key[50];
	safesnprintf(captcha_key, 50, "%X", (unsigned int)captcha_db.size()+1);
	safestrncpy(cd->captcha_key, captcha_key, sizeof(cd->captcha_key));
	ShowNotice("Load captcha id %d file %s answer %s key %s image_size %d upload size %d \n",id,filename.c_str(),answer.c_str(),cd->captcha_key,cd->image_size,cd->upload_size);
	this->put(cd->captcha_key,cd);

	return 1;
}

bool macro_read_captcha_db_loadbmp (std::string filename, std::shared_ptr<captcha_data> cd){

	std::string filepath = "db/captcha/" + filename;

	FILE *fp = fopen(filepath.c_str(), "rb");
	if(fp == NULL){
		ShowError("Can't open file at \"%s\"\n", filepath.c_str());
		return false;
	}
	nullpo_retr(false, fp);

	// Get the file size
	fseek(fp, 0, SEEK_END);
	const unsigned int file_len = (unsigned int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_len != CAPTCHA_BMP_SIZE) {
		ShowError("Invalid BMP file given at \"%s\"\n", filepath.c_str());
		fclose(fp);
		return false;
	}

	unsigned char *bmp_data = (unsigned char *)aMalloc(CAPTCHA_BMP_SIZE);

	if (fread(bmp_data, CAPTCHA_BMP_SIZE, 1, fp) != 1) {
		ShowError("Failed to read data from \"%s\"\n", filepath.c_str());
		fclose(fp);
		aFree(bmp_data);
		return false;
	}

	if (bmp_data[0] != 'B' || bmp_data[1] != 'M') {
		ShowError("%s: Invalid BMP file header given at \"%s\"\n", __func__, filepath);
		fclose(fp);
		aFree(bmp_data);
		return false;
	}

	// Variable init
	memset(cd->image_data, 0, CAPTCHA_BMP_SIZE);
	cd->image_size = 0;

	// Compress the data into the destination
	unsigned long com_size;
	encode_zip(cd->image_data, &com_size, bmp_data, CAPTCHA_BMP_SIZE);
	cd->image_size = static_cast<int>(com_size);

	aFree(bmp_data);
	fclose(fp);

	return true;
}

int do_init_macro()
{
	captcha_db.load();
	add_timer_func_list(macro_detector_timeout, "macro_detector_timeout");
	return 0;
}

void do_final_macro(void)
{
	captcha_db.clear();
}