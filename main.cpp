#include "pch.h"
#include "Chess.h"
#include "Engine.h"
#include "NetworkHandler.h"

int main() {

	Network::Init("172.20.10.2:5000");

	// Network::LoginResponse loginResp = Network::Login("Alban");
	// std::cout << "Login status code " << loginResp.statusCode << std::endl;
	// Player playing = loginResp.player;
	// std::cout << "Playing as " << playing << std::endl;
	// Player playing = Player::White;

	Chess c;
	Engine mm(c);

	{
		std::cout << "Thinking..." << std::endl;
		PROFILE_SCOPE;
		auto [move, score, mate_in] = mm.GetBestMove();
		std::cout << "Sending " << move << "[ " << score << " ]" << std::endl;
	}

	Timer::PrintDurations();

	// while (not c.IsGameOver()) {
	// 	std::cout << c.GetBoard().GetFen() << std::endl;
	// 	if (c.GetBoard().GetCurrentPlayer() == playing) {
	// 		std::cout << "Thinking..." << std::endl;
	// 		auto [move, score, mate_in] = mm.GetBestMove();
	// 		mm.ApplyMove(move);
	// 		std::cout << "Sending " << move << "[ " << score << " ]" << std::endl;
	// 		Network::HttpResponse resp = Network::SendMove(move);
	// 		std::cout << "Status: " << resp.statusCode << std::endl;
	// 		if (resp.statusCode != Network::StatusCode::OK)
	// 			break;
	// 	} else {
	// 		Move move = Network::GetMove();
	// 		std::cout << "Received " << move << std::endl;
	// 		mm.ApplyMove(move);
	// 	}
	// }

	std::cout << c.GetPGN() << std::endl;
}
