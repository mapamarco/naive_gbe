//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>

#include <naive_2dge/image.hpp>
#include <naive_2dge/texture.hpp>
#include <naive_2dge/types.hpp>

namespace naive_2dge
{

    class render_task
    {
    public:

		using type = std::variant<
			image::ptr,
			texture::ptr,
			rectangle
		>;

		using ptr = std::shared_ptr<render_task>;

        render_task();
        render_task(render_task& rhs) = delete;
        render_task(render_task&& rhs) = delete;
        render_task& operator=(render_task&& rhs) = delete;
        virtual ~render_task();

		void set_task(type task);
		type get_task();

		void set_colour(colour fill_colour);
		colour get_colour();

		void set_stretch(bool stretch);
		void set_position(std::uint32_t x, std::uint32_t y);
        void set_size(std::uint32_t w, std::uint32_t h);

		bool get_stretch() const;
		void get_position(std::int32_t& x, std::int32_t& y) const;

    private:

        struct impl;

        std::unique_ptr<impl>   impl_;
    };

}
