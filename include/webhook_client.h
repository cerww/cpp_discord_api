#pragma once
#include "webhook.h"
#include <string>
#include "requests.h"


struct webhook_client {
	webhook_client() = default;

	webhook_client(snowflake id, std::string token, boost::asio::io_context& ioc) :
		m_id(id),
		m_token(std::move(token)),
		m_ioc(ioc),
		m_strand(m_ioc.value()) {}

	webhook_client(snowflake id, std::string token, boost::asio::io_context::strand& strand) :
		m_id(id),
		m_token(std::move(token)),
		m_ioc(strand.context()),
		m_strand(strand) {}

	webhook_client(snowflake id, std::string token) :
		m_id(id),
		m_token(std::move(token)) {}

	//rq::execute_webhook send();


private:
	snowflake m_id;
	std::string m_token{};

	ref_or_inplace<boost::asio::io_context> m_ioc = std::nullopt;
	ref_or_inplace<boost::asio::io_context::strand> m_strand = std::nullopt;
};
