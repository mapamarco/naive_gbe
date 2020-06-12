//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/font.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <sstream>
#include <stdexcept>

namespace naive_2dge
{
	struct font::impl
	{
		TTF_Font* resource_ = nullptr;
	};

	font::font()
		: impl_(std::make_unique<font::impl>())
	{
	}

	font::~font()
	{
		close();
	}

	void font::open(const std::string& path, std::size_t size)
	{
		impl_->resource_ = TTF_OpenFont(path.c_str(), static_cast<int>(size));

		if (!impl_->resource_)
		{
			std::stringstream err;
			err << "Failed to load font '" << path << "'! Error: " << TTF_GetError();
			throw std::runtime_error(err.str());
		}
	}

	void font::close()
	{
		if (impl_->resource_)
		{
			TTF_CloseFont(impl_->resource_);
			impl_->resource_ = nullptr;
		}
	}

	bool font::is_open() const
	{
		return impl_->resource_ != nullptr;
	}

	void* font::get_resource()
	{
		return impl_->resource_;
	}
}
