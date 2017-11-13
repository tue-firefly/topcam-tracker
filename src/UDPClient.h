#ifndef UDP_CLIENT
#define UDP_CLIENT

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "DroneState.h"
#include <cstdint>

using boost::asio::ip::udp;

class UDPClient
{
public:
	UDPClient(
		const std::string& host, 
		const std::string& port
	) : io_service_(io_service_udp), socket_(io_service_udp, udp::endpoint(udp::v4(), 0)) {
		udp::resolver resolver(io_service_);
		udp::resolver::query query(udp::v4(), host, port);
		udp::resolver::iterator iter = resolver.resolve(query);
		endpoint_ = *iter;
	}

	~UDPClient()
	{
		socket_.close();
	}

<<<<<<< Updated upstream
	void send(const boost::array<double, 3>& msg) {
		socket_.send_to(boost::asio::buffer(msg), endpoint_);
=======
	void send(boost::asio::mutable_buffers_1 msg) {
		socket_.send_to(msg, endpoint_);
	}
	
	boost::asio::mutable_buffers_1 serialize(DroneState state) {
		FlatState* data = new FlatState;
		data->id = state.id;
		data->x = state.pos.x;
		data->y = state.pos.y;
		data->psi = state.psi;
		return boost::asio::buffer((void *) data, sizeof(FlatState));
>>>>>>> Stashed changes
	}
	
	void send_state(DroneState state) {
		boost::asio::mutable_buffers_1 buf = serialize(state);
        send(buf);
		free(boost::asio::buffer_cast<void *>(buf));
    }

private:
	boost::asio::io_service io_service_udp;
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint endpoint_;
<<<<<<< Updated upstream
};
=======
	
	struct __attribute__ ((packed)) FlatState {
		uint32_t id;
		double x;
		double y;
		double psi;
	};
};

#endif
>>>>>>> Stashed changes
