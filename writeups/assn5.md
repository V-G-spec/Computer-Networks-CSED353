Assignment 5 Writeup
=============

My name: Vansh Gupta

My POVIS ID: vgspec

My student ID (numeric): 49003814

This assignment took me about 7 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): [3,4]

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
The program structure is pretty much same as defined in the PDF of assignment 5, and what I learnt from the links provided for reference. For send_datagram, I converted the IP address, and then divided in cases of whether or not I found IP mapping and accordingly sent ARP request. For recv_frame, I handled IPv4 and ARP separately and filtered frames whose dst is not the network interface. Finally, tick was straightforward. Expired any IP-to-Ethernet mappings that have expired

Implementation Challenges:
Main challenge was finding it hard to process a lot of new information such as classes, methods, implementation specifics etc.

Remaining Bugs:
I cannot know for sure because of the concerns raised by me on PLMS which were not answered in time. Right now 0 test cases are passing, even though I feel my implementation is more or less correct

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
