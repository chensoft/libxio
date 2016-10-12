/**
 * Created by Jian Chen
 * @since  2016.09.19
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/net/net_socket.hpp>

// -----------------------------------------------------------------------------
// socket
chen::net::socket::socket(bsd::socket &&s) : _handle(std::move(s))
{
}

chen::net::socket::socket(ip::address::Type family, int type) : _handle(static_cast<int>(family), type)
{
}

chen::net::socket::~socket()
{
}

// property
chen::net::endpoint chen::net::socket::sock() const
{
    return this->_handle.sock();
}

chen::net::endpoint chen::net::socket::peer() const
{
    return this->_handle.peer();
}

std::error_code chen::net::socket::nonblocking(bool enable)
{
    return this->_handle.nonblocking(enable);
}

bool chen::net::socket::valid() const
{
    return this->_handle.valid();
}

chen::net::socket::operator bool() const
{
    return this->_handle;
}

int chen::net::socket::family() const
{
    return this->_handle.family();
}

int chen::net::socket::type() const
{
    return this->_handle.type();
}

int chen::net::socket::protocol() const
{
    return this->_handle.protocol();
}

// notify
chen::bsd::socket& chen::net::socket::handle()
{
    return this->_handle;
}

void chen::net::socket::onAccept(bsd::socket s, net::endpoint ep)
{
    // override this method when you implement a server
}

void chen::net::socket::onRead(std::vector<std::uint8_t> data, net::endpoint ep, std::error_code error)
{
    // override this method when you want to read data from remote
}

void chen::net::socket::onWrite(std::size_t size, net::endpoint ep, std::error_code error)
{
    // override this method when you want to know the write status
}

void chen::net::socket::onEnd()
{
    // override this method when you want to deal with connection refused, disconnect or other error
}