#include "pch.h"
#include "Chess.h"

Chess::Chess(const std::string& fen)
	: m_Board(BoardFromFen(fen)), m_Playing(fen.contains(" w ") ? Player::White : Player::Black),
	m_EnPassant(ParseEnPassantFromFen(fen)),
	m_WhiteCastlingRights(ParseCastlingRightsFromFen(fen, Player::White)),
	m_BlackCastlingRights(ParseCastlingRightsFromFen(fen, Player::Black))
{}

std::ostream& operator<<(std::ostream& ostream, const Chess& chess) {
	ostream << chess.GetFen() << std::endl;
	ostream << chess.m_Board << std::endl;

	return ostream;
}

std::string Chess::GetFen() const {
	std::string fen;
	int spaceCount = 0;
	for (int i = 0; i < m_Board.size(); i++) {
		char piece = m_Board[i];

		if (piece != ' ') {
			if (spaceCount) {
				fen += std::to_string(spaceCount);
				spaceCount = 0;
			}
			fen += piece;
		} else
			spaceCount++;

		if (i % 8 == 7) {
			if (spaceCount) {
				fen += std::to_string(spaceCount);
				spaceCount = 0;
			}
			if (i < m_Board.size() - 1)
				fen += '/';
		}
	}

	if (m_Playing == Player::White)
		fen += " w";
	else
		fen += " b";

	fen += " ";
	if (m_WhiteCastlingRights.first)
		fen += "K";
	if (m_WhiteCastlingRights.second)
		fen += "Q";
	if (m_BlackCastlingRights.first)
		fen += "k";
	if (m_BlackCastlingRights.second)
		fen += "q";
	if (m_WhiteCastlingRights == std::pair(false, false) and
		m_BlackCastlingRights == std::pair(false, false))
		fen += "-";

	if (m_EnPassant)
		fen += " " + Coord2Chess(m_EnPassant.value());
	else
		fen += " -";

	// deal w halfmove
	fen += " " + std::to_string(m_halfMovesRule);

	// deal with fullmove number
	fen += " " + std::to_string(m_fullMoves);

	return fen;
}

Board Chess::BoardFromFen(const std::string& fen) {
	Board board{};

	int index = 0;
	for (auto it = fen.begin(); *it != ' '; it++) {
		if (*it == '/') continue;
		if ('1' <= *it && *it <= '8')
			for (int i = 0; i < *it - '0'; i++)
				board[index++] = ' ';
		else
			board[index++] = *it;
	}

	return board;
}

const std::list<Move>& Chess::GetLegalMoves() const {
	// if this was already calculated
	if (!m_LegalMovesCache.empty())
		return m_LegalMovesCache;

	const CastlingRights& castlingRights = GetCurrentPlayer() == Player::White ? m_WhiteCastlingRights : m_BlackCastlingRights;
	m_LegalMovesCache = m_Board.GetPseudoLegalMoves(GetCurrentPlayer(), m_EnPassant, castlingRights);

	auto it = m_LegalMovesCache.begin();
	while (it != m_LegalMovesCache.end()) {
		Board copy = m_Board;
		copy.ApplyMove(*it);

		// if the resulting move makes a check, remove it
		if (copy.IsCheck(GetCurrentPlayer()))
			it = m_LegalMovesCache.erase(it); // catch the new iterator
		else
			it++;

		// TODO: check for castling through cemplace_backheck
		// // if the move is castling and the king is in check, remove it
		// if (fromPiece == 'K' and std::abs(it->from.first - it->to.first) == 2)
		// 	if (IsCheck())
		// 		it = m_LegalMovesCache.erase(it); // catch the new iterator
	}

	return m_LegalMovesCache;
}

// bool Chess::IsCurrentPlayerPiece(char piece) const {
// 	return IsPlayerPiece(piece, GetCurrentPlayer());
// }

