#pragma once

class ChessMatch;
class ChessObject
{
public:
	ChessObject(ChessMatch* match):m_match(match)
	{

	}
	~ChessObject();

public:
	ChessMatch* m_match=nullptr;
private:

};
