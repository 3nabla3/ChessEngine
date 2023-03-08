#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include "NetworkHandler.h"

NetworkHandler* NetworkHandler::s_Instance = nullptr;

void NetworkHandler::Init(const std::string& socket) {
	std::string prefix = "http://" + socket;

	m_SubmitMoveEndpoint = prefix + "/submitmove";
	m_GetTimeLeftEndpoint = prefix + "/gettimeleft";
	m_LoginEndpoint = prefix + "/login";
	m_GetMoveEndpoint = prefix + "/getmove";
}

LoginResponse NetworkHandler::Login(const std::string& username) {
	std::string postData("user=" + username);
	HttpResponse resp = Post(m_LoginEndpoint, postData);
	return {resp.response == "white" ? Player::White : Player::Black , resp.statusCode};
}

HttpResponse NetworkHandler::SendMove(const std::string& move) {
	std::string postData = std::string("from=") + move[0] + move[1] + "&to=" + move[2] + move[3];
	if (move.length() == 5)
		postData = postData + "&promote=" + move[4];
	return Post(m_SubmitMoveEndpoint, postData);
}

Move NetworkHandler::GetMove() {
	cURLpp::Easy request;
	request.setOpt(new cURLpp::options::Url(m_GetMoveEndpoint));
	request.setOpt(new cURLpp::options::Verbose(m_Verbose));

	std::stringstream resp;
	request.setOpt(new curlpp::options::WriteStream(&resp));

	// attach the cookies to the request
	request.setOpt(new cURLpp::options::CookieFile("")); // for some reason it breaks if removed
	request.setOpt(new cURLpp::options::CookieSession(true));
	for (const auto& cookie: m_CookieJar)
		request.setOpt(new cURLpp::options::CookieList(cookie));

	// do your thing
	request.perform();

	return Chess2Move(resp.str());
}

int NetworkHandler::GetTimeSecondsLeft(Player player) {
	// initialize the request
	cURLpp::Easy request;

	std::string endpoint = m_GetTimeLeftEndpoint +
			"?player=" + ( player == Player::White ? "w" : "b");

	request.setOpt(new cURLpp::options::Url(endpoint));
	request.setOpt(new cURLpp::options::Verbose(m_Verbose));

	std::stringstream resp;
	request.setOpt(new curlpp::options::WriteStream(&resp));

	// do your thing
	request.perform();

	// extract the response code
	auto statusCode = cURLpp::infos::ResponseCode::get(request);
	if (statusCode != 200)
		return -1;

	// return the response
	return std::stoi(resp.str());
}

StatusCode NetworkHandler::Int2Status(long status) {
	if (status == 200)
		return StatusCode::OK;
	if (status == 409)
		return StatusCode::IllegalMove;
	if (status == 422)
		return StatusCode::Unprocessable;
	if (status == 408)
		return StatusCode::OutOfTime;
	if (status == 425)
		return StatusCode::NotYourTurn;
	if (status == 401)
		return StatusCode::NotLoggedIn;
	if (status == 423)
		return StatusCode::GameFull;

	return StatusCode::CurlError;
}

HttpResponse NetworkHandler::Post(const std::string& endpoint, const std::string& data) {
	// initialize the request
	cURLpp::Easy request;
	request.setOpt(new cURLpp::options::Url(endpoint));
	request.setOpt(new cURLpp::options::Verbose(m_Verbose));

	// set the headers
	std::list<std::string> header;
	header.emplace_back("Content-Type: application/x-www-form-urlencoded");
	request.setOpt(new cURLpp::options::HttpHeader(header));

	// add the post fields
	request.setOpt(new cURLpp::options::PostFields(data));
	request.setOpt(new curlpp::options::PostFieldSize(static_cast<long>(data.size())));

	// throw out the response
	std::stringstream playerColor;
	request.setOpt(new curlpp::options::WriteStream(&playerColor));

	// attach the cookies to the request
	request.setOpt(new cURLpp::options::CookieFile("")); // for some reason it breaks if removed
	request.setOpt(new cURLpp::options::CookieSession(true));
	for (const auto& cookie: m_CookieJar)
		request.setOpt(new cURLpp::options::CookieList(cookie));

	// do your thing
	request.perform();

	// extract the cookies
	cURLpp::infos::CookieList::get(request, m_CookieJar);

	// extract the response code
	auto statusCode = cURLpp::infos::ResponseCode::get(request);
	return {playerColor.str(), Int2Status(statusCode)};
}
