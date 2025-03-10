![badge](https://github.com/poltamtamis/sql-query-optimizer/actions/workflows/cmake_test.yml/badge.svg)

## Contributors
<table>
  <tbody>
    <tr>
      <td align="center"><a href="https://github.com/poltamtamis"><img src="https://avatars.githubusercontent.com/u/59566389?v=4" width="100px;" alt="Polydoros Tamtamis"/><br /><sub><b>Polydoros Tamtamis</b></sub></a><br /><sub>sdi1900184</sub></td>
      <td align="center"><a href="https://github.com/chrisioan"><img src="https://avatars.githubusercontent.com/u/75933777?v=4" width="100px;" alt="Christos Ioannou"/><br /><sub><b>Christos Ioannou</b></sub></a><br /><sub>sdi1900222</sub></td>
    </tr>
  </tbody>
</table>
<br></br>

## About the project

The goal of the project is the development of an application that performs SQL queries to a database. Each query uses a number of relations on which different filters and joins are applied. 

A very efficient way of optimizing database queries is through the use of Partitioned Hash Joins. Since joins tend to be some of the most computationally expensive operations when it comes to database queries, optimizing them can greatly speed up the execution of a program. The optimizer takes advantage of cache locality, partitioning and hash joins to achieve this.

The step by step process is explained in detail <a href="Project_Report.pdf" class="">here</a>.


## Table of contents
* [Requirements](#requirements)
* [How To Run](#how-to-run)
* [Application Report](https://github.com/polt13/JJ_Project/blob/master/Project_Report.pdf)
* [Type Description](#type-description)
* [Creating Relations](#creating-relations)
* [Partitioning](#partitioning)
* [Hash Table](#hash-table)
* [Simple Vector](#simple-vector)
* [Query Execution](#query-execution)
<br/><br/>

## Requirements

* CMake
  ```sh
  sudo apt install cmake
  ```
* Make
  ```sh
  sudo apt install make
  ```
* Compiler with support for C++11 or newer
  ```sh
  sudo apt install g++
  ```
* ASan, gdb if compiled with debug flags
  ```sh
  sudo apt install libasan4-dbg
  ```
  ```sh
  sudo apt install gdb
  ```
<br/><br/>

**NOTE**: To comply with best C++ practices, we're using the C++ version of the headers for all the C libraries we use. Namely, we opted for `cstdio` over `stdio.h`
and `cstring` over `string.h` (the latter is useful because of `memmove`). Every method is also wrapped in the `std` namespace, which helps avoid conflicts.
<br/><br/>

## How To Run 

When on the root directory of the project:

* `chmod +x runRelease.sh`

* `./runRelease` -> Compiles and runs `runTestharness` with the defined workload.
 
**If the above steps don't work, use `cmake . && make && cp Driver harness ./build`**
 
***

**Running the driver with specific input:**
 
 <!-- tsk -->
 ```sh
 ./Driver < input.txt
 ```
 
 **Driver** expects input in its `stdin`. It first reads a list of the participating relations, the word 'Done' after it and then a list of the queries that are going to be executed.
 
 **If `runTestharness` is used for running the program, there's predefined input that's being fed to Driver**
 
 **If `Driver` is run as a standalone program input.txt shall be of the form:**
 
 ```
 r0
 r1
 r2
 Done
 query1
 query2
 ```
 with the relation names containing either the full path or the relative path to the `Driver` executable (e.g. ../workloads/small/r0)
<br/><br/>

## Type Description

`dataForm.h` contains the definitions for all of the important types. Since the majority of these classes contain methods that are very basic, we chose to include
their implementations directly in the header files for the sake of simplicity.
<br/><br/>

## Creating Relations

To create a relation, which is what `PartitionedHashJoin` operates on, you first need to create a `tuple` array. Each tuple consists of a key value (on which the Join operation
is performed) and a value (the rowID of the tuple). 

<ins>Because the `tuple` array is expected to be created created during runtime, it should be dynamically allocated on the heap using operator `new`. After passing
the pointer to the allocated array as an argument to the constructor of a `relation`, the user should not attempt to delete the array, as this is automatically
handled by the destructor of the `relation`.</ins>

`test.cpp` contains a few examples on how to create a relation.
<br/><br/>

## Partitioning

The size of the L2 is defined in `partitioner.h`. It is set to a relatively small value by default, to showcase that partitioning works as intended. You can change
the value as you see fit.

When it comes to partitioning, you can configure the methods the way you see fit. 

* By default, `partition` decides how many passes it needs to do based on the size of the relation. If you want to force `partition` to not perform partitioning, 1 pass,
or 2 passes you can configure it by setting the 2nd argument of `partition` to `0 (no partition), 1 (1 pass), 2 (2 passes)` respectively. -1 is the default value for the 
2nd argument, which indicates that `partition` automatically determines how many partitions it needs to create, based on the size of the relation.

* `partition` uses the last 2 bits of the key for the first pass of the partitioning and the last 4 bits for the 2nd pass by default. If you want to tweak these values, 
pass them as the final two arguments of `partition`. 

For example, if you need to partition a relation using 4 bits for the first pass and 8 bits for the 2nd (if it comes to it), use `partition(r,-1,4,8)`. If you want to
force the 2nd pass regardless of the size of the relation, use `partition(r,2,4,8)`.

To partition a relation, you need to first create a `Partitioner` object specifically for it. That is because each partitioner maintains an internal state for each relation it partitions.
Namely, during the partitioning phase it generates a histogram and a `psum` array which it later uses to generate the sorted `R'` relation.

Additionally, it maintains an integer which represents how many passes it made. This is useful because both relations need to have equal amount of partitions for PHJ.
When the `r` relationship is partitioned during `PartitionedHashJoin`, we use the afforementioned integer to force the 2nd partitioning to do as many passes as the first
one did.

`Partitioner` holds a pointer to a Histogram object, because a new one needs to be allocated on each pass. When a new one is generated, the previous one is no longer 
of any use - therefore, it can be safely deleted.

The size of the L2 Cache is defined in `partitioner.h`.
<br/><br/>

## Hash Table

As it has been discussed in class' Piazza Forum, the ``Hash Table`` is implemented as a ``circle``, which means when we get to the "last" bucket, the next one is the very first one. To do that, we have to apply ``modulo (%) num_buckets`` to the index, where *num_buckets* indicates the *HT's size*.

- For example, if **HT's size = 60** and **NBHD_SIZE = 32** then, **bucket[59] neighbourhood** consists of **bucket[59], bucket[0], ... bucket[30]**.

Same goes when we're on the "first" bucket and want to go further back.

- For example: If **HT's size = 60**, we are on **bucket[4]** and want to **go back 8 slots**, then we end up on **bucket[56]**.

In case a ``rehash`` is needed, we simply ``increase the HT's size by doubling it and adding 1``.
*All the tuples have to be re-inserted*.

Furthermore, there is a chance that ``multiple tuples with same keys but different rowIDs`` are inserted into the Hash Table. If the amount of such tuples **exceeds** the ``NBHD_SIZE``, then rehashing does not help at all (Neighbourhood will remain full). For this reason, we have implemented a ``Linked List`` (chaining) where, if the exact same key is found, the whole tuple is appended to the list. This List is part of ``class bucket`` fields.

The size of the Neighbourhood is defined in ``hashtable.h`` as ``NBHD_SIZE``.

**NOTE:** Some of the unit tests (e.g Full HT Insert, Swap HT Insert) will fail if NBHD_SIZE is changed because they are based on that specific size.
*Nevertheless, the whole implementation works just fine*.
<br/><br/>

## Simple Vector

We have implemented a simple version of the vector class using C++ templates since there are numerous situations where such a data structures is needed to operate
on different types. `simple_vector` uses a dynamic array under the hood that grows whenever its capacity is met. Changing how much the vector grows each time and the method of copying each element to a new location when the capacity is maxed out may have an impact on performance. By default, the starting capacity is 10.
<br/><br/>

## Query Execution

`QueryExec` contains all the relevant code for parsing each query, executing it and printing the checksum. After parsing, every filter predicate is stored in a way so
that every literal is on the right hand side. For instance, `5 < 0.1` is turned into `0.1 > 5`. This simplifies operations later - more specifically, we can avoid having multiple data types defined where the literal is either on the left or the right hand side in the case of filters.

A filter is defined as any predicate that operates on a single relation, meaning that the right handside (after parsing) is always a literal. The `filter` struct stores the `rel` field and the `col` field, as well as the literal in the `literal` field. It also stores the operator in a form of enum (`EQ` for `=`, `GREATER` for `>` and `LESS`for `<`) in the `op` field.

A join is defined as any predicate that operates on two relations. The `join` struct stores the `left_rel` and `right_rel` along with their respective columns and operator used between them.

For the purpose of maintaining information regarding which relations are filtered and which have been part of joins two different arrays are defined. `rel_is_filtered`,`rel_is_joined` indicate whether a relation has been through a filter or a join operation. 

The order of predicate execution is always filters first, joins second. That means as soon as the first join happens, all filters have already been computed.

***

An array of simple vectors, `filtered[4]` contains all the filtered rowIDs for a relation `r` (only useful if `rel_is_filtered[r] = true`). When a relation is first joined with another, we use the rowIDs in the `filtered` vector to create the relation (or the entire relation if the relation hasn't been through a filter). 

A different array of simple vectors contains the rowIDs for the relation that have already been joined. When a join happens, we update the rest of the relations (with the matching index) based on the join result.

**NOTE 1: We assume that at most 4 relations are part of a query for simplicity purposes. This can easily be tweaked if needed.**

**NOTE 2 [Part 2]: When it comes to joins, we expect to only have a single intermediate result at a given time. If a situation arises where more than one intermediate result is needed, we can later work around it by changing the order of predicate execution, so that we always end up with 1 intermediate result**

