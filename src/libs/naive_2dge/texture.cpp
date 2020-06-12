//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/texture.hpp>
#include <naive_2dge/engine.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <sstream>
#include <stdexcept>


namespace naive_2dge
{
	struct texture::impl
	{
		SDL_Texture*	resource_ = nullptr;
		uint32_t        x_ = 0;
		uint32_t        y_ = 0;
		uint32_t        w_ = 0;
		uint32_t        h_ = 0;
	};

	texture::texture()
		: impl_(std::make_unique<texture::impl>())
	{
	}

	texture::~texture()
	{
		if (impl_->resource_)
		{
			SDL_DestroyTexture(impl_->resource_);
			impl_->resource_ = nullptr;
		}

	}

	void texture::create(const std::string& path, std::uint16_t width, std::uint16_t height, void* renderer)
	{
		impl_->resource_ =
			SDL_CreateTexture(
				reinterpret_cast<SDL_Renderer*>(renderer),
				SDL_PIXELFORMAT_RGBA32,
				SDL_TEXTUREACCESS_STREAMING,
				width,
				height);

		impl_->w_ = width;
		impl_->h_ = height;
	}

	void texture::create(void* resource, void* renderer)
	{
		auto surface = reinterpret_cast<SDL_Surface*>(resource);

		impl_->resource_ =
			SDL_CreateTextureFromSurface(
				reinterpret_cast<SDL_Renderer*>(renderer),
				surface);

		impl_->w_ = surface->w;
		impl_->h_ = surface->h;
	}

	void* texture::get_resource()
	{
		return impl_->resource_;
	}

	void texture::get_size(std::uint32_t& w, std::uint32_t& h) const
	{
		w = impl_->w_;
		h = impl_->h_;
	}

	std::pair<std::uint16_t, std::uint16_t> texture::get_size() const
	{
		return { impl_->w_, impl_->h_ };
	}

	void texture::set_size(std::uint32_t w, std::uint32_t h)
	{
		impl_->w_ = w;
		impl_->h_ = h;
	}

	void texture::set_position(std::uint32_t x, std::uint32_t y)
	{
		impl_->x_ = x;
		impl_->y_ = y;
	}
}
