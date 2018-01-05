#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

#include <yats/Output.h>
#include <yats/Identifier.h>
#include <yats/Input.h>

/*
parallel_sort has been programmed demonstrate the use and construction
of the pipeline. parallel_sort sorts the elements in a vector with the aim
to utilize more than one thread in the future.

There are three tasks. split_task defines a vector which is supposed to be sorted.
This vector is split into two vectors of about the same size.

These two vectors are sorted individually in sort_task. This step can be parallized.

In join_task the two vectors are merged again and the sorted sequence is shown on the console.
*/

/// <summary>
/// <para>Prints the elements of a vector to the console</para>
/// </summary>
/// <param name = "v">Vector to print to the console</param>
void print_vector(const std::vector<int>& v)
{
	for (auto e : v)
		std::cout << e << " ";
	std::cout << std::endl;
}

/// <summary>
/// <para>Task to split a defined vector into two vectors of equal size +/- one.</para>
/// </summary>
class split_task
{
public:
	split_task() : m_numbers(std::vector<int> { 5, 7, 6, 3, 35, 0, 1, 76, 9 }) {}

	~split_task() = default;

	/// <summary>
	/// <para>Splits m_numbers into two vectors of about equal size. These two vectors
	/// are output 0 and output 1.</para>
	/// </summary>
	yats::OutputBundle<yats::Output<std::vector<int>, "left"_id>, yats::Output<std::vector<int>, "right"_id>> run()
	{
		std::cout << "Running split_task" << std::endl;
		std::cout << "Vector to sort: ";
		print_vector(m_numbers);

		size_t half_size = m_numbers.size() / 2;

		std::vector<int> v1(m_numbers.cbegin(), m_numbers.cbegin() + half_size);
		std::vector<int> v2(m_numbers.cbegin() + half_size, m_numbers.cend());

		return std::make_tuple(v1, v2);
	}

protected:
	/// <summary>
	/// <para>Vector to split.</para>
	/// </summary>
	const std::vector<int> m_numbers;
};

/// <summary>
/// <para>Task to sort the input vector.</para>
/// </summary>
class sort_task
{
public:
	sort_task() = default;
	~sort_task() = default;

	/// <summary>
	/// <para>Sorts the input vector.</para>
	/// </summary>
	/// <param name = "v">Vector to sort</param>
	yats::OutputBundle<yats::Output<std::vector<int>, "sorted_vector"_id >> run(yats::Input<std::vector<int>, "v"_id> v)
	{
		std::cout << "Running sort_task" << std::endl;
		// TODO: implement operator * and/or operator-> for output and input
		std::sort(((std::vector<int>&)v).begin(), ((std::vector<int>&)v).end());
		//print_vector(v);
		return std::make_tuple((std::vector<int>&)v);
	}
};

/// <summary>
/// <para>Task to merge two sorted vectors.</para>
/// </summary>
class merge_task
{
public:
	merge_task() = default;
	~merge_task() = default;

	/// <summary>
	/// <para>Joins two sorted vectors</para>
	/// </summary>
	/// <param name = "v1">Vector 1</param>
	/// <param name = "v2">Vector 2</param>
	void run(yats::Input<std::vector<int>, "left"_id> v1, yats::Input<std::vector<int>, "right"_id> v2)
	{
		std::cout << "Running merge_task" << std::endl;
		std::vector<int> sorted_vector;
		
		std::merge(((std::vector<int>&)v1).begin(), ((std::vector<int>&)v1).end(),
			((std::vector<int>&)v2).begin(), ((std::vector<int>&)v2).end(),
			std::back_inserter(sorted_vector));

		std::cout << "Sorted vector: ";
		print_vector(sorted_vector);
	}
};