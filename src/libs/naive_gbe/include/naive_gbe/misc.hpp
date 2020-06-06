//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <string>
#include <system_error>

#include <naive_gbe/types.hpp>

namespace naive_gbe
{
	buffer load_file(std::string const& file_name, std::error_code& ec);
}
