/*
MIT License

Copyright (c) 2024 Chi Huu Huynh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef INERTIA_HPP
#define INERTIA_HPP

#pragma once

#include <variant>

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>

#include <inja/inja.hpp>
#include <picosha2.h>

class Inertia
{
private:
	// This is the path to the Inertia.html file. This file is used to render the SPA.
	static const std::string getInertiaHtmlPath() {
		std::string document_root = drogon::app().getDocumentRoot();
		return (std::filesystem::path(std::move(document_root)).parent_path() / "Inertia.html").string();
	}
	class Lazy;
public:
	using StringOrLazy = std::variant<std::string, std::function<std::string()>, Lazy>;

	static Lazy lazy(std::function<std::string()> func) {
		return Lazy(func);
	}

	static drogon::HttpResponsePtr newInertiaResponse(
		const drogon::HttpRequestPtr& req,
		std::unordered_map<std::string, StringOrLazy>& prop_functions,
		const std::string& component,
		const std::string& url
	) {
		if (req->getHeader("X-Requested-With") != "XMLHttpRequest" || req->getHeader("X-Inertia") != "true") {
			const Json::Value json_result = getJsonResult(
				prop_functions,
				component,
				url
			);
			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";

			const std::string output = Json::writeString(std::move(builder), std::move(json_result));

			inja::json inja_json;
			inja_json["data-page"] = std::move(output);

			inja::Environment env;
			std::string result = env.render_file(Inertia::getInertiaHtmlPath(), std::move(inja_json));
			auto response = drogon::HttpResponse::newHttpResponse();
			response->setBody(std::move(result));
			return response;
		}
		const std::string& inertia_version = req->getHeader("X-Inertia-Version");
		if (inertia_version != getAssetVersion()) {
			auto response = drogon::HttpResponse::newHttpResponse();
			response->setStatusCode(drogon::HttpStatusCode::k409Conflict);
			response->addHeader("X-Inertia-Location", url);
			return response;
		}
		const std::string& inertia_partial_component = req->getHeader("X-Inertia-Partial-Component");
		const std::string& inertia_partial_data = req->getHeader("X-Inertia-Partial-Data");
		if (inertia_partial_component != "" && inertia_partial_component == component && inertia_partial_data != "") {
			std::stringstream ss(inertia_partial_data);
			std::vector<std::string> props;
			while (ss.good())
			{
				std::string substr;
				std::getline(ss, substr, ',');
				props.push_back(substr);
			}
			return drogon::HttpResponse::newHttpJsonResponse(
				getJsonResult(
					prop_functions,
					component,
					url,
					std::make_optional<std::vector<std::string>>(props)
				)
			);
		}
		else {
			auto response = drogon::HttpResponse::newHttpJsonResponse(getJsonResult(prop_functions, component, url));
			response->addHeader("Vary", "Accept");
			response->addHeader("X-Inertia", "true");
			return response;
		}
	}
private:
	class Lazy {
	public:
		Lazy(std::function<std::string()> func) : _func(func) {}
		std::string operator()() const {
			return _func();
		}
	private:
		std::function<std::string()> _func;
	};

	static std::string getAssetVersion() {
		std::string src_str = drogon::app().getCustomConfig()["Version"].asString();
		std::vector<unsigned char> hash(picosha2::k_digest_size);
		picosha2::hash256(src_str.begin(), src_str.end(), hash.begin(), hash.end());
		std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
		return hex_str;
	}

	/**
	* @brief Get the Json Result object
	 * 
	 * @param prop_functions Map of prop name and prop value
	 * @param component Name of the component
	 * @param url URL of the page
	 * @param props Partial component props
	 * @return Json::Value 
	 *
	*/
	static Json::Value getJsonResult(
		std::unordered_map<std::string, StringOrLazy> prop_functions,
		const std::string& component,
		const std::string& url,
		std::optional<std::vector<std::string>> props = std::nullopt
	) {
		Json::Value json;
		json["component"] = component;
		json["url"] = url;
		auto asset_version = getAssetVersion();
		json["version"] = asset_version;
		if (props.has_value()) {
			for (auto& prop : props.value()) {
				const auto prop_pair = prop_functions.find(prop);
				if (prop_pair == prop_functions.end()) {
					continue;
				}
				const StringOrLazy& string_or_lazy = prop_pair->second;
				if (std::holds_alternative<std::string>(string_or_lazy)) {
					json["props"][prop_pair->first] = std::get<std::string>(string_or_lazy);
					continue;
				}
				if (std::holds_alternative<std::function<std::string()>>(string_or_lazy)) {
					json["props"][prop_pair->first] = std::get<std::function<std::string()>>(string_or_lazy)();
					continue;
				}
				if (std::holds_alternative<Lazy>(string_or_lazy)) {
					json["props"][prop_pair->first] = std::get<Lazy>(string_or_lazy)();
					continue;
				}
			}
		}
		else {
			for (auto& [prop_name, string_or_lazy] : prop_functions) {
				if (std::holds_alternative<std::string>(string_or_lazy)) {
					json["props"][prop_name] = std::get<std::string>(string_or_lazy);
					continue;
				}
				if (std::holds_alternative<std::function<std::string()>>(string_or_lazy)) {
					json["props"][prop_name] = std::get<std::function<std::string()>>(string_or_lazy)();
					continue;
				}
			}
		}
		return json;
	}
};

#endif