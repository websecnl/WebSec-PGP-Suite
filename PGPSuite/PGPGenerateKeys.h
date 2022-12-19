/*
 *
 * Copyright (c) 2018-2023
 * Author: WebSec B.V.
 * Developer: Koen Blok
 * Website: https://websec.nl
 *
 * Permission to use, copy, modify, distribute this software
 * and its documentation for non-commercial purposes is hereby granted exclusivley
 * under the terms of the GNU GPLv3 License.
 *
 * Most importantly:
 *  1. The above copyright notice appear in all copies and supporting documents.
 *  2. The application / code will not be used or reused for commercial purposes.
 *  3. All modifications are documented.
 *  4. All new releases will remain open source and contain the same license.
 *
 * WebSec B.V. makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * please read the full license agreement for more information:
 * https://github.com/websecnl/PGPSuite/LICENSE.md
 */
#pragma once

#include "pgpsuite_common.h"
#include "IOTools.h"
#include <rnp\rnp.h>
#include "rnp_wrappers.h"
#include "Utils.h"

namespace pgp
{
	/* should really go somewhere else but, i cba */
	bool generic_cin_pass_provider(rnp_ffi_t           ffi,
		void* app_ctx,
		rnp_key_handle_t    key,
		const char* pgp_context,
		char                buf[],
		size_t              buf_len);

	/* @brief Generate a keyring with settings from a json file
	@param pubkey_file: filename for public keyring
	@param secret_file: filename for secret keyring
	@param key_data: key settings in json format
	@param passprovider: password provider to be used by the ffi context */
	pgp::OpRes generate_keys(std::string pubkey_file = "pubring.pgp", 
		std::string secret_file = "secring.pgp", 
		std::string_view key_data = "keygen.json", 
		rnp_password_cb passprovider = generic_cin_pass_provider);
}

