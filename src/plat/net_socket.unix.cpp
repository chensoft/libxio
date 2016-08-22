/**
 * Created by Jian Chen
 * @since  2016.08.14
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifndef _WIN32

#include <socket/net/net_socket.hpp>
#include <socket/net/net_error.hpp>
#include <chen/base/num.hpp>
#include <chen/sys/sys.hpp>
#include <cstring>

// -----------------------------------------------------------------------------
// helper
namespace
{
//    int af(chen::net::socket::Family family)
//    {
//        switch (family)
//        {
//            case chen::net::socket::Family::IPv4:
//                return AF_INET;
//
//            case chen::net::socket::Family::IPv6:
//                return AF_INET6;
//
//            case chen::net::socket::Family::Unix:
//                return AF_UNIX;
//        }
//
//        return 0;
//    }
//
//    int type(chen::net::socket::Protocol protocol)
//    {
//        switch (protocol)
//        {
//            case chen::net::socket::Protocol::TCP:
//                return SOCK_STREAM;
//
//            case chen::net::socket::Protocol::UDP:
//                return SOCK_DGRAM;
//
//            case chen::net::socket::Protocol::RAW:
//                return SOCK_RAW;
//        }
//
//        return 0;
//    }

    struct sockaddr_storage addr(const chen::net::address &a, std::uint16_t p, socklen_t &l)
    {
        struct sockaddr_storage ret{};

        switch (a.type())
        {
            case chen::net::address::Type::IPv4:
            {
                auto &v4 = a.v4();
                auto  in = (struct sockaddr_in*)&ret;

                in->sin_family      = AF_INET;
                in->sin_port        = chen::num::swap(p);
                in->sin_addr.s_addr = chen::num::swap(v4.addr());

                l = sizeof(struct sockaddr_in);

                break;
            }

            case chen::net::address::Type::IPv6:
            {
                auto &v6 = a.v6();
                auto  in = (struct sockaddr_in6*)&ret;

                in->sin6_family   = AF_INET6;
                in->sin6_port     = chen::num::swap(p);
                in->sin6_scope_id = v6.scope();

                ::memcpy(in->sin6_addr.s6_addr, v6.addr().data(), 16);

                l = sizeof(struct sockaddr_in6);

                break;
            }

            default:
                break;
        }

        return ret;
    }

    chen::net::endpoint addr(const struct sockaddr *sock)
    {
        switch (sock->sa_family)
        {
            case AF_INET:
            {
                auto in = (struct sockaddr_in*)sock;
                return chen::net::endpoint(chen::net::address(chen::num::swap(in->sin_addr.s_addr)),
                                           chen::num::swap(in->sin_port));
            }

            case AF_INET6:
            {
                auto in = (struct sockaddr_in6*)sock;
                return chen::net::endpoint(chen::net::address(chen::net::version6(in->sin6_addr.s6_addr, 128, in->sin6_scope_id)),
                                           chen::num::swap(in->sin6_port));
            }

            default:
                return nullptr;
        }
    }

    chen::net::endpoint addr(const struct sockaddr_storage *sock)
    {
        return addr((const struct sockaddr*)sock);
    }
}


// -----------------------------------------------------------------------------
// socket
chen::net::socket::socket() : socket(nullptr)
{
}

chen::net::socket::socket(std::nullptr_t) : _fd(0)
{
}

chen::net::socket::socket(socket_t fd) : _fd(fd)
{
}

chen::net::socket::socket(int family, int type, int protocol) : _fd(0)
{
    this->reset(family, type, protocol);
}

chen::net::socket::socket(socket &&o)
{
    *this = std::move(o);
}

chen::net::socket& chen::net::socket::operator=(socket &&o)
{
    if (this == &o)
        return *this;

    this->_fd = o._fd;
    o._fd     = 0;

    return *this;
}

chen::net::socket::~socket()
{
    this->close();
}

// reset
void chen::net::socket::reset(socket_t fd)
{
    if (this->_fd && !this->close())
        throw error_socket("socket: " + sys::error());

    this->_fd = fd;
}

void chen::net::socket::reset(int family, int type, int protocol)
{
    if (this->_fd && !this->close())
        throw error_socket("socket: " + sys::error());

    auto fd = ::socket(family, type, protocol);
    if (fd < 0)
        throw error_socket("socket: " + sys::error());

    this->_fd = fd;
}

// connection
bool chen::net::socket::connect(const endpoint &ep) noexcept
{
    return this->connect(ep.addr(), ep.port());
}

bool chen::net::socket::connect(const address &addr, std::uint16_t port) noexcept
{
    socklen_t len = 0;
    auto in = ::addr(addr, port, len);
    return !::connect(this->_fd, (struct sockaddr *)&in, len);
}

bool chen::net::socket::bind(const endpoint &ep) noexcept
{
    return this->bind(ep.addr(), ep.port());
}

bool chen::net::socket::bind(const address &addr, std::uint16_t port) noexcept
{
    socklen_t len = 0;
    auto in = ::addr(addr, port, len);
    return !::bind(this->_fd, (struct sockaddr *)&in, len);
}

bool chen::net::socket::listen() noexcept
{
    return this->listen(SOMAXCONN);
}

bool chen::net::socket::listen(int backlog) noexcept
{
    return !::listen(this->_fd, backlog);
}

chen::net::socket chen::net::socket::accept()
{
    struct sockaddr_storage in{};
    socklen_t len = 0;

    int fd = ::accept(this->_fd, (struct sockaddr*)&in, &len);
    return fd >= 0 ? socket(fd) : nullptr;
}

// data
ssize_t chen::net::socket::send(const void *data, std::size_t size, int flags) noexcept
{
    return ::send(this->_fd, data, size, flags);
}

ssize_t chen::net::socket::send(const std::vector<std::uint8_t> &data, int flags) noexcept
{
    return this->send(data.data(), data.size(), flags);
}

ssize_t chen::net::socket::send(const void *data, std::size_t size, int flags, const endpoint &ep) noexcept
{
    socklen_t len = 0;
    auto in  = ::addr(ep.addr(), ep.port(), len);
    return ::sendto(this->_fd, data, size, flags, (struct sockaddr*)&in, len);
}

ssize_t chen::net::socket::send(const std::vector<std::uint8_t> &data, int flags, const endpoint &ep) noexcept
{
    return this->send(data.data(), data.size(), flags, ep);
}

ssize_t chen::net::socket::recv(std::vector<std::uint8_t> &out, std::size_t size, int flags) noexcept
{
    return ::recv(this->_fd, out.data(), size, flags);
}

std::vector<std::uint8_t> chen::net::socket::recv(std::size_t size, int flags) noexcept
{
    std::vector<std::uint8_t> ret;
    this->recv(ret, size, flags);
    return ret;
}

ssize_t chen::net::socket::recv(std::vector<std::uint8_t> &out, std::size_t size, int flags, endpoint &ep) noexcept
{
    struct sockaddr_storage in{};
    socklen_t len = 0;

    auto ret = ::recvfrom(this->_fd, out.data(), size, flags, (struct sockaddr*)&in, &len);
    if (ret >= 0)
        ep = ::addr(&in);

    return ret;
}

std::vector<std::uint8_t> chen::net::socket::recv(std::size_t size, int flags, endpoint &ep) noexcept
{
    std::vector<std::uint8_t> ret;
    this->recv(ret, size, flags, ep);
    return ret;
}

// error
std::error_code chen::net::socket::error() const noexcept
{
    if (!this->_fd)
        return std::error_code(0, std::system_category());

    int val = 0;
    socklen_t len = 0;

    if (!::getsockopt(this->_fd, SOL_SOCKET, SO_ERROR, &val, &len))
        return std::error_code(val, std::system_category());
    else
        return std::error_code(errno, std::system_category());
}

// close
bool chen::net::socket::close() noexcept
{
    // treat closed as true
    if (!this->_fd)
        return true;

    // close the socket
    if (::close(this->_fd))
        return false;

    this->_fd = 0;
    return true;
}

bool chen::net::socket::shutdown(Shutdown flag) noexcept
{
    // treat closed as true
    if (!this->_fd)
        return true;

    // shutdown the socket
    switch (flag)
    {
        case Shutdown::Read:
            return !::shutdown(this->_fd, SHUT_RD);

        case Shutdown::Write:
            return !::shutdown(this->_fd, SHUT_WR);

        case Shutdown::Both:
            return !::shutdown(this->_fd, SHUT_RDWR);
    }

    return true;
}

// info
chen::net::endpoint chen::net::socket::local() const noexcept
{
    if (!this->_fd)
        return nullptr;

    struct sockaddr_storage in{};
    socklen_t len = sizeof(in);

    if (::getsockname(this->_fd, (struct sockaddr*)&in, &len) != 0)
        return nullptr;
    else
        return ::addr(&in);
}

chen::net::endpoint chen::net::socket::remote() const noexcept
{
    if (!this->_fd)
        return nullptr;

    struct sockaddr_storage in{};
    socklen_t len = sizeof(in);

    if (::getpeername(this->_fd, (struct sockaddr*)&in, &len) != 0)
        return nullptr;
    else
        return ::addr(&in);
}

// empty
bool chen::net::socket::empty() const noexcept
{
    return !this->_fd;
}

chen::net::socket::operator bool() const noexcept
{
    return !this->empty();
}

// resolve
std::vector<chen::net::address> chen::net::socket::resolve(const std::string &host) noexcept
{
    struct addrinfo *info = nullptr;
    struct addrinfo hint{};

    hint.ai_family   = AF_UNSPEC;    // IPv4 or IPv6
    hint.ai_socktype = SOCK_STREAM;  // prevent return same addresses

    if (::getaddrinfo(host.c_str(), nullptr, &hint, &info))
        return {};

    std::vector<chen::net::address> ret;

    for (struct addrinfo *ptr = info; ptr != nullptr; ptr = ptr->ai_next)
        ret.emplace_back(addr(ptr->ai_addr).addr());

    ::freeaddrinfo(info);

    return ret;
}

#endif