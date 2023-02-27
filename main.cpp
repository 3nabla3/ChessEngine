#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	// NetworkHandler::Get().Init("localhost:5000");

	Chess c("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w qKQ - 0 1");
	// Chess c;
	std::cout << c << std::endl;
	// c.ApplyMove("e2e4");
	// std::cout << c << std::endl;
	// c.ApplyMove("d4e3");
	// std::cout << c << std::endl;
	//
	// c.ApplyMove("h2h4");
	// std::cout << c << std::endl;
	// c.ApplyMove("b7b5");
	// std::cout << c << std::endl;
	// c.ApplyMove("c5b6");
	// std::cout << c << std::endl;

	// srand(time(nullptr));
	//
	// for (int games = 0; games < 100; games++) {
	// 	Chess c;
	//
	// 	for (int i = 0; i < 200; i++) {
	// 		auto moves = c.GetLegalMoves();
	//
	// 		if (moves.empty())
	// 			break;
	//
	// 		int randMoveIndex = rand() % moves.size();
	//
	// 		int j = 0;
	// 		Move chosen;
	// 		for (const auto& move: moves) {
	// 			if (j++ == randMoveIndex) {
	// 				chosen = move;
	// 				break;
	// 			}
	// 		}
	//
	// 		c.ApplyMove(chosen);
	// 	}
	// 	std::cout << c.GetPGN() << std::endl;
	// }

	return 0;
}
