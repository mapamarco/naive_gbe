//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <string_view>
#include <functional>

namespace naive_gbe
{
	template <typename DurationType>
	class benchmark
	{
		std::size_t num_samples_ = 1;

	public:

		benchmark() = default;

		benchmark(std::size_t num_samples) :
			num_samples_(num_samples)
		{
		}

		struct result
		{
			std::string_view title;

			DurationType total		= {};

			DurationType min		= {};

			DurationType max		= {};

			double average			= 0.0;

			std::vector<DurationType> durations = {};

			constexpr std::string_view to_string(std::chrono::nanoseconds const&) const
			{
				return "ns";
			}

			constexpr std::string_view to_string(std::chrono::microseconds const&) const
			{
				return "us";
			}

			constexpr std::string_view to_string(std::chrono::milliseconds const&) const
			{
				return "ms";
			}

			constexpr std::string_view to_string(std::chrono::seconds const&) const
			{
				return "s";
			}

			constexpr std::string_view to_string(std::chrono::minutes const&) const
			{
				return "m";
			}

			constexpr std::string_view to_string(std::chrono::hours const&) const
			{
				return "h";
			}

			bool operator<(result const& rhs) const
			{
				return total < rhs.total;
			}

			friend std::ostream& operator<<(std::ostream& out, result const& r)
			{
				auto flags = out.flags();
				auto precision = out.precision();

				out << "{ "
					<< " \"benchmark\": { "
					<< "\"unit\": \"" << r.to_string(r.total) << "\", "
					<< "\"samples\": " << r.durations.size() << ", "
					<< "\"avg\": " << std::fixed << std::setprecision(2) << r.average << ", "
					<< "\"min\": " << r.min.count() << ", "
					<< "\"max\": " << r.max.count() << ", "
					<< "\"total\": " << r.total.count() << ", "
					<< "\"title\": \"" << r.title << "\" }"
					<< " }";

				out.precision(precision);
				out.flags(flags);

				return out;
			}
		};

		result run(std::string_view title, std::function<void(void)> work)
		{
			if (num_samples_ == 0)
				return result{ title };

			using namespace std::chrono;
			using tp = high_resolution_clock::time_point;

			tp start, end;
			std::vector<DurationType> v;
			v.reserve(num_samples_);

			for (std::size_t i = 0; i < num_samples_; ++i)
			{
				start = high_resolution_clock::now();
				work();
				end = high_resolution_clock::now();

				v.emplace_back(
					duration_cast<DurationType>(end - start)
				);
			}

			result r;

			auto [it_min, it_max] = std::minmax_element(v.begin(), v.end());
			r.min = *it_min;
			r.max = *it_max;

			std::for_each(v.begin(), v.end(), [&r](auto& duration)
				{
					r.total = r.total + duration;
				});

			r.average = static_cast<double>(r.total.count()) / v.size();
			r.durations = std::move(v);
			r.title = title;

			return r;
		}
	};
}
