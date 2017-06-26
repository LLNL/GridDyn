#ifndef GRIDDYNSERVER_HEADER_
#define GRIDDYNSERVER_HEADER_

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>


//class for creating a udp socket for data transmission
class pmu_udp_socket
{
public:
	int index;
	//PMU not allowed to have multiple packets in flight at the same time
	boost::mutex send_lock;
	boost::asio::ip::udp::socket socket_;
	pmu_udp_socket(boost::asio::io_service &ios, boost::asio::ip::udp::endpoint ep)
		:socket_(ios, ep) {
		index = 0;
	};
	void data_sent(const boost::system::error_code &error, std::size_t size)
	{
		send_lock.unlock();
	};
};

//class for establishing a TCP connection

class pmu_tcp_acc
{
public:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
	pmu_tcp_acc(boost::asio::io_service &ios, boost::asio::ip::tcp::endpoint ep)
		:io_service_(ios), acceptor_(ios, ep) {};
};

class pmu_tcp_session
{
public:
	int index;
	enum session_state_t { accepting = 0, waiting = 1, sending = 2, halted = 3 };
	session_state_t cstate;
	boost::asio::ip::tcp::socket socket_;
	//PMU not allowed to have multiple packets in flight at the same time
	boost::try_mutex send_lock;

	std::vector<unsigned char> recv_buffer_;
	pmu_tcp_session(boost::asio::io_service &ios)
		:socket_(ios) {
		index = 0;
		recv_buffer_.resize(65536); cstate = accepting;
	};
	void data_sent(const boost::system::error_code &error, std::size_t size)
	{
		send_lock.unlock();
	};
};

//class for creating a virtual PMU server
class gridDynServer
{

public:
	enum ip_protocol_t { tcp = 1, udp = 2 };

	int port;
	ip_protocol_t ip_protocol;

	int timeBase;



protected:
	int vid;
	int current_connections; //the current number of connected sockets
	int max_connections;  //the maximum number of transmit sockets
						  //timing information

	double intervalError;

	//PMU unit itself


	//thread for the send data loop
	boost::thread send_thread;



	//network connection information
	pmu_udp_socket *udpsock;
	pmu_tcp_acc *tcpacc;

	//boost::asio::io_service ioserve;
	boost::asio::ip::udp::endpoint remote_endpoint_udp;
	boost::asio::ip::udp::endpoint remote_endpoint_udp_send;
	boost::asio::ip::udp::endpoint local_endpoint_udp;

	boost::asio::ip::tcp::endpoint local_endpoint_tcp;
	boost::mutex session_lock;
	std::vector<pmu_tcp_session *> active_tcp_sessions;

	//command frame buffer
	std::vector<unsigned char> recv_buffer_;

	//frame buffers for send data
	std::vector<unsigned char> dataFrame;
	std::vector<unsigned char> header;
	std::vector<unsigned char> cfg1;
	std::vector<unsigned char> cfg2;




public:
	gridDynServer();
	~gridDynServer();

	//threaded loop for transmitting data
	virtual void send_data();

	// function for responding to UDP requests
	virtual void pmu_udp(const boost::system::error_code &error, std::size_t size);
	//function for responding to tcp requests
	virtual void pmu_tcp(pmu_tcp_session *session, const boost::system::error_code &error, std::size_t size);
	//function for accepting TCP connection requests
	virtual void tcp_accept(pmu_tcp_session *session, const boost::system::error_code &error);
	// hook for executing alternate command and control functions
	virtual void command_loop() {};

	//function for starting the PMU server
	virtual void start_server(boost::asio::io_service &ios);
	// halt the server
	virtual void stop_server();
	//initialize the server
	virtual void initialize();
	//returns the PMU id
	virtual int id() { return 0; };
	//helper function to set various parameters
	virtual void set(std::string param, int val);
	virtual void set(std::string param, std::string val) { return; };

};

#endif

