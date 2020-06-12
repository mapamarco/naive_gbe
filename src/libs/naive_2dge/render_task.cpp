//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/render_task.hpp>
#include <naive_2dge/detail/sdl.hpp>

namespace naive_2dge
{
	struct render_task::impl
	{
		image::ptr      image_		= nullptr;
		texture::ptr    texture_	= nullptr;
		std::uint32_t   x_			= 0;
		std::uint32_t   y_			= 0;
		std::uint32_t   w_			= 0;
		std::uint32_t   h_			= 0;
	};

	render_task::render_task() :
		impl_(std::make_unique<render_task::impl>())
	{
	}

	render_task::~render_task() = default;

	image::ptr render_task::get_image()
	{
		return impl_->image_;
	}

	texture::ptr render_task::get_texture()
	{
		return impl_->texture_;
	}

	void render_task::set_image(image::ptr image)
	{
		impl_->image_ = image;
	}

	void render_task::set_texture(texture::ptr texture)
	{
		impl_->texture_ = texture;
	}

	void render_task::set_position(std::uint32_t x, std::uint32_t y)
	{
		impl_->x_ = x;
		impl_->y_ = y;
	}

	void render_task::set_size(std::uint32_t w, std::uint32_t h)
	{
		impl_->w_ = w;
		impl_->h_ = h;
	}

	void render_task::get_position(std::uint32_t & x, std::uint32_t & y) const
	{
		x = impl_->x_;
		y = impl_->y_;
	}

	void render_task::get_size(std::uint32_t & w, std::uint32_t & h) const
	{
		if (impl_->image_)
		{
			impl_->image_->get_size(w, h);
		}
		else if (impl_->texture_)
		{
			w = impl_->w_;
			h = impl_->h_;
		}
	}
}

