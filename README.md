# YaTS - Yet another Task Scheduler

[![Build Status](https://travis-ci.org/hpicgs/yats.svg?branch=master)](https://travis-ci.org/hpicgs/yats)
[![Build status](https://ci.appveyor.com/api/projects/status/ijg3megtnjlcky8y?svg=true)](https://ci.appveyor.com/project/Highlife1911/yats)

YaTS is a general-purpose task scheduler library written in C++ with a focus on computer graphical rendering tasks.
Its main goal is to allow parallel execution of connected and unconnected tasks automatically, without explicit handling from the user.

## Some basics
The time required to find a solution to a problem can be reduced if the problem can be divided into independent subproblems.
These subproblems can then be distributed onto several threads and be solved in parallel, reducing the overall computation time. 

Figure 1 shows an example pipeline. The pipeline consists out of five tasks (subproblems).
![figure 1]( https://github.com/hpicgs/yats/blob/master/docs/images/abstract_example_pipeline.png "Abstract example pipeline")

In general, a task consumes an arbitrary number of inputs and produces an arbitrary number of outputs.
Task 4 can be computed in parallel to Task 2 and Task 3 after Task 1 has finished computing its outputs.

With YaTS you will only need to connect the tasks and the scheduler will make sure every task is executed in the correct order and will parallelize execution where possible.

## Design guidelines
YaTS is header-only and requires at least C++14. We hope we have kept the default configuration simple and easy to understand.

Being a university open source project, it was important to us that we preferred language features over user defined objects.

An example:
Obviously every task requires some kind of "run function" which is called by the scheduler when the task is to be executed.
One way of solving this problem is by having the developers inherit from some abstract task class.
We did not want to dictate the developers how to design their classes and structs.
We simply consider everything a task which has a run function.
This is validated at compile time.

## Scheduler
When you create a pipeline you can specify how many threads the scheduler may use to runs tasks on.
The scheduler uses a simple greedy algorithm to run tasks as soon they can be run, and a suitable thread is available.

## Constraints
Of course, not every task can be freely executed on every thread.
There may be constraints which need to be considered.
YaTS supports the following constraints:
* A task can be run on any thread (default)
* A task must be run on the main thread. This can be important when interacting with the GUI
* A task must be run on specific threads. This constraint is important if global resources are shared between tasks.

## Export pipeline
YaTS offers the functionality to export the created pipeline in a format that can be converted into an image file using graphviz DOT (https://www.graphviz.org/).
Figure 2 shows a graphical representation of a small example pipeline consisting out of two tasks.
![figure 2]( https://github.com/hpicgs/yats/blob/master/docs/images/debug_output1.png "Exported pipeline")

## When not to use YaTS
YaTS should not be used in case of
* Simple (synchronous) multithreading
* Only data parallelism and no task parallelism
* A strict program flow
* Distributed analytics

## When to use YaTS
YaTS is well suited for clearly defined subproblems with well defined communication between Tasks and if the pipeline should be executed more than once.

