//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/engine.hpp>
#include <naive_2dge/fps_counter.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <sstream>
#include <vector>
#include <unordered_map>
#include <exception>
#include <variant>

namespace naive_2dge
{
	struct render_task
	{
		using resource_type = std::variant<
			image::ptr,
			texture::ptr,
			rectangle
		>;

		std::int16_t	x_			= 0;
		std::int16_t	y_			= 0;
		std::uint16_t	w_			= 0;
		std::uint16_t	h_			= 0;
		float			scale_		= 1.0f;
		bool			stretch_	= false;
		colour			colour_		= { 0, 0, 0, 0 };
		resource_type	resource_	= {};
	};

	struct engine::impl
	{
		using font_map		= std::unordered_map<std::string, font::ptr>;
		using image_map		= std::unordered_map<std::string, image::ptr>;
		using texture_map	= std::unordered_map<std::string, texture::ptr>;
		using task_list		= std::vector<render_task>;

		bool                keep_running_	= true;
		int                 exit_code_		= EXIT_SUCCESS;
		std::string         title_			= "unknown";
		std::string         assets_dir_		= "";
		fps_counter			fps_			= { 500 };
		SDL_Window*			window_			= nullptr;
		SDL_Renderer*		renderer_		= nullptr;
		SDL_Event           event_;
		task_list           tasks_;
		font_map            fonts_;
		image_map           images_;
		texture_map			textures_;

		void init_video();
		void init_image();
		void init_font();
		void create_window(const std::string& title, uint32_t width, uint32_t height);
		void create_renderer();
		void close_video();
		void close_image();
		void close_font();
		void close_renderer();
		void render_texture(render_task const& task, SDL_Texture* texture);
		void render_rectangle(render_task const& task, rectangle const& rect);
	};

	void throw_error(std::string const& description, std::string const& detail)
	{
		throw std::runtime_error(description + ". Error: " + detail);
	}

