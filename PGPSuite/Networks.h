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

        do {
            std::array<char, 1024> buf;
            size_t bytes_transferred = ssock.read_some(asio::buffer(buf), ec);
            if (!ec) response.append(buf.data(), buf.data() + bytes_transferred);
        } while (!ec);

        return response;
    }

    inline std::string get_network_data()
    {
        return query("www.vapt.nl", "443", "GET /pgpsuite_version.txt HTTP/1.1\r\nHost: vapt.nl\r\nAccept: text/plain\r\nConnection: close\r\n\r\n");
    }
}
