# cLoxLox
cLoxLox is a modern c++ implementation heavily reliant upon *jlox* from Part II of [Crafting Interpreters](https://www.craftinginterpreters.com/) by Robert Nystrom. This project was developed as an assignment for CS 403 at the University of Alabama during Fall of 2023. Throughout the development of cLL, I attempted to stay as true to the original implementation as I could. Since the orginal language of *jlox* was Java, it was a bit challenge for a novice programmer (as myself) to translate this into C++. I spent countless hours learning new C++ concepts (virtual, shared pointer, std::any, etc) and chasing bugs. For more information of my challenges, look under [Challenges](#challenges) or skip it if you don't want to hear me complain.

cLL has been tested on Windows 10 and 11 with Microsoft's VS Code. If you would like to test it on your own system, please follow the instructions found under the [Building](#building) header. If you have any issues, please email me at: hbrousseau@crimson.ua.edu

## Files and Reading the Code
I have submitted a zip file for the Lox interpreter partitioned for each chapter that I have worked through (up until chapter 13, which is the final piece of the Lox puzzle). I attempted to comment throughout all of the chapters what the code was attempting to do and when/where I was finding issues. Up until chapter 13, I ran tests through a REPL to verify the output of each section.

## Building
Clone the repository to a location most convenient to you.
```
git clone https://github.com/hbrousseau/cLoxLox.git
```

Navigate inside ch13Checkpoint to build the entire project.
```
cd ch13Checkpoint
```

To compile the entire interpreter, run `make`

To compile GenerateAST.cpp, run
```
g++ GenerateAST.cpp -o GenerateAST
```

To run GenerateAST (Windows specific),
```
./GenerateAST.exe "directory for the generated files"
```

I found test cases and a Makefile for a c++ implementation of Lox [here](https://github.com/the-lambda-way/CppLox/tree/c9d65280108cfd2ce7e42a9d2a7fc09c95c21296/chapter13)

This individual seemed to have grabbed the functions the author used throughout the textbook to discuss each chapter's goal. 

Test cases can be run individually by `make test-name` or run all at once by `make test-all`

## Usage
cLL has two usages: either as a REPL (read, print, eval loop) or, given a source file, cLL will attempt to execute the code and exit the program. To run the program as a REPL:
```
./cLL.exe
```

Otherwise, to execute a Lox source file:
```
./cLL.exe [Lox script]
```

## Testing
Testing, as described in [Building](#building), can be run simultaneously or individually. The test cases run through the cruxes of each chapter, mainly using code provided by the textbook. I will say that I am not very familiar with testing suites; however, all of the output statements were printed as follows:
```
testing cLL with test-statements.lox ...
--- tests/test-statements.lox.expected  2023-11-18 07:15:44.270884600 -0600
+++ -   2023-11-29 14:08:51.869431200 -0600
@@ -1,3 +1,3 @@
-one
-true
-3.000000
+one
+true
+3.000000
make[1]: *** [Makefile:89: test-statements] Error 1
```
I understand that it says there are differences between the expected and my output. However, if I were to run the REPL with the same test case, my output would look as expected. Do with this information as you will. 

**Note:** I have recently updated the ch13Checkpoint folder that contains a test_output.txt that depicts every test, its expected output, and its actual output. The snippet seen above is how it was printed within the output file, sectioned per test case. I will admit I don't understand why I have a difference in output and had tried printing out the hexadecimal data to attempt to find the disparity. With my naive eyes, I couldn't find anything. It might be something small that I have missed time and time again. 

**Note2:** If you would like to generate your own output text, I would either make a copy of the test_output.txt file and delete the contents so the output would read directly inside that file. Or, you can modify the Makefile at the OUTPUT_FILE variable to save it inside a different folder. If you end up modifying the Makefile, you could also just change the filename. 

On line 32, you could modify the OUTPUT_FILE variable as this to change the text file's name:
```
OUTPUT_FILE := YessickOutput.txt
```

Or inside a different folder:
```
OUTPUT_FILE := testFolderName/test_output.txt
```

## Challenges 
Translating Java to C++ provided many interesting challenges for me and broadened my C++ horizons. I had used virtuals possibly once before, but never to this extent. I also had no knowledge of any container or shared pointers, and there was a learning curve for me to wrap my head around the shared pointers. 

Beginning in chapter 8, I encountered some frustrating bugs, reflected in my code's comments. From then on, I decided it was best for me to make notes where code had either been updated or added to pre-existing code for more efficient debugging. The thought was that since I tested something in the last chapter, it should be pretty okay in the next if something was really broken. However, as seen in chapter 12, that wasn't necessarily the case. In chapter 12, I had to completely restructure my Interpreter.hpp file to follow the structure the book had laid out for me. I didn't notice that the author gave us where exactly to order our code. I was already in too deep when I saw this and thought it wouldn't matter. But, using good old C++, I should have known better. Numerous days of debugging led to this discovery. Also, I prefer using <unordered_map>s in C++; however, the compiler on my laptop was none-too-thrilled about it and resulted in strange compiler errors. So, ultimately, I changed to using <map> instead. I will fully acknowledge it could have been a user error in that instance. However, it worked fine in the previous chapters, so I'm going to say my compiler was just tired of my error-prone business. 