	void engine::impl::init_video()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
			throw_error("Could not initialise SDL2", SDL_GetError());
	}

	void engine::impl::init_image()
	{
		if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
			throw_error("Could not initialise SDL2_image", IMG_GetError());
	}

	void engine::impl::init_font()
	{
		if (TTF_Init() == -1)
			throw_error("Could not initialise SDL2_ttf", TTF_GetError());
	}

	void engine::impl::create_window(const std::string& title, uint32_t width, uint32_t height)
	{
		window_ = SDL_CreateWindow(
			title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

		if (!window_)
			throw_error("Could not create window", SDL_GetError());
	}

	void engine::impl::create_renderer()
	{
		//if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		//	throw_error("Linear texture filtering not enabled", SDL_GetError());

		renderer_ = SDL_CreateRenderer(
			window_,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (!renderer_)
			throw_error("Could not create renderer", SDL_GetError());

		if (SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND))
			throw_error("Could not set blender mode", SDL_GetError());
	}

	void engine::impl::close_video()
	{
		SDL_Quit();
	}

	void engine::impl::close_image()
	{
		for (auto& resource : images_)
			resource.second->close();
		IMG_Quit();
	}

	void engine::impl::close_font()
	{
		for (auto& resource : fonts_)
			resource.second->close();
		TTF_Quit();
	}

	void engine::impl::close_renderer()
	{
		if (renderer_)
		{
			SDL_DestroyRenderer(renderer_);
			renderer_ = nullptr;
		}

		if (window_)
		{
			SDL_DestroyWindow(window_);
			window_ = nullptr;
		}
	}

	engine::engine()
		: impl_(std::make_unique<engine::impl>())
	{
	}

	engine::~engine()
	{
		impl_->close_font();
		impl_->close_image();
		impl_->close_video();
	}

	void engine::set_icon(std::string const& icon)
	{
		std::string icon_path = impl_->assets_dir_ + "/" + icon;
		SDL_Surface* surface = IMG_Load(icon_path.c_str());

		if (!surface)
			throw_error("Unable to load icon '" + icon + "'", SDL_GetError());

		SDL_SetWindowIcon(impl_->window_, surface);
		SDL_FreeSurface(surface);
	}

	void engine::set_window_size(std::uint16_t width, std::uint16_t height)
	{
		SDL_SetWindowSize(impl_->window_, width, height);
		SDL_SetWindowPosition(impl_->window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	std::pair<std::uint16_t, std::uint16_t> engine::get_window_size()
	{
		int width, height;

		SDL_GetWindowSize(impl_->window_, &width, &height);

		return { width, height };
	}

	bool engine::is_fullscreen()
	{
		return SDL_GetWindowFlags(impl_->window_) & SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	void engine::toggle_fullscreen()
	{
		std::uint32_t flags = 0;

		if (!is_fullscreen())
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

		if (SDL_SetWindowFullscreen(impl_->window_, flags) < 0)
			throw_error("Unable to set fullscreen", SDL_GetError());
	}

	void engine::set_assets_dir(std::string const& assets_dir)
	{
		impl_->assets_dir_ = assets_dir;
	}

	void engine::init(const std::string& title, uint32_t width, uint32_t height)
	{
		impl_->init_video();
		impl_->init_image();
		impl_->init_font();
		impl_->create_window(title, width, height);
		impl_->create_renderer();
	}

	bool engine::keep_running()
	{
		return impl_->keep_running_;
	}

	SDL_Rect create_sdl_rect(std::int16_t x, std::int16_t y, std::uint16_t w, std::uint16_t h, float scale)
	{
		return SDL_Rect
		{
			static_cast<int>(x * scale),
			static_cast<int>(y * scale),
			static_cast<int>(w * scale),
			static_cast<int>(h * scale)
		};
	}

	SDL_Rect create_sdl_rect(rectangle const& rect, float scale)
	{
		return create_sdl_rect(rect.x, rect.y, rect.w, rect.h, scale);
	}

	void engine::impl::render_texture(render_task const& task, SDL_Texture* texture)
	{
		SDL_Rect src = { 0, 0, task.w_, task.h_ };

		if (task.stretch_)
		{
			if (SDL_RenderCopy(renderer_, texture, &src, nullptr))
				throw_error("Could not render texture", SDL_GetError());
		}
		else
		{
			SDL_Rect dst = create_sdl_rect(task.x_, task.y_, task.w_, task.h_, task.scale_);

			if (SDL_RenderCopy(renderer_, texture, &src, &dst))
				throw_error("Could not render texture", SDL_GetError());
		}
	}

	void engine::impl::render_rectangle(render_task const& task, rectangle const& rectangle)
	{
		SDL_Rect rect = create_sdl_rect(rectangle, task.scale_);

		if (SDL_SetRenderDrawColor(renderer_, task.colour_.r, task.colour_.g, task.colour_.b, task.colour_.a))
			throw_error("Could not set render draw color", SDL_GetError());

		if (SDL_RenderFillRect(renderer_, &rect))
			throw_error("Could not render rectangle", SDL_GetError());
	}

	void engine::render()
	{
		if (SDL_SetRenderDrawColor(impl_->renderer_, 0, 0, 0, 0))
			throw_error("Could not set blender mode", SDL_GetError());

		if (SDL_RenderClear(impl_->renderer_))
			throw_error("Could not clear renderer", SDL_GetError());

		for (auto const& task : impl_->tasks_)
		{
			if (std::holds_alternative<image::ptr>(task.resource_))
			{
				auto resource = std::get<image::ptr>(task.resource_);
				auto texture = reinterpret_cast<SDL_Texture*>(resource->get_resource());
				impl_->render_texture(task, texture);
			}
			else if (std::holds_alternative<texture::ptr>(task.resource_))
			{
				auto resource = std::get<texture::ptr>(task.resource_);
				auto texture = reinterpret_cast<SDL_Texture*>(resource->get_resource());
				impl_->render_texture(task, texture);
			}
			else if (std::holds_alternative<rectangle>(task.resource_))
			{
				impl_->render_rectangle(task, std::get<rectangle>(task.resource_));
			}
		}

		SDL_RenderPresent(impl_->renderer_);

		++impl_->fps_;

		impl_->tasks_.clear();
	}

	void engine::exit(int exit_code)
	{
		impl_->exit_code_ = exit_code;
		impl_->keep_running_ = false;
	}

	void engine::cancel_exit()
	{
		impl_->keep_running_ = true;
	}

	int engine::get_exit_code()
	{
		return impl_->exit_code_;
	}

	float engine::get_fps() const
	{
		return impl_->fps_.get_fps();
	}

	void engine::show_cursor(bool enabled) const
	{
		SDL_ShowCursor(enabled ? SDL_ENABLE : SDL_DISABLE);
	}

	image::ptr engine::create_image(const std::string& name, const std::string& path)
	{
		auto resource = std::make_shared<image>();

		if (resource)
		{
			resource->open(impl_->assets_dir_ + "/" + path);
			impl_->images_[name] = resource;
		}

		return resource;
	}

	texture::ptr engine::create_texture(std::string const& name, std::uint16_t width, std::uint16_t height)
	{
		auto resource = std::make_shared<texture>();

		if (resource)
		{
			resource->create(name, width, height, impl_->renderer_);
			impl_->textures_[name] = resource;
		}

		return resource;
	}

	font::ptr engine::create_font(const std::string& name, const std::string& path, std::size_t size)
	{
		auto it = impl_->fonts_.find(name);
		if (it != std::end(impl_->fonts_))
			return it->second;

		auto resource = std::make_shared<font>();

		if (resource)
		{
			resource->open(impl_->assets_dir_ + "/" + path, size);
			impl_->fonts_[name] = resource;
		}

		return resource;
	}

	image::ptr engine::get_image(const std::string& name)
	{
		return impl_->images_.at(name);
	}

	texture::ptr engine::get_texture(const std::string& name)
	{
		return impl_->textures_.at(name);
	}

	font::ptr engine::get_font(const std::string& name)
	{
		return impl_->fonts_.at(name);
	}

	void engine::draw(rectangle const& rect, colour colour)
	{
		render_task task;

		task.resource_ = rect;
		task.colour_ = colour;

		impl_->tasks_.emplace_back(std::move(task));
	}

	void engine::draw(image::ptr image, std::uint32_t x, std::uint32_t y)
	{
		render_task task;

		task.resource_ = image;
		task.x_ = x;
		task.y_ = y;
		image->get_size(task.w_, task.h_);

		impl_->tasks_.emplace_back(std::move(task));
	}

	void engine::draw(texture::ptr texture, std::uint32_t x, std::uint32_t y, bool stretch)
	{
		render_task task;

		task.resource_ = texture;
		task.x_ = x;
		task.y_ = y;
		texture->get_size(task.w_, task.h_);
		task.stretch_ = stretch;

		impl_->tasks_.emplace_back(std::move(task));
	}

	void engine::get_text_size(const std::string& text, font::ptr font, std::uint32_t& w, std::uint32_t& h)
	{
		auto resource = reinterpret_cast<TTF_Font*>(font->get_resource());

		SDL_Surface* surface = TTF_RenderText_Solid(resource, text.c_str(), SDL_Color{ 0, 0, 0, 0 });

		if (!surface)
		{
			std::stringstream err;
			err << "failed to render text '" << text << "'! Error: " << IMG_GetError();
			throw std::runtime_error(err.str());
		}

		w = surface->w;
		h = surface->h;

		SDL_FreeSurface(surface);
	}

	void engine::draw(std::string const& text, font::ptr font, std::uint16_t x, std::uint16_t y, colour c, float scale)
	{
		auto font_raw = reinterpret_cast<TTF_Font*>(font->get_resource());

		SDL_Surface* surface = TTF_RenderText_Solid(font_raw, text.c_str(), SDL_Color{ c.r, c.g, c.b, c.a });

		if (!surface)
			throw_error("failed to render text '" + text + "'", TTF_GetError());

		auto resource = std::make_shared<texture>();
		resource->create(surface, impl_->renderer_);

		render_task task;

		task.resource_ = resource;
		task.x_ = x;
		task.y_ = y;
		task.w_ = surface->w;
		task.h_ = surface->h;
		task.scale_ = scale;

		impl_->tasks_.emplace_back(std::move(task));

		SDL_FreeSurface(surface);
	}
}

