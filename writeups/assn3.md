Assignment 3 Writeup
=============

## My name: Vansh Gupta

## My POVIS ID: vgspec

## My student ID (numeric): 49003814

### This assignment took me about 10 hours to do (including the time on studying, designing, and writing the code).

### If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

### - **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

## Program Structure and Design of the TCPSender:
For this assignment, the problem statement acted as my guide throughout the whole thing as I found myself referring to it while covering cases.

First, I knew I had to add a class for the Alarm (The worksheet also hinted towards this) and added bunch of functions that may or may not be useful in the future. Then started off by defining some pvt vars that I thought may prove useful in the future (and the ones that .cc file requested in some function)


Next, I started with .cc implementation and saved fill_window and ack_received for the end as they visibly required lot more effort and thinking. 
After that, I divided problem into subproblems (as I normally do using a pen and paper) and carefully read the requirements from the worksheet to formulate different parts of the function. Like setting sequence number, length of data to be read, cases of when sendind syn, setting fin, non-zero length condition, window size increment to 1 if 0 etc.

### Implementation Challenges:
This time I faced a huge issue with the debugging. Most of it was syntax error, but since the file size was big and I use nano, navigating to the problem, then to other file, then running make and checking if it is fixed or not took a lot of time. After that, I kept missing some or the other condition in my if statements which took a lot of time too

At times, I found myself wondering if I should make code more modular by introducing sub-functions or not. But since they were not being called often, I stuck with defining everything on the go

### Remaining Bugs:
There is definitely a bug that still remains. All of my t_send test cases are failing because of timeout. So it is probably an inf loop but I cannot figure out where it is going wrong. As I have a midterm tomorrow that I have not started preparing for, I shall only look into this for another 30 mins before calling it in and submitting.  
But I can see a lot of scope for errors. Perhaps if the test cases were more comprehensive, my code would give up. But it just might work as I implemented it after considering a lot of things and spending a lot of time.

#### - Optional: I had unexpected difficulty with: [describe]

#### - Optional: I think you could make this assignment better by: [describe]

#### - Optional: I was surprised by: [describe]

#### - Optional: I'm not sure about: [describe]
