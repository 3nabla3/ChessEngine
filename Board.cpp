#include "pch.h"
#include "Board.h"

Board::Board(const std::string& fen)
	:m_Board(BoardFromFen(fen)), m_Playing(fen.contains(" w ") ? Player::White : Player::Black),
	 m_EnPassant(ParseEnPassantFromFen(fen)),
	 m_WhiteCastlingRights(ParseCastlingRightsFromFen(fen, Player::White)),
	 m_BlackCastlingRights(ParseCastlingRightsFromFen(fen, Player::Black))
{}

std::ostream& operator<<(std::ostream& ostream, const Board& board) {
	int rowCount = Board::SIZE;
	ostream << rowCount-- << " | ";

	for (int i = 0; i < Board::SIZE * Board::SIZE; i++) {
		char piece = board[i];
		if (piece == ' ')
			piece = '.';
		ostream << piece << " ";

		if (i % 8 == 7 and rowCount > 0) {
			ostream << std::endl << rowCount-- << " | ";
		}
	}

	ostream << "\n   ----------------";
	ostream << "\n    a b c d e f g h";

	return ostream;
}

// assume the move is legal when calling this function
void Board::ApplyMove(const Move& move) {
	PROFILE_SCOPE;
	const auto& from = move.from;
	const auto& to = move.to;

	char& pieceToMove = GetPieceRef(from);
	char& pieceToReplace = GetPieceRef(to);

	// check if a pawn was moved or if a piece was taken
	if (pieceToMove == 'p' or pieceToMove == 'P' or pieceToReplace != ' ')
		m_halfMovesRule = 0;
	else
		m_halfMovesRule++;

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

	UpdateCastlingRights(move);

	// if the move played was en passant, remove the correct piece
	if (pieceToMove == 'p' and pieceToReplace == ' ' and from.first != to.first)
			GetPieceRef(to.first, to.second + 1) = ' ';
	else if (pieceToMove == 'P' and pieceToReplace == ' ' and from.first != to.first)
		GetPieceRef(to.first, to.second - 1) = ' ';

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

	// remove the piece from its staring square
	pieceToMove = ' ';

	if (m_Playing == Player::Black)
		m_fullMoves++;
	m_Playing = GetNotCurrentPlayer();
}

unsigned long Board::s_LegalMovesCacheHits = 0;
unsigned long Board::s_LegalMovesCacheMisses = 0;
std::shared_timed_mutex Board::s_CacheMutex;
std::unordered_map<std::string, std::vector<Move>> Board::s_FenToLegalMovesCache;

// Check all the pseudo legal moves and remove the ones that violate the check rules
const std::vector<Move>& Board::GetLegalMoves() const {
	PROFILE_SCOPE;
	std::string fen = GetPositionalFen();
	{
		std::shared_lock lock(s_CacheMutex);
		if (s_FenToLegalMovesCache.contains(fen)) {
			s_LegalMovesCacheHits++;
			return s_FenToLegalMovesCache.at(fen);
		}
	}

	s_LegalMovesCacheMisses++;
	std::vector<Move> moves = GetPseudoLegalMoves();
	auto it = moves.begin();
	while (it != moves.end()) {

		// if the move is a castling move, check if it's legal
		char piece = GetPiece(it->from);
		if ((piece == 'K' or piece == 'k') and std::abs(it->from.first - it->to.first) > 1) {
			if (!IsCastlingLegal(it->to.first == 6)) {
				it = moves.erase(it); // catch the new iterator
				continue;
			}
		}

		Board copy = *this;
		// make sure the move does not put the king in check
		copy.ApplyMove(*it);
		copy.m_Playing = GetCurrentPlayer();

		if (copy.IsCheck())
			it = moves.erase(it); // catch the new iterator
		else
			it++;
	}

	{
		std::lock_guard lock(s_CacheMutex);
		s_FenToLegalMovesCache[fen] = moves;
		return s_FenToLegalMovesCache[fen];
	}
}

