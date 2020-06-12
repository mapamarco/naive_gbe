//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <memory>
#include <string>

namespace naive_2dge
{
    class texture
    {
    public:

        using ptr = std::shared_ptr<texture>;

		texture();

		texture(texture&&) = delete;

		texture(const texture&) = delete;

		texture& operator=(const texture&) = delete;

		virtual ~texture();

        void create(const std::string& path, std::uint16_t width, std::uint16_t height, void* renderer);

		void create(void* surface, void* renderer);

		void* get_resource();

        void get_size(std::uint32_t& w, std::uint32_t& h) const;

    private:

        struct impl;

        std::unique_ptr<impl>   impl_;
    };
}
