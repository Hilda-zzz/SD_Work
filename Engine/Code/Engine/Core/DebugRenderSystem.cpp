#include "DebugRenderSystem.hpp"
#include <vector>
#include "Vertex_PCU.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "VertexUtils.hpp"
#include <Engine/Math/MathUtils.hpp>
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "EngineCommon.hpp"
extern Clock*g_systemClock;
extern EventSystem* g_theEventSystem;
static bool s_isDebugRenderMode = true;

static Renderer* s_curRenderer = nullptr;
static BitmapFont* s_curFont = nullptr;

class DebugRenderGeometry
{
public:
	DebugRenderGeometry() {};
	DebugRenderGeometry(const Vec3& position,float duration, const Rgba8&  startColor, const Rgba8&  endColor,DebugRenderMode debugRenderMode)
		:m_position(position),m_duration(duration),m_startColor(startColor),m_endColor(endColor),m_mode(debugRenderMode)
	{
		m_startTime = (float)g_systemClock->GetTotalSeconds();
	}
	~DebugRenderGeometry() {};

	void UpdateVerts()
	{
		if (m_duration == -1.f)
		{
			return;
		}
		else if (m_duration == 0.f)
		{
			m_isDestroyed = true;
			return;
		}
		else
		{
			float passedTime = (float)g_systemClock->GetTotalSeconds() - m_startTime;
			if (passedTime > m_duration)
			{
				m_isDestroyed = true;
				return;
			}
			float timeFraction = passedTime / m_duration;
			Rgba8 curColor = Interpolate(m_startColor, m_endColor, timeFraction);
			m_curColor = curColor;
			for (int i = 0; i < (int)m_verts.size(); i++)
			{
				m_verts[i].m_color = curColor;
			}
		}
	}

	void Render(const Camera& camera) const
	{
		//?
		if (m_isBillboard)
		{
			Mat44 opposingMat = GetBillboardMatrix(BillboardType::FULL_OPPOSING,camera.GetOrientation().GetAsMatrix_IFwd_JLeft_KUp(),camera.GetPosition(), m_position);
			s_curRenderer->SetModelConstants(opposingMat, Rgba8::WHITE);
		}
		else
		{
			Mat44 modelToWorld = Mat44::MakeTranslation3D(m_position);
			s_curRenderer->SetModelConstants(modelToWorld, Rgba8::WHITE);
		}

		//?
		//Texture* m_gridTex = s_curRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
		//s_curRenderer->BindTexture(nullptr);

		if (m_isWire)
		{
			s_curRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
			s_curRenderer->BindTexture(nullptr);
		}
		else if(m_isText)
		{
			s_curRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
			s_curRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
			s_curRenderer->BindTexture(&s_curFont->GetTexture());
		}
		else
		{
			s_curRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
			s_curRenderer->BindTexture(nullptr);
		}

		if (m_mode==DebugRenderMode::USE_DEPTH)
		{
			//?
			s_curRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
			s_curRenderer->SetBlendMode(BlendMode::OPAQUE);
			s_curRenderer->DrawVertexArray(m_verts);
		}
		else if (m_mode == DebugRenderMode::ALWAYS)
		{
			s_curRenderer->SetDepthMode(DepthMode::DISABLED);
			s_curRenderer->SetBlendMode(BlendMode::OPAQUE);
			s_curRenderer->DrawVertexArray(m_verts);
		}
		else if (m_mode == DebugRenderMode::X_RAY)
		{
			//?
			s_curRenderer->SetDepthMode(DepthMode::READ_ONLY_ALWYAS);
			s_curRenderer->SetBlendMode(BlendMode::ALPHA);
			std::vector<Vertex_PCU> transparentVerts;
			transparentVerts.reserve(m_verts.size());
			for (int i = 0; i < (int)m_verts.size(); i++)
			{
				transparentVerts.push_back(m_verts[i]);
				Rgba8 curColor = m_verts[i].m_color;
				transparentVerts[i].m_color =Rgba8(curColor.r,curColor.g,curColor.b,150);
			}
			s_curRenderer->DrawVertexArray(transparentVerts);
			s_curRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
			s_curRenderer->SetBlendMode(BlendMode::OPAQUE);
			s_curRenderer->DrawVertexArray(m_verts);
		}
		s_curRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	}
	std::vector<Vertex_PCU> m_verts;
	Vec3 m_position = Vec3(0.f, 0.f, 0.f);
	float m_duration=-1.f;
	float m_startTime = -1.f;
	Rgba8 m_startColor=Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	Rgba8 m_curColor = Rgba8::WHITE;
	DebugRenderMode m_mode = DebugRenderMode::USE_DEPTH;
	bool m_isDestroyed = false;
	bool m_isWire = false;
	bool m_isText = false;
	bool m_isBillboard = false;
	std::string m_text = "";
};

