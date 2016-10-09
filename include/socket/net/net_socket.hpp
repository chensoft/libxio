/**
 * Created by Jian Chen
 * @since  2016.09.19
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/net/net_endpoint.hpp>
#include <socket/bsd/bsd_socket.hpp>

namespace chen
{
    namespace net
    {
        class socket
        {
        public:
            socket(socket_t fd);
            socket(ip::address::Type family, int type);

            virtual ~socket() = default;

        public:
            /**
             * Sock & Peer endpoint
             */
            net::endpoint sock() const;
            net::endpoint peer() const;

            /**
             * Non-blocking status
             */
            bool nonblocking() const;
            bool nonblocking(bool enable);

            /**
             * Check socket is valid
             */
            bool valid() const;
            operator bool() const;

            /**
             * Native socket handle
             */
            socket_t native() const;

        protected:
            friend class proactor;

            /**
             * Internal socket
             */
            bsd::socket& handle();

            /**
             * Event callbacks
             */
            virtual void onEventRead(std::vector<std::uint8_t> data, std::error_code error) = 0;
            virtual void onEventWrite(std::size_t size, std::error_code error) = 0;
            virtual void onEventEnd() = 0;  // connection refused, disconnect or other error

        protected:
            bsd::socket _handle;
        };
    }
}