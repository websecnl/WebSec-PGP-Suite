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

#include <asio.hpp>
#include <asio/ssl.hpp>

#define PGPSUITE_SIG "--start--"
#define PGPSUITE_SIG_LEN sizeof(PGPSUITE_SIG)

namespace suite::net
{
    inline std::string query(std::string host, std::string port, std::string request)
    {
        asio::io_service svc;
        asio::ssl::context ctx(asio::ssl::context::method::sslv23_client);
        asio::ssl::stream<asio::ip::tcp::socket> ssock(svc, ctx);

        asio::ip::tcp::resolver resolver(svc);

        try
        {
            auto it = resolver.resolve({ host, port });
            asio::connect(ssock.lowest_layer(), it);
        }
        catch (const std::exception& e)
        {
            return "";
        }

        ssock.handshake(asio::ssl::stream_base::handshake_type::client);

        asio::write(ssock, asio::buffer(request));

        std::string response;
        asio::error_code ec;

        while (!ec)
        {
            std::array<char, 1024> buf;
            size_t bytes_transferred = ssock.read_some(asio::buffer(buf), ec);
            if (!ec) response.append(buf.data(), buf.data() + bytes_transferred);
        }

        return response;
    }

    inline std::string get_network_data()
    {
        return query("www.vapt.nl", "443", "GET /pgpsuite_version.txt HTTP/1.1\r\nHost: vapt.nl\r\nAccept: text/plain\r\nConnection: close\r\n\r\n");
    }
}
