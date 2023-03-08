#pragma once
#include <iostream>
#include "Chess.h"

enum class StatusCode {
	OK, IllegalMove, OutOfTime, NotYourTurn, Unprocessable, NotLoggedIn, GameFull, CurlError
};

static std::ostream &operator<<(std::ostream &os, const StatusCode &status) {
	if (status == StatusCode::OK)
		os << "OK" << std::endl;
	else if (status == StatusCode::IllegalMove)
		os << "IllegalMove" << std::endl;
	else if (status == StatusCode::OutOfTime)
		os << "OutOfTime" << std::endl;
	else if (status == StatusCode::NotYourTurn)
		os << "NotYourTurn" << std::endl;
	else if (status == StatusCode::Unprocessable)
		os << "Unprocessable" << std::endl;
	else if (status == StatusCode::NotLoggedIn)
		os << "NotLoggedIn" << std::endl;
	else if (status == StatusCode::GameFull)
		os << "GameFull" << std::endl;
	else if (status == StatusCode::CurlError)
		os << "CurlError" << std::endl;
	return os;
}

typedef struct {
	std::string response;
	StatusCode statusCode;
} HttpResponse;

typedef struct {
	Player player;
	StatusCode statusCode;
} LoginResponse;

class NetworkHandler {
public:
	static NetworkHandler& Get() {
		if (!s_Instance)
			s_Instance = new NetworkHandler();
		return *s_Instance;
	}

	void Init(const std::string& socket);
	int GetTimeSecondsLeft(Player player);
	LoginResponse Login(const std::string& username);

	HttpResponse SendMove(const std::string& move);
	HttpResponse SendMove(const Move& move) { return SendMove(Move2Chess(move)); }
	Move GetMove();

private:
	HttpResponse Post(const std::string& endpoint, const std::string& data);
	static StatusCode Int2Status(long status);

	std::string m_SubmitMoveEndpoint;
	std::string m_GetTimeLeftEndpoint;
	std::string m_LoginEndpoint;
	std::string m_GetMoveEndpoint;

	bool m_Verbose = false;
	std::list<std::string> m_CookieJar;

	static NetworkHandler* s_Instance;
};
