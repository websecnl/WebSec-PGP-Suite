#pragma once

#include "Networks.h"

#include <wx/txtstrm.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>

namespace suite::ver
{
    inline bool compare(const wxString& a, const wxString& b)
    {
        return a == b;
    }

    /* Will write a proper parser once it is required to receive more info from server, for now enjoy this monstrosity */
    inline std::string parse_version(const std::string& response)
    {
        auto start = response.find(PGPSUITE_SIG);

        if (response.npos == start) return "";

        start = response.find('=', start);
        auto end = response.find(';', start);

        if (start == response.npos || end == response.npos) return "";

        return response.substr(start + 1, end - start - 1);
    }

    inline std::string retrieve_version()
    {
        const auto response = net::get_network_data();

        return parse_version(response);
    }

    inline static wxString get_local_version()
    {
        wxFileInputStream fstream(_("version.txt"));
        wxTextInputStream input( fstream );
        return input.ReadLine();
    }
}