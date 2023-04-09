#include "pch.h"
#include "StaticEvaluator.h"

// the static eval is from the perspective of the current player
Score StaticEvaluator::Evaluate(const Board& board) {
	PROFILE_SCOPE;
	if (board.GetLegalMoves().empty()) {
		// checkmate
		if (board.IsCheck())
			return LOSS;
			// stalemate
		else
			return 0.f;
	}

	return DefaultEvaluation(board);
}

// always returns a score from the perspective of the current player
Score StaticEvaluator::DefaultEvaluation(const Board& board) {
	Score score = 0;
	for (int i = 0; i < Board::SIZE * Board::SIZE; i++) {
		switch (board[i]) {
			case 'Q': score += 9; break;
			case 'R': score += 5; break;
			case 'B':
			case 'N': score += 3; break;
			case 'P': score += 1; break;

			case 'q': score -= 9; break;
			case 'r': score -= 5; break;
			case 'b':
			case 'n': score -= 3; break;
			case 'p': score -= 1; break;
		}
	}
	// flip the score if we need to, to ensure that the score is from the perspective of the current player
	// a positive score means that the current player is winning
	return score * (board.IsWhiteTurn() ? 1.f : -1.f);
}