static std::vector<DebugRenderGeometry*> s_debugRenderListWorld;
static std::vector<DebugRenderGeometry*> s_debugRenderListScreen;
static std::vector<DebugRenderGeometry*> s_debugRenderListMsg;

void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	g_theEventSystem->SubscribeEventCallbackFuction("DebugRenderClear", Command_DebugRenderClear, true);
	g_theEventSystem->SubscribeEventCallbackFuction("DebugRenderToggle", Command_DebugRenderToggle, true);

	s_curRenderer = config.m_renderer;
	s_curFont =s_curRenderer->CreateOrGetBitmapFont(config.m_fontName.c_str());
	s_debugRenderListWorld.reserve(100);
	s_debugRenderListScreen.reserve(50);
	s_isDebugRenderMode = true;

	DebugAddWorldBasis(Mat44(),-1.f,DebugRenderMode::USE_DEPTH);

	Mat44 xTextMat = Mat44::MakeTranslation3D(Vec3(1.3f, 0.f, 0.25f));
	xTextMat.Append(Mat44::MakeZRotationDegrees(-90.f));
	DebugAddWorldText("x - forward", xTextMat, 0.2f, Vec2(0.5f, 0.5f), -1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH);

	Mat44 yTextMat = Mat44::MakeTranslation3D(Vec3(0.f, 1.f, 0.25f));
	yTextMat.Append(Mat44::MakeZRotationDegrees(180.f));
	DebugAddWorldText("y - left", yTextMat, 0.2f, Vec2(0.5f, 0.5f), -1.f, Rgba8::GREEN, Rgba8::GREEN,DebugRenderMode::USE_DEPTH);

	Mat44 zTextMat = Mat44::MakeTranslation3D(Vec3(0.f, -0.2f, 0.7f));
	zTextMat.Append(Mat44::MakeZRotationDegrees(180.f));
	zTextMat.Append(Mat44::MakeXRotationDegrees(90.f));
	DebugAddWorldText("z - up", zTextMat, 0.2f, Vec2(0.5f, 0.5f), -1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH);

	char buffer[256]="1";
	DebugAddMessage(std::string(buffer), -1.f, Rgba8::WHITE, Rgba8::RED);
}

void DebugRenderSystemShutdown()
{
	for (DebugRenderGeometry* geometry : s_debugRenderListWorld)
	{
		delete geometry;
		geometry = nullptr;
	}

	for (DebugRenderGeometry* geometry : s_debugRenderListScreen)
	{
		delete geometry;
		geometry = nullptr;
	}
}

void DebugRenderSetVisible()
{
	s_isDebugRenderMode = true;
}

void DebugRenderSetHidden()
{
	s_isDebugRenderMode = false;
}

void DebugRenderClear()
{
	for (DebugRenderGeometry* geometry : s_debugRenderListWorld)
	{
		delete geometry;
		geometry = nullptr;
	}
	s_debugRenderListWorld.clear();

	for (DebugRenderGeometry* geometry : s_debugRenderListScreen)
	{
		delete geometry;
		geometry = nullptr;
	}
	s_debugRenderListScreen.clear();
}

void DebugRenderBeginFrame()
{
	
	for (DebugRenderGeometry* geometry : s_debugRenderListWorld)
	{
		geometry->UpdateVerts();
	}

	for (DebugRenderGeometry* geometry : s_debugRenderListScreen)
	{
		geometry->UpdateVerts();
	}

	for (DebugRenderGeometry* geometry : s_debugRenderListMsg)
	{
		geometry->UpdateVerts();
	}
}

void DebugRenderWorld(const Camera& camera)
{
	if (s_isDebugRenderMode)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer),
			"player position: %.2f, %.2f, %.2f",
			camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		s_debugRenderListMsg[0]->m_text = std::string(buffer);

		s_curRenderer->BeginCamera(camera);
		for (DebugRenderGeometry* geometry : s_debugRenderListWorld)
		{
			geometry->Render(camera);
		}
		s_curRenderer->EndCamera(camera);
	}
}

