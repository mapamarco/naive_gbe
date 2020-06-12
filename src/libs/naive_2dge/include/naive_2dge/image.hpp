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
    class image
    {
    public:

        using ptr = std::shared_ptr<image>;

        image();

        image(image&&) = delete;

        image(const image&) = delete;

        image& operator=(const image&) = delete;

        virtual ~image();

        void open(const std::string& path);

        void close();

        bool is_open() const;

        void* get_resource();

        void get_size(std::uint32_t& w, std::uint32_t& h) const;

    private:

        struct impl;

        std::unique_ptr<impl>   impl_;
    };
}
