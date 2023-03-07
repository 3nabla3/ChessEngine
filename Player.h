#pragma once

enum class Player: bool {
	White, Black
};

static std::ostream& operator<<(std::ostream& ostream, const Player player){
	if (player == Player::White)
		ostream << "white";
	else
		ostream << "black";
	return ostream;
}