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
		type			task_		= {};
		std::uint32_t   x_			= 0;
		std::uint32_t   y_			= 0;
		std::uint32_t   w_			= 0;
		std::uint32_t   h_			= 0;
		bool			stretch_	= false;
		colour			colour_		= { 0, 0, 0, 0 };
	};

	render_task::render_task() :
		impl_(std::make_unique<render_task::impl>())
	{
	}

	render_task::~render_task() = default;

	void render_task::set_task(type task)
	{
		impl_->task_ = task;
	}

	render_task::type render_task::get_task()
	{
		return impl_->task_;
	}

	void render_task::set_colour(colour fill_colour)
	{
		impl_->colour_ = fill_colour;
	}

	colour render_task::get_colour()
	{
		return impl_->colour_;
	}

	void render_task::set_stretch(bool stretch)
	{
		impl_->stretch_ = stretch;
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

	bool render_task::get_stretch() const
	{
		return impl_->stretch_;
	}

	void render_task::get_position(std::int32_t & x, std::int32_t & y) const
	{
		x = impl_->x_;
		y = impl_->y_;
	}
}

