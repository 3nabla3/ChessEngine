#include "Board.h"

typedef float Score;

class StaticEvaluator {
public:
	[[nodiscard]] static Score Evaluate(const Board& board);

	constexpr static Score LOSS = -1000;
	constexpr static Score WIN =   1000;
private:
	[[nodiscard]] static Score DefaultEvaluation(const Board& board);
	float m_LegalMovesNumWeight = 0.5f;
};
