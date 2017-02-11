
WORD LIST
========

The word list project aims to create a set of open word lists to be used in open source games.

The idea is to start from an open list of words or even a dictionary and by adding, removing and filtering words create a list suitable to be used in games such as word puzzles.


Usage
-----

To build the word list for language xx, execute

    make filter LN=xx

The produced word list will be in

   build/xx.bin

To build everything just run

   make update



FAQ
---

Q. Under which license are these lists distributed?

A. This project itself is GPLv2. But some list have an original license that they will retain. See the corresponding <lang>.LICENSE file.


Q. These words are not good! They are unsuitable for my game! Fix them, or else!!

A. This is an open source project and you have the chance to improve it by submitting your own changes.
Note however that the main word list is meant to contain all possible words while some games may require certain words to be excluded (e.g. words ending with plural 's).
In that case you should use our lists as a starting point and then filter our any words that are not suitable for your particular need.



Q. How do I limit the word list to words of a certain length?

A. Change the <lang>.config file to your liking and rebuild the list.



Q. What is the format of the produced word list?

A. The <lang>.bin file is contains a sorted list of zero-terminated words plus a small header and footer. The sorted list of words allows use of quasi-binary search, which is a good compromise w.r.t. memory usage, speed and code complexity.



Q. Why does the word list also contain language statistics?

A. This information can be used to help create better randomly generated game levels.



Q. I don't understand the file format. Can you help me add it to my game?

A. Use the common.c and test.c files as an example.


Q. With one byte per character, wow do you handle non ASCII characters?

A. the <lang>.map file can be used to map characters outside a-z to something else such as 1-9. In your game you will have to translate them back to the original charset.
