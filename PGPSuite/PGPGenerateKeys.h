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

