/**
 * Created by Jian Chen
 * @since  2016.09.25
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifdef __linux__

#include <socket/bsd/bsd_epoll.hpp>
#include <socket/config.hpp>
#include <chen/sys/sys.hpp>
#include <sys/eventfd.h>
#include <sys/epoll.h>

// -----------------------------------------------------------------------------
// epoll
const int chen::bsd::epoll::FlagOnce = EPOLLONESHOT;
const int chen::bsd::epoll::FlagEdge = EPOLLET;

chen::bsd::epoll::epoll()
{
    // create epoll file descriptor
    if ((this->_fd = ::epoll_create1(0)) < 0)
        throw std::system_error(sys::error(), "epoll: failed to create epoll");

    // create eventfd to receive wake message
    if ((this->_ef = ::eventfd(0, 0)) < 0)
    {
        ::close(this->_fd);
        throw std::system_error(sys::error(), "epoll: failed to create eventfd");
    }

    this->set(this->_ef, OpcodeRead, FlagEdge);
}

chen::bsd::epoll::~epoll()
{
    ::close(this->_fd);
    ::close(this->_ef);
}

// modify
void chen::bsd::epoll::set(int fd, int opcode, int flag)
{
    struct ::epoll_event event{};

    if (opcode & OpcodeRead)
        event.events |= EPOLLIN;

    if (opcode & OpcodeWrite)
        event.events |= EPOLLOUT;

    event.events |= flag | EPOLLRDHUP;
    event.data.fd = fd;

    if (::epoll_ctl(this->_fd, EPOLL_CTL_MOD, fd, &event) != 0)
    {
        if ((errno != ENOENT) || (::epoll_ctl(this->_fd, EPOLL_CTL_ADD, fd, &event) != 0))
            throw std::system_error(sys::error(), "epoll: failed to set event");
    }
}

void chen::bsd::epoll::del(int fd)
{
    if ((::epoll_ctl(this->_fd, EPOLL_CTL_DEL, fd, nullptr) != 0) && (errno != ENOENT) && (errno != EBADF))
        throw std::system_error(sys::error(), "epoll: failed to delete event");
}

// poll
std::size_t chen::bsd::epoll::poll(std::vector<Data> &cache, std::size_t count, double timeout)
{
    if (!count)
        return 0;

    struct ::epoll_event events[count];
    int result = ::epoll_wait(this->_fd, events, static_cast<int>(count), timeout < 0 ? -1 : static_cast<int>(timeout * 1000));

    // poll next events
    if (result <= 0)
    {
        // EINTR maybe triggered by debugger, treat it as user request to stop
        if ((errno == EINTR) || !result)
            return 0;
        else
            throw std::system_error(sys::error(), "epoll: failed to poll event");
    }

    // check return data
    auto length = cache.size();

    for (std::size_t i = 0; i < static_cast<std::size_t>(result); ++i)
    {
        auto &event = events[i];

        if (event.data.fd == this->_ef)
        {
            // user request to stop
            ::eventfd_t dummy;
            ::eventfd_read(this->_ef, &dummy);
            return {};
        }
        else
        {
            if (i < length)
                cache[i] = Data(event.data.fd, this->event(event.events));
            else
                cache.emplace_back(Data(event.data.fd, this->event(event.events)));
        }
    }

    return static_cast<std::size_t>(result);
}

std::vector<chen::bsd::epoll::Data> chen::bsd::epoll::poll(std::size_t count, double timeout)
{
    std::vector<chen::bsd::epoll::Data> ret;
    this->poll(ret, count, timeout);
    return ret;
}

void chen::bsd::epoll::stop()
{
    // notify wake message via eventfd
    if (::eventfd_write(this->_ef, 1) != 0)
        throw std::system_error(sys::error(), "epoll: failed to wake the epoll");
}

// misc
chen::bsd::epoll::Event chen::bsd::epoll::event(unsigned events)
{
    if ((events & EPOLLRDHUP) || (events & EPOLLERR) || (events & EPOLLHUP))
        return Event::End;

    if (events & EPOLLIN)
        return Event::Read;
    else if (events & EPOLLOUT)
        return Event::Write;
    else
        throw std::runtime_error("epoll: unknown event detect");
}

#endif