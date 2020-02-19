## Exchange and Brown

Version 0.1

(c) 2015 - 2018 Manuel R. Ciosici and UNSILO.com

-------------------------------------------
### Table of Contents 

  * [Introduction](#markdown-header-introduction)
  * [Building](#markdown-header-building)
  * [Usage](#markdown-header-usage)
  * [License](#markdown-header-license)
  * [Contact](#markdown-header-contact)
  * [References](#markdown-header-references)
  

------------------------------------
### Introduction

This repository contains software for clustering words based on their distributional similarity using Brown clustering, Exchange or a combination of the two.

------------------------------------
### Building

In order to compile you need the following:

 * A C++ compiler that supports C++14 and OpenMP (e.g., the newest 
 [GNU compiler](https://gcc.gnu.org/)) 
 
 * The `CMake` program for building the code

#### Instructions for building (OS X or Linux):

Using a terminal, change working directory to the src directory and
> mkdir cmake-build-release
> 
> cd cmake-build-release
> 
> cmake -DCMAKE\_BUILD\_TYPE=Release -DCMAKE\_C\_COMPILER=gcc-8 -DCMAKE\_CXX\_COMPILER=g++-8 ..
> 
> cmake --build . --target help -- -j

You can replace help with either one of the listed build targets, but you might also want to use:
> cmake --build . --target all -- -j

In order to build all targets.

------------------------------------
### Usage

#### Simple
Build target *simple_brown* and run the binary with the same name.

> ./simple_brown --help

#### Advanced
Running on a new corpus requires that you first build the targets *Brown* and *exchange_runner* and is split into phases such reading, reordering, clustering. There are a few binaries that deal with miscellaneous tasks.

##### Reading
> ./Brown read --help

Provides information of what parameters are necessary. This binary reads a plain text corpus into a binary object that is used for all later phases.

It is also possible to read skip-grams using the command

> ./Brown read-skip --help

##### Reordering

> ./Brown reorder --help

This binary reads the binary corpus file from the previous step and reorders words in the vocabulary so that low ID words are high-frequency words. This is necessary for both Brown and Exchange. This binary can be extended to implement other reordering strategies if needed.

##### Clustering

###### Brown clustering

> ./Brown induce_brown --help

Runs the Brown algorithm on top of the binary corpus file. It will output both a flat clustering and a hierarchical one. The hierarchical clustering is the same format as the format used by [wcluster](https://github.com/percyliang/brown-cluster).

###### Exchange clustering

> ./exchange_runner --help

Allows you to create clusters using Exchange. The difference between EXCHANGE and EXCHANGE_STEPS is that EXCHANGE_STEPS outputs the clustering at the end of every single iteration which allows for model selection.

###### Brown clustering on top of Exchange

Run Exchange as defined in the previous step and then Brown on top of it. And then use the following binary:

> ./compute_brown\_over\_clusters --help


#### Miscellaneous
 1. To get some basic information about a clustering:

 > ./clustering_facts --help
 
 2. To get the Average Mutual Information of a flat clustering:
 > ./print\_clustering_ami --help

------------------------------------
### License

This software is subject to the terms of 
[The MIT License](http://opensource.org/licenses/MIT), 
which [has been included in this repository](LICENSE.md).


------------------------------------
### Contact

Please contact 
Manuel R. Ciosici ([manuelrciosici@gmail.com](mailto:manuelrciosici@gmail.com)) with comments, questions, or bugs.


------------------------------------
### References

 1. P. F. Brown, P. V. deSouza, R. L. Mercer, V. J. Della Pietra, and J. C. Lai. 
(1992) 
"Class-based _n_-gram models of natural language."
_Computational Linguistics_ 18(4): 467--479.
http://dl.acm.org/ft_gateway.cfm?id=176316

 2. Ciosici, M. R. (2016). Improving Quality of Hierarchical Clustering for Large Data Series. _Aarhus University_. Retrieved from http://arxiv.org/abs/1608.01238



------------------------------------
