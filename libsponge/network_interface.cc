#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address)
    , _ip_address(ip_address)
    , outstanding_message_map{}
    , forwarding_table{}
    , unsent_datagram_map{} {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    
    auto forwardingTableEntity = forwarding_table.find(next_hop_ip);
    if (forwarding_table.end() != forwardingTableEntity) {
        EthernetFrame ethernetFrame;
        EthernetHeader ethernetHeader;

        ethernetHeader.src = _ethernet_address;
        ethernetHeader.dst = forwardingTableEntity->second.ethernet_address;
        ethernetHeader.type = EthernetHeader::TYPE_IPv4;

        ethernetFrame.payload() = dgram.serialize();
        ethernetFrame.header() = ethernetHeader;

        _frames_out.push(ethernetFrame);
        return;
    }

    auto internet_datagram = unsent_datagram_map.find(next_hop_ip);
    if (internet_datagram != unsent_datagram_map.end()) {
        internet_datagram->second.push(dgram);
    } else {
        queue<InternetDatagram> internet_datagram_queue;
        internet_datagram_queue.push(dgram);
        unsent_datagram_map.insert(std::pair<uint32_t, queue<InternetDatagram>>(next_hop_ip, internet_datagram_queue));
    }

    auto outstanding_message = outstanding_message_map.find(next_hop_ip);
    if (outstanding_message != outstanding_message_map.end()) {
        return;
    }

    outstanding_message_map.insert(std::pair<uint32_t, size_t>(next_hop_ip, 0));

    ARPMessage arp_message;
    arp_message.sender_ethernet_address = _ethernet_address;
    arp_message.sender_ip_address = _ip_address.ipv4_numeric();
    arp_message.opcode = arp_message.OPCODE_REQUEST;
    arp_message.target_ethernet_address = ETHERNET_TARGET_ADDRESS;
    arp_message.target_ip_address = next_hop_ip;

    EthernetHeader hdr;
    hdr.type = EthernetHeader::TYPE_ARP;
    hdr.dst = ETHERNET_BROADCAST;
    hdr.src = _ethernet_address;

    EthernetFrame frame;
    frame.header() = hdr;
    frame.payload() = arp_message.serialize();
    _frames_out.push(frame);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != ETHERNET_BROADCAST && frame.header().dst != _ethernet_address) {
        return {};
    }

    Buffer payload_single = frame.payload().concatenate();
    if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage arp_message;

        if (arp_message.parse(payload_single) != ParseResult::NoError ||
            arp_message.target_ip_address != _ip_address.ipv4_numeric()) {
            return {};
        }
        ForwardingTableEntity forwarding_table_entity = {arp_message.sender_ethernet_address, 0};
        forwarding_table.insert(
            std::pair<uint32_t, ForwardingTableEntity>(arp_message.sender_ip_address, forwarding_table_entity));

        auto unsent_datagram = unsent_datagram_map.find(arp_message.sender_ip_address);
        if (unsent_datagram != unsent_datagram_map.end()) {
            while (!unsent_datagram->second.empty()) {
                send_datagram(unsent_datagram->second.front(), Address::from_ipv4_numeric(unsent_datagram->first));
                unsent_datagram->second.pop();
            }
        }

        if (arp_message.opcode != arp_message.OPCODE_REQUEST) {
            outstanding_message_map.erase(arp_message.sender_ip_address);
        } else {
            ARPMessage reply_arp_message;
            reply_arp_message.opcode = reply_arp_message.OPCODE_REPLY;
            reply_arp_message.sender_ethernet_address = _ethernet_address;
            reply_arp_message.sender_ip_address = _ip_address.ipv4_numeric();
            reply_arp_message.target_ethernet_address = arp_message.sender_ethernet_address;
            reply_arp_message.target_ip_address = arp_message.sender_ip_address;

            EthernetHeader reply_ethernet_header;
            reply_ethernet_header.dst = arp_message.sender_ethernet_address;
            reply_ethernet_header.type = EthernetHeader::TYPE_ARP;
            reply_ethernet_header.src = _ethernet_address;

            EthernetFrame reply_ethernet_frame;
            reply_ethernet_frame.header() = reply_ethernet_header;
            reply_ethernet_frame.payload() = reply_arp_message.serialize();

            _frames_out.push(reply_ethernet_frame);
        }
    } else if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram internet_datagram;
        if (internet_datagram.parse(payload_single) != ParseResult::NoError) {
            return {};
        }

        return internet_datagram;
    }
    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    for (auto it = outstanding_message_map.begin(); it != outstanding_message_map.end();) {
        if (it->second + ms_since_last_tick <= 5000) {
            it->second += ms_since_last_tick;
            it++;
        } else {
            it = outstanding_message_map.erase(it);
        }
    }

    for (auto it = forwarding_table.begin(); it != forwarding_table.end();) {
        if (it->second.time_elapsed + ms_since_last_tick <= 30000) {
            it->second.time_elapsed += ms_since_last_tick;
            it++;
        } else {
            it = forwarding_table.erase(it);
        }
    }
}
