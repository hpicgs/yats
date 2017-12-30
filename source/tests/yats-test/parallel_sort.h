#include <algorithm>
#include <iostream>
#include <vector>

#include <yats/Output.h>
#include <yats/Input.h>

namespace yats
{
	/// <summary>
	/// <para>parallel_sort has been programmed demonstrate the use and construction
	/// of the pipeline. parallel_sort sorts the elements in a vector with the aim
	/// to utilize more than one thread in the future.</para>
	/// <para>There are three tasks. split_task defines a vector which is supposed to be sorted.
	/// This vector is split into two vectors of about the same size.</para>
	/// <para>These two vectors are sorted individually in sort_task. This step can be parallized.</para>
	/// <para>In join_task the two vectors are merged again and the sorted sequence is shown on the console.</para>
	/// </summary>
	namespace parallel_sort
	{
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
			yats::OutputBundle<yats::Output<std::vector<int>, 0>, yats::Output<std::vector<int>, 1>> run()
			{
				std::cout << "Running split_task" << std::endl;
				std::cout << "Vector to sort: ";
				print_vector(m_numbers);

				std::vector<int> v1;
				std::vector<int> v2;

				size_t half_size = m_numbers.size() / 2;

				std::copy_n(m_numbers.cbegin(), half_size, std::back_inserter(v1));
				//print_vector(v1);

				std::copy(m_numbers.cbegin() + half_size,
					m_numbers.cend(), std::back_inserter(v2));
				//print_vector(v2);

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
			yats::OutputBundle<yats::Output<std::vector<int>, 0>> run(yats::Input<std::vector<int>, 0> v)
			{
				std::cout << "Running sort_task" << std::endl;
				// FRAGE: Brauche ich die Konstruktion ((std::vector<int>&)v) ?
				std::sort(((std::vector<int>&)v).begin(), ((std::vector<int>&)v).end());
				//print_vector(v);
				return std::make_tuple((std::vector<int>&)v);
			}
		};

		/// <summary>
		/// <para>Task to join two sorted vectors.</para>
		/// </summary>
		class join_task
		{
		public:
			join_task() = default;
			~join_task() = default;

			/// <summary>
			/// <para>Joins two sorted vectors</para>
			/// </summary>
			/// <param name = "v1">Vector 1</param>
			/// <param name = "v2">Vector 2</param>
			void run(yats::Input<std::vector<int>, 0> v1, yats::Input<std::vector<int>, 1> v2)
			{
				std::cout << "Running join_task" << std::endl;
				std::vector<int> sorted_vector;
				int i = 0;
				int j = 0;

				while (i < ((std::vector<int>&)v1).size() || j < ((std::vector<int>&)v2).size())
				{
					if (i >= ((std::vector<int>&)v1).size())
					{
						sorted_vector.push_back(((std::vector<int>&)v2)[j]);
						j++;
					}
					else if (j >= ((std::vector<int>&)v2).size())
					{
						sorted_vector.push_back(((std::vector<int>&)v1)[i]);
						i++;
					}
					else if (((std::vector<int>&)v1)[i] < ((std::vector<int>&)v2)[j])
					{
						sorted_vector.push_back(((std::vector<int>&)v1)[i]);
						i++;
					}
					else
					{
						sorted_vector.push_back(((std::vector<int>&)v2)[j]);
						j++;
					}
				}
				std::cout << "Sorted vector: ";
				print_vector(sorted_vector);
			}
		};
	} // namespace parallel_sort
} // namespace yats