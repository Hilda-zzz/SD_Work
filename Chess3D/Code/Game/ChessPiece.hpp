#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/ChessObject.hpp"
#include "Game/ChessPieceDefinition.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/ZCylinder.hpp"
#include "Engine/ResourceManager/StaticMeshComponent.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

class Texture;
class ChessMatch;
class Shader;

enum class Faction {
	NONE,
	BLACK,
	WHITE
};

class ChessPiece:public ChessObject
{
public:
	ChessPiece(ChessMatch* match,Faction facion,PieceType type,IntVec2 const& gridPos);
	~ChessPiece() {};

	void Update();
	void Renderer() const;
	void RenderShadowTexture() const;

	IntVec2 GetAimGridPos();
	void SetAimGridPos(IntVec2 const& gridPos);

	IntVec2 GetGridPos();
	void SetGridPos(IntVec2 const& gridPos);

	IntVec2 GetPrevGridPos();
	void SetPrevGridPos(IntVec2 const& gridPos);

	Vec3 GetCurPos();
	void SetCurPos(Vec3 const& curPos);

	PieceType GetPieceType();
	void SetPieceType(PieceType newType);
	Faction GetFaction();
	bool GetIsFirstMove();
	void SetIsFirstMove(bool isFirstMove);

	void UpdateCurPosFromCurGridPos();
	Vec3 GetPosFromGridPos(IntVec2 const& gridPos);

public:
	//animation
	Timer m_animationTimer;
	bool m_isSlide = false;
	bool m_isHop = false;
	ChessPieceDefinition* m_def = nullptr;
	ZCylinder m_collider;
	bool m_isImpacted = false;
	bool m_isSelected = false;

private:
	//ChessMatch* m_match = nullptr;
	Faction m_faction = Faction::NONE;
	PieceType m_pieceType = PieceType::NONE;
	IntVec2 m_aimGridPos= IntVec2(0, 0);
	IntVec2 m_gridPos = IntVec2(0, 0);
	IntVec2 m_prevCoord = IntVec2(0, 0);
	Vec3 m_curPos = Vec3(0.f, 0.f,0.f);
	EulerAngles m_orientaion = EulerAngles(0.f, 0.f, 0.f);
	Texture* m_texWhite = nullptr;
	Texture* m_texBlack = nullptr;
	Texture* m_texNormalWhite = nullptr;
	Texture* m_texNormalBlack = nullptr;
	Texture* m_texSGEWhite = nullptr;
	Texture* m_texSGEBlack = nullptr;
	bool m_isFirstMove = true;

	//new 
	float secondSinceMoved;
	int m_turnLastMoved;

	// model
	StaticMeshComponent m_staticMeshComponent;

	// shadow
	Shader* m_shadowShader = nullptr;
	
	std::vector<Vertex_PCUTBN> m_cubeTestVerts;
	std::vector<unsigned int>m_cubeTestIndices;
};
