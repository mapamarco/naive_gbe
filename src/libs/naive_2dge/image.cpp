//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/image.hpp>
#include <naive_2dge/engine.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <sstream>
#include <stdexcept>


namespace naive_2dge
{
	struct image::impl
	{
		SDL_Texture* resource_ = nullptr;
		uint32_t        x_ = 0;
		uint32_t        y_ = 0;
		uint32_t        w_ = 0;
		uint32_t        h_ = 0;
	};

	image::image()
		: impl_(std::make_unique<image::impl>())
	{
		SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "image::ctor()");
		//impl_->engine_ = engine;
	}

	image::~image()
	{
		SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "image::dtor()");
		close();
	}

	void image::open(const std::string& path)
	{
		SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "image::open() - path: %s", path.c_str());

		SDL_Surface* surface = IMG_Load(path.c_str());

		if (!surface)
		{
			std::stringstream err;
			err << "Failed to load image '" << path << "'! Error: " << IMG_GetError();
			throw std::runtime_error(err.str());
		}

		//impl_->resource_ = SDL_CreateTextureFromSurface(
		//	detail::service::get_instance().get_renderer(),
		//	surface);

		impl_->w_ = surface->w;
		impl_->h_ = surface->h;

		SDL_FreeSurface(surface);
	}

	void image::close()
	{
		SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "image::close()");

		if (impl_->resource_)
		{
			SDL_DestroyTexture(impl_->resource_);
			impl_->resource_ = nullptr;
		}
	}

	bool image::is_open() const
	{
		bool result = impl_->resource_ != nullptr;
		SDL_LogVerbose(SDL_LOG_CATEGORY_AUDIO, "image::is_open() : %d", result);

		return result;
	}

	void* image::get_resource()
	{
		return impl_->resource_;
	}

	void image::get_size(std::uint32_t& w, std::uint32_t& h) const
	{
		w = impl_->w_;
		h = impl_->h_;
	}
}
