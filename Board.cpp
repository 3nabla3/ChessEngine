#include "pch.h"
#include "Board.h"


std::ostream& operator<<(std::ostream& ostream, const Board& board) {
	int rowCount = 8;
	ostream << rowCount-- << " | ";

	for (int i = 0; i < board.size(); i++) {
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

void Board::ApplyMove(const Move& move) {
	const auto& from = move.from;
	const auto& to = move.to;

	char& pieceToMove = GetPieceRef(from);
	char& pieceToReplace = GetPieceRef(to);

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
}

std::list<Move> Board::GetPseudoLegalMoves(Player player, std::optional<Coord> enPassant, const CastlingRights& castlingRights) const {
	std::list<Move> legalMoves;
	for (int col = 0; col < SIZE; col++)
		for (int row = 0; row < SIZE; row++) {
			char piece = GetPiece(col, row);

			if (!IsPlayerPiece(piece, player)) continue;

			const auto& moves = GetLegalMovesForPiece(col, row, player, enPassant, castlingRights);
			legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
		}
	return legalMoves;
}

std::list<Move> Board::GetLegalMovesForPiece(int col, int row, Player player, std::optional<Coord> enPassant, const CastlingRights& castlingRights) const {
	std::list<Move> legalMoves;

	switch (GetPiece(col, row)) {
		case 'r':
		case 'R':
			return GetPseudoLegalMovesRook(col, row, player);
		case 'n':
		case 'N':
			return GetPseudoLegalMovesKnight(col, row, player);
		case 'b':
		case 'B':
			return GetPseudoLegalMovesBishop(col, row, player);
		case 'q':
		case 'Q':
			return GetPseudoLegalMovesQueen(col, row, player);
		case 'k':
		case 'K':
			return GetPseudoLegalMovesKing(col, row, player, castlingRights);
		case 'p':
		case 'P':
			return GetPseudoLegalMovesPawn(col, row, player, enPassant);
		default:
			std::cout << "This should never happen! (Invalid Piece)" << std::endl;
	}

	// should never reach this code
	return {};
}

std::list<Move> Board::GetPseudoLegalMovesRook(int col, int row, Player player) const {
	std::list<Move> moves;

	// scan to the right
	for (int xd = 1; xd < SIZE - col; xd++) {
		char piece = GetPiece(col + xd, row);

		// if there is a piece blocking the scan, the scan stops
		if (IsPlayerPiece(piece, player))
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
		if (IsPlayerPiece(piece, player))
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
		if (IsPlayerPiece(piece, player))
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
		if (IsPlayerPiece(piece, player))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col, row + yd}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::list<Move> Board::GetPseudoLegalMovesKnight(int col, int row, Player player) const {
	std::list<Move> moves;

	// check the moves one up and one down from the squares two left and two right
	for (int xd = -2; xd <= 2; xd += 4)
		for (int yd = -1; yd <= 1; yd+=2) {
			int x = col + xd;
			int y = row + yd;
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsPlayerPiece(GetPiece(x, y), player))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	// check the moves one left and one right from the squares two up and two down
	for (int xd = -1; xd <= 1; xd += 2)
		for (int yd = -2; yd <= 2; yd+=4) {
			int x = col + xd;
			int y = row + yd;
			// if the position is in bounds
			if (0 <= x and x <= 7 and 0 <= y and y <= 7)
				if (!IsPlayerPiece(GetPiece(x, y), player))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}

	return moves;
}

std::list<Move> Board::GetPseudoLegalMovesBishop(int col, int row, Player player) const {
	std::list<Move> moves;

	// scan up right
	for (int d = 1; col + d < 8 and row + d < 8; d++) {
		char piece = GetPiece(col + d, row + d);
		if (IsPlayerPiece(piece, player))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan up left
	for (int d = 1; col - d >= 0 and row + d < 8; d++) {
		char piece = GetPiece(col - d, row + d);
		if (IsPlayerPiece(piece, player))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row + d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down right
	for (int d = 1; col + d < 8 and row - d >= 0; d++) {
		char piece = GetPiece(col + d, row - d);
		if (IsPlayerPiece(piece, player))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col + d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	// scan down left
	for (int d = 1; col - d >= 0 and row - d >= 0; d++) {
		char piece = GetPiece(col - d, row - d);
		if (IsPlayerPiece(piece, player))
			break;

		moves.emplace_back(Coord({col, row}), Coord({col - d, row - d}));
		// if the piece was an opponent's piece, the scan stops
		if (piece != ' ')
			break;
	}

	return moves;
}

std::list<Move> Board::GetPseudoLegalMovesQueen(int col, int row, Player player) const {
	// get the rook moves
	std::list<Move> moves = GetPseudoLegalMovesRook(col, row, player);

	// append the bishop moves
	const std::list<Move>& temp = GetPseudoLegalMovesBishop(col, row, player);
	moves.insert(moves.end(), temp.begin(), temp.end());

	return moves;
}

std::list<Move> Board::GetPseudoLegalMovesKing(int col, int row, Player player, const CastlingRights& castlingRights) const {
	std::list<Move> moves;

	for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++) {
			int x = col + dx;
			int y = row + dy;
			// if the position is in bounds
			if (0 <= x and x < 8 and 0 <= y and y < 8) {
				if (!IsPlayerPiece(GetPiece(col + dx, row + dy), player))
					moves.emplace_back(Coord({col, row}), Coord({x, y}));
			}
		}

	// check castling
	if (player == Player::White) {
		// check white king side castling
		if (castlingRights.first and GetPiece(5, 0) == ' ' and GetPiece(6, 0) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({6, 0}));
		// check white queen side castling
		if (castlingRights.second and GetPiece(1, 0) == ' ' and GetPiece(2, 0) == ' ' and GetPiece(3, 0) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({2, 0}));
	} else {
		// check black king side castling
		if (castlingRights.first and GetPiece(5, 7) == ' ' and GetPiece(6, 7) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({6, 7}));
		// check black queen side castling
		if (castlingRights.second and GetPiece(1, 7) == ' ' and GetPiece(2, 7) == ' ' and GetPiece(3, 7) == ' ')
			moves.emplace_back(Coord({col, row}), Coord({2, 7}));
	}
	return moves;
}

std::list<Move> Board::GetPseudoLegalMovesPawn(int col, int row, Player player, std::optional<Coord> enPassant) const {
	std::list<Move> moves;

	// Get some important information depending on the color playing
	int dy;
	int startingRow;

	if (player == Player::White) {
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
		if (!IsPlayerPiece(piece, player) and piece != ' ') {
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
		if (enPassant) {
			const Coord& epCoord = enPassant.value();
			if (epCoord.first == x and epCoord.second == y)
				moves.emplace_back(Coord({col, row}), Coord({x, y}));
		}
	}

	return moves;
}

bool Board::IsCheck(Player player) const {
	// TODO: avoid calling this
	std::list<Move> moves = GetPseudoLegalMoves(Player::White == player ? Player::Black : Player::White,
												std::nullopt, {false, false});

	// check if one of the move's destination is the king
	return std::ranges::any_of(moves, [this](const Move& move) {
		char piece = GetPiece(move.to);
		return (piece == 'k' or piece == 'K');
	});
}