// generate all the legal moves for the current player regardless of the king being in check
std::vector<Move> Board::GetPseudoLegalMoves() const {
	std::vector<Move> legalMoves;
	for (int col = 0; col < SIZE; col++)
		for (int row = 0; row < SIZE; row++) {
			char piece = GetPiece(col, row);

			if (!IsPlayerPiece(piece, GetCurrentPlayer())) continue;

			const auto& moves = GetLegalMovesForPiece(col, row);
			legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
		}
	return legalMoves;
}

std::vector<Move> Board::GetLegalMovesForPiece(int col, int row) const {
	std::vector<Move> legalMoves;

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

std::vector<Move> Board::GetPseudoLegalMovesRook(int col, int row) const {
	std::vector<Move> moves;

	// scan to the right
	for (int xd = 1; xd < SIZE - col; xd++) {
		char piece = GetPiece(col + xd, row);

		// if there is a piece blocking the scan, the scan stops
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
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
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
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
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
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
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col, row + yd}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::vector<Move> Board::GetPseudoLegalMovesKnight(int col, int row) const {
	std::vector<Move> moves;

	// check the moves one up and one down from the squares two left and two right
	for (int xd = -2; xd <= 2; xd += 4)
		for (int yd = -1; yd <= 1; yd+=2) {
			int x = col + xd;
			int y = row + yd;
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsPlayerPiece(GetPiece(x, y), GetCurrentPlayer()))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	// check the moves one left and one right from the squares two up and two down
	for (int xd = -1; xd <= 1; xd += 2)
		for (int yd = -2; yd <= 2; yd+=4) {
			int x = col + xd;
			int y = row + yd;
			// if the position is in bounds
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsPlayerPiece(GetPiece(x, y), GetCurrentPlayer()))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	return moves;
}

std::vector<Move> Board::GetPseudoLegalMovesBishop(int col, int row) const {
	std::vector<Move> moves;

	// scan up right
	for (int d = 1; col + d < 8 and row + d < 8; d++) {
		char piece = GetPiece(col + d, row + d);
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan up left
	for (int d = 1; col - d >= 0 and row + d < 8; d++) {
		char piece = GetPiece(col - d, row + d);
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down right
	for (int d = 1; col + d < 8 and row - d >= 0; d++) {
		char piece = GetPiece(col + d, row - d);
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down left
	for (int d = 1; col - d >= 0 and row - d >= 0; d++) {
		char piece = GetPiece(col - d, row - d);
		if (IsPlayerPiece(piece, GetCurrentPlayer()))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::vector<Move> Board::GetPseudoLegalMovesQueen(int col, int row) const {
	// get the rook moves
	std::vector<Move> moves = GetPseudoLegalMovesRook(col, row);

	// append the bishop moves
	const std::vector<Move>& temp = GetPseudoLegalMovesBishop(col, row);
	moves.insert(moves.end(), temp.begin(), temp.end());

	return moves;
}

std::vector<Move> Board::GetPseudoLegalMovesKing(int col, int row) const {
	std::vector<Move> moves;

	for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++) {
			int x = col + dx;
			int y = row + dy;
			// if the position is in bounds
			if (0 <= x and x < 8 and 0 <= y and y < 8) {
				if (!IsPlayerPiece(GetPiece(col + dx, row + dy), GetCurrentPlayer()))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
			}
		}

	// check castling
	if (IsWhiteTurn()) {
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

std::vector<Move> Board::GetPseudoLegalMovesPawn(int col, int row) const {
	std::vector<Move> moves;

	// Get some important information depending on the color playing
	int dy;
	int startingRow;

	if (IsWhiteTurn()) {
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
		if (!IsPlayerPiece(piece, GetCurrentPlayer()) and piece != ' ') {
			// if white can promote
			if (row + dy == 7)
				for (char promote : std::string("QRBN"))
					moves.emplace_back(Coord({col, row}), Coord({x, y}), promote);
				// if black can promote
			else if (row + dy == 0)
				for (char promote : std::string("qrbn"))
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

bool Board::IsCheckmate() const {
	return GetLegalMoves().empty() and IsCheck();
}

bool Board::IsDraw() const {
	if (m_halfMovesRule >= 100)
		return true;
	if (GetLegalMoves().empty() and !IsCheck())
		return true;
	return false;
}

bool Board::IsCheck() const {
	PROFILE_SCOPE;
	// kings coordinate
	char myKing = IsWhiteTurn() ? 'K' : 'k';
	// find the kings coordinate on the board
	int kingCol = - 1, kingRow = -1;
	for (int i = 0; i < Board::SIZE * Board::SIZE; i++) {
		char piece = m_Board[i];
		if (piece == myKing) {
			kingCol = i % Board::SIZE;
			kingRow = Board::SIZE - 1 - (i / Board::SIZE);
			break;
		}
	}

	// for each piece p
	// if the king was a [p] could it eat a [p] of the other color?

	char otherQueen = IsWhiteTurn() ? 'q' : 'Q';
	bool queenChecking = std::ranges::any_of(GetPseudoLegalMovesQueen(kingCol, kingRow), [this, otherQueen](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherQueen);
	});
	if (queenChecking) return true;

	char otherRook = IsWhiteTurn() ? 'r' : 'R';
	bool rookChecking = std::ranges::any_of(GetPseudoLegalMovesRook(kingCol, kingRow), [this, otherRook](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherRook);
	});

	char otherBishop = IsWhiteTurn() ? 'b' : 'B';
	bool bishopChecking = std::ranges::any_of(GetPseudoLegalMovesBishop(kingCol, kingRow), [this, otherBishop](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherBishop);
	});
	if (rookChecking or bishopChecking) return true;

	char otherKnight = IsWhiteTurn() ? 'n' : 'N';
	bool knightChecking = std::ranges::any_of(GetPseudoLegalMovesKnight(kingCol, kingRow), [this, otherKnight](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherKnight);
	});
	if (knightChecking) return true;

	char otherKing = IsWhiteTurn() ? 'k' : 'K';
	bool kingChecking = std::ranges::any_of(GetPseudoLegalMovesKing(kingCol, kingRow), [this, otherKing](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherKing);
	});
	if (kingChecking) return true;

	// if the king is on the 2nd or 7th rank, it can't be checked by a pawn
	if (IsWhiteTurn() and kingRow > 5 or
		!IsWhiteTurn() and kingRow < 2)
		return false;

	char otherPawn = IsWhiteTurn() ? 'p' : 'P';
	bool pawnChecking = std::ranges::any_of(GetPseudoLegalMovesPawn(kingCol, kingRow), [this, otherPawn](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == otherPawn);
	});

	return pawnChecking;
}

bool Board::IsGameOver() const {
	return IsCheckmate() or IsDraw();
}

std::string Board::GetFen() const {
	std::string fen;
	int spaceCount = 0;
	for (int i = 0; i < Board::SIZE * Board::SIZE; i++) {
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
			if (i < Board::SIZE * Board::SIZE - 1)
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

void Board::UpdateCastlingRights(const Move& move) {
	const auto& from = move.from;
	const auto& to = move.to;
	char& pieceToMove = GetPieceRef(from);
	char& pieceToReplace = GetPieceRef(to);

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
		// if a rook is taken, castling is not allowed anymore
	else if (pieceToReplace == 'R') {
		if (to == Coord({0, 0}))
			m_WhiteCastlingRights.second = false;
		else if (to == Coord({7, 0}))
			m_WhiteCastlingRights.first = false;
	} else if (pieceToReplace == 'r') {
		if (to == Coord({0, 7}))
			m_BlackCastlingRights.second = false;
		else if (to == Coord({7, 7}))
			m_BlackCastlingRights.first = false;
	}
}

bool Board::IsCastlingLegal(bool kingSide) const {
	if (IsCheck())
		return false;

	// make sure the king doesn't castle through check
	Board copy = *this;
	if (kingSide) {
		if (IsWhiteTurn())
			copy.ApplyMove("e1f1");
		else
			copy.ApplyMove("e8f8");
	}
	else {
		if (IsWhiteTurn())
			copy.ApplyMove("e1d1");
		else
			copy.ApplyMove("e8d8");
	}

	copy.m_Playing = GetCurrentPlayer();
	return !copy.IsCheck();
}

RawBoard Board::BoardFromFen(const std::string& fen) {
	RawBoard board{};

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

std::optional<Coord> Board::ParseEnPassantFromFen(const std::string& fen) {
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

CastlingRights Board::ParseCastlingRightsFromFen(const std::string& fen, Player player) {
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