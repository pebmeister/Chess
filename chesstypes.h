#pragma once

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };
enum class Color { White, Black };

struct Piece {
	PieceType type;
	Color color;

	std::string toString() const
	{
		auto ch = '?';

		switch (type) {
			case PieceType::None:
				ch = '.';
				break;
			case PieceType::Pawn:
				ch = 'P';
				break;
			case PieceType::Knight:
				ch = 'N';
				break;
			case PieceType::Bishop:
				ch = 'B';
				break;
			case PieceType::Rook:
				ch = 'R';
				break;
			case PieceType::Queen:
				ch = 'Q';
				break;
			case PieceType::King:
				ch = 'K';
				break;
			default:
				ch = '?';
				break;
		}
		if (color == Color::Black)
			ch = std::tolower(ch);

		return std::string("") + ch;
	}
};
