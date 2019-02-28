#ifndef LIBRDMA_ASIO_LIBRARY_H
#define LIBRDMA_ASIO_LIBRARY_H

#include "librdmacmcpp.h"

#include <boost/asio.hpp>

// Based on: https://lists.boost.org/boost-users/2015/03/83928.php

namespace rdma {
    namespace asio {
        typedef boost::asio::ip::udp::socket socket_type;

        class EventChannel : public boost::asio::io_service::service {
            socket_type socket;
            std::unique_ptr<rdma::event::Channel> event_channel;
        public:
            EventChannel(boost::asio::io_service& io_service) :
                event_channel(rdma::createEventChannel()),
                boost::asio::io_service::service(io_service),
                socket(io_service)
            {
                socket.assign(boost::asio::ip::udp::v4(),
                        event_channel->getFD());
            }

            [[nodiscard]]
            std::unique_ptr<rdma::ID> createID(void *context, PortSpace ps)
            { return event_channel->createID(context, ps); }

            template <class Handler>
            void async_wait(Handler handler)
            {
                socket.async_receive(boost::asio::null_buffers(), 0,
                    [this, handler] (const boost::system::error_code& ec, size_t) {
                        handler(ec);
                    });
            }

            template <class Handler>
            void async_get_event(Handler handler)
            {
                async_wait([this, handler] (const boost::system::error_code& ec) {
                    handler(ec, event_channel->getEvent());
                });
            }

            void shutdown_service()
            {
                event_channel.reset();
            }
        };
    }
}

#endif
