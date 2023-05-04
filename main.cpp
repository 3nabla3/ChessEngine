#include "pch.h"
#include "Chess.h"
#include "Engine.h"
#include "NetworkHandler.h"
#include "BoardOptimized.h"

// consteval std::array<uint64_t, 64> GenerateValues() {
// 	std::array<uint64_t, 64> values{};
// 	for (int i = 0; i < 64; i++) {
// 		values[i] = 1ULL << i;
// 	}
// 	return values;
// }
//
// std::array<uint64_t, 64> values = GenerateValues();
//



int main() {
	BoardOptimized b("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	BoardOptimized::PrintBitBoard(b.GetWhiteKnights());

// 	Network::Init("172.20.10.2:5000");
// 	Network::LoginResponse r = Network::Login("alban");
// 	std::cout << "Playing as " << r.player << '\n';
// 	Player player = r.player;
//
// 	Chess c;
// 	Engine mm(c);
//
// 	while (not c.IsGameOver()) {
// 		std::cout << c << '\n';
// 		if (c.GetBoard().GetCurrentPlayer() == player) {
// 			auto move = c.GetRandomLegalMove();
// 			std::cout << "Playing " << move << '\n';
// 			auto resp = Network::SendMove(move);
// 			if (resp.statusCode != Network::StatusCode::OK)
// 				std::cout << "ERROR: " << resp.statusCode << '\n';
// 			mm.ApplyMove(move);
// 		}
// 		else {
// 			Move move = Network::GetMove();
// 			std::cout << "Received " << move << '\n';
// 			mm.ApplyMove(move);
// 		}
// 	}
//
// 	std::cout << c.GetPGN() << '\n';
}

