#pragma once
#include <nlohmann/json.hpp>
#include "timestamp.h"

//i can prolly use mixins to do this in half as many lines

namespace cacheless {
struct embed_footer {

	embed_footer& set_text(std::string s) {
		text = std::move(s);
		return *this;
	}

	embed_footer& set_icon_url(std::string s) {
		icon_url = std::move(s);
		return *this;
	}

	embed_footer& set_proxy_icon_url(std::string s) {
		proxy_icon_url = std::move(s);
		return *this;
	}

	std::string text;
	std::optional<std::string> icon_url;
	std::optional<std::string> proxy_icon_url;

};

inline void from_json(const nlohmann::json& json, embed_footer& footer) {
	footer.text = json["text"].get<std::string>();
	footer.icon_url = json.value("icon_url", std::optional<std::string>());
	footer.proxy_icon_url = json.value("proxy_icon_url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& out, const embed_footer& footer) {
	out["text"] = footer.text;
	//no optional support yet
	if (footer.icon_url.has_value()) {
		out["icon_url"] = footer.icon_url.value();
	}
	if (footer.proxy_icon_url.has_value()) {
		out["proxy_icon_url"] = footer.proxy_icon_url.value();
	}
}

struct embed_image {


	embed_image& set_url(std::string s) noexcept {
		url = std::move(s);
		return *this;
	}

	embed_image& set_proxy_url(std::string s) noexcept {
		proxy_url = std::move(s);
		return *this;
	}

	embed_image& set_height(int s) noexcept {
		height = s;
		return *this;
	}

	embed_image& set_width(int s) noexcept {
		width = s;
		return *this;
	}

	std::optional<std::string> url;
	std::optional<std::string> proxy_url;
	std::optional<int> height;
	std::optional<int> width;

};

inline void from_json(const nlohmann::json& json, embed_image& image) {
	image.url = json.value("url", std::optional<std::string>());
	image.proxy_url = json.value("proxy_url", std::optional<std::string>());
	image.height = json.value("height", std::optional<int>());
	image.width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_image& image) {
	if (image.url.has_value()) {
		json["url"] = image.url.value();
	}
	if (image.proxy_url.has_value()) {
		json["proxy_url"] = image.proxy_url.value();
	}
	if (image.height.has_value()) {
		json["height"] = image.height.value();
	}
	if (image.width.has_value()) {
		json["width"] = image.width.value();
	}
}

struct embed_thumbnail {

	embed_thumbnail& set_url(std::string s) noexcept {
		url = std::move(s);
		return *this;
	}

	embed_thumbnail& set_proxy_url(std::string s) noexcept {
		proxy_url = std::move(s);
		return *this;
	}

	embed_thumbnail& set_height(int s) noexcept {
		height = s;
		return *this;
	}

	embed_thumbnail& set_width(int s) noexcept {
		width = s;
		return *this;
	}

	std::optional<std::string> url;
	std::optional<std::string> proxy_url;
	std::optional<int> height;
	std::optional<int> width;
	
};

inline void from_json(const nlohmann::json& json, embed_thumbnail& thumbnail) {
	thumbnail.url = json.value("url", std::optional<std::string>());
	thumbnail.proxy_url = json.value("proxy_url", std::optional<std::string>());
	thumbnail.height = json.value("height", std::optional<int>());
	thumbnail.width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_thumbnail& thumbnail) {
	if (thumbnail.url.has_value()) {
		json["url"] = thumbnail.url.value();
	}
	if (thumbnail.proxy_url.has_value()) {
		json["proxy_url"] = thumbnail.proxy_url.value();
	}
	if (thumbnail.height.has_value()) {
		json["height"] = thumbnail.height.value();
	}
	if (thumbnail.width.has_value()) {
		json["width"] = thumbnail.width.value();
	}
}

struct embed_video {

	embed_video& set_url(std::string s) noexcept {
		url = std::move(s);
		return *this;
	}

	embed_video& set_height(int s) noexcept {
		height = s;
		return *this;
	}

	embed_video& set_width(int s) noexcept {
		width = s;
		return *this;
	}

	std::optional<std::string> url;
	std::optional<int> height;
	std::optional<int> width;

};


inline void from_json(const nlohmann::json& json, embed_video& video) {
	video.url = json.value("url", std::optional<std::string>());
	video.height = json.value("height", std::optional<int>());
	video.width = json.value("width", std::optional<int>());
}

inline void to_json(nlohmann::json& json, const embed_video& video) {
	if (video.url.has_value()) {
		json["url"] = video.url.value();
	}
	if (video.height.has_value()) {
		json["height"] = video.height.value();
	}
	if (video.width.has_value()) {
		json["width"] = video.width.value();
	}
}

struct embed_provider {
	embed_provider& set_name(std::string s) {
		name = std::move(s);
		return *this;
	}

	embed_provider& set_url(std::string s) {
		url = std::move(s);
		return *this;
	}

	std::optional<std::string> name;
	std::optional<std::string> url;

};

inline void from_json(const nlohmann::json& json, embed_provider& provider) {
	provider.name = json.value("name", std::optional<std::string>());
	provider.url = json.value("url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& json, const embed_provider& provider) {
	if (provider.name.has_value()) {
		json["name"] = provider.name.value();
	}
	if (provider.url.has_value()) {
		json["url"] = provider.url.value();
	}
}

struct embed_author {
	embed_author& set_name(std::string s) noexcept {
		name = std::move(s);
		return *this;
	}

	embed_author& set_url(std::string s) noexcept {
		url = std::move(s);
		return *this;
	}

	embed_author& set_icon_url(std::string s) noexcept {
		icon_url = std::move(s);
		return *this;
	}

	embed_author& set_proxy_icon_url(std::string s) noexcept {
		proxy_icon_url = std::move(s);
		return *this;
	}

	std::optional<std::string> name;
	std::optional<std::string> url;
	std::optional<std::string> icon_url;
	std::optional<std::string> proxy_icon_url;

};

inline void from_json(const nlohmann::json& data, embed_author& author) {
	author.name = data.value("name", std::optional<std::string>());
	author.url = data.value("url", std::optional<std::string>());
	author.icon_url = data.value("icon_url", std::optional<std::string>());
	author.proxy_icon_url = data.value("proxy_icon_url", std::optional<std::string>());
}

inline void to_json(nlohmann::json& json, const embed_author& author) {
	if (author.name.has_value()) {
		json["name"] = author.name.value();
	}
	if (author.url.has_value()) {
		json["url"] = author.url.value();
	}
	if (author.icon_url.has_value()) {
		json["icon_url"] = author.icon_url.value();
	}
	if (author.proxy_icon_url.has_value()) {
		json["proxy_icon_url"] = author.proxy_icon_url.value();
	}
}

struct embed_field {
	embed_field& set_name(std::string new_name) {
		name = std::move(new_name);
		return *this;
	}

	embed_field& set_value(std::string new_name) {
		value = std::move(new_name);
		return *this;
	}

	embed_field& set_is_inline(bool value) {
		is_inline = value;
		return *this;
	}

	std::string name;
	std::string value;
	std::optional<bool> is_inline;

};

inline void from_json(const nlohmann::json& json, embed_field& field) {
	field.name = json["name"].get<std::string>();
	field.value = json["value"].get<std::string>();
	field.is_inline = json.value("inline", std::optional<bool>());
}

inline void to_json(nlohmann::json& json, const embed_field& field) {
	json["name"] = field.name;
	json["value"] = field.value;
	if (field.is_inline.has_value()) {
		json["inline"] = field.is_inline.value();
	}
}

struct embed {
	embed& set_title(std::string s) {
		title = std::move(s);
		return *this;
	}

	embed& set_description(std::string s) {
		description = std::move(s);
		return *this;
	}

	embed& set_url(std::string s) {
		url = std::move(s);
		return *this;
	}

	embed& set_timestamp(timestamp s) {
		timestamp = s;
		return *this;
	}

	embed& set_color(int s) {
		color = s;
		return *this;
	}

	embed& set_footer(embed_footer s) {
		footer = std::move(s);
		return *this;
	}

	embed& set_image(embed_image s) {
		image = std::move(s);
		return *this;
	}

	embed& set_thumbnail(embed_thumbnail s) {
		thumbnail = std::move(s);
		return *this;
	}

	embed& set_video(embed_video s) {
		video = std::move(s);
		return *this;
	}

	embed& set_provider(embed_provider s) {
		provider = std::move(s);
		return *this;
	}

	embed& set_author(embed_author s) {
		author = std::move(s);
		return *this;
	}

	template<typename ...fields>
	embed& add_fields(fields ... s) {
		if (!this->fields.has_value()) {
			this->fields.emplace();
		}
		(this->fields.value().emplace_back(std::move(s)), ...);
		return *this;
	}

	std::optional<std::string> title;
	std::optional<std::string> description;
	std::optional<std::string> url;
	std::optional<timestamp> timestamp;
	std::optional<int> color;
	std::optional<embed_footer> footer;
	std::optional<embed_image> image;
	std::optional<embed_thumbnail> thumbnail;
	std::optional<embed_video> video;
	std::optional<embed_provider> provider;
	std::optional<embed_author> author;
	std::optional<std::vector<embed_field>> fields;
};

static constexpr int jashdasdasd = sizeof(embed);

inline void from_json(const nlohmann::json& json, embed& embed) {
	embed.title = json.value("title", std::optional<std::string>());
	embed.description = json.value("description", std::optional<std::string>());
	embed.url = json.value("url", std::optional<std::string>());
	//embed.m_timestamp = json.value("timestamp", std::optional<::timestamp>());
	embed.color = json.value("color", std::optional<int>());
	embed.footer = json.value("footer", std::optional<embed_footer>());
	embed.image = json.value("image", std::optional<embed_image>());
	embed.author = json.value("author", std::optional<embed_author>());
	embed.thumbnail = json.value("thumbnail", std::optional<embed_thumbnail>());
	embed.video = json.value("video", std::optional<embed_video>());
	embed.provider = json.value("provider", std::optional<embed_provider>());
	//embed.m_fields = json.value("fields", std::optional<std::vector<embed_field>>());
	//std::optional support is ;-; rn

	const auto has_fields_it = json.find("fields");
	if (has_fields_it != json.end()) {
		embed.fields = has_fields_it->get<std::vector<embed_field>>();
	}

}

inline void to_json(nlohmann::json& json, const embed& embed) {
	if (embed.title.has_value()) {
		json["title"] = embed.title.value();
	}
	if (embed.description.has_value()) {
		json["description"] = embed.description.value();
	}
	if (embed.url.has_value()) {
		json["url"] = embed.url.value();
	}
	if (embed.author.has_value()) {
		json["author"] = embed.author.value();
	}
	if (embed.footer.has_value()) {
		json["footer"] = embed.footer.value();
	}
	if (embed.provider.has_value()) {
		json["provider"] = embed.provider.value();
	}
	if (embed.video.has_value()) {
		json["video"] = embed.video.value();
	}
	if (embed.fields.has_value()) {
		json["fields"] = embed.fields.value();
	}
	if (embed.color.has_value()) {
		json["color"] = embed.color.value();
	}
	if (embed.image.has_value()) {
		json["image"] = embed.image.value();
	}
	if (embed.thumbnail.has_value()) {
		json["thumbnail"] = embed.thumbnail.value();
	}
	/*
	if (embed.m_timestamp.has_value()) {
		json["author"] = embed.author().value();
	}
	*/

}

/*


 */

}

namespace thingy_workyausdhasjdl {
template<typename tag, template<typename> typename ...Property>
struct embed_object :Property<embed_object<tag, Property...>>... {

	friend void from_json(const nlohmann::json&, embed_object<tag, Property...>&);
	friend void to_json(nlohmann::json&, const embed_object<tag, Property...>&);
};

template<typename Obj_t>
struct name_property {
	std::optional<std::string_view> name() const noexcept { return m_value; };

	Obj_t& set_name(std::string s) {
		m_value = std::move(s);
		return (Obj_t&)(*this);
	}

private:
	std::optional<std::string> m_value;

	friend void from_json(const nlohmann::json& json, name_property<Obj_t>& me) {
		me.m_value = json.value("name", std::optional<std::string>());
	}

	friend void to_json(nlohmann::json& json, const name_property<Obj_t>& me) {
		if (me.m_value.has_value()) {
			json["name"] = me.m_value;
		}
	}
};

#define MAKE_PROPERTY_AAA(type,name)


#undef MAKE_PROPERTY_AAA

}
