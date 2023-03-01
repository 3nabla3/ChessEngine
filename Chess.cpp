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

std::list<Move> Chess::GetLegalMovesForPiece(int col, int row) const {
	std::list<Move> legalMoves;

	switch (GetPiece(col, row)) {
		case 'r':
		case 'R':
			return GetPseudoLegalMovesRook(col, row);
		case 'n':
		case 'N':
			return GetPseudoLegalMovesKnight(col, row);
		case 'b':
		case 'B':
			return GetPseudoLegalMovesBishop(col, row);
		case 'q':
		case 'Q':
			return GetPseudoLegalMovesQueen(col, row);
		case 'k':
		case 'K':
			return GetPseudoLegalMovesKing(col, row);
		case 'p':
		case 'P':
			return GetPseudoLegalMovesPawn(col, row);
		default:
			std::cout << "This should never happen! (Invalid Piece)" << std::endl;
	}

	// should never reach this code
	return {};
}

std::list<Move> Chess::GetPseudoLegalMovesRook(int col, int row) const {
	std::list<Move> moves;

	// scan to the right
	for (int xd = 1; xd < SIZE - col; xd++) {
		char piece = GetPiece(col + xd, row);

		// if there is a piece blocking the scan, the scan stops
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + xd, row}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			 break;
	}

	// scan up
	for (int yd = 1; yd < SIZE - row; yd++) {
		char piece = GetPiece(col, row + yd);

		// if there is a piece blocking the scan, the scan stops
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col, row + yd}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan to the left
	for (int xd = -1; xd >= -col; xd--) {
		char piece = GetPiece(col + xd, row);

		// if there is a piece blocking the scan, the scan stops
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + xd, row}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down
	for (int yd = -1; yd >= -row; yd--) {
		char piece = GetPiece(col, row + yd);

		// if there is a piece blocking the scan, the scan stops
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col, row + yd}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::list<Move> Chess::GetPseudoLegalMovesKnight(int col, int row) const {
	std::list<Move> moves;

	// check the moves one up and one down from the squares two left and two right
	for (int xd = -2; xd <= 2; xd += 4)
		for (int yd = -1; yd <= 1; yd+=2) {
			int x = col + xd;
			int y = row + yd;
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsCurrentPlayerPiece(GetPiece(x, y)))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	// check the moves one left and one right from the squares two up and two down
	for (int xd = -1; xd <= 1; xd += 2)
		for (int yd = -2; yd <= 2; yd+=4) {
			int x = col + xd;
			int y = row + yd;
			// if the position is in bounds
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsCurrentPlayerPiece(GetPiece(x, y)))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	return moves;
}

