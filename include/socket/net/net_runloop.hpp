/**
 * Created by Jian Chen
 * @since  2016.10.15
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/net/net_socket.hpp>
#include <socket/bsd/bsd_poller.hpp>
#include <socket/bsd/bsd_kqueue.hpp>
#include <socket/bsd/bsd_epoll.hpp>
#include <unordered_map>
#include <functional>

namespace chen
{
    namespace net
    {
        /**
         * An event runloop implement using bsd::reactor
         */
        class runloop
        {
        public:
            /**
             * Operation code type
             * @attention specific meaning can refer to bsd::reactor
             */
            static constexpr int OpcodeRead  = bsd::reactor::OpcodeRead;
            static constexpr int OpcodeWrite = bsd::reactor::OpcodeWrite;

            /**
             * Specific flag type
             * @attention specific meaning can refer to bsd::reactor
             */
            static const int FlagOnce;
            static const int FlagEdge;

            /**
             * Reactor event type
             * @attention specific meaning can refer to bsd::reactor
             */
            enum class Event {Timeout = 1, Read, Write, End};

            typedef std::function<void (Event type)> callback_type;

        public:
            runloop() = default;

        public:
            /**
             * Set events and callback
             */
            void set(net::socket *ptr, int opcode, callback_type callback);
            void set(net::socket *ptr, int opcode, int flag, callback_type callback);

            /**
             * Remove events & callbacks
             * @attention socket should call this method when it is destroyed
             */
            void del(net::socket *ptr);

        public:
            /**
             * Start the loop
             * @attention this method will not return unless user request to stop
             */
            void start();

            /**
             * Stop the loop
             * @attention you can call this method in callback or other thread
             */
            void stop();

        private:
            runloop(const runloop&) = delete;
            runloop& operator=(const runloop&) = delete;

        private:
            bsd::reactor _reactor;
            std::unordered_map<net::socket*, callback_type> _callbacks;
        };
    }
}