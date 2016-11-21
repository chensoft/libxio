/**
 * Created by Jian Chen
 * @since  2016.11.12
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/base/basic_socket.hpp>
#include <unordered_map>
#include <vector>

#ifndef _WIN32
#include <poll.h>
#endif

namespace chen
{
    /**
     * poll for Unix, WSAPoll for Windows(reactor model)
     * Windows's most efficient model is IOCP, but IOCP is a proactor model, so we use WSAPoll here
     * you should not use this class directly unless you want to implement your own event-based model
     * @attention poller does not support edge-triggered, so it's always level-triggered
     */
    class poller
    {
    public:
        /**
         * Read(LT): event always occurs if the recv buffer has unread data
         * ---------------------------------------------------------------------
         * Write(LT): event always occurs if the send buffer is not full
         * ---------------------------------------------------------------------
         * @attention since the socket has its own send buffer, you don't need to monitor the write event from the start
         * usually you should call send() first, if the method return EAGAIN then to wait for the write event occurs
         */
        static constexpr int OpcodeRead  = 1 << 0;
        static constexpr int OpcodeWrite = 1 << 1;
        static constexpr int OpcodeRW    = OpcodeRead | OpcodeWrite;

        /**
         * Edge: JUST A PLACEHOLDER HERE
         * Once: event occurs only once
         */
        static constexpr int FlagEdge = 0;  // no effect, in order to keep ABI compatible with kqueue & epoll
        static constexpr int FlagOnce = 1;  // we simulate this flag in poller

        /**
         * Readable: read event occurs, you can read data from socket
         * ---------------------------------------------------------------------
         * Writable: you can write data to remote host
         * ---------------------------------------------------------------------
         * Ended: socket disconnected or connection refused
         * ---------------------------------------------------------------------
         * @attention you must monitor the read event if you want to know the end event
         * this behavior is different than Linux's epoll
         * in epoll, end event will always be monitored
         * ---------------------------------------------------------------------
         * @attention you should read the rest of the data even if you received the end event
         * because server may send last message and then close the connection immediately
         * poller may report Readable & Ended event or only report the Ended event
         */
        enum class Event {Readable = 1, Writable, Ended};

        typedef struct Data
        {
            Data() = default;
            Data(handle_t fd, Event ev) : fd(fd), ev(ev) {}

            handle_t fd = invalid_handle;
            Event ev;
        } Data;

    public:
        poller();
        ~poller();

    public:
        /**
         * Set events for fd, if Ended event occurs then fd will be removed
         * @param opcode OpcodeRead, OpcodeWrite or combination of them
         * @param flag just allow FlagOnce, use FlagEdge has no effect
         */
        void set(handle_t fd, int opcode, int flag = 0);

        /**
         * Delete all events for fd
         */
        void del(handle_t fd);

    public:
        /**
         * Poll events, with an optional timeout
         * when timeout is negative, it means wait forever, usually you can pass -1 to it
         * when timeout is zero, the poll method will return immediately, an event may or may not return
         * when timeout is positive, the time unit is second, e.g: 1.15 means 1.15 seconds to wait
         * @param cache pre allocated cache, if size < count then push result to it if needed
         * @param count how many events you want to monitor, just a hint, final events may greater than this
         * @return the final events count, or zero if user request to stop, timeout or interrupted
         * @attention the number of events may greater than count because we treat read
         * and write as separate events, but poll may report them as a single event
         */
        std::size_t poll(std::vector<Data> &cache, std::size_t count, double timeout = -1);

        /**
         * Poll events, return vector directly
         * @return empty if user request to stop, timeout or interrupted
         */
        std::vector<Data> poll(std::size_t count, double timeout = -1);

        /**
         * Stop the poll if poller is waiting for events
         */
        void stop();

    private:
        poller(const poller&) = delete;
        poller& operator=(const poller&) = delete;

    private:
#ifdef _WIN32
		basic_socket _up;  // wake socket
#else
        int _pp[2]{};  // pipe handle
#endif

        bool _wk = false;  // is working
        std::unordered_map<handle_t, std::pair<::pollfd, int>> _fds;  // all fds
    };
}