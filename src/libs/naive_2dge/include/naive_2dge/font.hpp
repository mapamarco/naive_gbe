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
    class font
    {
    public:

        using ptr = std::shared_ptr<font>;

        font();

        font(font&&) = delete;

        font(const font&) = delete;

        font& operator=(const font&) = delete;

        virtual ~font();

        void open(const std::string& path, std::size_t size);

        void close();

        bool is_open() const;

        void* get_resource();

    private:

        struct impl;

        std::unique_ptr<impl>   impl_;
    };

}
