# CS214-CompSys-Project-2-File-Analysis

Summary:

In this assignment you will make use of the filesystem API and the POSIX thread library to implement a simple plagiarism checker.

Plagiarism is a self-defeating shortcut that plagues many student bodies. For those unacquainted with it, plagiarism is the act of claiming someone else's work as your own. Paradoxically, many college and university students pay large sums to access experts in their professed fields, but do not make use of them. They instead substitute the work of others, guaranteeing little to no utility for the tuition dollars they spend, making subsequent courses even more difficult and all but assuring they will not have the skills they ought to once employed! It is naturally all but impossible to imagine, but it certainly does happen.

Your task will be to write an extremely simple plagiarism detector using some basic computational linguistics. Given a base directory to work from your detector will scan it for all files and subdirectories. It should start a thread to handle each file or subdirectory it finds. A subdirectory should be handled by doing the same operation, namely scanning it for all files and subdirectories and making a thread to handle each. A file should be handled by reading in its contents and building a discrete distribution of all the words that appear in it. Once done, your detector should compare all distributions and classify 'suspicious' files.
