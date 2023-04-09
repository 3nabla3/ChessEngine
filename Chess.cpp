#include "pch.h"
#include "Chess.h"

Chess::Chess(const std::string& fen)
	: m_Board(fen)
{}

std::ostream& operator<<(std::ostream& ostream, const Chess& chess) {
	ostream << chess.m_Board.GetFen() << std::endl;
	ostream << chess.m_Board << std::endl;

	return ostream;
}

std::string Chess::TrimFen(const std::string& fen) {
	size_t indexOfSpace = fen.find(' ');
	std::string trimmedFen = fen.substr(0, indexOfSpace);
	return trimmedFen;
}

bool Chess::IsGameOver() const {
	if (m_Board.IsGameOver())
		return true;
	std::string fen = TrimFen(m_Board.GetFen());
	// if the fen appears three times it is a draw
	if (std::count(m_ReachedFens.begin(), m_ReachedFens.end(), fen) >= 3)
		return true;
	return false;
}

Chess& Chess::ApplyMove(const Move& move) {
	if (not IsMoveLegal(move)) {
		std::cout << "Move is not legal!" << std::endl;
		std::cout << move << std::endl;
		exit(1);
	}
	std::stringstream ss;
	if (m_Board.IsWhiteTurn())
		ss << m_Board.GetFullMoves() << ". ";

	m_Board.ApplyMove(move);

	ss << move.from << move.to;
	if (move.promote)
		ss << "=" << move.promote.value();
	ss << " ";
	m_PGN += ss.str();

	m_ReachedFens.push_back(TrimFen(m_Board.GetFen()));
	return *this;
}

const std::vector<Move>& Chess::GetLegalMoves() const {
	return m_Board.GetLegalMoves();
}

bool Chess::IsMoveLegal(const Move& move) const {
	return std::ranges::any_of(m_Board.GetLegalMoves(), [move](const Move& legalMove) {
		return legalMove == move;
	});
}

template<typename Iter, typename RandomGenerator>
static Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
	std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
}

template<typename Iter>
static Iter select_randomly(Iter start, Iter end) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return select_randomly(start, end, gen);
}

Move Chess::GetRandomLegalMove() const {
	const auto& moves = GetLegalMoves();
	return *select_randomly(moves.begin(), moves.end());
}