//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <naive_2dge/detail/sdl.hpp>

#include <naive_2dge/image.hpp>
#include <naive_2dge/types.hpp>
#include <naive_2dge/texture.hpp>
#include <naive_2dge/font.hpp>

namespace naive_2dge
{
    class engine
    {
    public:

		enum window_flags
		{

		};

        engine();
        virtual ~engine();

        void init(std::string const& title, uint32_t width, uint32_t height);

		bool is_fullscreen();

		void toggle_fullscreen();

		void set_icon(std::string const& icon);

		void set_window_size(std::uint16_t width, std::uint16_t height);

		std::pair<std::uint16_t, std::uint16_t> get_window_size();

		void set_assets_dir(std::string const& assets_dir);

		bool keep_running();

		void exit(int exit_code);

		void cancel_exit();

		int get_exit_code();

		void render();

		void show_cursor(bool enabled) const;

		float get_fps() const;

		image::ptr create_image(const std::string& name, const std::string& path);
		image::ptr get_image(const std::string& name);

		texture::ptr create_texture(std::string const& name, std::uint16_t width, std::uint16_t height);
		texture::ptr get_texture(std::string const& name);

        font::ptr create_font(const std::string& name, const std::string& path, std::size_t size);
        font::ptr get_font(const std::string& name);
        void get_text_size(const std::string& text, font::ptr font, std::uint32_t& w, std::uint32_t& h);

		void draw(rectangle const& rect, colour colour);

		void draw(image::ptr, std::uint32_t x, std::uint32_t y);

		void draw(texture::ptr texture, std::uint32_t x, std::uint32_t y, bool stretch);

		void draw(std::string const& text, font::ptr font, std::uint16_t x, std::uint16_t y, colour c, float scale = 1.0f);

    private:

        struct impl;
        std::unique_ptr<impl>   impl_;
    };
}
