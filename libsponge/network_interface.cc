#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

void NetworkInterface::send_helper() {
    uint32_t cnt = 0;
    for (std::pair<Address, InternetDatagram> iter : _dgram) {
        uint32_t tmp = iter.first.ipv4_numeric();
        if ((_mapStatus.find(tmp) != _mapStatus.end()) && _mapStatus[tmp] >= 0) {
            send_datagram(iter.second, iter.first);
            _dgram.erase(_dgram.begin() + cnt);
        }
        cnt += 1;
    }
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    EthernetFrame frame;
    if (_addrMap.find(next_hop_ip) != _addrMap.end()) {  // Directly send frame
        frame.header().type = EthernetHeader::TYPE_IPv4;
        frame.header().src = _ethernet_address;
        frame.header().dst = _addrMap[next_hop_ip];
        frame.payload() = dgram.serialize();
        _frames_out.push(frame);
    } else if (_mapStatus.find(next_hop_ip) == _mapStatus.end()) {  // Send ARP request
        ARPMessage arp;

        arp.opcode = ARPMessage::OPCODE_REQUEST;
        arp.sender_ethernet_address = _ethernet_address;
        arp.sender_ip_address = _ip_address.ipv4_numeric();
        arp.target_ethernet_address = EthernetAddress{0};  // Can also just remove this

        frame.payload() = arp.serialize();
        frame.header().type = EthernetHeader::TYPE_IPv4;
        frame.header().src = _ethernet_address;
        frame.header().dst = _addrMap[next_hop_ip];

        std::pair<Address, InternetDatagram> tmp = make_pair(next_hop, dgram);
        _dgram.push_back(tmp);
        _mapStatus[next_hop_ip] = -1;
        _frames_out.push(frame);

    } else if (_mapStatus[next_hop_ip] < 0 && _mapStatus.find(next_hop_ip) != _mapStatus.end()) {
        std::pair<Address, InternetDatagram> tmp = make_pair(next_hop, dgram);
        _dgram.push_back(tmp);
        _frames_out.push(frame);
    }
    return;
    // DUMMY_CODE(dgram, next_hop, next_hop_ip);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    optional<InternetDatagram> ret = nullopt;
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST)
        return ret;
    else if (frame.header().type == EthernetHeader::TYPE_IPv4 && frame.header().dst == _ethernet_address) {
        InternetDatagram dgram;
        auto state = dgram.parse(Buffer(frame.payload()));
        if (state == ParseResult::NoError) {
            ret = dgram;
        }
    } else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage arp;
        if (arp.parse(Buffer(frame.payload())) == ParseResult::NoError) {
            _addrMap[arp.sender_ip_address] = arp.sender_ethernet_address;
            _mapStatus[arp.sender_ip_address] = 0;
        }
        if (arp.opcode == ARPMessage::OPCODE_REQUEST && arp.target_ip_address == _ip_address.ipv4_numeric()) {
            arp.sender_ethernet_address = _ethernet_address;
            arp.sender_ip_address = _ip_address.ipv4_numeric();
            arp.opcode = ARPMessage::OPCODE_REPLY;
            arp.target_ethernet_address = arp.sender_ethernet_address;
            arp.target_ip_address = arp.sender_ip_address;
            EthernetFrame back;
            back.header().type = EthernetHeader::TYPE_ARP;
            back.header().src = _ethernet_address;
            back.header().dst = arp.target_ethernet_address;
            _frames_out.push(back);
        }
    }
    send_helper();
    // DUMMY_CODE(frame);
    return ret;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    // if (_mapStatus.size() == 0) {
    //    send_helper();
    //    return;
    //}
    map<uint32_t, int>::iterator iter;
    for (iter = _mapStatus.begin(); iter != _mapStatus.end(); iter++) {
        if (_mapStatus.size() == 0) {
            send_helper();
            return;
        }
        if (_mapStatus[iter->first] >= 0) {
            _mapStatus[iter->first] += ms_since_last_tick;
            if (_mapStatus[iter->first] >= MAX_CACHE_TIME) {
                _mapStatus.erase(iter->first);
                _addrMap.erase(iter->first);
            }
        } else if (_mapStatus[iter->first] < 0) {
            _mapStatus[iter->first] -= ms_since_last_tick;
            if (_mapStatus[iter->first] < -1 * MAX_WAITING_TIME)
                _mapStatus.erase(iter->first);
        }
    }
    send_helper();
}
