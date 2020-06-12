//
//            Copyright (c) Marco Amorim 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <naive_2dge/engine.hpp>
#include <naive_2dge/render_task.hpp>
#include <naive_2dge/fps_counter.hpp>
#include <naive_2dge/detail/sdl.hpp>

#include <sstream>
#include <vector>
#include <unordered_map>
#include <exception>

namespace naive_2dge
{
	struct engine::impl
	{
		using font_map		= std::unordered_map<std::string, font::ptr>;
		using image_map		= std::unordered_map<std::string, image::ptr>;
		using texture_map	= std::unordered_map<std::string, texture::ptr>;
		using task_list		= std::vector<render_task::ptr>;

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

	void engine::render()
	{
		if (SDL_SetRenderDrawColor(impl_->renderer_, 0, 0, 0, 0))
			throw_error("Could not set blender mode", SDL_GetError());

		if (SDL_RenderClear(impl_->renderer_))
			throw_error("Could not clear renderer", SDL_GetError());

		std::int32_t x, y;
		std::uint32_t w, h;
		SDL_Texture* texture = nullptr;
		image::ptr image = nullptr;

		for (auto task : impl_->tasks_)
		{
			auto task2 = task->get_task();

			if (std::holds_alternative<image::ptr>(task2))
			{
				auto resource = std::get<image::ptr>(task2);

				resource->get_size(w, h);
				texture = reinterpret_cast<SDL_Texture*>(resource->get_resource());
			}
			else if (std::holds_alternative<texture::ptr>(task2))
			{
				auto resource = std::get<texture::ptr>(task2);

				resource->get_size(w, h);
				texture = reinterpret_cast<SDL_Texture*>(resource->get_resource());
			}
			else if (std::holds_alternative<rectangle>(task2))
			{
				auto resource = std::get<rectangle>(task2);

				SDL_Rect rect{ resource.x, resource.y, resource.w, resource.h };

				auto c = task->get_colour();

				if (SDL_SetRenderDrawColor(impl_->renderer_, c.r, c.g, c.b, c.a))
					throw_error("Could not render rectangle", SDL_GetError());

				if (SDL_RenderFillRect(impl_->renderer_, &rect))
					throw_error("Could not render rectangle", SDL_GetError());

				continue;
			}

			task->get_position(x, y);

			SDL_Rect src = { 0, 0, static_cast<int>(w), static_cast<int>(h) };

			if (task->get_stretch())
			{
				if (SDL_RenderCopy(impl_->renderer_, texture, &src, nullptr))
					throw_error("Could not render texture", SDL_GetError());
			}
			else
			{
				SDL_Rect dst = { x, y, static_cast<int>(w), static_cast<int>(h) };

				if (SDL_RenderCopy(impl_->renderer_, texture, &src, &dst))
					throw_error("Could not render texture", SDL_GetError());
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

	SDL_Event& engine::get_input()
	{
		return impl_->event_;
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
		auto task = std::make_unique<render_task>();

		task->set_task(rect);
		task->set_colour(colour);
		impl_->tasks_.push_back(std::move(task));
	}

	void engine::draw(image::ptr image, std::uint32_t x, std::uint32_t y)
	{
		auto task = std::make_unique<render_task>();

		task->set_task(image);
		task->set_position(x, y);

		impl_->tasks_.push_back(std::move(task));
	}

	void engine::draw(texture::ptr texture, std::uint32_t x, std::uint32_t y, bool stretch)
	{
		auto task = std::make_unique<render_task>();

		task->set_task(texture);
		task->set_position(x, y);
		task->set_stretch(stretch);

		impl_->tasks_.push_back(std::move(task));
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

	void engine::draw(const std::string& text, font::ptr font, std::uint32_t x, std::uint32_t y, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	{
		auto resource = reinterpret_cast<TTF_Font*>(font->get_resource());

		SDL_Surface* surface = TTF_RenderText_Solid(resource, text.c_str(), SDL_Color{ r, g, b, a });

		if (!surface)
			throw_error("failed to render text '" + text + "'", TTF_GetError());

		auto resource2 = std::make_shared<texture>();
		{
			resource2->create(surface, impl_->renderer_);
		}

		auto task = std::make_unique<render_task>();
		{
			task->set_task(resource2);
			task->set_position(x, y);
			task->set_size(surface->w, surface->h);
		}

		impl_->tasks_.push_back(std::move(task));

		SDL_FreeSurface(surface);
	}
}

