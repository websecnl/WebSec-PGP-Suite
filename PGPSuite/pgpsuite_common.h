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