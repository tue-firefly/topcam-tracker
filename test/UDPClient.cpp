#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include "../src/DroneState.h"
#include "../src/UDPClient.h"

using namespace std;

BOOST_AUTO_TEST_CASE(udp_send_state)
{
    DroneState state;
    state.id = 1;
    state.pos.x = 2;
    state.pos.y = 3;
    state.psi = -40000;
    UDPClient client("127.0.0.1", "6000");
    client.send_state(state);
    cout << "Sent!\n";
    
}
