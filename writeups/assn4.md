Assignment 4 Writeup
=============

My name: Vansh Gupta

My POVIS ID: vgspec

My student ID (numeric): 49003814

This assignment took me about 11 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): [3]

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): I could not run all test cases [0.00, 0.00]

Program Structure and Design of the TCPConnection:

The biggest ease of this assignment that I realized quite later was the FAQs. They gave me a clear structure as to how to go around attempting the assignment. Since all the data structure part was already done in previous assignment, I did not sweat over it much and just followed the steps listed in the assignment document. After figuring out what all private members would be required for .hh by creating a high level design, I moved on to .cc file where I wrote the functions in the order suggested in the document. Then, I do what I always do, took a pen and paper and made multiple cases for longer functions and tried to see if it could be modularised. I made mistake with shutdowns as I somehow missed that part of doc, otherwise I would have modularised that as well based on all the pre-reqs

Implementation Challenges:

The biggest implementation challenge was to memorise all the classes and functions for all of the previous assignments. I worked on this assignment over 4-5 days and every time I had to write a function, I would have to go through all of my previous assignment submissions and the documentation. Secondly, I felt that because there was a chance that my prev submission had a mistake and not this one, I was not sure how much and what to fix. I did not understand the debugging logs and could not get the wireshark setup running as I am still quite new to programming like this

Remaining Bugs:

There are quite a few bugs remaining that I don not quite understand myself. There is a timeout issue and there is a segment does not exist error. In several of my commits, I was facing TCPHeader too short exception as well. I feel I made a change in some file that was causing this, and if I were able to investigate it properly and mitigate it, many many more test cases would have passed

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
