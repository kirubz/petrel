/*
 * Copyright (c) 2016 Andreas Pohl
 * Licensed under MIT (see COPYING)
 *
 * Author: Andreas Pohl
 */

#include "http2_client.h"
#include "make_unique.h"

#include <boost/asio.hpp>
#include <boost/fiber/all.hpp>
#include <sstream>

namespace petrel {
namespace lib {

DECLARE_LIB_BEGIN(http2_client);
ADD_LIB_METHOD(connect);
ADD_LIB_METHOD(disconnect);
ADD_LIB_METHOD(get);
ADD_LIB_METHOD(read_timeout);
DECLARE_LIB_BUILTIN_END();

namespace bs = boost::system;
namespace bf = boost::fibers;

int http2_client::connect(lua_State* L) {
    m_host = luaL_checkstring(L, 1);
    if (lua_isstring(L, 2)) {
        m_port = lua_tostring(L, 2);
    } else {
        m_port = "80";
    }
    if (lua_isboolean(L, 3)) {
        m_ssl = lua_toboolean(L, 3);
    }
    if (lua_isboolean(L, 4)) {
        m_ssl_verify = lua_toboolean(L, 4);
    }

    if (m_ssl) {
        m_tls = std::make_unique<ba::ssl::context>(ba::ssl::context::sslv23);
        m_tls->set_default_verify_paths();
        if (m_ssl_verify) {
            m_tls->set_verify_mode(ba::ssl::verify_peer);
        }
        bs::error_code ec;
        http2::client::configure_tls_context(ec, *m_tls);
        if (ec) {
            luaL_error(L, "configure_tls_context failed: %s", ec.message().c_str());
        }
    }

    bf::promise<bs::error_code> promise;
    bf::future<bs::error_code> future(promise.get_future());

    // TODO: use resolver_cache (extend the nghttp2 client interface, so we can pass in an endpoint iterator)
    if (m_ssl) {
        m_session = std::make_unique<http2::client::session>(io_service(), *m_tls, m_host, m_port);
    } else {
        m_session = std::make_unique<http2::client::session>(io_service(), m_host, m_port);
    }
    m_session->read_timeout(m_read_timeout);
    m_session->on_connect([&promise](ba::ip::tcp::resolver::iterator) { promise.set_value(bs::error_code()); });
    m_session->on_error([&promise](const bs::error_code& ec) { promise.set_value(ec); });

    auto ec = future.get();
    if (ec) {
        luaL_error(L, "connect failed: %s", ec.message().c_str());
    }

    // remove error handler
    m_session->on_error([](const bs::error_code&) {});

    m_connected = true;
    return 0;
}

int http2_client::disconnect(lua_State*) {
    close();
    return 0;
}

int http2_client::get(lua_State* L) {
    auto* path = luaL_checkstring(L, 1);

    if (!m_connected) {
        luaL_error(L, "not connected");
    }

    bf::promise<bs::error_code> promise;
    bf::future<bs::error_code> future(promise.get_future());
    m_session->on_error([&promise](const bs::error_code& ec) { promise.set_value(ec); });

    std::string url = "http://";
    url += m_host;
    url += ":";
    url += m_port;
    url += path;

    bs::error_code ec;
    std::ostringstream content_strm;
    int status;
    auto req = m_session->submit(ec, "GET", url);
    req->on_response([&promise, &content_strm, &status](const http2::client::response& res) {
        status = res.status_code();
        res.on_data([&promise, &content_strm](const uint8_t* data, std::size_t len) {
            if (len == 0) {
                promise.set_value(bs::error_code());
            } else {
                content_strm.write(reinterpret_cast<const char*>(data), len);
            }
        });
    });

    ec = future.get();
    if (ec) {
        luaL_error(L, "get failed: %s", ec.message().c_str());
    }

    // remove error handler
    m_session->on_error([](const bs::error_code&) {});

    lua_pushinteger(L, status);
    lua_pushstring(L, content_strm.str().c_str());

    return 2;
}

int http2_client::read_timeout(lua_State* L) {
    int t = luaL_checkinteger(L, 1);
    m_read_timeout = bp::millisec(t);
    return 0;
}

}  // lib
}  // petrel
