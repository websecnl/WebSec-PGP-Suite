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

#include <string>

/* Used to check if an rnp function resulted in success */
constexpr int RNP_SUCCESS{ 0 };

namespace pgp
{
	/* Operation Result
	To be returned by pgp operations, if OpRes::_what is not empty there is an error */
	struct OpRes
	{
	private:
		std::string _what;
	public:	
		OpRes() = default;
		/* @param no_err: True if no error, False if error */
		OpRes(bool no_err) 
			: _what(no_err ? "" : "Generic error\n")
		{}
		/* @brief Constructs result with a custom message of what went wrong */
		OpRes(std::string&& what)
			: _what(what)
		{}
		/* @brief Constructs result with a custom message of what went wrong */
		OpRes(const char* what)
			: _what(what)
		{}

		operator bool() const { return !error(); }
		bool error() const { return !_what.empty(); }

		const auto& what() const
		{
			return _what;
		}
	};
}