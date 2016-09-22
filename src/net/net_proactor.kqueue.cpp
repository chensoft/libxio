/**
 * Created by Jian Chen
 * @since  2016.09.19
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if !defined(__linux__) && !defined(_WIN32)

#include "net_proactor.kqueue.hpp"
#include <chen/sys/sys.hpp>

// -----------------------------------------------------------------------------
// proactor_detail
chen::net::proactor::proactor_detail::proactor_detail()
{
    if ((this->_fd = ::kqueue()) < 0)
        throw std::system_error(sys::error(), "proactor: failed to create kqueue");
}

chen::net::proactor::proactor_detail::~proactor_detail()
{
    ::close(this->_fd);
}

void chen::net::proactor::proactor_detail::send(net::socket *ptr, std::vector<std::uint8_t> &&data)
{
    this->_send[ptr].push(std::move(data));
    this->write(ptr);
}

void chen::net::proactor::proactor_detail::recv(net::socket *ptr, std::size_t size)
{
    this->_recv[ptr].push(chunk(size));
    this->read(ptr);
}

void chen::net::proactor::proactor_detail::remove(net::socket *ptr)
{
    this->_send.erase(ptr);
    this->_recv.erase(ptr);
}

void chen::net::proactor::proactor_detail::start()
{
    // todo add exit method
    struct kevent event{};

    while (true)
    {
        if (::kevent(this->_fd, nullptr, 0, &event, 1, nullptr) != 1)
            throw std::system_error(sys::error(), "proactor: failed to wait event");

        auto ptr = static_cast<net::socket*>(event.udata);
        if (!ptr)
            throw std::system_error(sys::error(), "proactor: event happened but no related socket");

        // connection refused, disconnect or other error
        if (event.flags & EV_EOF)
        {
            // todo clear cache first?
            ptr->onEventEOF();
            continue;
        }

        // simulate proactor, notify callback after send or receive data
        auto &list = this->_send[ptr];
        if (list.empty())
            continue;

        if (event.filter & EVFILT_WRITE)
        {
            // todo record original size
            auto &chunk = list.front();
            auto length = ptr->handle().send(chunk.data(), chunk.size());

            if (length >= 0)
            {
                chunk.resize(chunk.size() - length);

                if (chunk.empty())
                {
                    // all data have been sent
                    list.pop();
                    ptr->onEventSend(static_cast<std::size_t>(length), {});
                }
                else
                {
                    // wait for next write time point
                    this->write(ptr);
                }
            }
            else
            {
                ptr->onEventSend(0, sys::error());
            }
        }
        else if (event.filter & EVFILT_READ)
        {
            auto chunk = std::move(list.front());
            auto error = ptr->handle().recv(chunk);

            list.pop();

            ptr->onEventRecv(std::move(chunk), error);
        }
        else
        {
            throw std::system_error(sys::error(), "proactor: event happened but flags and filter are unknown");
        }
    }
}

void chen::net::proactor::proactor_detail::stop()
{
    // todo
}

// helper
void chen::net::proactor::proactor_detail::write(net::socket *ptr)
{
    struct kevent event{};
    EV_SET(&event, ptr->handle().native(), EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, ptr);

    if (::kevent(this->_fd, &event, 1, nullptr, 0, nullptr) < 0)
        throw std::system_error(sys::error(), "proactor: failed to add write event");
}

void chen::net::proactor::proactor_detail::read(net::socket *ptr)
{
    struct kevent event{};
    EV_SET(&event, ptr->handle().native(), EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, ptr);

    if (::kevent(this->_fd, &event, 1, nullptr, 0, nullptr) < 0)
        throw std::system_error(sys::error(), "proactor: failed to add read event");
}

#endif