std::list<Move> Chess::GetPseudoLegalMovesBishop(int col, int row) const {
	std::list<Move> moves;

	// scan up right
	for (int d = 1; col + d < 8 and row + d < 8; d++) {
		char piece = GetPiece(col + d, row + d);
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan up left
	for (int d = 1; col - d >= 0 and row + d < 8; d++) {
		char piece = GetPiece(col - d, row + d);
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down right
	for (int d = 1; col + d < 8 and row - d >= 0; d++) {
		char piece = GetPiece(col + d, row - d);
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down left
	for (int d = 1; col - d >= 0 and row - d >= 0; d++) {
		char piece = GetPiece(col - d, row - d);
		if (IsCurrentPlayerPiece(piece))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::list<Move> Chess::GetPseudoLegalMovesQueen(int col, int row) const {
	// get the rook moves
	std::list<Move> moves = GetPseudoLegalMovesRook(col, row);

	// append the bishop moves
	const std::list<Move>& temp = GetPseudoLegalMovesBishop(col, row);
	moves.insert(moves.end(), temp.begin(), temp.end());

	return moves;
}

std::list<Move> Chess::GetPseudoLegalMovesKing(int col, int row) const {
	std::list<Move> moves;

	for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++) {
			int x = col + dx;
			int y = row + dy;
			// if the position is in bounds
			if (0 <= x and x < 8 and 0 <= y and y < 8) {
				if (!IsCurrentPlayerPiece(GetPiece(col + dx, row + dy)))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
			}
		}

	// check castling
	if (GetCurrentPlayer() == Player::White) {
		// check white king side castling
		if (m_WhiteCastlingRights.first and GetPiece(5, 0) == ' ' and GetPiece(6, 0) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({6, 0}));
		// check white queen side castling
		if (m_WhiteCastlingRights.second and GetPiece(1, 0) == ' ' and GetPiece(2, 0) == ' ' and GetPiece(3, 0) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({2, 0}));
	} else {
		// check black king side castling
		if (m_BlackCastlingRights.first and GetPiece(5, 7) == ' ' and GetPiece(6, 7) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({6, 7}));
		// check black queen side castling
		if (m_BlackCastlingRights.second and GetPiece(1, 7) == ' ' and GetPiece(2, 7) == ' ' and GetPiece(3, 7) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({2, 7}));
	}
	return moves;
}

std::list<Move> Chess::GetPseudoLegalMovesPawn(int col, int row) const {
	std::list<Move> moves;

	// Get some important information depending on the color playing
	int dy;
	int startingRow;

	if (GetCurrentPlayer() == Player::White) {
		startingRow = 1;
		dy = 1;
	} else {
		startingRow = 6;
		dy = -1;
	}

	// check one move ahead if the pawn can move
	char piece = GetPiece(col, row + dy);
	if (piece == ' ') {
		// if white can promote
		if (row + dy == 7)
			for (char promote : std::string("QRBN"))
				moves.emplace_back(Coord({col, row}), Coord({col, row + dy}), promote);
		// if black can promote
		else if (row + dy == 0)
			for (char promote : std::string("qrbn"))
				moves.emplace_back(Coord({col, row}), Coord({col, row + dy}), promote);
		// if no one can promote
		else
			moves.emplace_back(Coord({col, row}), Coord({col, row + dy}));

		// if the pawn is on the starting line, check an additional move ahead
		if (row == startingRow) {
			piece = GetPiece(col, row + dy * 2);
			if (piece == ' ')
				moves.emplace_back(Coord({col, row}), Coord({col, row + dy * 2}));
		}
	}

	// check if the pawn can take
	for (int x = col - 1; x <= col + 1; x += 2) {
		// make sure we are not checking out of bounds
		if (x > 7 or x < 0) continue;

		int y = row + dy;
		piece = GetPiece(x, y);
		// pawns can only move diag if they are taking a piece
		if (!IsCurrentPlayerPiece(piece) and piece != ' ') {
			// if white can promote
			if (row + dy == 7)
				for (char promote : "QRBN")
					moves.emplace_back(Coord({col, row}), Coord({x, y}), promote);
				// if black can promote
			else if (row + dy == 0)
				for (char promote : "qrbn")
					moves.emplace_back(Coord({col, row}), Coord({x, y}), promote);
				// if no one can promote
			else
				moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

		// check for en passant
		if (m_EnPassant) {
			const Coord& epCoord = m_EnPassant.value();
			if (epCoord.first == x and epCoord.second == y)
				moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}
	}

	return moves;
}

std::list<Move> Chess::GetPseudoLegalMoves() const {
	std::list<Move> legalMoves;
	for (int col = 0; col < SIZE; col++)
		for (int row = 0; row < SIZE; row++) {
			char piece = GetPiece(col, row);
			if (!IsCurrentPlayerPiece(piece)) continue;

			const auto& moves = GetLegalMovesForPiece(col, row);
			legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
		}
	return legalMoves;
}

const std::list<Move>& Chess::GetLegalMoves() const {
	// if this was already calculated
	if (!m_LegalMovesCache.empty())
		return m_LegalMovesCache;

	m_LegalMovesCache = GetPseudoLegalMoves();

	auto it = m_LegalMovesCache.begin();
	while (it != m_LegalMovesCache.end()) {
		// hack my way to be able to modify the board
		auto& hackedBoardRef = (Board&)m_Board;
		const auto& from = it->from;
		const auto& to = it->to;

		// get the indices
		int fromIndex = CoordToIndexInBoard(from.first, from.second);
		int toIndex = CoordToIndexInBoard(to.first, to.second);

		// manually apply the move
		char fromPiece = hackedBoardRef[fromIndex];
		char toPiece = hackedBoardRef[toIndex];
		hackedBoardRef[fromIndex] = ' ';
		hackedBoardRef[toIndex] = fromPiece;

		auto& hackedPlayerRef = (Player&)m_Playing;
		hackedPlayerRef = GetNotCurrentPlayer();

		// if the resulting move makes a check, remove it
		if (IsCheck())
			it = m_LegalMovesCache.erase(it); // catch the new iterator
		else
			it++;

		// revert the move
		hackedBoardRef[fromIndex] = fromPiece;
		hackedBoardRef[toIndex] = toPiece;
		hackedPlayerRef = GetNotCurrentPlayer();
	}

	return m_LegalMovesCache;
}

bool Chess::IsCheck() const {
	// TODO: avoid calling this
	std::list<Move> moves = GetPseudoLegalMoves();

	// check if one of the move's destination is the king
	return std::ranges::any_of(moves, [this](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == 'k' or piece == 'K');
	});
}

bool Chess::IsCurrentPlayerPiece(char piece) const {
	if (GetCurrentPlayer() == Player::White) {
		// if piece is uppercase
		if ('A' <= piece and piece <= 'Z')
			return true;
	} else {
		// if piece is lowercase
		if ('a' <= piece and piece <= 'z')
			return true;
	}
	return false;
}

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

	char& pieceToMove = GetPieceRef(from.first, from.second);
	char& pieceToReplace = GetPieceRef(to.first, to.second);

	// if the move played was en passant, remove the correct piece
	if (to == m_EnPassant) {
		if (pieceToMove == 'p')
			GetPieceRef(to.first, to.second + 1) = ' ';
		else if (pieceToMove == 'P')
			GetPieceRef(to.first, to.second - 1) = ' ';
	}

	// if a white pawn moved two ranks
	if (pieceToMove == 'P' and from.second == 1 and to.second == 3) {
		// if there are black pawns on the side
		if (to.first - 1 >= 0 and GetPiece(to.first - 1, 3) == 'p' ||
			to.first + 1 <= 7 and GetPiece(to.first + 1, 3) == 'p')
			m_EnPassant = Coord({to.first, 2});
	} else if (pieceToMove == 'p' and from.second == 6 and to.second == 4) {
		if (to.first - 1 >= 0 and GetPiece(to.first - 1, 4) == 'P' ||
			to.first + 1 <= 7 and GetPiece(to.first + 1, 4) == 'P')
			m_EnPassant = Coord({to.first, 5});
	} else {
		m_EnPassant = {};
	}

	// check if a pawn was moved or if a piece was taken
	if (pieceToMove == 'p' or pieceToMove == 'P' or pieceToReplace != ' ')
		m_halfMovesRule = 0;
	else
		m_halfMovesRule++;

	// if there is a promotion, replace the piece with the correct one
	if (move.promote)
		pieceToReplace = move.promote.value();
	else
		pieceToReplace = pieceToMove;

	// if the move is a castling, move the rook too
	if (pieceToMove == 'K' and from == Coord({4, 0}) and to == Coord({6, 0})) {
		GetPieceRef(7, 0) = ' ';
		GetPieceRef(5, 0) = 'R';
	} else if (pieceToMove == 'K' and from == Coord({4, 0}) and to == Coord({2, 0})) {
		GetPieceRef(0, 0) = ' ';
		GetPieceRef(3, 0) = 'R';
	} else if (pieceToMove == 'k' and from == Coord({4, 7}) and to == Coord({6, 7})) {
		GetPieceRef(7, 7) = ' ';
		GetPieceRef(5, 7) = 'r';
	} else if (pieceToMove == 'k' and from == Coord({4, 7}) and to == Coord({2, 7})) {
		GetPieceRef(0, 7) = ' ';
		GetPieceRef(3, 7) = 'r';
	}

	// the king or the rooks move, castling is not allowed
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

	// remove the piece from its staring square
	pieceToMove = ' ';

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

	// delete the cache
	m_LegalMovesCache.clear();
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