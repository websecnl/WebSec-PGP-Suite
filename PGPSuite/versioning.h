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

    /* Checks if the local version is up-to-date, otherwise it displays a dialogue giving the latest version
    @return If local version is up-to-date */
    inline bool verify_local_version()
    {
        const auto local_version = get_local_version();
        const auto latest_version = retrieve_version();

        const auto res = compare(local_version, latest_version);

        if (!res)
        {
            wxMessageBox(_("Current version: ") + local_version + _("\nNew version: ") + latest_version, _("New version available."));
        }

        return res;
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
        auto version = parse_version(response);

        if (version.size() == 0)
        {
            wxMessageBox(_("Could not reach host, check your internet connection."), _("Error."), wxICON_ERROR);
            return "";
        }

        return version;
    }

    inline wxString get_local_version()
    {
        wxFileInputStream fstream(_("version.txt"));
        wxTextInputStream input( fstream );
        return input.ReadLine();
    }
}