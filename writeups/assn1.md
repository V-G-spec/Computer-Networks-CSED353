Assignment 1 Writeup
=============

My name: Vansh Gupta

My POVIS ID: vgspec

My student ID (numeric): 49003814

This assignment took me about 5 hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:


Implementation Challenges:
First challenge that I faced was finding the write DS for my buffer and trackmap. One that allowed random access and push/pop functions to some extent
Next big challenge was eof. I am not sure how to make a global variable to tell me at what index eof was true

Remaining Bugs:
As mentioned in most recent commit, some of the eof are not working. Looking at the error logs, it seems my code is unable to handle the case where eof was 1 at one of the earlier inputs.

- Optional: I had unexpected difficulty with: merging updates with pvt repo

- Optional: I think you could make this assignment better by: -

- Optional: I was surprised by: How complicated abstraction can be if you do not know the exact functioning

- Optional: I'm not sure about: Whether the few test cases that failed failed because of the reason I have in mind (and stated). But I will certainly look into this if I get some time today.