void DebugRenderScreen(const Camera& camera)
{
	if (s_isDebugRenderMode)
	{
		std::vector<Vertex_PCU> msgVerts;
		msgVerts.reserve(1000);

		s_curRenderer->BeginCamera(camera);
		for (DebugRenderGeometry* geometry : s_debugRenderListScreen)
		{
			geometry->Render(camera);
		}

		int i = 0;
		for (DebugRenderGeometry* geometry : s_debugRenderListMsg)
		{
			Vec2 Bl = Vec2(10.f, 800.f - (i + 1) * 20.f);
			Vec2 Tr = Bl + Vec2(500.f, 20.f);
			AABB2 textBox = AABB2(Bl, Tr);
			s_curFont->AddVertsForTextInBox2D(msgVerts, geometry->m_text, textBox, 10.f, geometry->m_curColor, 1.f, Vec2(0.f, 0.f));
			i++;
		}
		s_curRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		s_curRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		s_curRenderer->BindTexture(&s_curFont->GetTexture());
		s_curRenderer->DrawVertexArray(msgVerts);

		s_curRenderer->EndCamera(camera);
	}
}

void DebugRenderEndFrame()
{
	for (DebugRenderGeometry* geometry : s_debugRenderListWorld)
	{
		if (geometry->m_isDestroyed)
		{
			s_debugRenderListWorld.erase(
				std::remove(s_debugRenderListWorld.begin(), s_debugRenderListWorld.end(), geometry),
				s_debugRenderListWorld.end()
			);
		}
	}
	for (DebugRenderGeometry* geometry : s_debugRenderListScreen)
	{
		if (geometry->m_isDestroyed)
		{
			s_debugRenderListScreen.erase(
				std::remove(s_debugRenderListScreen.begin(), s_debugRenderListScreen.end(), geometry),
				s_debugRenderListScreen.end()
			);
		}
	}

	for (DebugRenderGeometry* geometry : s_debugRenderListMsg)
	{
		if (geometry->m_isDestroyed)
		{
			s_debugRenderListMsg.erase(
				std::remove(s_debugRenderListMsg.begin(), s_debugRenderListMsg.end(), geometry),
				s_debugRenderListMsg.end()
			);
		}
	}
}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newPoint = new DebugRenderGeometry(pos,duration, startColor, endColor, mode);
	AddVertsForSphere3D(newPoint->m_verts, pos, radius, startColor);
	s_debugRenderListWorld.push_back(newPoint);
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newLine = new DebugRenderGeometry(Vec3(0.f,0.f,0.f), duration, startColor, endColor, mode);
	AddVertsForCylinder3D(newLine->m_verts, start, end, radius,startColor);
	s_debugRenderListWorld.push_back(newLine);
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newCylinder = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, mode);
	AddVertsForCylinder3D(newCylinder->m_verts, base, top, radius, startColor,AABB2::ZERO_TO_ONE,32);
	newCylinder->m_isWire = true;
	s_debugRenderListWorld.push_back(newCylinder);
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newSphere = new DebugRenderGeometry(center, duration, startColor, endColor, mode);
	AddVertsForSphere3D(newSphere->m_verts,center, radius, startColor);
	newSphere->m_isWire = true;
	s_debugRenderListWorld.push_back(newSphere);
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	float len = (end - start).GetLength();
	Vec3 dir =(end - start).GetNormalized();
	DebugRenderGeometry* newLine = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, mode);
	AddVertsForCylinder3D(newLine->m_verts, start, end-dir*0.6f, radius, startColor);
	s_debugRenderListWorld.push_back(newLine);

	DebugRenderGeometry* newCone = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, mode);
	AddVertsForCone3D(newCone->m_verts, start + dir * (len - 0.6f), start + dir * len, radius+0.1f, startColor);
	s_debugRenderListWorld.push_back(newCone);
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newWorldText = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, mode);
	s_curFont->AddVertsForText3DAtOriginXForward(newWorldText->m_verts, textHeight, text, startColor, 1.f,alignment);
	newWorldText->m_isText = true;
	TransformVertexArray3D(newWorldText->m_verts, transform);
	s_debugRenderListWorld.push_back(newWorldText);
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugRenderGeometry* newWorldText = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, mode);
	s_curFont->AddVertsForText3DAtOriginXForward(newWorldText->m_verts, textHeight, text, startColor, 1.f, alignment);
	newWorldText->m_isText = true;
	newWorldText->m_isBillboard = true;
	newWorldText->m_position = origin;
	s_debugRenderListWorld.push_back(newWorldText);
}

