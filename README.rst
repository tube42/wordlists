
WORD LIST
=========

This project aims to create a set of free to use word lists for use in open
source games.

Starting from a number of open word repositories (or even a dictionaries in some
cases), we add, remove and filter entries to create a list of suitable words.
These words are then stored in a format that makes searching simple and fast.

License
-------

This project itself is GPLv2. But some words come from sources that may have their own licenses that they will retain.

See the corresponding data/<language>.LICENSE file.


Building
--------

The word repositories together with configuration files are under data/.
Under src/ you will find some short programs to manage them:

To build all lists execute::

    make update

The results will be in the build/ directory.

To search the list for one of the languages execute:::

    make test LANGUAGE=en


To do this in Java::

    make javatest LANGUAGE=en


FAQ
---


* Q: I demand you add support for <my language>!!!
* A: This is an open source project. You can add it yourself.

* Q: These words are not good! They are unsuitable for my game! Fix them, or else!!
* A: This is an open source project and you have the chance to improve it by submitting your own changes. Note however that the main word list is meant to contain all possible words while some games may require certain words to be excluded (e.g. words ending with plural 's). In that case you should use our lists as a starting point and then filter our any words that are not suitable for your particular need.

* Q: How do I limit the word list to words of a certain length?
* A: Change the data/<lang>.config file to your liking and rebuild the list.

* Q: Why does the word list also contain language statistics?
* A: This information can be used to help create better randomly generated game levels.

* Q: What is the format of the produced word list?
* A: The build/<lang>.bin file is contains a sorted list of zero-terminated words plus a small header and footer. The sorted list of words allows use a quasi-binary search, which is a good compromise w.r.t. memory usage, speed and code complexity.


* Q: I don't understand the file format. Can you help me add it to my game?
* A: Use the wordlist.c and test.c files as an example. For an example in hava see Wordlist.java.


* Q: With one byte per character, how do you handle non ASCII characters?
* A: the data/<lang>.map file can be used to map characters outside a-z to something else such as 1-9. In your game you will have to translate them back to the original charset. To help you with this, the charset at the end of the file contains both ASCII and unicode representations.


* Q: I want to create a new list and my data is not UTF-8, how do I convert it?
* A: try:

    iconv --from-code=ISO-8859-1 --to-code=UTF-8  input > output

* Q: I am creating a new list and upper-case unicode characters are not recognized
* A: The idea is to only include lower-case. A tool used internally cannot handle unicode case conversion. Open your list and manually replace those letters with lower-case.
