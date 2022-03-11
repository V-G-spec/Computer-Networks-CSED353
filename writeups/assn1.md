Assignment 1 Writeup
=============

My name: Vansh Gupta

My POVIS ID: vgspec

My student ID (numeric): 49003814

This assignment took me about 5 hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:  
## Push Substring
This function takes the arguments data of type string, index of type size_t, and eof, a boolean value. My implementation, after ignoring the invalid case, follows the guidelines in the assignment, and infers that they can be divided in 2 cases. These cases are wrt to the gap/offset and whether buffer stream will be ahead or lagging.  
Both the cases have similar templates where we first figure out how many bytes will be read, and then store them in the buffer stream while checking and updating the trackmap.  
Note: The trackmap is a list that keeps track of which bytes have been filled and which haven't. This acts like a bitmap and helps when we are writing data to the output stream.  
Once both the cases are independently handled, I call the function defragment whose job is to correspond bits from the trackmap and accordingly add them to the output stream.

Implementation Challenges:  
First challenge that I faced was finding the write DS for my buffer and trackmap. One that allowed random access and push/pop functions to some extent
Next big challenge was eof. I am not sure how to make a global variable to tell me at what index eof was true. I will now try to alter my trackmap from a list of boolean to list of size_t, which stores 2 upon reaching eof. I may not have sufficient time to update that progress here.  
In the beginning, the cases were ovewhelming, which is when I decided to resort to using pen and paper and took some examples and accordingly divided the problems in simpler sub problems. A function that makes the least assumptions on inputs mitigates the need to proof for much extent.  
I mostly tested my code using the given tests as I am not sure how to run my own. For debugging, I took help from stackoverflow to understand what that error meant and then read my code several times to make required changes. Other than the eof error, I only ran into syntax errors which are relatively easier to solve

Remaining Bugs:  
As mentioned in most recent commit, some of the eof are not working. Looking at the error logs, it seems my code is unable to handle the case where eof was 1 at one of the earlier inputs.  

- Optional: I had unexpected difficulty with: merging updates with pvt repo

- Optional: I think you could make this assignment better by: Providing a nice way of running our own test cases and checking out the results.

- Optional: I was surprised by: How complicated abstraction can be if you do not know the exact functioning

- Optional: I'm not sure about: Whether the few test cases that failed failed because of the reason I have in mind (and stated). But I will certainly look into this if I get some time today.
