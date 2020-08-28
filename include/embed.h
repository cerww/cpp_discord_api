#pragma once
#include <nlohmann/json.hpp>
#include "../common/common/optional_from_json.h"

//i can prolly use mixins to do this in half as many lines


struct embed_footer {
	std::string_view text() const noexcept { return m_text; }

	std::optional<std::string_view> icon_url() const noexcept { return m_icon_url; }

	std::optional<std::string_view> proxy_icon_url() const noexcept { return m_proxy_icon_url; }

	embed_footer& set_text(std::string s) {
		m_text = std::move(s);
		return *this;
	}

	embed_footer& set_icon_url(std::string s) {
		m_icon_url = std::move(s);
		return *this;
	}

	embed_footer& set_proxy_icon_url(std::string s) {
		m_proxy_icon_url = std::move(s);
		return *this;
	}

private:
	std::string m_text;
	std::optional<std::string> m_icon_url;
	std::optional<std::string> m_proxy_icon_url;


	friend void from_json(const nlohmann::json&, embed_footer&);
	friend void to_json(nlohmann::json&, const embed_footer&);
};

inline void from_json(const nlohmann::json& json, embed_footer& footer) {
	footer.m_text = json["text"].get<std::string>();
	footer.m_icon_url = json.value("icon_url", std::optional<std::string>());
	footer.m_proxy_icon_url = json.value("proxy_icon_url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& out, const embed_footer& footer) {
	out["text"] = footer.m_text;
	//no optional support yet
	if (footer.m_icon_url.has_value()) {
		out["icon_url"] = footer.m_icon_url.value();
	}
	if (footer.m_proxy_icon_url.has_value()) {
		out["proxy_icon_url"] = footer.m_proxy_icon_url.value();
	}
}

struct embed_image {
	std::optional<std::string_view> url() const noexcept { return m_url; }

	std::optional<std::string_view> proxy_url() const noexcept { return m_proxy_url; }

	std::optional<int> height() const noexcept { return m_height; }

	std::optional<int> width() const noexcept { return m_width; }

	embed_image& set_url(std::string s) noexcept {
		m_url = std::move(s);
		return *this;
	}

	embed_image& set_proxy_url(std::string s) noexcept {
		m_proxy_url = std::move(s);
		return *this;
	}

	embed_image& set_height(int s) noexcept {
		m_height = s;
		return *this;
	}

	embed_image& set_width(int s) noexcept {
		m_width = s;
		return *this;
	}

private:
	std::optional<std::string> m_url;
	std::optional<std::string> m_proxy_url;
	std::optional<int> m_height;
	std::optional<int> m_width;

	friend void from_json(const nlohmann::json&, embed_image&);
	friend void to_json(nlohmann::json&, const embed_image&);
};

inline void from_json(const nlohmann::json& json, embed_image& image) {
	image.m_url = json.value("url", std::optional<std::string>());
	image.m_proxy_url = json.value("proxy_url", std::optional<std::string>());
	image.m_height = json.value("height", std::optional<int>());
	image.m_width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_image& image) {
	if (image.m_url.has_value()) {
		json["url"] = image.m_url.value();
	}
	if (image.m_proxy_url.has_value()) {
		json["proxy_url"] = image.m_proxy_url.value();
	}
	if (image.m_height.has_value()) {
		json["height"] = image.m_height.value();
	}
	if (image.m_width.has_value()) {
		json["width"] = image.m_width.value();
	}
}

struct embed_thumbnail {
	std::optional<std::string_view> url() const noexcept {
		return m_url;
	}

	std::optional<std::string_view> proxy_url() const noexcept {
		return m_proxy_url;
	}

	std::optional<int> height() const noexcept {
		return m_height;
	}

	std::optional<int> width() const noexcept {
		return m_width;
	}

	embed_thumbnail& set_url(std::string s) noexcept {
		m_url = std::move(s);
		return *this;
	}

	embed_thumbnail& set_proxy_url(std::string s) noexcept {
		m_proxy_url = std::move(s);
		return *this;
	}

	embed_thumbnail& set_height(int s) noexcept {
		m_height = s;
		return *this;
	}

	embed_thumbnail& set_width(int s) noexcept {
		m_width = s;
		return *this;
	}

private:
	std::optional<std::string> m_url;
	std::optional<std::string> m_proxy_url;
	std::optional<int> m_height;
	std::optional<int> m_width;
	friend void from_json(const nlohmann::json&, embed_thumbnail&);
	friend void to_json(nlohmann::json&, const embed_thumbnail&);
};

inline void from_json(const nlohmann::json& json, embed_thumbnail& thumbnail) {
	thumbnail.m_url = json.value("url", std::optional<std::string>());
	thumbnail.m_proxy_url = json.value("proxy_url", std::optional<std::string>());
	thumbnail.m_height = json.value("height", std::optional<int>());
	thumbnail.m_width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_thumbnail& thumbnail) {
	if (thumbnail.m_url.has_value()) {
		json["url"] = thumbnail.m_url.value();
	}
	if (thumbnail.m_proxy_url.has_value()) {
		json["proxy_url"] = thumbnail.m_proxy_url.value();
	}
	if (thumbnail.m_height.has_value()) {
		json["height"] = thumbnail.m_height.value();
	}
	if (thumbnail.m_width.has_value()) {
		json["width"] = thumbnail.m_width.value();
	}
}

struct embed_video {
	std::optional<std::string_view> url() const noexcept {
		return m_url;
	}

	std::optional<int> height() const noexcept {
		return m_height;
	}

	std::optional<int> width() const noexcept {
		return m_width;
	}

	embed_video& set_url(std::string s) noexcept {
		m_url = std::move(s);
		return *this;
	}

	embed_video& set_height(int s) noexcept {
		m_height = s;
		return *this;
	}

	embed_video& set_width(int s) noexcept {
		m_width = s;
		return *this;
	}

private:
	std::optional<std::string> m_url;
	std::optional<int> m_height;
	std::optional<int> m_width;

	friend void from_json(const nlohmann::json&, embed_video&);
	friend void to_json(nlohmann::json&, const embed_video&);
};


inline void from_json(const nlohmann::json& json, embed_video& video) {
	video.m_url = json.value("url", std::optional<std::string>());
	video.m_height = json.value("height", std::optional<int>());
	video.m_width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_video& video) {
	if (video.m_url.has_value()) {
		json["url"] = video.m_url.value();
	}
	if (video.m_height.has_value()) {
		json["height"] = video.m_height.value();
	}
	if (video.m_width.has_value()) {
		json["width"] = video.m_width.value();
	}
}

struct embed_provider {
	std::optional<std::string_view> name() const noexcept {
		return m_name;
	}

	std::optional<std::string_view> url() const noexcept {
		return m_url;
	}

	embed_provider& set_name(std::string s) {
		m_name = std::move(s);
		return *this;
	}

	embed_provider& set_url(std::string s) {
		m_url = std::move(s);
		return *this;
	}

private:
	std::optional<std::string> m_name;
	std::optional<std::string> m_url;

	friend void from_json(const nlohmann::json&, embed_provider&);
	friend void to_json(nlohmann::json&, const embed_provider&);
};

inline void from_json(const nlohmann::json& json, embed_provider& provider) {
	provider.m_name = json.value("name", std::optional<std::string>());
	provider.m_url = json.value("url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& json, const embed_provider& provider) {
	if (provider.m_name.has_value()) {
		json["name"] = provider.m_name.value();
	}
	if (provider.m_url.has_value()) {
		json["url"] = provider.m_url.value();
	}
}

struct embed_author {
	std::optional<std::string_view> name() const noexcept { return m_name; }

	std::optional<std::string_view> url() const noexcept { return m_url; }

	std::optional<std::string_view> icon_url() const noexcept { return m_icon_url; }

	std::optional<std::string_view> proxy_icon_url() const noexcept { return m_proxy_icon_url; }

	embed_author& set_name(std::string s) noexcept {
		m_name = std::move(s);
		return *this;
	}

	embed_author& set_url(std::string s) noexcept {
		m_url = std::move(s);
		return *this;
	}

	embed_author& set_icon_url(std::string s) noexcept {
		m_icon_url = std::move(s);
		return *this;
	}

	embed_author& set_proxy_icon_url(std::string s) noexcept {
		m_proxy_icon_url = std::move(s);
		return *this;
	}

private:
	std::optional<std::string> m_name;
	std::optional<std::string> m_url;
	std::optional<std::string> m_icon_url;
	std::optional<std::string> m_proxy_icon_url;

	friend void from_json(const nlohmann::json&, embed_author&);
	friend void to_json(nlohmann::json&, const embed_author&);
};

inline void from_json(const nlohmann::json& data, embed_author& author) {
	author.m_name = data.value("name", std::optional<std::string>());
	author.m_url = data.value("url", std::optional<std::string>());
	author.m_icon_url = data.value("icon_url", std::optional<std::string>());
	author.m_proxy_icon_url = data.value("proxy_icon_url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& json, const embed_author& author) {
	if (author.m_name.has_value()) {
		json["name"] = author.m_name.value();
	}
	if (author.m_url.has_value()) {
		json["url"] = author.m_url.value();
	}
	if (author.m_icon_url.has_value()) {
		json["icon_url"] = author.m_icon_url.value();
	}
	if (author.m_proxy_icon_url.has_value()) {
		json["proxy_icon_url"] = author.m_proxy_icon_url.value();
	}
}

struct embed_field {
	std::string_view name() const noexcept { return m_name; }

	std::string_view value() const noexcept { return m_value; }

	std::optional<bool> is_inline() const noexcept { return m_inline; }

	embed_field& set_name(std::string name) {
		m_name = std::move(name);
		return *this;
	}

	embed_field& set_value(std::string value) {
		m_value = std::move(value);
		return *this;
	}

	embed_field& set_is_inline(bool value) {
		m_inline = value;
		return *this;
	}

private:
	std::string m_name;
	std::string m_value;
	std::optional<bool> m_inline;

	friend void from_json(const nlohmann::json&, embed_field&);
	friend void to_json(nlohmann::json&, const embed_field&);
};

inline void from_json(const nlohmann::json& json, embed_field& field) {
	field.m_name = json["name"].get<std::string>();
	field.m_value = json["value"].get<std::string>();
	field.m_inline = json.value("inline", std::optional<bool>());
}

inline void to_json(nlohmann::json& json, const embed_field& field) {
	json["name"] = field.m_name;
	json["value"] = field.m_value;
	if (field.m_inline.has_value()) {
		json["inline"] = field.m_inline.value();
	}
}

struct embed {
	std::optional<std::string_view> title() const noexcept {
		return m_title;
	}

	std::optional<std::string_view> description() const noexcept {
		return m_description;
	}

	std::optional<std::string_view> url() const noexcept {
		return m_url;
	}

	std::optional<timestamp> timestamp() const noexcept { return m_timestamp; }

	std::optional<int> color() const noexcept { return m_color; }

	const std::optional<embed_footer>& footer() const noexcept { return m_footer; }

	const std::optional<embed_image>& image() const noexcept { return m_image; }

	const std::optional<embed_thumbnail>& thumbnail() const noexcept { return m_thumbnail; }

	const std::optional<embed_video>& video() const noexcept { return m_video; }

	const std::optional<embed_provider>& provider() const noexcept { return m_provider; }

	const std::optional<embed_author>& author() const noexcept { return m_author; }

	std::optional<std::span<const embed_field>> fields() const noexcept { return m_fields; }

	embed& set_title(std::string s) {
		m_title = std::move(s);
		return *this;
	}

	embed& set_description(std::string s) {
		m_description = std::move(s);
		return *this;
	}

	embed& set_url(std::string s) {
		m_url = std::move(s);
		return *this;
	}

	embed& set_timestamp(::timestamp s) {
		m_timestamp = s;
		return *this;
	}

	embed& set_color(int s) {
		m_color = s;
		return *this;
	}

	embed& set_footer(embed_footer s) {
		m_footer = std::move(s);
		return *this;
	}

	embed& set_image(embed_image s) {
		m_image = std::move(s);
		return *this;
	}

	embed& set_video(embed_video s) {
		m_video = std::move(s);
		return *this;
	}

	embed& set_provider(embed_provider s) {
		m_provider = std::move(s);
		return *this;
	}

	embed& set_author(embed_author s) {
		m_author = std::move(s);
		return *this;
	}

	template<typename /*std::convertible_to<embed_field>*/...fields>
	embed& add_fields(fields... s) {
		if(!m_fields.has_value()) {
			m_fields.emplace();
		}
		(m_fields.value().emplace_back(std::move(s)),...);
		return *this;
	}

private:
	std::optional<std::string> m_title;
	std::optional<std::string> m_description;
	std::optional<std::string> m_url;
	std::optional<::timestamp> m_timestamp;
	std::optional<int> m_color;
	std::optional<embed_footer> m_footer;
	std::optional<embed_image> m_image;
	std::optional<embed_thumbnail> m_thumbnail;
	std::optional<embed_video> m_video;
	std::optional<embed_provider> m_provider;
	std::optional<embed_author> m_author;
	std::optional<std::vector<embed_field>> m_fields;
	friend void from_json(const nlohmann::json& json, embed& embed);

	friend void to_json(nlohmann::json&, const embed&);
};

static constexpr int jashdasdasd = sizeof(embed);

inline void from_json(const nlohmann::json& json, embed& embed) {
	embed.m_title = json.value("title", std::optional<std::string>());
	embed.m_description = json.value("description", std::optional<std::string>());
	embed.m_url = json.value("url", std::optional<std::string>());
	//embed.m_timestamp = json.value("timestamp", std::optional<::timestamp>());
	embed.m_color = json.value("color", std::optional<int>());
	embed.m_footer = json.value("footer", std::optional<embed_footer>());
	embed.m_image = json.value("image", std::optional<embed_image>());
	embed.m_author = json.value("author", std::optional<embed_author>());
	embed.m_thumbnail = json.value("thumbnail", std::optional<embed_thumbnail>());
	embed.m_video = json.value("video", std::optional<embed_video>());
	embed.m_provider = json.value("provider", std::optional<embed_provider>());
	//embed.m_fields = json.value("fields", std::optional<std::vector<embed_field>>());
	//std::optional support is ;-; rn

	const auto has_fields_it = json.find("fields");
	if(has_fields_it != json.end()) {
		embed.m_fields = has_fields_it->get<std::vector<embed_field>>();
	}
	
}

inline void to_json(nlohmann::json& json, const embed& embed) {
	if(embed.m_title.has_value()) {
		json["title"] = embed.title().value();
	}
	if (embed.m_description.has_value()) {
		json["description"] = embed.description().value();
	}
	if (embed.url().has_value()) {
		json["url"] = embed.url().value();
	}
	if (embed.author().has_value()) {
		json["author"] = embed.author().value();
	}
	if (embed.footer().has_value()) {
		json["footer"] = embed.footer().value();
	}
	if (embed.provider().has_value()) {
		json["provider"] = embed.provider().value();
	}
	if (embed.video().has_value()) {
		json["video"] = embed.video().value();
	}
	if (embed.fields().has_value()) {
		json["fields"] = embed.m_fields.value();
	}
	if (embed.color().has_value()) {
		json["color"] = embed.color().value();
	}
	if (embed.image().has_value()) {
		json["image"] = embed.image().value();
	}
	if (embed.thumbnail().has_value()) {
		json["thumbnail"] = embed.thumbnail().value();
	}
	/*
	if (embed.m_timestamp.has_value()) {
		json["author"] = embed.author().value();
	}
	*/
	
}

/*


 */


namespace thingy_workyausdhasjdl {
	template<typename tag, template<typename> typename ...Property>
	struct embed_object :Property<embed_object<tag, Property...>>... {

		friend void from_json(const nlohmann::json&, embed_object<tag, Property...>&);
		friend void to_json(nlohmann::json&, const embed_object<tag, Property...>&);
	};
	
	template<typename Obj_t>
	struct name_property {
		std::optional<std::string_view> name()const noexcept { return m_value; };
		
		Obj_t& set_name(std::string s) {
			m_value = std::move(s);
			return (Obj_t&)(*this);
		}
		
	private:
		std::optional<std::string> m_value;
		friend void from_json(const nlohmann::json& json, name_property<Obj_t>& me) {
			me.m_value = json.value("name",std::optional<std::string>());
		}
		friend void to_json(nlohmann::json& json,const name_property<Obj_t>& me) {
			if(me.m_value.has_value()) {
				json["name"] = me.m_value;
			}
		}
	};
#define MAKE_PROPERTY_AAA(type,name)

	
#undef MAKE_PROPERTY_AAA

}
