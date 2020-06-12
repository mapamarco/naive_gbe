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

#include <naive_2dge/image.hpp>
#include <naive_2dge/texture.hpp>

namespace naive_2dge
{

    class render_task
    {
    public:
        using ptr = std::shared_ptr<render_task>;
        render_task();
        render_task(render_task& rhs) = delete;
        render_task(render_task&& rhs) = delete;
        render_task& operator=(render_task&& rhs) = delete;
        virtual ~render_task();

		image::ptr get_image();
		texture::ptr get_texture();

		void set_image(image::ptr image);
		void set_texture(texture::ptr texture);
		void set_position(std::uint32_t x, std::uint32_t y);
        void set_size(std::uint32_t w, std::uint32_t h);
        void get_position(std::uint32_t& x, std::uint32_t& y) const;
        void get_size(std::uint32_t& w, std::uint32_t& h) const;

    private:

        struct impl;

        std::unique_ptr<impl>   impl_;
    };

}
