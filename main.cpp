#include "pch.h"
#include "Chess.h"
#include "NetworkHandler.h"

int main() {
	// NetworkHandler::Get().Init("localhost:5000");

	// Chess c("rnbqkbnr/ppp1pppp/8/2P5/3p4/8/PP1PPPPP/RNBQKBNR w KQkq - 0 1");
	// c.ApplyMove(Chess2Move("e2e4"));
	// std::cout << c << std::endl;
	// c.ApplyMove(Chess2Move("d4e3"));
	// std::cout << c << std::endl;
	//
	// c.ApplyMove(Chess2Move("h2h4"));
	// std::cout << c << std::endl;
	// c.ApplyMove(Chess2Move("b7b5"));
	// std::cout << c << std::endl;
	// c.ApplyMove(Chess2Move("c5b6"));
	// std::cout << c << std::endl;

	srand(time(nullptr));

	for (int games = 0; games < 100; games++) {
		Chess c;

		for (int i = 0; i < 200; i++) {
			auto moves = c.GetLegalMoves();

			if (moves.empty())
				break;

			int randMoveIndex = rand() % moves.size();

			int j = 0;
			Move chosen;
			for (const auto& move: moves) {
				if (j++ == randMoveIndex) {
					chosen = move;
					break;
				}
			}

			c.ApplyMove(chosen);
		}
		std::cout << c.GetPGN() << std::endl;
	}

	return 0;
}