void DebugAddWorldBasis(const Mat44& transform, float duration, DebugRenderMode mode)
{
	DebugRenderGeometry* xLine = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration,Rgba8::RED, Rgba8::RED, mode);
	AddVertsForCylinder3D(xLine->m_verts,Vec3(0.f,0.f,0.f), Vec3(0.6f, 0.f, 0.f), 0.08f, Rgba8::RED);
	s_debugRenderListWorld.push_back(xLine);
	DebugRenderGeometry* xCone = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, Rgba8::RED, Rgba8::RED, mode);
	AddVertsForCone3D(xCone->m_verts, Vec3(0.6f, 0.f, 0.f), Vec3(1.f, 0.f, 0.f), 0.15f, Rgba8::RED,AABB2::ZERO_TO_ONE, 16);
	s_debugRenderListWorld.push_back(xCone);

	DebugRenderGeometry* yLine = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, Rgba8::GREEN, Rgba8::GREEN, mode);
	AddVertsForCylinder3D(yLine->m_verts, Vec3(0.f, 0.f, 0.f), Vec3(0.0f, 0.6f, 0.f), 0.08f, Rgba8::GREEN);
	s_debugRenderListWorld.push_back(yLine);
	DebugRenderGeometry* yCone = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, Rgba8::GREEN, Rgba8::GREEN, mode);
	AddVertsForCone3D(yCone->m_verts, Vec3(0.f, 0.6f, 0.f), Vec3(0.f, 1.f, 0.f), 0.15f, Rgba8::GREEN,AABB2::ZERO_TO_ONE,16);
	s_debugRenderListWorld.push_back(yCone);

	DebugRenderGeometry* zLine = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, Rgba8::BLUE, Rgba8::BLUE, mode);
	AddVertsForCylinder3D(zLine->m_verts, Vec3(0.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.6f), 0.08f, Rgba8::BLUE);
	s_debugRenderListWorld.push_back(zLine);
	DebugRenderGeometry* zCone = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, Rgba8::BLUE, Rgba8::BLUE, mode);
	AddVertsForCone3D(zCone->m_verts, Vec3(0.f, 0.f, 0.6f), Vec3(0.f, 0.f, 1.f), 0.15f, Rgba8::BLUE, AABB2::ZERO_TO_ONE, 16);
	s_debugRenderListWorld.push_back(zCone);

	TransformVertexArray3D(xLine->m_verts, transform);
	TransformVertexArray3D(yLine->m_verts, transform);
	TransformVertexArray3D(zLine->m_verts, transform);
	TransformVertexArray3D(xCone->m_verts, transform);
	TransformVertexArray3D(yCone->m_verts, transform);
	TransformVertexArray3D(zCone->m_verts, transform);
}

void DebugAddScreenText(const std::string& text, const AABB2& textBox,
	float textHeight, const Vec2& alignment,
	float duration, const Rgba8& startColor,
	const Rgba8& endColor)
{
	DebugRenderGeometry* newScreenText = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, DebugRenderMode::USE_DEPTH);
	s_curFont->AddVertsForTextInBox2D(newScreenText->m_verts, text, textBox, textHeight,Rgba8::WHITE,1.f,alignment);
	newScreenText->m_isText = true;
	//newScreenText->m_position = Vec3(position.x,position.y,0.f);
	s_debugRenderListScreen.push_back(newScreenText);
}

void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugRenderGeometry* newScreenMsg = new DebugRenderGeometry(Vec3(0.f, 0.f, 0.f), duration, startColor, endColor, DebugRenderMode::USE_DEPTH);
	newScreenMsg->m_text = text;
	s_debugRenderListMsg.push_back(newScreenMsg);
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	if (s_isDebugRenderMode == true)
	{
		DebugRenderSetHidden();
	}
	else
	{
		DebugRenderSetVisible();
	}
	return true;
}
