#include "webapi_controller.hpp"

#include <string>

#include "../common/showmsg.hpp"
#include "../common/sql.hpp"

#include "auth.hpp"
#include "http.hpp"
#include "sqllock.hpp"
#include "webutils.hpp"
#include "web.hpp"

HANDLER_FUNC(register_post){
if (!req.has_file("AID") || !req.has_file("WorldName")) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}	
}