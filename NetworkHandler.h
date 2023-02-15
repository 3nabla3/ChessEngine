#pragma once
#include <iostream>

enum class ErrorCode {
	OK, IllegalMove, OutOfTime, NotYourTurn, CurlError
};

class NetworkHandler {
public:
	static void Init(const std::string& hostnamePort);
	static ErrorCode SendMove(const std::string& move);
	static int GetTimeSecondsLeft();

private:
	static std::string s_SubmitMoveEndpoint;
	static std::string s_GetTimeLeftEndpoint;
};