std::optional<Coord> Chess::ParseEnPassantFromFen(const std::string& fen) {
	// if we split the fen by spaces, what index is the EP info
	int indexInFen = 3;
	// current in the split array
	int index = 0;
	// current index of char in string
	int charIndex = 0;

	while (index < indexInFen) {
		if (fen[charIndex++] == ' ')
			index++;
	}

	// if the fen has the info
	if (fen[charIndex] != '-')
		return Chess2Coord(&fen[charIndex]);

	return {};
}

CastlingRights Chess::ParseCastlingRightsFromFen(const std::string& fen, Player player) {
	// if we split the fen by spaces, what index is the Castling info
	int indexInFen = 2;
	// current in the split array
	int index = 0;
	// current index of char in string
	int charIndex = 0;

	while (index < indexInFen) {
		if (fen[charIndex++] == ' ')
			index++;
	}

	CastlingRights cr{false, false};
	// if there is no castling rights
	if (fen[charIndex] == '-')
		return cr;

	// iterate over the castling rights
	while (fen[charIndex] != ' ') {
		if (fen[charIndex] == 'K' and player == Player::White ||
			fen[charIndex] == 'k' and player == Player::Black)
			cr.first = true;
		else if (fen[charIndex] == 'Q' and player == Player::White ||
				 fen[charIndex] == 'q' and player == Player::Black)
			cr.second = true;
		charIndex++;
	}
	return cr;
}

Chess& Chess::ApplyMove(const Move& move) {
	if (!IsMoveLegal(move)) {
		std::cout << "Move is not legal!" << std::endl;
		std::cout << move << std::endl;
		exit(1);
	}

	const auto& from = move.from;
	const auto& to = move.to;

	char& pieceToMove = m_Board.GetPieceRef(from);
	char& pieceToReplace = m_Board.GetPieceRef(to);

	// check if a pawn was moved or if a piece was taken
	if (pieceToMove == 'p' or pieceToMove == 'P' or pieceToReplace != ' ')
		m_halfMovesRule = 0;
	else
		m_halfMovesRule++;

	// if a white pawn moved two ranks
	if (pieceToMove == 'P' and from.second == 1 and to.second == 3) {
		// if there are black pawns on the side
		if (to.first - 1 >= 0 and m_Board.GetPiece(to.first - 1, 3) == 'p' ||
			to.first + 1 <= 7 and m_Board.GetPiece(to.first + 1, 3) == 'p')
			m_EnPassant = Coord({to.first, 2});
	} else if (pieceToMove == 'p' and from.second == 6 and to.second == 4) {
		if (to.first - 1 >= 0 and m_Board.GetPiece(to.first - 1, 4) == 'P' ||
			to.first + 1 <= 7 and m_Board.GetPiece(to.first + 1, 4) == 'P')
			m_EnPassant = Coord({to.first, 5});
	} else {
		m_EnPassant = {};
	}

	// the king or the rooks move, castling is not allowed anymore
	if (pieceToMove == 'K')
		m_WhiteCastlingRights = {false, false};
	else if (pieceToMove == 'k')
		m_BlackCastlingRights = {false, false};
	else if (pieceToMove == 'R') {
		if (from == Coord({0, 0}))
			m_WhiteCastlingRights.second = false;
		else if (from == Coord({7, 0}))
			m_WhiteCastlingRights.first = false;
	} else if (pieceToMove == 'r') {
		if (from == Coord({0, 7}))
			m_BlackCastlingRights.second = false;
		else if (from == Coord({7, 7}))
			m_BlackCastlingRights.first = false;
	}

	m_Board.ApplyMove(move);
	m_LegalMovesCache.clear();

	std::stringstream ss;
	// keep track of the full moves
	if (m_Playing == Player::Black)
		m_fullMoves++;
	else
		ss << m_fullMoves << ". ";

	ss << move.from << move.to;
	if (move.promote)
		ss << "=" << move.promote.value();
	ss << " ";

	m_PGN += ss.str();
	m_Playing = GetNotCurrentPlayer();

	return *this;
}

bool Chess::IsMoveLegal(const Move& move) const {
	return std::ranges::any_of(GetLegalMoves(), [move](const Move& legalMove) {
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