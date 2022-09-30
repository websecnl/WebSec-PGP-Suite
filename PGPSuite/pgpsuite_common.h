#pragma once

/* Used to check if an rnp function resulted in success */
constexpr int RNP_SUCCESS{ 0 };

namespace suite
{
	/* Operation Result
	To be returned by pgp operations, if OpRes::what is not empty there is an error */
	struct OpRes
	{
	private:
		std::string _what;
	public:	
		OpRes() = default;
		OpRes(std::string&& what)
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